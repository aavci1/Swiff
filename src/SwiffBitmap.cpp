#include "SwiffBitmap.h"

#include <QDebug>

namespace Swiff {
  Bitmap::Bitmap() : Item(Swiff::ItemBitmap)  {
  }

  void Bitmap::paint(QPainter &painter) {
    painter.drawImage(0, 0, mImage);
  }

  QRect Bitmap::bounds() const {
    return mImage.rect();
  }

  QVector<QPair<QString, QString> > Bitmap::properties() const {
    QVector<QPair<QString, QString> > p;

    p.append(QPair<QString, QString>(QString("Width"), QString::number(mImage.width())));
    p.append(QPair<QString, QString>(QString("Height"), QString::number(mImage.height())));

    return p;
  }

  void removeWrongMarker(u8 *data, u32 &size) {
    if (size < 4)
      return;
    for (u32 i = 0; i < size - 4; ++i) {
      if ((data[i] == 0xFF) && (data[i + 1] == 0xD9) && (data[i + 2] == 0xFF) && (data[i + 3] == 0xD8)) {
        for (u32 j = i; j < size - 4; ++j)
          data[j] = data[j + 4];
        size = size - 4;
        break;
      }
    }
  }

  Bitmap *parseBitsJPEG(BitStream &bitstream, int version, u32 byteIndex, u32 length) {
    // create the bitmap
    Bitmap *bitmap = new Bitmap();
    // read the character id
    bitstream >> bitmap->mCharacterId;
    if (version == 2) {
      // read the image bytes
      u32 dataLength = length - 2;
      u8 *imageData = new u8[dataLength];
      bitstream.readBytes(imageData, dataLength);
      // clean erroneous markers
      removeWrongMarker(imageData, dataLength);
      // load the image from the data
      bitmap->mImage.loadFromData(imageData, dataLength);
      // clean up
      delete [] imageData;
    } else if (version == 3 || version == 4) {
      // read the image bytes
      u32 dataLength = 0;
      bitstream >> dataLength;
      if (version == 4) {
        qDebug() << "WARN: DefineBitsJPEG4 - Deblocking parameter skipped!";
        bitstream.skip(2);
      }
      u8 *imageData = new u8[dataLength];
      bitstream.readBytes(imageData, dataLength);
      // clean erroneous markers
      removeWrongMarker(imageData, dataLength);
      // load the image from the data
      bitmap->mImage.loadFromData(imageData, dataLength);
      // clean up
      delete [] imageData;
      // read the alpha mask if the format is JPEG and it is present
      if (bitstream.byteIndex() < byteIndex + length) {
        // create compressed alpha data bitstream
        u32 compressedLength = byteIndex + length - bitstream.byteIndex();
        u8 *compressedAlphaData = new u8[compressedLength];
        // read the compressed alpha data
        bitstream.readBytes(compressedAlphaData, compressedLength);
        // uncompress the alpha data
        QByteArray alphaData = BitStream::uncompress(compressedAlphaData, compressedLength);
        // clean up
        delete [] compressedAlphaData;
        // create the empty alpha channel image
        QImage alphaChannel(bitmap->mImage.width(), bitmap->mImage.height(), QImage::Format_Indexed8);
        // construct the alpha channel image
        for (u16 i = 0; i < bitmap->mImage.height(); ++i) {
          memcpy(alphaChannel.scanLine(i), alphaData.constData() + i * bitmap->mImage.width(), bitmap->mImage.width());
        }
        // set the alpha channel
        bitmap->mImage.setAlphaChannel(alphaChannel);
      }
    }

    return bitmap;
  }

  Bitmap *parseBitsLossless(BitStream &bitstream, int version, u32 length) {
    // create the bitmap
    Bitmap *bitmap = new Bitmap();
    u8 format = 0, colorTableSize = 0;
    u16 width = 0, height = 0;
    bitstream >> bitmap->mCharacterId >> format >> width >> height;
    u32 compressedSize = 0;
    if (format == 3) {
      bitstream >> colorTableSize;
      compressedSize = length - 8;
    } else {
      compressedSize = length - 7;
    }
    u8 *compressedData = new u8[compressedSize];
    // read the compressed data
    bitstream.readBytes(compressedData, compressedSize);
    // uncompress the data
    QByteArray data = BitStream::uncompress(compressedData, compressedSize);
    // clean up
    delete [] compressedData;
    // 8 bit colormapped
    switch (format) {
    case 3: {
        // 8-bit color-mapped
        // create the image
        bitmap->mImage = QImage(width, height, QImage::Format_Indexed8);
        // create the color table
        QVector<QRgb> colorTable;
        for (u16 i = 0; i < colorTableSize + 1; ++i) {
          if (version == 1)
            colorTable.append(qRgb(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]));
          else if (version == 2)
            colorTable.append(qRgba(data[i * 4], data[i * 4 + 1], data[i * 4 + 2], data[i * 4 + 3]));
        }
        // assign the color table
        bitmap->mImage.setColorTable(colorTable);
        // calculate stride
        int stride = width;
        if (width & 3)
          stride += 4 - (stride & 3);
        // copy index data
        for (int i = 0; i < height; ++i) {
          for (int j = 0; j < width; ++j) {
            if (version == 1)
              bitmap->mImage.scanLine(i)[j] = data[(colorTableSize + 1) * 3 + stride * i + j];
            else if (version == 2)
              bitmap->mImage.scanLine(i)[j] = data[(colorTableSize + 1) * 4 + stride * i + j];
          }
        }
      }
      break;
    case 4: {
        // 15-bit RGB
        // creat the image
        bitmap->mImage = QImage(width, height, QImage::Format_RGB555);
        // copy the image data
        memcpy(bitmap->mImage.scanLine(0), data.constData(), data.length());
      }
      break;
    case 5: {
        // 24-bit RGB or 32-bit ARGB
        // create the image
        bitmap->mImage = QImage(width, height, QImage::Format_ARGB32_Premultiplied);
        for (int i = 0; i < height; ++i) {
          u8 *src = (u8*)data.constData() + i * 4 * width;
          u8 *dest = bitmap->mImage.scanLine(i);
          for (int j = 0; j < width; ++j) {
            dest[j * 4 + 0] = src[j * 4 + 3];
            dest[j * 4 + 1] = src[j * 4 + 2];
            dest[j * 4 + 2] = src[j * 4 + 1];
            dest[j * 4 + 3] = src[j * 4 + 0];
          }
        }
      }
      break;
    }

    return bitmap;
  }
}
