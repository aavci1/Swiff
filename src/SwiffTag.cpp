#include "SwiffBitStream.h"
#include "SwiffTag.h"

namespace Swiff {
  void Tag::parse(BitStream &bitstream) {
    u16 idAndLength = 0;
    bitstream >> idAndLength;
    mId = idAndLength >> 6;
    mLength = idAndLength & 0x3f;
    if (mLength >= 0x3f) {
      bitstream >> mLength;
    }
    mByteIndex = bitstream.byteIndex();
  }
}
