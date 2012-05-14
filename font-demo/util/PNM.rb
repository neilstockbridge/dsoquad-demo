
module Checking

  def check_that its_true
    raise StandardError.new unless its_true
  end

end


module PNM

  class Image

    attr_reader :width, :height

    def initialize width, height, data
      @width = width
      @height = height
      @data = data
    end

    # Use:
    #  r, g, b = pixel_at  x, y
    #
    def pixel_at x, y
      pixel_base = 3 * (@width * y + x)
      (0..2).map {|i| @data[ pixel_base + i] }
    end

  end


  class << self

    def p6_from stream
      width = height = 0
      expecting = :file_type

      stream.each_line do |line|
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
            width, height = $1.to_i, $2.to_i
            expecting = :depth
          when :depth
            check_that  line.match %r"^(\d+)$"
            highest_value = $1.to_i
            check_that 255 == highest_value
            break
        end
      end
      data = stream.read
      check_that  (3 * width * height) == data.length
      check_that  0 == stream.read.length
      Image.new  width, height, data
    end

    include Checking

  end # of class methods

end

