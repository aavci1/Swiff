#include "SwiffRemoveObject.h"

#include "SwiffBitStream.h"

namespace Swiff {
  void RemoveObject::parse(BitStream &bitstream) {
    Tag::parse(bitstream);
    bitstream >> mCharacterId >> mDepth;
  }
}
