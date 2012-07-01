#ifndef SWIFFSOUND_H
#define SWIFFSOUND_H

#include "SwiffItem.h"

namespace Swiff {
  class SoundPrivate;

  class Sound : public Item {
  public:
    Sound();
    ~Sound();
    /** \copydoc Item::parse */
    void parse(BitStream &bitstream, int version);
    /** \copydoc Item::paint */
    void paint(QPainter &painter);
    /** \copydoc Item::bounds */
    QRect bounds() const;
    /** \copydoc Item::properties. */
    QVector<QPair<QString, QString> > properties() const;

    u8 format() const;
    u8 rate() const;
    u8 size() const;
    u8 type() const;
    u32 sampleCount() const;
    u8 *data() const;
    u32 dataSize() const;
    void setData(u8 *data, u32 size);
    void appendData(u8 *data, u32 size);
  private:
    SoundPrivate *d;
  };
}

#endif
