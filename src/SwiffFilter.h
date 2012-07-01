#ifndef SWIFFFILTER_H
#define SWIFFFILTER_H

#include <QColor>

#include "SwiffTypes.h"

namespace Swiff {
  class BitStream;

  class Filter {
  public:
    static const u8 DropShadow = 0;
    static const u8 Blur = 1;
    static const u8 Glow = 2;
    static const u8 Bevel = 3;
    static const u8 GradientGlow = 4;
    static const u8 Convolution = 5;
    static const u8 ColorMatrix = 6;
    static const u8 GradientBevel = 7;
    static const u8 Unknown = 255;

    u8 mFilterId;

    Filter() : mFilterId(Unknown) {
    }

    virtual ~Filter() {
    }

    static Filter *fromFilterId(u8 filterId);

    virtual void parse(BitStream &bitstream) = 0;
  };
  /**
    R'        r0 r1 r2 r3 r4   R
    G'        g0 g1 g2 g3 g4   G
    B'      = b0 b1 b2 b3 b4 * B
    A'        a0 a1 a2 a3 a4   A
    1          0  0  0  0  1   I
  */
  class ColorMatrixFilter : public Filter {
  public:
    float mMatrix[20];

    ColorMatrixFilter() {
      mFilterId = Filter::ColorMatrix;
    }

    void parse(BitStream &bitstream);
  };
  /***/
  class ConvolutionFilter : public Filter {
  public:
    u8 mMatrixX;
    u8 mMatrixY;
    float mDivisor;
    float mBias;
    float *mMatrix;
    QColor mDefaultColor;
    u8 mFlags;

    ConvolutionFilter() : mDivisor(0), mBias(0), mMatrix(0) {
      mFilterId = Filter::Convolution;
    }

    ~ConvolutionFilter() {
      delete [] mMatrix;
    }

    u8 clamp() const {
      return mFlags & 0x02;
    }

    u8 preserveAlpha() const {
      return mFlags & 0x01;
    }

    void parse(BitStream &bitstream);
  };

  class BlurFilter : public Filter {
  public:
    f32 mBlurX;
    f32 mBlurY;
    u8 mPasses;

    BlurFilter() : mBlurX(0), mBlurY(0) {
      mFilterId = Filter::Blur;
    }

    void parse(BitStream &bitstream);
  };

  class DropShadowFilter : public Filter {
  public:
    QColor mDropShadowColor;
    f32 mBlurX;
    f32 mBlurY;
    f32 mAngle;
    f32 mDistance;
    f16 mStrength;
    u8 mFlags;

    DropShadowFilter() : mBlurX(0), mBlurY(0), mAngle(0), mDistance(0), mStrength(0) {
      mFilterId = Filter::DropShadow;
    }

    u8 innerShadow() const {
      return mFlags & 0x80;
    }

    u8 knockout() const {
      return mFlags & 0x40;
    }

    u8 compositeSource() const {
      return mFlags & 0x20;
    }

    u8 passes() const {
      return mFlags & 0x1f;
    }

    void parse(BitStream &bitstream);
  };

  class GlowFilter : public Filter {
  public:
    QColor mGlowColor;
    f32 mBlurX;
    f32 mBlurY;
    f16 mStrength;
    u8 mFlags;

    GlowFilter() : mBlurX(0), mBlurY(0), mStrength(0) {
      mFilterId = Filter::Glow;
    }

    u8 innerGlow() const {
      return mFlags & 0x80;
    }

    u8 knockout() const {
      return mFlags & 0x40;
    }

    u8 passes() const {
      return mFlags & 0x1f;
    }

    void parse(BitStream &bitstream);
  };

  class BevelFilter : public Filter {
  public:
    QColor mShadowColor;
    QColor mHighlightColor;
    f32 mBlurX;
    f32 mBlurY;
    f32 mAngle;
    f32 mDistance;
    f16 mStrength;
    u8 mFlags;

    BevelFilter() : mBlurX(0), mBlurY(0), mAngle(0), mDistance(0), mStrength(0) {
      mFilterId = Filter::Bevel;
    }

    u8 innerShadow() const {
      return mFlags & 0x80;
    }

    u8 knockout() const {
      return mFlags & 0x40;
    }

    u8 compositeSource() const {
      return mFlags & 0x20;
    }
    u8 onTop() const {
      return mFlags & 0x10;
    }
    u8 passes() const {
      return mFlags & 0x0f;
    }

    void parse(BitStream &bitstream);
  };

  class GradientGlowFilter : public Filter {
  public:
    u8 mColorCount;
    QColor *mGradientColors;
    u8 *mRatios;
    f32 mBlurX;
    f32 mBlurY;
    f32 mAngle;
    f32 mDistance;
    f16 mStrength;
    u8 mFlags;

    GradientGlowFilter() : mGradientColors(0), mRatios(0), mBlurX(0), mBlurY(0), mAngle(0), mDistance(0), mStrength(0) {
      mFilterId = Filter::GradientGlow;
    }

    ~GradientGlowFilter() {
      delete [] mGradientColors;
      delete [] mRatios;
    }

    u8 innerGlow() const {
      return mFlags & 0x80;
    }

    u8 knockout() const {
      return mFlags & 0x40;
    }

    u8 passes() const {
      return mFlags & 0x1f;
    }

    void parse(BitStream &bitstream);
  };

  class GradientBevelFilter : public Filter {
  public:
    u8 mColorCount;
    QColor *mGradientColors;
    u8 *mRatios;
    f32 mBlurX;
    f32 mBlurY;
    f32 mAngle;
    f32 mDistance;
    f16 mStrength;
    u8 mFlags;

    GradientBevelFilter() : mGradientColors(0), mRatios(0), mBlurX(0), mBlurY(0), mAngle(0), mDistance(0), mStrength(0) {
      mFilterId = Filter::GradientBevel;
    }

    ~GradientBevelFilter() {
      delete [] mGradientColors;
      delete [] mRatios;
    }

    u8 innerShadow() const {
      return mFlags & 0x80;
    }

    u8 knockout() const {
      return mFlags & 0x40;
    }

    u8 compositeSource() const {
      return mFlags & 0x20;
    }
    u8 onTop() const {
      return mFlags & 0x10;
    }
    u8 passes() const {
      return mFlags & 0x0f;
    }

    void parse(BitStream &bitstream);
  };
}

#endif
