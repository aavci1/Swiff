#include "SwiffExporter.h"

#include <SwiffDocument.h>
#include <SwiffItem.h>
#include <SwiffFrame.h>
#include <SwiffSound.h>
#include <SwiffSprite.h>
#include <SwiffText.h>
#include <SwiffVideo.h>

#include <QByteArray>
#include <QBuffer>
#include <QDataStream>
#include <QFile>
#include <QSvgGenerator>
#include <QTextStream>
#include <QTime>

SwiffExporter::SwiffExporter() {
}

QPixmap SwiffExporter::toPixmap(Swiff::Item *item, QSize maxSize) {
  // calculate bounds
  float top = item->bounds().top();
  float left = item->bounds().left();
  float width = item->bounds().width();
  float height = item->bounds().height();
  // set maximum set when necessary
  if (item->mItemType == Swiff::ItemShape || item->mItemType == Swiff::ItemButton ||
      item->mItemType == Swiff::ItemFont || item->mItemType == Swiff::ItemText) {
    if (maxSize.width() == 0 || maxSize.height() == 0)
      maxSize = QSize(int(width * 0.05f), int(height * 0.05f));
  }
  // if width or height is zero return an empty pixmap
  if (width == 0 || height == 0)
    return QPixmap();
  // scale values
  float xscale = 1.0f, yscale = 1.0f, scale = 1.0f;
  // calculate x scale
  if (maxSize.width() != 0 && width > maxSize.width())
    xscale = maxSize.width() / width;
  // calculate y scale
  if (maxSize.height() != 0 && height > maxSize.height())
    yscale = maxSize.height() / height;
  // calculate scale
  scale = qMin(xscale, yscale);
  // the pixmap to be rendered into
  QPixmap pixmap(width * scale, height * scale);
  pixmap.fill(Qt::transparent);
  // create painter
  QPainter painter(&pixmap);
  // scale the painter as needed
  painter.scale(scale, scale);
  // translate the painter as needed
  painter.translate(-left, -top);
  // enable anti-aliasing
  painter.setRenderHint(QPainter::Antialiasing);
  // do the actual painting
  item->paint(painter);
  // return the pixmap
  return pixmap;
}

QString SwiffExporter::toSVG(Swiff::Item *item, QSize maxSize) {
  QSvgGenerator generator;
  // render into a byte array
  QByteArray byteArray;
  // create a buffer on byte array
  QBuffer buffer(&byteArray);
  // open buffer for writing
  buffer.open(QIODevice::WriteOnly);
  // set output device
  generator.setOutputDevice(&buffer);
  // calculate bounds
  float top = item->bounds().top();
  float left = item->bounds().left();
  float width = item->bounds().width();
  float height = item->bounds().height();
  // set maximum set when necessary
  if (item->mItemType == Swiff::ItemShape || item->mItemType == Swiff::ItemButton ||
      item->mItemType == Swiff::ItemFont || item->mItemType == Swiff::ItemText) {
    if (maxSize.width() == 0 || maxSize.height() == 0)
      maxSize = QSize(int(width * 0.05f), int(height * 0.05f));
  }
  // if width or height is zero return an empty string
  if (width == 0 || height == 0)
    return QString();
  // scale values
  float xscale = 1.0f, yscale = 1.0f, scale = 1.0f;
  // calculate x scale
  if (maxSize.width() != 0 && width > maxSize.width())
    xscale = maxSize.width() / width;
  // calculate y scale
  if (maxSize.height() != 0 && height > maxSize.height())
    yscale = maxSize.height() / height;
  // calculate scale
  scale = qMin(xscale, yscale);
  // set svg size
  generator.setSize(QSize(width * scale, height * scale));
  // set svg viewbox size
  generator.setViewBox(QRect(0, 0, width * scale, height * scale));
  // create painter
  QPainter painter(&generator);
  // scale the painter as needed
  painter.scale(scale, scale);
  // translate the painter as needed
  painter.translate(-left, -top);
  // enable anti-aliasing
  painter.setRenderHint(QPainter::Antialiasing);
  // do the actual painting
  item->paint(painter);
  // end painting
  painter.end();
  // close the buffer
  buffer.close();
  // return result string
  return QString::fromUtf8(byteArray.constData());
}

void SwiffExporter::exportItem(Swiff::Item *item, QString path, const char *format) {
  if (strcmp(format, "TXT") == 0) {
    // open the file
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
      return;
    QTextStream out(&file);
    // save content to the file
    out << static_cast<Swiff::Text *>(item)->text();
    // close the file
    file.close();
  } else if (strcmp(format, "SVG") == 0) {
    // convert shape to svg string
    QString svgString = toSVG(item);
    // open the file
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
      return;
    QTextStream out(&file);
    // save content to the file
    out << svgString;
    // close the file
    file.close();
  } else {
    // convert bitmap to pixmap
    QPixmap pixmap = toPixmap(item);
    // save pixmap to disk
    pixmap.save(path, format);
  }
}

QPixmap SwiffExporter::toPixmap(Swiff::Frame *frame, QSize maxSize) {
  // calculate bounds
  float top = frame->parent()->document()->frameSize().top();
  float left = frame->parent()->document()->frameSize().left();
  float width = frame->parent()->document()->frameSize().width();
  float height = frame->parent()->document()->frameSize().height();
  // set maximum set when necessary
  if (maxSize.width() == 0 || maxSize.height() == 0)
    maxSize = QSize(int(width * 0.05f), int(height * 0.05f));
  // if width or height is zero return an empty pixmap
  if (width == 0 || height == 0)
    return QPixmap();
  // scale values
  float xscale = 1.0f, yscale = 1.0f, scale = 1.0f;
  // calculate x scale
  if (maxSize.width() != 0 && width > maxSize.width())
    xscale = maxSize.width() / width;
  // calculate y scale
  if (maxSize.height() != 0 && height > maxSize.height())
    yscale = maxSize.height() / height;
  // calculate scale
  scale = qMin(xscale, yscale);
  // the pixmap to be rendered into
  QPixmap pixmap(width * scale, height * scale);
  pixmap.fill(Qt::transparent);
  // create painter
  QPainter painter(&pixmap);
  // scale the painter as needed
  painter.scale(scale, scale);
  // translate the painter as needed
  painter.translate(-left, -top);
  // enable anti-aliasing
  painter.setRenderHint(QPainter::Antialiasing);
  // set parent sprite's frame index
  frame->parent()->gotoFramePtr(frame);
  // do the actual painting
  frame->parent()->paint(painter);
  // return the pixmap
  return pixmap;
}

void SwiffExporter::exportFrame(Swiff::Frame *frame, QString path, const char *format) {
  // convert frame to pixmap
  QPixmap pixmap = toPixmap(frame);
  // save pixmap to disk
  pixmap.save(path, format);
}

void SwiffExporter::exportSound(Swiff::Sound *sound, QString path, const char *format) {
  if (strcmp(format, "MP3") == 0 && sound->format() == 2) {
    // open the file
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
      return;
    file.write((char *)(sound->data()), sound->dataSize());
    // close the file
    file.close();
  } else if (strcmp(format, "WAV") == 0 && sound->format() == 0) {
    // open the file
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
      return;
    file.write((char *)(sound->data()), sound->dataSize());
    // close the file
    file.close();
  }
}

void SwiffExporter::exportVideo(Swiff::Video *video, QString path, const char *format) {
  if (strcmp(format, "FLV") == 0) {
    // open the file
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
      return;
    // create stream
    QDataStream stream(&file);
    // set byte order to big endian
    stream.setByteOrder(QDataStream::BigEndian);
    // file signature
    stream << u8('F') << u8('L') << u8('V');
    // file version
    stream << u8(0x01);
    // flags 5 reserved, 1 audio, 1 reserved, 1 video
    stream << u8(0x01);
    // header size
    stream << u32(0x00000009);
    // first previous tag size, always zero
    stream << u32(0x00000000);
    // onMetaData tag
    // TODO: calculate duration and frame rate
    double framerate = 10; //document()->framesPerSecond();
    double videodatarate = 195;
    double duration = video->frames().size() / framerate;
    // onMetaData tag type
    stream << u8(0x12);
    // data size, u24
    stream << u8(0x00) << u8(0x00) << u8(0x72);
    // timestamp, u24
    stream << u8(0x00) << u8(0x00) << u8(0x00);
    // timestamp extended, u8
    stream << u8(0x00);
    // stream id, u24
    stream << u8(0x00) << u8(0x00) << u8(0x00);
    // object type, u8
    stream << u8(0x02);
    // onMetada string
    stream << u16(0x000A) << u8('o') << u8('n') << u8('M') << u8('e') << u8('t') << u8('a') << u8('D') << u8('a') << u8('t') << u8('a');
    // value type, u8 -- array : 0x08
    stream << u8(0x08);
    // array length, u32
    stream << u32(0x00000004); // --> 18
    // duration string
    stream << u16(0x0008) << u8('d') << u8('u') << u8('r') << u8('a') << u8('t') << u8('i') << u8('o') << u8('n');
    // duration value
    stream << u8(0x00) << duration; // --> 19
    // width string
    stream << u16(0x0005) << u8('w') << u8('i') << u8('d') << u8('t') << u8('h');
    // width value
    stream << u8(0x00) << double(video->width()); // --> 16
    // height string
    stream << u16(0x0006) << u8('h') << u8('e') << u8('i') << u8('g') << u8('h') << u8('t');
    // height value
    stream << u8(0x00) << double(video->height()); // -> 17
    // video data rate string
    stream << u16(0x000D) << u8('v') << u8('i') << u8('d') << u8('e') << u8('o') << u8('d') << u8('a') << u8('t') << u8('a') << u8('r') << u8('a') << u8('t') << u8('e');
    // videodatarate value
    stream << u8(0x00) << videodatarate; // -> 24
    // frame rate string
    stream << u16(0x0009) << u8('f') << u8('r') << u8('a') << u8('m') << u8('e') << u8('r') << u8('a') << u8('t') << u8('e');
    // framerate value
    stream << u8(0x00) << framerate; // -> 20
    // write previous tag size
    stream << u32(0x0000007D);
    // write video tags
    u32 timestamp = 0;
    u8 timestampExtended = 0;
    for (int i = 0; i < video->frames().size(); ++i) {
      // tag type
      stream << u8(0x09);
      // calculate data size
      u32 datasize = video->frames().at(i).second;
      // write timestamp
      stream << u8(datasize >> 16) << u8(datasize >> 8) << u8(datasize);
      // write timestamp
      stream << u8(timestamp >> 16) << u8(timestamp >> 8) << u8(timestamp) << timestampExtended;
      // stream id, u24
      stream << u8(0x00) << u8(0x00) << u8(0x00);
      // TODO: prepend flags to codecs
      stream << u8(video->codec() | 0x20);
      // write video data
      stream.writeRawData((char *)(video->frames().at(i).first), video->frames().at(i).second);
      // update previous tag size
      stream << u32(video->frames().at(i).second + 11);
      // calculate timestamp
      timestamp += 1000.0f / framerate;
    }
    // close the file
    file.close();
  }
}
