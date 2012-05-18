
# Use:
#
#  bs = Bitstream.new
#  bs << 1
#  bs << 0
#  bs << 1
#  bytes = bs.bytes
#
class Bitstream

  attr_reader :length, :bytes

  def initialize
    @bytes = []
    @length = 0
  end

  def << bit
    # if a new byte is required..
    if 0 == @length % 8
      @bytes << 0x00
    end
    position = @length % 8
    @bytes[-1] |= (bit << position)
    @length += 1
  end

end

