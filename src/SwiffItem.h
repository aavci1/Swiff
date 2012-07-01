#ifndef ITEM_H
#define ITEM_H

#include "SwiffBitStream.h"

#include <QPainter>

namespace Swiff {
  enum ItemType {
    ItemUnknown = 0x00,
    ItemShape = 0x01,
    ItemBitmap = 0x02,
    ItemFont = 0x04,
    ItemSound = 0x08,
    ItemButton = 0x10,
    ItemSprite = 0x20,
    ItemText = 0x40,
    ItemVideo = 0x80 // custom item
  };

  class Item {
  public:
    Item(ItemType itemType = ItemUnknown) : mItemType(itemType), mCharacterId(0) {
    }
    virtual ~Item() {}
    /** Parses the item using \param bitstream. */
    virtual void parse(BitStream &bitstream, int version) {}
    /** Paints the item using the \param painter. */
    virtual void paint(QPainter &painter) = 0;
    /** \returns bounds of the item. */
    virtual QRect bounds() const = 0;
    /** \returns properties of the item as text. */
    virtual QVector<QPair<QString, QString> > properties() const = 0;
  public:
    ItemType mItemType;
    u16 mCharacterId;
  };
}

#endif
