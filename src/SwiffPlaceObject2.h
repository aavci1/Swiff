#ifndef SWIFFPLACEOBJECT2_H
#define SWIFFPLACEOBJECT2_H

#include "SwiffClipEventFlags.h"
#include "SwiffCxForm.h"
#include "SwiffTag.h"

#include <QMatrix>
#include <QVector>

namespace Swiff {
  class BitStream;
  class ClipActionRecord;

  class PlaceObject2 : public Tag {
  public:
    u8 mFlags;
    u16 mDepth;
    u16 mCharacterId;
    QMatrix mMatrix;
    CxForm mColorTransform;
    u16 mRatio;
    QString mObjectName;
    u16 mClipDepth;
    ClipEventFlags mAllEventFlags;
    QVector<ClipActionRecord *> mClipActions;

    PlaceObject2() : Tag("PlaceObject2"), mFlags(0), mDepth(0), mCharacterId(0), mColorTransform(true),
      mRatio(0), mObjectName(""), mClipDepth(0) {
    }

    ~PlaceObject2();

    inline u8 hasMove() {
      return mFlags & 0x01;
    }

    inline u8 hasCharacter() {
      return mFlags & 0x02;
    }

    inline u8 hasMatrix() {
      return mFlags & 0x04;
    }
    inline u8 hasColorTransform() {
      return mFlags & 0x08;
    }

    inline u8 hasRatio() {
      return mFlags & 0x10;
    }

    inline u8 hasName() {
      return mFlags & 0x20;
    }

    inline u8 hasClipDepth() {
      return mFlags & 0x40;
    }

    inline u8 hasClipActions() {
      return mFlags & 0x80;
    }

    void parse(BitStream &bitstream);
  };
}

#endif
