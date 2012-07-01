#ifndef SWIFFEXPORTER_H
#define SWIFFEXPORTER_H

#include <SwiffTypes.h>

#include <QPixmap>

namespace Swiff {
  class Item;
  class Frame;
  class Sound;
  class Video;
};

class SwiffExporter
{
public:
  /** Converts \p item to a pixmap. Pixmap will be scaled to fit into \p maxSize. */
  static QPixmap toPixmap(Swiff::Item *item, QSize maxSize = QSize(0, 0));
  /** Converts \p item to a SVG string. */
  static QString toSVG(Swiff::Item *item, QSize maxSize = QSize(0, 0));
  /** Exports \p item into \p path in \p format. */
  static void exportItem(Swiff::Item *item, QString path, const char *format);
  /** Converts \p frame to a pixmap. Pixmap will be scaled to fit into \p maxSize. */
  static QPixmap toPixmap(Swiff::Frame *frame, QSize maxSize = QSize(0, 0));
  /** Exports \p frame into \p path in \p format. */
  static void exportFrame(Swiff::Frame *frame, QString path, const char *format);
  /** Exports \p sound into \p path in \p format. */
  static void exportSound(Swiff::Sound *sound, QString path, const char *format);
  /** Exports \p video into \p path in \p format. */
  static void exportVideo(Swiff::Video *video, QString path, const char *format);
private:
  SwiffExporter();
};

#endif // SWIFFEXPORTER_H
