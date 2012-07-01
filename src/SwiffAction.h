#ifndef SWIFFACTION_H
#define SWIFFACTION_H

#include "SwiffTypes.h"

namespace Swiff {
  class BitStream;

  class Action {
  public:
    /** Constructor */
    Action() : mActionCode(0), mByteIndex(0), mLength(0), mPayLoad(0) {}
    ~Action() {
      delete mPayLoad;
    }
    /** Parses the action code and length using the \param bitstream. */
    void parse(BitStream &bitstream);
    /** \returns action code of the action. */
    u8 actionCode();
    /** \returns byte index of the action in the bitstream. */
    u32 byteIndex();
    /** \returns length of the action. */
    u16 length();
    /** \returns total length of the action including the header. */
    u16 totalLength();
    /** \returns payload of the action. */
    u8 *payload();
  private:
    u8 mActionCode;
    u32 mByteIndex;
    u16 mLength;
    u8 *mPayLoad;
  };
}

#endif
