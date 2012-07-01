#ifndef SWIFFTAG_H
#define SWIFFTAG_H

#include "SwiffTypes.h"

#include <QString>

namespace Swiff {
  class BitStream;

  class Tag {
  public:
    Tag() : mId(0), mLength(0), mName("Unknown") {
    }

    Tag(const char *name) : mId(0), mLength(0), mName(name) {
    }

    virtual ~Tag() {
    }

    u16 id() const {
      return mId;
    }
    u32 length() const {
      return mLength;
    }
    u32 index() const {
      return mByteIndex;
    }
    QString name() const {
      return mName;
    }
    virtual void parse(BitStream &bitstream);
  protected:
    u16 mId;
    u32 mLength;
    u32 mByteIndex;
    QString mName;
  };
}

#endif
