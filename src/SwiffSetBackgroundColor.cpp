#include "SwiffSetBackgroundColor.h"

#include "SwiffBitStream.h"

namespace Swiff {
  void SetBackgroundColor::parse(BitStream &bitstream) {
    Tag::parse(bitstream);
    bitstream >> mBackgroundColor;
  }
}
