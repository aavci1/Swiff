#ifndef SWIFFBUTTON_H
#define SWIFFBUTTON_H

#include "SwiffItem.h"

namespace Swiff {
  class BitStream;
  class ButtonPrivate;
  class Sprite;

  class Button : public Item {
  public:
    Button(Sprite *parent = 0);
    ~Button();

    /** \copydoc Item::parse */
    void parse(BitStream &bitstream, int version);
    /** \copydoc Item::paint */
    void paint(QPainter &painter);
    /** \copydoc Item::bounds */
    QRect bounds() const;
    /** \copydoc Item::properties. */
    QVector<QPair<QString, QString> > properties() const;

    ButtonState state() const;
    void setState(const ButtonState state);

    void processTransition(ButtonStateTransition transition);
    bool contains(const QMatrix &matrix, const QPointF &point) const;

    Button *clone();
  private:
    ButtonPrivate *d;
  };
}

#endif
