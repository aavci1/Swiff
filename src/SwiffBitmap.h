#ifndef SWIFFBITMAP_H
#define SWIFFBITMAP_H

#include "SwiffItem.h"

#include <QImage>

namespace Swiff {
  class BitStream;

  class Bitmap : public Item {
  public:
    Bitmap();

    /** \copydoc Item::paint */
    void paint(QPainter &painter);
    /** \copydoc Item::bounds. */
    QRect bounds() const;
    /** \copydoc Item::properties. */
    QVector<QPair<QString, QString> > properties() const;
  public:
    QImage mImage;
  };

  void removeWrongMarker(u8 *data, u32 &size);

  Bitmap *parseBitsJPEG(BitStream &bitstream, int version, u32 byteIndex, u32 length);
  Bitmap *parseBitsLossless(BitStream &bitstream, int version, u32 length);
}

#endif
