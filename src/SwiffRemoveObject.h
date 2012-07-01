#ifndef SWIFFREMOVEOBJECT_H
#define SWIFFREMOVEOBJECT_H

#include "SwiffTag.h"

namespace Swiff {
  class RemoveObject : public Tag {
  public:
    u16 mCharacterId;
    u16 mDepth;

    RemoveObject() : Tag("RemoveObject"), mCharacterId(0), mDepth(0) {
    }

    void parse(BitStream&);
  };
}

#endif
