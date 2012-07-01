#ifndef SWIFFVIDEO_H
#define SWIFFVIDEO_H

#include "SwiffItem.h"

namespace Swiff {
  class VideoPrivate;

  class Video : public Item {
  public:
    Video();
    ~Video();
    /** \copydoc Item::parse */
    void parse(BitStream &bitstream, int version);
    /** \copydoc Item::paint */
    void paint(QPainter &painter);
    /** \copydoc Item::bounds */
    QRect bounds() const;
    /** \copydoc Item::properties. */
    QVector<QPair<QString, QString> > properties() const;

    u8 codec() const;
    u16 width() const;
    u16 height() const;

    void appendData(u8 *data, u32 size);
    QVector<QPair<u8 *, u32> > frames();
  private:
    VideoPrivate *d;
  };
}

#endif
