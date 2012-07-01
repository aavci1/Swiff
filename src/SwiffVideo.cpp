#include "SwiffVideo.h"

namespace Swiff {
  class VideoPrivate {
  public:
    VideoPrivate() : frameCount(0), width(0), height(0), flags(0), codec(0) {

    }

    ~VideoPrivate() {
      for (int i = 0; i < frames.size(); ++i)
        delete frames.at(i).first;
    }

    u16 frameCount;
    u16 width;
    u16 height;
    u8 flags;
    /* Video formats:
       Sorenson H.263     2
       Screen Video       3
       VP6                4
       VP6 with alpha     5
       Screen Video v2    6
    */
    u8 codec;
    QVector<QPair<u8 *, u32> > frames;
  };

  Video::Video() : Item(Swiff::ItemVideo), d(new VideoPrivate()) {
  }

  Video::~Video() {
    delete d;
  }

  void Video::parse(BitStream &bitstream, int /*version*/) {
    bitstream >> d->frameCount >> d->width >> d->height >> d->flags >> d->codec;
  }

  void Video::paint(QPainter &painter) {
  }

  QRect Video::bounds() const {
    return QRect();
  }

  QVector<QPair<QString, QString> > Video::properties() const {
    QVector<QPair<QString, QString> > p;

    p.append(QPair<QString, QString>(QString("Width"), QString::number(d->width)));
    p.append(QPair<QString, QString>(QString("Height"), QString::number(d->height)));
    p.append(QPair<QString, QString>(QString("Frame Count"), QString::number(d->frameCount)));
    switch (d->codec) {
    case 2:
      p.append(QPair<QString, QString>(QString("Format"), QString("Sorenson H.263")));
      break;
    case 3:
      p.append(QPair<QString, QString>(QString("Format"), QString("Screen Video")));
      break;
    case 4:
      p.append(QPair<QString, QString>(QString("Format"), QString("VP6")));
      break;
    case 5:
      p.append(QPair<QString, QString>(QString("Format"), QString("VP6 with Alpha")));
      break;
    case 6:
      p.append(QPair<QString, QString>(QString("Format"), QString("Screen Video v2")));
      break;
    }

    return p;
  }

  u8 Video::codec() const {
    return d->codec;
  }

  u16 Video::width() const {
    return d->width;
  }

  u16 Video::height() const {
    return d->height;
  }

  void Video::appendData(u8 *data, u32 size) {
    d->frames.append(QPair<u8 *, u32>(data, size));
  }

  QVector<QPair<u8 *, u32> > Video::frames() {
    return d->frames;
  }
}
