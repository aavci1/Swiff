#ifndef SWIFFPLACEOBJECT3_H
#define SWIFFPLACEOBJECT3_H

#include "SwiffClipEventFlags.h"
#include "SwiffCxForm.h"
#include "SwiffTag.h"

#include <QMatrix>
#include <QVector>

namespace Swiff {
  class BitStream;
  class ClipActionRecord;
  class Filter;

  class PlaceObject3 : public Tag {
  public:
    u16 mFlags;
    u16 mDepth;
    QString mClassName;
    u16 mCharacterId;
    QMatrix mMatrix;
    CxForm mColorTransform;
    u16 mRatio;
    QString mObjectName;
    u16 mClipDepth;
    QVector<Filter *> mSurfaceFilters;
    u8 mBlendMode;
    u8 mBitmapCache;
    ClipEventFlags mAllEventFlags;
    QVector<ClipActionRecord *> mActionRecords;

    PlaceObject3() : Tag("PlaceObject3"), mFlags(0), mClassName(""), mColorTransform(true), mObjectName("") {
    }

    ~PlaceObject3();

    u16 hasClipActions() {
      return mFlags & 0x8000;
    }
    u16 hasClipDepth() {
      return mFlags & 0x4000;
    }
    u16 hasName() {
      return mFlags & 0x2000;
    }
    u16 hasRatio() {
      return mFlags & 0x1000;
    }
    u16 hasColorTransform() {
      return mFlags & 0x0800;
    }
    u16 hasCharacter() {
      return mFlags & 0x0400;
    }
    u16 hasMatrix() {
      return mFlags & 0x0200;
    }
    u16 hasMove() {
      return mFlags & 0x0100;
    }
    u16 hasImage() {
      return mFlags & 0x0010;
    }
    u16 hasClassName() {
      return mFlags & 0x0008;
    }
    u16 hasCacheAsBitmap() {
      return mFlags & 0x0004;
    }
    u16 hasBlendMode() {
      return mFlags & 0x0002;
    }
    u16 hasFilterList() {
      return mFlags & 0x0001;
    }

    void parse(BitStream &bitstream);
  };
}

#endif
