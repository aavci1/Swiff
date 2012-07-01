#include "SwiffPlaceObject2.h"

#include "SwiffBitStream.h"
#include "SwiffClipActionRecord.h"

namespace Swiff {
  PlaceObject2::~PlaceObject2() {
    // clean up
    qDeleteAll(mClipActions);
  }

  void PlaceObject2::parse(BitStream &bitstream) {
    // parse the header
    Tag::parse(bitstream);
    // parse the flags
    bitstream >> mFlags >> mDepth;
    // process flags
    if (hasCharacter()) {
      bitstream >> mCharacterId;
    }
    if (hasMatrix()) {
      bitstream >> mMatrix;
    }
    if (hasColorTransform()) {
      bitstream >> mColorTransform;
    }
    if (hasRatio()) {
      bitstream >> mRatio;
    }
    if (hasName()) {
      bitstream >> mObjectName;
    }
    if (hasClipDepth()) {
      bitstream >> mClipDepth;
    }
    // enable clip actions
    if (hasClipActions()) {
      u16 reserved = 0;
      bitstream >> reserved >> mAllEventFlags;
      while (bitstream.currentu32() != 0x00000000) {
        ClipActionRecord *record = new ClipActionRecord();
        bitstream >> (*record);
        mClipActions.append(record);
      }
      bitstream >> reserved;
      bitstream >> reserved;
    }
  }
}
