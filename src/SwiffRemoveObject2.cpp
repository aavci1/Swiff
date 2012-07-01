#include "SwiffRemoveObject2.h"

#include "SwiffBitStream.h"

namespace Swiff {
  void RemoveObject2::parse(BitStream &bitstream) {
    Tag::parse(bitstream);
    bitstream >> mDepth;
  }
}
