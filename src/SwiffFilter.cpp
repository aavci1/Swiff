#include "SwiffBitStream.h"
#include "SwiffFilter.h"

namespace Swiff {
  Filter *Filter::fromFilterId(u8 filterId) {
    switch (filterId) {
    case Swiff::ColorMatrix:
      return new ColorMatrixFilter();
      break;
    case Swiff::Convolution:
      return new ConvolutionFilter();
      break;
    case Swiff::Bevel:
      return new BevelFilter();
      break;
    case Swiff::Blur:
      return new BlurFilter();
      break;
    case Swiff::DropShadow:
      return new DropShadowFilter();
      break;
    case Swiff::Glow:
      return new GlowFilter();
      break;
    case Swiff::GradientBevel:
      return new GradientBevelFilter();
      break;
    case Swiff::GradientGlow:
      return new GradientGlowFilter();
      break;
    }
    return 0;
  }

  void ColorMatrixFilter::parse(BitStream &bitstream) {
    bitstream >> mFilterId;
    for (int i = 0; i < 20; ++i) {
      bitstream >> mMatrix[i];
    }
  }

  void ConvolutionFilter::parse(BitStream &bitstream) {
    bitstream >> mFilterId >> mMatrixX >> mMatrixY >> mDivisor >> mBias;
    mMatrix = new float[mMatrixX * mMatrixY];
    for (u16 i = 0; i < mMatrixX * mMatrixY; ++i) {
      bitstream >> mMatrix[i];
    }
    bitstream >> mDefaultColor >> mFlags;
  }

  void BlurFilter::parse(BitStream &bitstream) {
    bitstream >> mFilterId;
    bitstream >> mBlurX;
    bitstream >> mBlurY;
    bitstream >> mPasses;
    mPasses >>= 3;
  }

  void DropShadowFilter::parse(BitStream &bitstream) {
    bitstream >> mFilterId;
    bitstream >> mDropShadowColor;
    bitstream >> mBlurX;
    bitstream >> mBlurY;
    bitstream >> mAngle;
    bitstream >> mDistance;
    bitstream >> mStrength;
    bitstream >> mFlags;
  }

  void GlowFilter::parse(BitStream &bitstream) {
    bitstream >> mFilterId;
    bitstream >> mGlowColor;
    bitstream >> mBlurX;
    bitstream >> mBlurY;
    bitstream >> mStrength;
    bitstream >> mFlags;
  }

  void BevelFilter::parse(BitStream &bitstream) {
    bitstream >> mFilterId;
    bitstream >> mShadowColor >> mHighlightColor;
    bitstream >> mBlurX;
    bitstream >> mBlurY;
    bitstream >> mAngle;
    bitstream >> mDistance;
    bitstream >> mStrength;
    bitstream >> mFlags;
  }

  void GradientGlowFilter::parse(BitStream &bitstream) {
    bitstream >> mFilterId;
    bitstream >> mColorCount;
    mGradientColors = new QColor[mColorCount];
    for (u8 i = 0; i < mColorCount; ++i) {
      bitstream >> mGradientColors[i];
    }
    mRatios = new u8[mColorCount];
    for (u8 i = 0; i < mColorCount; ++i) {
      bitstream >> mRatios[i];
    }
    bitstream >> mBlurX;
    bitstream >> mBlurY;
    bitstream >> mAngle;
    bitstream >> mDistance;
    bitstream >> mStrength;
    bitstream >> mFlags;
  }

  void GradientBevelFilter::parse(BitStream &bitstream) {
    bitstream >> mFilterId;
    bitstream >> mColorCount;
    mGradientColors = new QColor[mColorCount];
    for (u8 i = 0; i < mColorCount; ++i) {
      bitstream >> mGradientColors[i];
    }
    mRatios = new u8[mColorCount];
    for (u8 i = 0; i < mColorCount; ++i) {
      bitstream >> mRatios[i];
    }
    bitstream >> mBlurX;
    bitstream >> mBlurY;
    bitstream >> mAngle;
    bitstream >> mDistance;
    bitstream >> mStrength;
    bitstream >> mFlags;
  }
}
