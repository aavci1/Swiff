#include "SwiffFont.h"

#include "SwiffBitStream.h"
#include "SwiffSprite.h"

#include "SwiffShapePrivate.h"

#include <QDebug>
#include <QPainter>

namespace Swiff {
  class FontPrivate {
  public:
    FontPrivate() : mFlags(0), mLanguageCode(0), mSize(1024), isBold(false), isItalic(false) {
    }

    QVector<QPainterPath> mGlyphs;
    QString mName;
    QString mCopyright;
    QString mCodeTable;
    u8 mFlags;
    u8 mLanguageCode;
    int mSize;
    bool isBold;
    bool isItalic;
  };

  Font::Font(Sprite *parent) : Item(Swiff::ItemFont), d(new FontPrivate()) {
  }

  void Font::parse(BitStream &bitstream, int version) {
    bitstream >> mCharacterId;
    if (version == 1) {
      // find out glyph count
      u16 glyphCount = bitstream.currentu16() / 2;
      // save start of the offset table
      u32 offsetTableStart = bitstream.byteIndex();
      // read offset table entries
      u16 *offsetTable = new u16[glyphCount];
      for (int i = 0; i < glyphCount; ++i)
        bitstream >> offsetTable[i];
      for (int i = 0; i < glyphCount; ++i) {
        bitstream.seek(offsetTableStart + offsetTable[i]);
        // parse fill bits and line bits
        u8 fillBits = bitstream.readUnsignedBits(4);
        u8 lineBits = bitstream.readUnsignedBits(4);
        // create glyph object
        d->mGlyphs << parseGlyph(bitstream, fillBits, lineBits);
      }
      // clean up
      delete offsetTable;
    } else if (version == 2 || version == 3) {
      bitstream >> d->mFlags >> d->mLanguageCode;
      // set font type
      d->isBold = d->mFlags & 0x01;
      d->isItalic = d->mFlags & 0x02;
      // read length of the font name
      u8 fontNameLength = 0;
      bitstream >> fontNameLength;
      // read the font name
      u8 *fontName = new u8[fontNameLength + 1];
      fontName[fontNameLength] = 0;
      bitstream.readBytes(fontName, fontNameLength);
      // set the font name
      d->mName = QString((char *)(fontName));
      // delete temporary var
      delete fontName;
      // read number of glyphs
      u16 glyphCount = 0;
      bitstream >> glyphCount;
      if (d->mFlags & 0x08) {
        // save start of the offset table
        u32 offsetTableStart = bitstream.byteIndex();
        // read offset table entries
        u32 *offsetTable = new u32[glyphCount];
        for (int i = 0; i < glyphCount; ++i)
          bitstream >> offsetTable[i];
        u32 codeTableOffset = 0;
        bitstream >> codeTableOffset;
        for (int i = 0; i < glyphCount; ++i) {
          bitstream.seek(offsetTableStart + offsetTable[i]);
          // parse fill bits and line bits
          u8 fillBits = bitstream.readUnsignedBits(4);
          u8 lineBits = bitstream.readUnsignedBits(4);
          // create glyph object
          d->mGlyphs << parseGlyph(bitstream, fillBits, lineBits);
        }
        // font has wide
        if (d->mFlags & 0x04) {
          u16 code;
          for (int i = 0; i < d->mGlyphs.size(); ++i) {
            bitstream >> code;
            d->mCodeTable = d->mCodeTable.append(QChar(code));
          }
        } else {
          u8 code;
          for (int i = 0; i < d->mGlyphs.size(); ++i) {
            bitstream >> code;
            d->mCodeTable = d->mCodeTable.append(QChar(code));
          }
        }
      } else {
        // save start of the offset table
        u32 offsetTableStart = bitstream.byteIndex();
        // read offset table entries
        u16 *offsetTable = new u16[glyphCount];
        for (int i = 0; i < glyphCount; ++i)
          bitstream >> offsetTable[i];
        u16 codeTableOffset = 0;
        bitstream >> codeTableOffset;
        for (int i = 0; i < glyphCount; ++i) {
          bitstream.seek(offsetTableStart + offsetTable[i]);
          // parse fill bits and line bits
          u8 fillBits = bitstream.readUnsignedBits(4);
          u8 lineBits = bitstream.readUnsignedBits(4);
          // create glyph object
          d->mGlyphs << parseGlyph(bitstream, fillBits, lineBits);
        }
        // font has wide
        if (d->mFlags & 0x04) {
          u16 code;
          for (int i = 0; i < d->mGlyphs.size(); ++i) {
            bitstream >> code;
            d->mCodeTable = d->mCodeTable.append(QChar(code));
          }
        } else {
          u8 code;
          for (int i = 0; i < d->mGlyphs.size(); ++i) {
            bitstream >> code;
            d->mCodeTable = d->mCodeTable.append(QChar(code));
          }
        }
      }
      qDebug() << "WARN: parse remaining of the DefineFont2";
      if (version == 3)
        d->mSize = 20480;
    } else if (version == 4) {
      bitstream >> d->mFlags;
      // set bold and italic settings
      d->isBold = d->mFlags & 0x01;
      d->isItalic = d->mFlags & 0x02;
      bitstream >> d->mName;
      // if font has font data
      if (d->mFlags & 0x04) {
        // TODO: load font data
      }
      qDebug() << "WARN: Implement DefineFont4";
    }
  }

  void Font::parseInfo(BitStream &bitstream, int version) {
    // read length of the font name
    u8 fontNameLength = 0;
    bitstream >> fontNameLength;
    // read the font name
    u8 *fontName = new u8[fontNameLength + 1];
    fontName[fontNameLength] = 0;
    bitstream.readBytes(fontName, fontNameLength);
    // set the font name
    d->mName = QString((char *)(fontName));
    // delete temporary var
    delete fontName;
    // read font flags
    bitstream >> d->mFlags;
    if (version == 2) {
      bitstream >> d->mLanguageCode;
    }
    // set bold and italic settings
    d->isBold = d->mFlags & 0x02;
    d->isItalic = d->mFlags & 0x04;
    // FontFlagsWideCodes
    if (d->mFlags & 0x01) {
      u16 code;
      for (int i = 0; i < d->mGlyphs.size(); ++i) {
        bitstream >> code;
        d->mCodeTable = d->mCodeTable.append(QChar(code));
      }
    } else {
      u8 code;
      for (int i = 0; i < d->mGlyphs.size(); ++i) {
        bitstream >> code;
        d->mCodeTable = d->mCodeTable.append(QChar(code));
      }
    }
  }

  void Font::paint(QPainter &painter) {
    QBrush brush(QColor(0, 0, 0, 255));
    // paint all glyphs with blak color
    for (int i = 0; i < d->mGlyphs.size(); ++i) {
      painter.fillPath(d->mGlyphs.at(i), brush);
      painter.translate(d->mGlyphs.at(i).boundingRect().right(), 0);
    }
  }

  QRect Font::bounds() const {
    float top = 0, bottom = 0, width = 0;
    // put all glyphs side by side
    for (int i = 0; i < d->mGlyphs.size(); ++i) {
      QRectF glyphBounds = d->mGlyphs.at(i).boundingRect();
      if (glyphBounds.top() < top)
        top = glyphBounds.top();
      if (glyphBounds.bottom() > bottom)
        bottom = glyphBounds.bottom();
      width += glyphBounds.left() + glyphBounds.width();
    }
    // add two pixels at the end
    width += 40;
    // return total bounds
    return QRect(0, top, width, bottom - top);
  }

  QVector<QPair<QString, QString> > Font::properties() const {
    QVector<QPair<QString, QString> > p;

    p.append(QPair<QString, QString>(QString("Width"), QString::number(int(bounds().width() * 0.05f))));
    p.append(QPair<QString, QString>(QString("Height"), QString::number(int(bounds().height() * 0.05f))));
    if (d->mCopyright.length() != 0)
      p.append(QPair<QString, QString>(QString("Copyright"), d->mCopyright));
    p.append(QPair<QString, QString>(QString("CodeTable"), d->mCodeTable));

    return p;
  }

  QString Font::name() const {
    if (d->mName.length() == 0)
      return d->mName;
    // create name string
    QString name = d->mName;
    // font is bold
    if (d->isBold)
      name = QString("%1 Bold").arg(name);
    // font is italic
    if (d->isItalic)
      name = QString("%1 Italic").arg(name);
    // return name of the font
    return name;
  }

  void Font::setName(QString name) {
    d->mName = name;
  }

  QString Font::copyright() const {
    return d->mCopyright;
  }

  void Font::setCopyright(QString copyright) {
    d->mCopyright = copyright;
  }

  int Font::languageCode() {
    return d->mLanguageCode;
  }

  QString Font::codeTable() {
    return d->mCodeTable;
  }

  int Font::size() {
    return d->mSize;
  }

  void Font::paintGlyph(QPainter &painter, QBrush &brush, u32 index) {
    painter.fillPath(d->mGlyphs.at(index), brush);
  }
}
