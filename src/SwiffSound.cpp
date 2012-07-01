#include "SwiffSound.h"

namespace Swiff {
  class SoundPrivate {
  public:
    SoundPrivate() : format(0), rate(0), size(0), type(0), sampleCount(0), data(0), dataSize(0) {

    }

    ~SoundPrivate() {
      delete data;
    }

    /* Sound formats:
      Uncompressed, native-endian       0
      ADPCM                             1
      MP3                               2
      Uncompressed, little-endian       3
      Nellymoser 16 kHz                 4
      Nellymoser 8 kHz                  5
      Nellymoser                        6
      Speex                             11
    */
    u8 format;
    u8 rate; // 0 = 5.5 kHz, 1 = 11 kHz, 2 = 22 kHz, 3 = 44 kHz
    u8 size; // 0 = 8 bit, 1 = 16 bit
    u8 type; // 0 = mono, 1 = stereo
    u32 sampleCount;
    u8 *data;
    u32 dataSize;
  };

  Sound::Sound() : Item(Swiff::ItemSound), d(new SoundPrivate()) {
  }

  Sound::~Sound() {

  }

  void Sound::parse(BitStream &bitstream, int /*version*/) {
    d->format = bitstream.readUnsignedBits(4);
    d->rate = bitstream.readUnsignedBits(2);
    d->size = bitstream.readUnsignedBits(1);
    d->type = bitstream.readUnsignedBits(1);
    bitstream >> d->sampleCount;
  }

  void Sound::paint(QPainter &painter) {
  }

  QRect Sound::bounds() const {
    return QRect();
  }

  QVector<QPair<QString, QString> > Sound::properties() const {
    QVector<QPair<QString, QString> > p;

    switch (d->format) {
    case 0:
    case 3:
      p.append(QPair<QString, QString>(QString("Format"), QString("WAV")));
      break;
    case 1:
      p.append(QPair<QString, QString>(QString("Format"), QString("ADPCM")));
      break;
    case 2:
      p.append(QPair<QString, QString>(QString("Format"), QString("MP3")));
      break;
    case 4:
    case 5:
    case 6:
      p.append(QPair<QString, QString>(QString("Format"), QString("Nellymoser")));
      break;
    case 11:
      p.append(QPair<QString, QString>(QString("Format"), QString("Speex")));
      break;
    }
    switch (d->rate) {
    case 0:
      p.append(QPair<QString, QString>(QString("Rate"), QString("5.5 kHZ")));
      break;
    case 1:
      p.append(QPair<QString, QString>(QString("Rate"), QString("11 kHZ")));
      break;
    case 2:
      p.append(QPair<QString, QString>(QString("Rate"), QString("22 kHZ")));
      break;
    case 4:
      p.append(QPair<QString, QString>(QString("Rate"), QString("44 kHZ")));
      break;
    }
    if (d->type == 0)
      p.append(QPair<QString, QString>(QString("Type"), QString("Mono")));
    else if (d->type == 1)
      p.append(QPair<QString, QString>(QString("Type"), QString("Stereo")));

    return p;
  }

  u8 Sound::format() const {
    return d->format;
  }

  u8 Sound::rate() const {
    return d->rate;
  }

  u8 Sound::size() const {
    return d->size;
  }

  u8 Sound::type() const {
    return d->type;
  }

  u32 Sound::sampleCount() const {
    return d->sampleCount;
  }

  u8 *Sound::data() const {
    return d->data;
  }

  u32 Sound::dataSize() const {
    return d->dataSize;
  }

  void Sound::setData(u8 *data, u32 size) {
    // delete old data
    delete d->data;
    // update data pointer and size
    d->data = data;
    d->dataSize = size;
  }

  void Sound::appendData(u8 *data, u32 size) {
    // create a temporary bitstream
    u8 *bitstream = new u8[d->dataSize + size];
    // copy data to the bitstream
    memcpy(bitstream, d->data, d->dataSize);
    memcpy(bitstream + d->dataSize, data, size);
    // update data
    setData(bitstream, d->dataSize + size);
  }
}
