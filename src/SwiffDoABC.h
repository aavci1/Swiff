#ifndef SWIFFDOABC_H
#define SWIFFDOABC_H

#include "SwiffTag.h"

namespace Swiff {
  class ABCData;
  class BitStream;

  class DoABC : public Tag {
  public:
    DoABC();
    ~DoABC();

    void parse(BitStream&);
  public:
    u32 mFlags;
    QString mName;
    ABCData *mABCData;
  };
}

#endif
