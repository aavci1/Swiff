#ifndef SWIFFLINESTYLE_H
#define SWIFFLINESTYLE_H

#include "SwiffBitStream.h"
#include "SwiffTag.h"

#include <QColor>

namespace Swiff {
  class LineStyle {
  public:
    int mVersion;
    u16 mWidth;
    u16 mFlags;
    u16 mMiterLimitFactor;
    QColor mColor;
    QBrush mBrush;

    LineStyle(int version = 1) : mVersion(version), mFlags(0), mMiterLimitFactor(0) {
      if (mVersion <= 2) {
        mColor.setRgba(qRgba(0, 0, 0, 255));
      }
    }

    ~LineStyle() {
    }
    /**
      0 = Round cap
      1 = No cap
      2 = Square cap
    */
    u8 startCapStyle() const {
      return (mFlags >> 14) & 0x03;
    }
    /**
      0 = Round join
      1 = Bevel join
      2 = Miter join
    */
    u8 joinStyle() const {
      return (mFlags >> 12) & 0x03;
    }

    u8 hasFillFlag() const {
      return (mFlags >> 11) & 0x01;
    }

    u8 noHorizontalScale() const {
      return (mFlags >> 10) & 0x01;
    }

    u8 noVerticalScale() const {
      return (mFlags >> 9) & 0x01;
    }

    u8 pixelHinting() const {
      return (mFlags >> 8) & 0x01;
    }

    u8 noClose() const {
      return (mFlags >> 2) & 0x01;
    }
    /**
      0 = Round cap
      1 = No cap
      2 = Square cap
    */
    u8 endCapStyle() const {
      return mFlags & 0x03;
    }
  };

  inline BitStream &operator >> (BitStream &bitstream, LineStyle &l) {
    bitstream >> l.mWidth;
    if (l.mVersion <= 3) {
      bitstream >> l.mColor;
    } else {
      bitstream >> l.mFlags;
      if (l.joinStyle() == 2) {
        bitstream >> l.mMiterLimitFactor;
      }
      if (l.hasFillFlag()) {
        if (l.mVersion <= 2)
          l.mBrush.setColor(QColor(0, 0, 0, 255));
        else
          l.mBrush.setColor(QColor(0, 0, 0, 0));
        bitstream >> l.mBrush;
      } else {
        bitstream >> l.mColor;
      }
    }
    return bitstream;
  }
}

#endif
