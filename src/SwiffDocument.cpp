#include "SwiffDocument.h"

#include "SwiffBitStream.h"
#include "SwiffSprite.h"

#include <QFile>
#include <QMap>
#include <QStack>
#include <QRect>
#include <QString>
#include <QStringList>

namespace Swiff {
  class DocumentPrivate {
  public:
    DocumentPrivate() : mSprite(0), mCompressed(false), mVersion(0), mFrameRate(25), mJPEGTables(0), mJPEGTablesSize(0) {
    }

    ~DocumentPrivate() {
      delete mJPEGTables;
      delete mSprite;
      qDeleteAll(mDictionary);
    }

    Sprite *mSprite;
    QMap<u16, Item *> mDictionary;
    bool mCompressed;
    u8 mVersion;
    QRect mFrameSize;
    f16 mFrameRate;
    u32 mFlags;
    QStringList mConstantPool;
    QMap<QString, Variable> mVariables;
    QStack<Variable> mStack;
    QString mTarget;
    QString mTrueValue;
    QString mFalseValue;
    u8 *mJPEGTables;
    u32 mJPEGTablesSize;
  };

  Document::Document(QString path) : d(0), mPath(path) {
    mPath = path;
  }

  Document::~Document() {
    close();
  }

  bool Document::open() {
    QFile file(mPath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly))
      return false;
    // create private members
    d = new DocumentPrivate();
    // read header
    QByteArray header = file.read(4);
    // check if the document is compressed
    d->mCompressed = (header.at(0) == 'C');
    // set version
    d->mVersion = header.at(3);
    // set true and false values
    d->mTrueValue = (d->mVersion <= 4) ? QString("1") : QString("true");
    d->mFalseValue = (d->mVersion <= 4) ? QString("0") : QString("false");
    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
    // read uncompressed file size
    u32 size;
    stream >> size;
    size -= 8;
    // read rest of the file
    u8 *data = new u8[size];
    stream.readRawData(reinterpret_cast<char *>(data), size);
    // close file
    file.close();
    // create bitstream
    BitStream bitstream(data, size, d->mCompressed);
    // set document
    bitstream.setDocument(this);
    // read frame size frame rate and flags
    bitstream >> d->mFrameSize >> d->mFrameRate;
    // FileAttributes tag
    if (d->mVersion >= 8) {
      // skip header
      bitstream.skip(2);
      // read in the flags
      bitstream >> d->mFlags;
    }
    // parse the root sprite
    d->mSprite = new Sprite(this);
    d->mSprite->parse(bitstream, 1);
    // success
    return true;
  }

  void Document::close() {
    delete d;
  }

  QString Document::path() const {
    return mPath;
  }

  u8 Document::version() const {
    return d->mVersion;
  }

  QRect Document::frameSize() const {
    return d->mFrameSize;
  }

  float Document::frameRate() const {
    return d->mFrameRate;
  }

  u8 Document::useDirectBlit() const {
    return d->mFlags & 0x40000000;
  }

  u8 Document::useGPU() const {
    return d->mFlags & 0x20000000;
  }

  u8 Document::hasMetaData() const {
    return d->mFlags & 0x10000000;
  }

  u8 Document::actionScript3() const {
    return d->mFlags & 0x08000000;
  }

  u8 Document::useNetwork() const {
    return d->mFlags & 0x01000000;
  }

  Sprite *Document::sprite() const {
    return d->mSprite;
  }

  QMap<u16, Item *> &Document::dictionary() const {
    return d->mDictionary;
  }

  QStringList &Document::constantPool() const {
    return d->mConstantPool;
  }

  QMap<QString, Variable> &Document::variables() const {
    return d->mVariables;
  }

  QStack<Variable> &Document::stack() const {
    return d->mStack;
  }

  QString &Document::target() const {
    return d->mTarget;
  }

  const QString &Document::trueValue() const {
    return d->mTrueValue;
  }

  const QString &Document::falseValue() const {
    return d->mFalseValue;
  }

  u8 *Document::JPEGTables() {
    return d->mJPEGTables;
  }

  u32 Document::JPEGTablesSize() {
    return d->mJPEGTablesSize;
  }

  void Document::setJPEGTables(u8 *jpegTables, u32 size) {
    // delete previous values
    delete d->mJPEGTables;
    // allocate space for the new value
    d->mJPEGTables = new u8[size];
    // copy the actual value
    memcpy(d->mJPEGTables, jpegTables, size);
    d->mJPEGTablesSize = size;
  }
}
