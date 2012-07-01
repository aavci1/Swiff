#ifndef SWIFFCXFORM_H
#define SWIFFCXFORM_H

#include "SwiffBitStream.h"

namespace Swiff {
  class CxForm {
  public:
    s16 mRedMultTerm;
    s16 mGreenMultTerm;
    s16 mBlueMultTerm;
    s16 mAlphaMultTerm;
    s16 mRedAddTerm;
    s16 mGreenAddTerm;
    s16 mBlueAddTerm;
    s16 mAlphaAddTerm;
    bool mHasAlpha;

    CxForm(bool hasAlpha = false) : mRedMultTerm(256), mGreenMultTerm(256),
      mBlueMultTerm(256), mAlphaMultTerm(256), mRedAddTerm(0),
      mGreenAddTerm(0), mBlueAddTerm(0), mAlphaAddTerm(0), mHasAlpha(hasAlpha) {
    }

//    void transform(Color &color) {
//      color.mRed = qBound(0, ((color.mRed * mRedMultTerm) / 256) + mRedAddTerm, 255);
//      color.mGreen = qBound(0, ((color.mGreen * mGreenMultTerm) / 256) + mGreenAddTerm, 255);
//      color.mBlue = qBound(0, ((color.mBlue * mBlueMultTerm) / 256) + mBlueAddTerm, 255);
//      if (hasAlpha()) {
//        color.mAlpha = qBound(0, ((color.mAlpha * mAlphaMultTerm) / 256) + mAlphaAddTerm, 255);
//      }
//    }

    bool hasAlpha() const {
      return mHasAlpha;
    }

    void setHasAlpha(bool alpha) {
      mHasAlpha = alpha;
    }
  };

  inline BitStream &operator >> (BitStream &bitstream, CxForm &c) {
    // align to byte boundry
    bitstream.align();
    // read the bits
    u8 hasAddTerms = 0;
    hasAddTerms = bitstream.readUnsignedBits(1);
    u8 hasMultTerms = 0;
    hasMultTerms = bitstream.readUnsignedBits(1);
    u8 bitCount = 0;
    bitCount = bitstream.readUnsignedBits(4);
    if (hasMultTerms) {
      c.mRedMultTerm = bitstream.readSignedBits(bitCount);
      c.mGreenMultTerm = bitstream.readSignedBits(bitCount);
      c.mBlueMultTerm = bitstream.readSignedBits(bitCount);
      if (c.hasAlpha()) {
        c.mAlphaMultTerm = bitstream.readSignedBits(bitCount);
      }
    }
    if (hasAddTerms) {
      c.mRedAddTerm = bitstream.readSignedBits(bitCount);
      c.mGreenAddTerm = bitstream.readSignedBits(bitCount);
      c.mBlueAddTerm = bitstream.readSignedBits(bitCount);
      if (c.hasAlpha()) {
        c.mAlphaAddTerm = bitstream.readSignedBits(bitCount);
      }
    }
    return bitstream;
  }
}

#endif
