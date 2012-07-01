#include "SwiffPlaceObject3.h"

#include "SwiffBitStream.h"
#include "SwiffClipActionRecord.h"
#include "SwiffFilter.h"

namespace Swiff {
  PlaceObject3::~PlaceObject3() {
    // clean up
    qDeleteAll(mSurfaceFilters);
    qDeleteAll(mActionRecords);
  }

  void PlaceObject3::parse(BitStream &bitstream) {
    // parse the header
    Tag::parse(bitstream);
    // parse the flags
    bitstream >> mFlags >> mDepth;
    // process flags
    if (hasClassName() || (hasImage() && hasCharacter())) {
      bitstream >> mClassName;
    }
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
    if (hasFilterList()) {
      u8 filterCount = 0;
      bitstream >> filterCount;
      for (u8 i = 0; i < filterCount; ++i) {
        Filter *filter = Filter::fromFilterId(bitstream.currentu8());
        filter->parse(bitstream);
        mSurfaceFilters.append(filter);
      }
    }
    if (hasBlendMode()) {
      bitstream >> mBlendMode;
    }
    if (hasCacheAsBitmap()) {
      bitstream >> mBitmapCache;
    }
    // enable clip actions
    if (hasClipActions()) {
      u16 reserved = 0;
      bitstream >> reserved >> mAllEventFlags;
      while (bitstream.currentu32() != 0x00000000) {
        ClipActionRecord *record = new ClipActionRecord();
        bitstream >> (*record);
        mActionRecords.append(record);
      }
      bitstream >> reserved;
      bitstream >> reserved;
    }
  }
}
