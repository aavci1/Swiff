#include "SwiffAction.h"
#include "SwiffBitStream.h"

namespace Swiff {
  void Action::parse(BitStream &bitstream)  {
    bitstream >> mActionCode;
    if (mActionCode & 0x80)
      bitstream >> mLength;
    // save byte index
    mByteIndex = bitstream.byteIndex();
    // read pay load if exists
    if (mLength) {
      // create pay load buffer
      mPayLoad = new u8[mLength];
      // read payload
      bitstream.readBytes(mPayLoad, mLength);
    }
  }

  u8 Action::actionCode() {
    return mActionCode;
  }

  u32 Action::byteIndex() {
    return mByteIndex;
  }

  u16 Action::length() {
    return mLength;
  }

  u16 Action::totalLength() {
    if (mActionCode & 0x80)
      return mLength + 3;
    return mLength + 1;
  }

  u8 *Action::payload() {
    return mPayLoad;
  }
}
