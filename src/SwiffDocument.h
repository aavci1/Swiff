#ifndef SWIFFDOCUMENT_H
#define SWIFFDOCUMENT_H

#include "SwiffTypes.h"

#include <QStack>

class QRect;

namespace Swiff {
  class DocumentPrivate;
  class Item;
  class Sprite;

  class Document {
  public:
    Document(QString path);
    ~Document();
    /** Opens the document. */
    bool open();
    /** Closes the document. */
    void close();
    /** \returns Path of the document. */
    QString path() const;
    /** \returns Version of the document. */
    u8 version() const;
    /** \returns Frame size. */
    QRect frameSize() const;
    /** \returns Frame rate. */
    float frameRate() const;
    /** \returns true if hardware acceleration should be used. */
    u8 useDirectBlit() const;
    /** \returns true if hardware acceleration should be used. */
    u8 useGPU() const;
    /** \returns true if the file has meta data. */
    u8 hasMetaData() const;
    /** \returns true if the file uses ActionScript3. */
    u8 actionScript3() const;
    /** \returns true if the file uses network. */
    u8 useNetwork() const;
    /** \returns Root sprite. */
    Sprite *sprite() const;
    /** \returns Item dictionary. */
    QMap<u16, Item *> &dictionary() const;

    QMap<QString, Variable> &variables() const;
    QStringList &constantPool() const;
    QStack<Variable> &stack() const;
    QString &target() const;
    /** \returns string value to be used for boolean true depending on file version. */
    const QString &trueValue() const;
    /** \returns string value to be used for boolean false depending on file version. */
    const QString &falseValue() const;
    /** \returns the value defined by JPEGTables tag. */
    u8 *JPEGTables();
    /** \returns the size of the JPEGTables structure. */
    u32 JPEGTablesSize();
    /** Sets the value of the JPEGTables. */
    void setJPEGTables(u8 *jpegTables, u32 size);
  private:
    DocumentPrivate *d;
    QString mPath;
  };
}

#endif
