#!/usr/bin/env ruby
#
# Converts a PNM P6 image to a C struct
#
# Use:
#  lib/pnm2c.rb  image1 16bit < battery-symbol.pnm
#

if ARGV.count != 2  ||  ! %w"16bit CLUT16".include?( ARGV.last)
  $stderr.puts "Use: #{$0} variable-name 16bit|CLUT16"
  exit 1
end

variable_name, format = ARGV

def check_that its_true
  raise StandardError.new unless its_true
end


$width = $height = 0
expecting = :file_type

$stdin.each_line do |line|
  # Example PNM file:
  #P6
  ## CREATOR: GIMP PNM Filter Version 1.1
  #8 14
  #255
  #binary data, three bytes per pixel, red then green then blue
  next if line.start_with? "#"
  case expecting
    when :file_type
      check_that "P6\n" == line
      expecting = :dimensions
    when :dimensions
      check_that  line.match %r"^(\d+) (\d+)\n$"
      $width, $height = $1.to_i, $2.to_i
      expecting = :depth
    when :depth
      check_that  line.match %r"^(\d+)$"
      highest_value = $1.to_i
      check_that 255 == highest_value
      break
  end
end
$data = $stdin.read
check_that  3*$width*$height == $data.length
check_that  0 == $stdin.read.length

# An array.  The index is the index in the final CLUT and the values are the
# RGB565 colours
$clut = []
pixels = []

def with_each_pixel
  (0..$width-1).each do |x|
    ($height-1).downto(0).each do |y|
      pixel_base = 3 * ($width * y + x)
      r, g, b = (0..2).map {|i| $data[ pixel_base + i] }
      rgb = (r >> 3) | (g >> 2 << 5) | ( b >> 3 << 11)
      i = $clut.index  rgb
      if i.nil?
        raise StandardError.new "Out of colors" if 16 == $clut.length
        $clut << rgb
        i = $clut.index  rgb
      end
      yield x, ($height - 1 - y), rgb, i
    end
  end
end


case format

  when "16bit"

    with_each_pixel do |x, y, rgb, i|
      pixels << rgb
    end
    puts <<EOF
u16 const #{variable_name}_pixels[] = {
  #{pixels.map {|px| "0x%04x"% px}.join ", "}
};
Image16bit const #{variable_name} =
{
  width: #{$width},
  height: #{$height},
  pixels: #{variable_name}_pixels,
};
EOF

  when "CLUT16"
    with_each_pixel do |x, y, rgb, i|
      case y & 1
        when 0 # evenly-numbered row
          pixels << i
        when 1
          pixels[-1] |= (i << 4)
      end
    end

    puts <<EOF
u8 const #{variable_name}_data[] = {
  #{pixels.map {|px| "0x%02x"% px}.join ", "}
};
ImageCLUT16 const #{variable_name} =
{
  width: #{$width},
  height: #{$height},
  colors: {#{$clut.map {|cl| "0x%04x"% cl}.join ", "}},
  data: #{variable_name}_data,
};
EOF

end

