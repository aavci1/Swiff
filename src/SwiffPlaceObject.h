#ifndef SWIFFPLACEOBJECT_H
#define SWIFFPLACEOBJECT_H

#include "SwiffCxForm.h"
#include "SwiffTag.h"

#include <QMatrix>

namespace Swiff {
  class BitStream;

  class PlaceObject : public Tag {
  public:
    PlaceObject() : Tag("PlaceObject"), mHasColorTransform(false) {
    }

    void parse(BitStream &bitstream);

    bool hasColorTransform() {
      return mHasColorTransform;
    }
  public:
    u16 mCharacterId;
    u16 mDepth;
    QMatrix mMatrix;
    CxForm mColorTransform;
    bool mHasColorTransform;
  };
}

#endif
