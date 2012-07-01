#ifndef SWIFFCLIPACTIONRECORD_H
#define SWIFFCLIPACTIONRECORD_H

#include "SwiffAction.h"
#include "SwiffActionScriptEngine.h"
#include "SwiffBitStream.h"
#include "SwiffClipEventFlags.h"

#include <QList>

namespace Swiff {
  class ClipActionRecord {
  public:
    ClipEventFlags mEventFlags;
    u32 mSize;
    u8 mKeyCode;
    QVector<Action *> mActions;

    ClipActionRecord() : mSize(0), mKeyCode(0) {
    }

    ~ClipActionRecord() {
    }
  };

  inline BitStream &operator >> (BitStream &bitstream, ClipActionRecord& c) {
    bitstream >> c.mEventFlags;
    bitstream >> c.mSize;
    u32 end = bitstream.byteIndex() + c.mSize;
    if (c.mEventFlags.keyPress()) {
      bitstream >> c.mKeyCode;
    }
    c.mActions = ActionScriptEngine::parseActions(bitstream);
    bitstream.seek(end);
    return bitstream;
  }
}

#endif
