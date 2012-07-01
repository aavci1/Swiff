#ifndef SWIFFREMOVEOBJECT2_H
#define SWIFFREMOVEOBJECT2_H

#include "SwiffTag.h"

namespace Swiff {
  class RemoveObject2 : public Tag {
  public:
    u16 mDepth;

    RemoveObject2() : Tag("RemoveObject2"), mDepth(0) {
    }

    void parse(BitStream&);
  };
}

#endif
