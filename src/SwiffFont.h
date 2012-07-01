#ifndef SWIFFFONT_H
#define SWIFFFONT_H

#include "SwiffItem.h"

namespace Swiff {
  class BitStream;
  class FontPrivate;
  class Sprite;

  class Font : public Item {
  public:
    Font(Sprite *parent = 0);
    /** \copydoc Item::parse */
    void parse(BitStream &bitstream, int version);
    /** Parses the DefineFontInfo tag with \p version. */
    void parseInfo(BitStream &bitstream, int version);
    /** \copydoc Item::paint */
    void paint(QPainter &painter);
    /** \copydoc Item::bounds */
    QRect bounds() const;
    /** \copydoc Item::properties. */
    QVector<QPair<QString, QString> > properties() const;
    /** \returns name of the font. */
    QString name() const;
    /** Sets the name of the font. */
    void setName(QString name);
    /** \returns copyright of the font. */
    QString copyright() const;
    /** Sets the copyright of the font. */
    void setCopyright(QString copyright);
    /** \returns the language code for this font:
        1 = Latin (the western languages covered by Latin-1: English, French, German, and so on)
        2 = Japanese
        3 = Korean
        4 = Simplified Chinese
        5 = Traditional Chinese
   */
    int languageCode();
    /** \returns the code table for this font. */
    QString codeTable();
    /** \returns size of the EM-square. */
    int size();
    /** Paint the glyph at index \p index using \p brush. */
    void paintGlyph(QPainter &painter, QBrush &brush, u32 index);
  private:
    FontPrivate *d;
  };
}

#endif
