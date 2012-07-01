#ifndef SWIFFTEXT_H
#define SWIFFTEXT_H

#include "SwiffItem.h"

namespace Swiff {
  class BitStream;
  class Sprite;
  class TextPrivate;

  class Text : public Item {
  public:
    Text(Sprite *parent = 0);
    ~Text();
    /** \copydoc Item::parse */
    void parse(BitStream &bitstream, int version);
    /** \copydoc Item::paint */
    void paint(QPainter &painter);
    /** \copydoc Item::bounds */
    QRect bounds() const;
    /** \copydoc Item::properties. */
    QVector<QPair<QString, QString> > properties() const;
    /** \returns the content as string. */
    QString text() const;
  private:
    TextPrivate *d;
  };
}

#endif
