#ifndef SWIFFCLIPEVENTFLAGS_H
#define SWIFFCLIPEVENTFLAGS_H

#include "SwiffBitStream.h"

namespace Swiff {
  class ClipEventFlags {
  public:
    u32 mFlags;

    ClipEventFlags() : mFlags(0) {
    }

    ~ClipEventFlags() {
    }

    inline u8 keyUp() const {
      return mFlags & 0x80000000;
    }

    inline u8 keyDown() const {
      return mFlags & 0x40000000;
    }

    inline u8 mouseUp() const {
      return mFlags & 0x20000000;
    }

    inline u8 mouseDown() const {
      return mFlags & 0x10000000;
    }

    inline u8 mouseMove() const {
      return mFlags & 0x08000000;
    }

    inline u8 unload() const {
      return mFlags & 0x04000000;
    }

    inline u8 enterFrame() const {
      return mFlags & 0x02000000;
    }

    inline u8 load() const {
      return mFlags & 0x01000000;
    }

    inline u8 dragOver() const {
      return mFlags & 0x00800000;
    }

    inline u8 rollOut() const {
      return mFlags & 0x00400000;
    }

    inline u8 rollOver() const {
      return mFlags & 0x00200000;
    }

    inline u8 releaseOutside() const {
      return mFlags & 0x00100000;
    }

    inline u8 release() const {
      return mFlags & 0x00080000;
    }

    inline u8 press() const {
      return mFlags & 0x00040000;
    }

    inline u8 initialize() const {
      return mFlags & 0x00020000;
    }

    inline u8 data() const {
      return mFlags & 0x00010000;
    }

    inline u8 construct() const {
      return mFlags & 0x00000400;
    }

    inline u8 keyPress() const {
      return mFlags & 0x00000200;
    }

    inline u8 dragOut() const {
      return mFlags & 0x00000100;
    }
  };

  inline BitStream &operator >> (BitStream &bitstream, ClipEventFlags &c) {
    return bitstream >> c.mFlags;
  }
}

#endif
