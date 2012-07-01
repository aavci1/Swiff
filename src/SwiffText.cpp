#include "SwiffText.h"

#include "SwiffBitStream.h"
#include "SwiffDocument.h"
#include "SwiffFont.h"
#include "SwiffSprite.h"

#include <QMap>
#include <QMatrix>
#include <QPainter>
#include <QRect>

namespace Swiff {
  typedef QPair<u32, s32> GlyphPair;

  class TextRecord {
  public:
    TextRecord() : mColor(0, 0, 0, 255), mOffsetX(0), mOffsetY(0), mHeight(1024) {
    }

    u16 mFontId;
    QColor mColor;
    s16 mOffsetX;
    s16 mOffsetY;
    u16 mHeight;
    QVector<GlyphPair> mGlyphs;
  };

  class TextPrivate {
  public:
    TextPrivate(Sprite *parent) : mParent(parent) {
    }

    ~TextPrivate() {
      qDeleteAll(mTextRecords);
    }
    Sprite *mParent;
    QRect mBounds;
    QMatrix mMatrix;
    u8 mGlyphBits;
    u8 mAdvanceBits;
    QVector<TextRecord *> mTextRecords;
  };

  Text::Text(Sprite *parent) : Item(Swiff::ItemText), d(new TextPrivate(parent)) {
  }

  Text::~Text() {
    delete d;
  }

  void Text::parse(BitStream &bitstream, int version) {
    bitstream >> mCharacterId >> d->mBounds >> d->mMatrix >> d->mGlyphBits >> d->mAdvanceBits;
    // parse text records
    while (bitstream.currentu8()) {
      TextRecord *textRecord = new TextRecord();
      bitstream.skip(0, 4);
      u8 hasFont = bitstream.readUnsignedBits(1);
      u8 hasColor = bitstream.readUnsignedBits(1);
      u8 hasYOffset = bitstream.readUnsignedBits(1);
      u8 hasXOffset = bitstream.readUnsignedBits(1);
      if (hasFont)
        bitstream >> textRecord->mFontId;
      if (hasColor) {
        if (version == 2)
          textRecord->mColor.setAlpha(0);
        bitstream >> textRecord->mColor;
      }
      if (hasXOffset)
        bitstream >> textRecord->mOffsetX;
      if (hasYOffset)
        bitstream >> textRecord->mOffsetY;
      if (hasFont)
        bitstream >> textRecord->mHeight;
      u8 glyphCount = bitstream.readUnsignedBits(8);
      // read glyphs
      for (int i = 0; i < glyphCount; ++i) {
        u32 glyphIndex = bitstream.readUnsignedBits(d->mGlyphBits);
        s32 glyphAdvance = bitstream.readSignedBits(d->mAdvanceBits);
        // append glyph to the list
        textRecord->mGlyphs.append(GlyphPair(glyphIndex, glyphAdvance));
      }
      // append text record to the list
      d->mTextRecords.append(textRecord);
      // align to next byte boundry
      bitstream.align();
    }
    bitstream.skip(1, 0);
  }

  void Text::paint(QPainter &painter) {
    // set text matrix
    painter.setMatrix(d->mMatrix, true);
    foreach (const TextRecord *textRecord, d->mTextRecords) {
      // get font
      Font *font = static_cast<Font *>(d->mParent->document()->dictionary()[textRecord->mFontId]);
      if (font == 0)
        continue;
      // create brush
      QBrush brush(textRecord->mColor);
      // apply offset
      painter.translate(textRecord->mOffsetX, textRecord->mOffsetY);
      painter.save();
      // calculate scale for text height
      float scale = (float)(textRecord->mHeight) / font->size();
      // apply text height
      painter.scale(scale, scale);
      foreach (GlyphPair glyph, textRecord->mGlyphs) {
        // paint the glyph
        font->paintGlyph(painter, brush, glyph.first);
        // advance to the character position
        painter.translate(glyph.second / scale, 0);
      }
      // revert to original scale
      painter.scale(1.0f / scale, 1.0f / scale);
      painter.restore();
    }
  }

  QRect Text::bounds() const {
    return d->mBounds;
  }

  QVector<QPair<QString, QString> > Text::properties() const {
    QVector<QPair<QString, QString> > p;

    p.append(QPair<QString, QString>(QString("Width"), QString::number(int(d->mBounds.width() * 0.05f))));
    p.append(QPair<QString, QString>(QString("Height"), QString::number(int(d->mBounds.height() * 0.05f))));
    p.append(QPair<QString, QString>(QString("Text"), text()));

    return p;
  }

  QString Text::text() const {
    QString s;
    foreach (const TextRecord *textRecord, d->mTextRecords) {
      if (!d->mParent->document()->dictionary().contains(textRecord->mFontId))
        continue;
      Font *font = static_cast<Font *>(d->mParent->document()->dictionary()[textRecord->mFontId]);
      if (!font)
        continue;
      foreach (GlyphPair glyph, textRecord->mGlyphs)
        if (font->codeTable().length() > glyph.first)
          s.append(font->codeTable().at(glyph.first));
    }

    return s;
  }
}
