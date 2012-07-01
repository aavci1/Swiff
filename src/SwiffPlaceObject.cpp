#include "SwiffPlaceObject.h"

#include "SwiffBitStream.h"

namespace Swiff {
  void PlaceObject::parse(BitStream &bitstream) {
    Tag::parse(bitstream);
    bitstream >> mCharacterId >> mDepth >> mMatrix;
    bitstream.align();
    if (bitstream.byteIndex() != mByteIndex + mLength) {
      // if we are not finished yet read the color transform
      bitstream >> mColorTransform;
      mHasColorTransform = true;
    }
  }
}
