#ifndef SWIFFSHAPE_H
#define SWIFFSHAPE_H

#include "SwiffItem.h"

namespace Swiff {
  class BitStream;
  class ShapePrivate;
  class Sprite;

  class Shape : public Item {
  public:
    Shape(Sprite *parent = 0);
    ~Shape();

    /** \copydoc Item::parse */
    void parse(BitStream &bitstream, int version);
    /** \copydoc Item::paint */
    void paint(QPainter &painter);
    /** \copydoc Item::bounds */
    QRect bounds() const;
    /** \copydoc Item::properties. */
    QVector<QPair<QString, QString> > properties() const;

    u8 usesFillWindingRule() const;
    u8 usesNonScalingStrokes() const;
    u8 usesScalingStrokes() const;

    QPainterPath combined() const;
  private:
    ShapePrivate *d;
  };
}

#endif
