#ifndef SWIFFBUFFER_H
#define SWIFFBUFFER_H

#include "SwiffTypes.h"

class QByteArray;
class QBrush;
class QColor;
class QMatrix;
class QRect;
class QString;

namespace Swiff {
  class BitStreamPrivate;
  class Document;

  class BitStream {
  public:
    BitStream(u8 *data, u32 size, bool compressed = false);
    BitStream(const QByteArray &byteArray, bool compressed = false);
    ~BitStream();

    static QByteArray uncompress(const u8* compressed, const u32 &compressedLength);
    /** \returns document */
    Document *document() const;
    /** Sets the document this bitstream is related to. */
    void setDocument(Document *document);
    /** \returns data. */
    u8 *data() const;
    /** \returns Current byte index. */
    u32 byteIndex() const;
    /** \returns Current bit index. */
    u8 bitIndex() const;
    /** Positions to the first bit of the given byte index if the index is within size. */
    void seek(const u32 byteIndex, const u8 bitIndex = 7);
    /** Skips \param byteCount bytes and \param bitCount bits. */
    void skip(const u32 byteCount, const u8 bitCount = 0);
    /** Aligns the cursor to the first bit of the next byte. If the cursor is already at the first bit, it doesn't move. */
    void align();
    /** \returns Current byte. */
    u8 currentu8();
    /** \returns Current word. */
    u16 currentu16();
    /** \returns Current double word. */
    u32 currentu32();
    /** Reads \param byteCount bytes into the given bitstream \param data. Space for \param data must be allocated. */
    void readBytes(u8* data, u32 byteCount);
    /** Reads \param bitCount unsigned bits and \returns the result. */
    u32 readUnsignedBits(u8 bitCount);
    /** Reads \param bitCount signed bits and \returns the result. */
    s32 readSignedBits(u8 bitCount);
    /** Reads \param bitCount fixed bits and \returns the result. */
    f32 readFixedBits(u8 bitCount);

    BitStream &operator >> (u8 &u);
    BitStream &operator >> (u16 &u);
    BitStream &operator >> (u32 &u);
    BitStream &operator >> (s8 &s);
    BitStream &operator >> (s16 &s);
    BitStream &operator >> (s32 &s);
    BitStream &operator >> (float &f);
    BitStream &operator >> (double &f);
    BitStream &operator >> (f16 &f);
    BitStream &operator >> (f32 &f);
    BitStream &operator >> (u8* &s);
    BitStream &operator >> (QBrush &b);
    BitStream &operator >> (QColor &c);
    BitStream &operator >> (QMatrix &m);
    BitStream &operator >> (QRect &s);
    BitStream &operator >> (QString &str);

  private:
    BitStreamPrivate *d;
  };
}

#endif
