#include "SwiffBitStream.h"
#include "SwiffBitmap.h"
#include "SwiffDocument.h"

#include <QByteArray>
#include <QBrush>
#include <QColor>
#include <QMap>
#include <QMatrix>
#include <QRect>
#include <QString>

namespace Swiff {
  class BitStreamPrivate {
  public:
    BitStreamPrivate() : mData(0), mSize(0), mByteIndex(0), mBitIndex(7) {
    }
    ~BitStreamPrivate() {
      delete mData;
    }

    Document *mDocument;
    u8 *mData;
    u32 mSize;
    u32 mByteIndex;
    u8 mBitIndex;
  };

  BitStream::BitStream(u8 *data, u32 size, bool compressed) : d(new BitStreamPrivate()) {
    if (compressed) {
      QByteArray array = uncompress(data, size);
      d->mData = new u8[array.size()];
      memcpy(d->mData, array.constData(), array.size());
      d->mSize = array.size();
      // clean up
      delete data;
    } else {
      d->mData = data;
      d->mSize = size;
    }
  }

  BitStream::BitStream(const QByteArray &byteArray, bool compressed) : d(new BitStreamPrivate()) {
    if (compressed) {
      QByteArray array = uncompress((u8 *)byteArray.constData(), byteArray.size());
      d->mData = new u8[array.size()];
      memcpy(d->mData, array.constData(), array.size());
      d->mSize = array.size();
    } else {
      d->mData = new u8[byteArray.size()];
      memcpy(d->mData, byteArray.constData(), byteArray.size());
      d->mSize = byteArray.size();
    }
  }

  BitStream::~BitStream() {
    delete d;
  }

  QByteArray BitStream::uncompress(const u8* compressed, const u32 &compressedLength) {
    QByteArray c;
    QDataStream ds(&c, QIODevice::ReadWrite);
    ds << compressedLength;
    ds.writeRawData((char *)compressed, compressedLength);
    return qUncompress(c);
  }

  Document *BitStream::document() const {
    return d->mDocument;
  }

  void BitStream::setDocument(Document *document) {
    d->mDocument = document;
  }

  u8* BitStream::data() const {
    return d->mData;
  }

  u32 BitStream::byteIndex() const {
    return d->mByteIndex;
  }

  u8 BitStream::bitIndex() const {
    return d->mBitIndex;
  }

  void BitStream::seek(const u32 byteIndex, const u8 bitIndex) {
    if (byteIndex < d->mSize && bitIndex < 8) {
      d->mByteIndex = byteIndex;
      d->mBitIndex = bitIndex;
    }
  }

  void BitStream::skip(const u32 byteCount, const u8 bitCount) {
    d->mByteIndex += byteCount;
    for (int j = bitCount - 1; j >= 0; --j) {
      if (d->mBitIndex == 0) {
        d->mBitIndex = 7;
        d->mByteIndex++;
      } else {
        d->mBitIndex--;
      }
    }
  }

  void BitStream::align() {
    if (d->mBitIndex != 7) {
      seek(d->mByteIndex + 1);
    }
  }

  u8 BitStream::currentu8() {
    // align to next byte boundry
    align();
    // return current u8
    return d->mData[d->mByteIndex];
  }

  u16 BitStream::currentu16() {
    // align to next byte boundry
    align();
    // return current u16
    return d->mData[d->mByteIndex + 1] << 8 | d->mData[d->mByteIndex];
  }

  u32 BitStream::currentu32() {
    // align to next byte boundry
    align();
    // return current u32
    return d->mData[d->mByteIndex + 3] << 24 | d->mData[d->mByteIndex + 2]  << 16 |
           d->mData[d->mByteIndex + 1]  << 8 | d->mData[d->mByteIndex];
  }

  void BitStream::readBytes(u8* data, u32 byteCount) {
    align();
    for (u32 i = 0; i < byteCount; ++i) {
      data[i] = d->mData[d->mByteIndex];
      d->mByteIndex++;
    }
  }

  u32 BitStream::readUnsignedBits(u8 bitCount) {
    u32 u = 0;
    for (int j = bitCount - 1; j >= 0; --j) {
      u |= ((d->mData[d->mByteIndex] >> d->mBitIndex) & 0x01) << j;
      if (d->mBitIndex == 0) {
        // process next byte
        d->mBitIndex = 7;
        d->mByteIndex++;
      } else {
        d->mBitIndex--;
      }
    }
    return u;
  }

  s32 BitStream::readSignedBits(u8 bitCount) {
    s32 sign = ((d->mData[d->mByteIndex] >> d->mBitIndex) & 0x01) << 31;
    s32 s = readUnsignedBits(bitCount);
    if (sign) {
      for (int j = 32; j > bitCount; --j) {
        s |= sign;
        sign >>= 1;
      }
    }
    return s;
  }

  f32 BitStream::readFixedBits(u8 bitCount) {
    return readSignedBits(bitCount) * 0.0000152587890625f;
  }

  BitStream &BitStream::operator >> (u8 &u) {
    align();
    u = d->mData[d->mByteIndex];
    d->mByteIndex++;
    return *this;
  }

  BitStream &BitStream::operator >> (u16 &u) {
    align();
    u = d->mData[d->mByteIndex + 1] << 8 | d->mData[d->mByteIndex];
    d->mByteIndex += 2;
    return *this;
  }

  BitStream &BitStream::operator >> (u32 &u) {
    align();
    u = d->mData[d->mByteIndex + 3] << 24 | d->mData[d->mByteIndex + 2]  << 16 |
        d->mData[d->mByteIndex + 1]  << 8 | d->mData[d->mByteIndex];
    d->mByteIndex += 4;
    return *this;
  }

  BitStream &BitStream::operator >> (s8 &s) {
    align();
    s = d->mData[d->mByteIndex];
    d->mByteIndex++;
    return *this;
  }

  BitStream &BitStream::operator >> (s16 &s) {
    align();
    s = d->mData[d->mByteIndex + 1] << 8 | d->mData[d->mByteIndex];
    d->mByteIndex += 2;
    return *this;
  }

  BitStream &BitStream::operator >> (s32 &s) {
    align();
    s = d->mData[d->mByteIndex + 3] << 24 | d->mData[d->mByteIndex + 2]  << 16 |
        d->mData[d->mByteIndex + 1]  << 8 | d->mData[d->mByteIndex];
    d->mByteIndex += 4;
    return *this;
  }

  BitStream &BitStream::operator >> (f16 &f) {
    align();
    s16 s = (d->mData[d->mByteIndex + 1] << 8 | d->mData[d->mByteIndex]);
    f = s * 0.00390625f;
    d->mByteIndex += 2;
    return *this;
  }

  BitStream &BitStream::operator >> (f32 &f) {
    align();
    // read the float
    f = d->mData[d->mByteIndex + 3] << 24 | d->mData[d->mByteIndex + 2] << 16 |
        d->mData[d->mByteIndex + 1] << 8 | d->mData[d->mByteIndex];
    // increase the byte index
    d->mByteIndex += 4;
    // return bitstream
    return *this;
  }

  BitStream &BitStream::operator >> (float &f) {
    align();
    // read the float
    f = d->mData[d->mByteIndex + 3] << 24 | d->mData[d->mByteIndex + 2] << 16 |
        d->mData[d->mByteIndex + 1] << 8 | d->mData[d->mByteIndex];
    // increase the byte index
    d->mByteIndex += 4;
    // return bitstream
    return *this;
  }

  BitStream &BitStream::operator >> (double &f) {
    align();
    // read bytes seperately
    union {
      double a;
      u8 b[8];
    } x;
    *this >> x.b[7] >> x.b[6] >> x.b[5] >> x.b[4] >> x.b[3] >> x.b[2] >> x.b[1] >> x.b[0];
    // assign double value
    f = x.a;
    // return bitstream
    return *this;
  }

  BitStream &BitStream::operator >> (u8* &s) {
    align();
    u32 i = 0;
    for (i = 0; d->mData[d->mByteIndex + i] != 0; ++i);
    s = new u8[i + 1];
    s[i] = 0;
    for (u32 j = 0; j < i; ++j) {
      s[j] = d->mData[d->mByteIndex + j];
    }
    d->mByteIndex = d->mByteIndex + i + 1;
    return *this;
  }

  BitStream &BitStream::operator >> (QBrush &b) {
    u8 type;
    u16 bitmapId;
    QMatrix matrix;
    (*this) >> type;
    switch (type) {
    case Swiff::SolidFill: {
        QColor color = b.color();
        (*this) >> color;
        b = QBrush(color);
      }
      break;
    case Swiff::LinearGradientFill: {
        // read brush matrix
        (*this) >> matrix;
        QLinearGradient gradient(-16384, 0, 16384, 0);
        // set spread mode, compatible with flash
        gradient.setSpread(QGradient::Spread(this->readUnsignedBits(2)));
        u8 mInterpolationMode = this->readUnsignedBits(2);
        Q_UNUSED(mInterpolationMode);
        u8 recordCount = this->readUnsignedBits(4);
        // parse gradient colors
        for (int i = 0; i < recordCount; ++i) {
          u8 ratio = 0;
          QColor color = b.color();
          (*this) >> ratio >> color;
          gradient.setColorAt(ratio / 255.0, color);
        }
        b = QBrush(gradient);
        b.setMatrix(matrix);
      }
      break;
    case Swiff::RadialGradientFill: {
        // read brush matrix
        (*this) >> matrix;
        QRadialGradient gradient(0, 0, 16384);
        // set spread mode, compatible with flash
        gradient.setSpread(QGradient::Spread(this->readUnsignedBits(2)));
        u8 mInterpolationMode = this->readUnsignedBits(2);
        Q_UNUSED(mInterpolationMode);
        u8 recordCount = this->readUnsignedBits(4);
        // parse gradient colors
        for (int i = 0; i < recordCount; ++i) {
          u8 ratio = 0;
          QColor color = b.color();
          (*this) >> ratio >> color;
          gradient.setColorAt(ratio / 255.0, color);
        }
        b = QBrush(gradient);
        b.setMatrix(matrix);
      }
      break;
    case Swiff::FocalGradientFill: {
        // read brush matrix
        (*this) >> matrix;
        QRadialGradient gradient(0, 0, 16384);
        // set spread mode, compatible with flash
        gradient.setSpread(QGradient::Spread(this->readUnsignedBits(2)));
        u8 mInterpolationMode = this->readUnsignedBits(2);
        Q_UNUSED(mInterpolationMode);
        u8 recordCount = this->readUnsignedBits(4);
        // parse gradient colors
        for (int i = 0; i < recordCount; ++i) {
          u8 ratio = 0;
          QColor color = b.color();
          (*this) >> ratio >> color;
          gradient.setColorAt(ratio / 255.0, color);
        }
        f16 mFocalPoint = 0;
        (*this) >> mFocalPoint;
        gradient.setFocalPoint(16384 + 16384 * mFocalPoint , 0);
        b = QBrush(gradient);
        b.setMatrix(matrix);
      }
      break;
    case Swiff::RepeatingBitmapFill:
    case Swiff::ClippedBitmapFill:
    case Swiff::NonSmoothedBitmapFill:
    case Swiff::NonSmoothedClippedBitmapFill: {
        (*this) >> bitmapId >> matrix;
        if (d->mDocument && d->mDocument->dictionary().contains(bitmapId))
          b = QBrush(static_cast<Bitmap *>(d->mDocument->dictionary()[bitmapId])->mImage);
        b.setMatrix(matrix);
      }
      break;
    }
    return *this;
  }

  BitStream &BitStream::operator >> (QColor &c) {
    if (c.isValid() && c.alpha() == 255) {
      u8 r, g, b;
      // rgb color
      (*this) >> r >> g >> b;
      c.setRgba(qRgba(r, g, b, 255));
    } else {
      u8 r, g, b, a;
      // default rgba color
      (*this) >> r >> g >> b >> a;
      c.setRgba(qRgba(r, g, b, a));
    }
    return *this;
  }

  BitStream &BitStream::operator >> (QMatrix &m) {
    // align to byte boundry
    this->align();
    // read the bits
    u8 numBits = 0;
    u8 hasScale = this->readUnsignedBits(1);
    float scaleX = 1;
    float scaleY = 1;
    if (hasScale) {
      numBits = this->readUnsignedBits(5);
      scaleX = this->readFixedBits(numBits);
      scaleY = this->readFixedBits(numBits);
    }
    u8 hasRotateSkew = this->readUnsignedBits(1);
    float rs0 = 0;
    float rs1 = 0;
    if (hasRotateSkew) {
      numBits = this->readUnsignedBits(5);
      rs0 = this->readFixedBits(numBits);
      rs1 = this->readFixedBits(numBits);
    }
    numBits = this->readUnsignedBits(5);
    float translateX = this->readSignedBits(numBits);
    float translateY = this->readSignedBits(numBits);
    m.setMatrix(scaleX, rs0, rs1, scaleY, translateX, translateY);
    // align to byte boundry
    this->align();
    return *this;
  }

  BitStream &BitStream::operator >> (QRect &s) {
    // align to byte boundry
    this->align();
    // read the bits
    u8 bitCount = 0;
    bitCount = this->readUnsignedBits(5);
    s.setLeft(this->readSignedBits(bitCount));
    s.setRight(this->readSignedBits(bitCount));
    s.setTop(this->readSignedBits(bitCount));
    s.setBottom(this->readSignedBits(bitCount));
    return *this;
  }

  BitStream &BitStream::operator >> (QString &str) {
    align();
    // create a string from current bytes
    str = QString((char *)(d->mData + d->mByteIndex));
    // update byte index
    d->mByteIndex += str.length() + 1;
    // return a reference to bit stream
    return *this;
  }
}
