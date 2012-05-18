#!/usr/bin/env ruby
#
# Converts a PNM P6 glyph sheet to a C declaration of a font.
#
# The glyph sheet is 24-bit RGB:
#  + 3 rows of 32 columns each, encoding ASCII 32-127.  Glyph size is
#    determined by sheet size knowing this layout
#  + Black is background
#  + any colour other than white is ignored ( these colors can be used for base
#    lines, cell top-left marker etc.)
#  + White is font pixels
#
# The DSO Quad writes pixels up the screen first, then across.  The glyph data
# is therefore in columns, left-most column first, bottom-most pixel first
# within the column 8x10 pixels = 80 bits ( 10 bytes).  Glyph data is byte
# aligned but within a glyph it's bits because byte-aligning each column when
# there are 10 pixels per column would result in almost 50% waste
#
# Use:
#  mkfont.rb  font1 < font.pnm
#

$LOAD_PATH << File.dirname($0)

require "PNM.rb"
require "bitstream.rb"

class Array

  # Provides a new array made of arrays of elements from this array in groups
  # of the specified size.
  #
  def groups_of n
    grouped = [[]]
    self.each do |element|
      grouped << [] if n == grouped[-1].length
      grouped[-1] << element
    end
    grouped
  end
end

include Checking

if ARGV.count != 2
  $stderr.puts "Use: #{$0} variable-name bw|gray4"
  exit 1
end

variable_name, format = ARGV

glyph_sheet = PNM.p6_from $stdin

# The glyph sheet must have 3 rows with 32 glyphs in each row
max_glyph_width = glyph_sheet.width / 32.0
glyph_height = glyph_sheet.height / 3.0
# Check that the glyph sheet divides evenly in to 3 rows of 32 columns of glyphs
check_that  max_glyph_width.to_i == max_glyph_width
check_that  glyph_height.to_i == glyph_height
max_glyph_width = max_glyph_width.to_i
glyph_height = glyph_height.to_i

puts <<EOF

#include "font.h"

EOF

puts "Glyph const  #{variable_name}_glyphs[] = {"

bitstream = Bitstream.new
(32..127).each do |code|
  row = (code - " "[0]) / 32
  col = (code - " "[0]) % 32
  ox = max_glyph_width * col # "origin X" ( the left of the glyph cell on the sheet)
  oy = glyph_height * row # "origin Y" ( the top of the glyph cell on the sheet)
  # TODO: Determine "lift" by looking for blank rows at the bottom of the glyph cell
  # TODO: Determine width and height.  THEN loop and store bits.  NOTE: take care to detect height=0 for space

  # Go through all the pixels in the glyph cell and determine the bounding box
  # of the glyph pixels
  lowest = highest = left = right = nil
  (0..glyph_height-1).each do |y|
    (0..max_glyph_width-1).each do |x|
      # Note the calculation for Y.  "y" is in Quad ordinates ( lower ordinates
      # towards the bottom of the screen)
      r, g, b = glyph_sheet.pixel_at  ox+x, oy+glyph_height-1-y
      color = (r >> 4 << 8) | (g >> 4 << 4) | ( b >> 4)
      if [0x555, 0xaaa, 0xfff].include? color
        lowest = ( y if lowest.nil?) || [ lowest, y].min
        highest = ( y if highest.nil?) || [ highest, y].max
        left = ( x if left.nil?) || [ left, x].min
        right = ( x if right.nil?) || [ right, x].max
      end
    end
  end
  # if there were no pixels at all such as for space..
  if lowest.nil?
    left = 0
    right = 2
    lowest = 0
    highest = -1
  end
  lift = lowest
  height = (highest + 1) - lowest
  width = (right + 1) - left
  #p [ left, right, lowest, highest]
  #p [ lift, width, height]
  # Compute the offset of the glyph data *before* adding to the stream
  offset = (bitstream.length if "bw" == format) || (bitstream.length / 2)
  (0..width-1).each do |x|
    (0..height-1).each do |y|
      r, g, b = glyph_sheet.pixel_at  ox+x, oy+glyph_height-1-lift-y
      color = (r >> 4 << 8) | (g >> 4 << 4) | ( b >> 4)
      #print "%03x,"% color
      if "bw" == format
        bitstream << ( (1 if 0xfff == color) || 0 )
      else
        encoded = [ 0x000, 0x555, 0xaaa, 0xfff].index( color) || 0
        bitstream << (encoded & 0x1)
        bitstream << (encoded >> 1)
      end
    end
    #puts
  end
  puts "  { data:0x%04x, width:%u, height:%u, lift:%u},"% [ offset, width, height, lift]
end
glyph_data = bitstream.bytes

puts "};\n\n"

puts "u8 const  #{variable_name}_data[] = {\n  %s\n};\n\n"% glyph_data.map{|b| "0x%02x"% b}.groups_of(16).reduce([]){|ar,g| ar + [("/* 0x%04x */ "% (8 * 16 * ar.count))+ g.join(", ")]
}.join(",\n  ")

puts <<EOF
Font const  #{variable_name} =
{
  height:          #{glyph_height},
  first_character: ' ',
  glyphs:          96,
  glyph:           #{variable_name}_glyphs,
  glyph_data:      #{variable_name}_data,
  absent_code:     127,
};

EOF

