#ifndef _SWIFFSHAPEPRIVATE_H
#define _SWIFFSHAPEPRIVATE_H

#include "SwiffBitmap.h"
#include "SwiffDocument.h"
#include "SwiffEdge.h"
#include "SwiffLineStyle.h"
#include "SwiffSprite.h"

#include <QBrush>
#include <QColor>
#include <QVector>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QRect>
#include <QSharedPointer>

namespace Swiff {
  QPainterPath parseGlyph(BitStream &bitstream, u8 fillBits, u8 lineBits);

  class SubShape {
  public:

    SubShape(QBrush brush) : mBrush(brush), mFilled(true) {}
    SubShape(QPen pen) : mPen(pen), mFilled(false) {}

    bool isFilled() {
      return mFilled;
    }
  public:
    QVector<QSharedPointer<Edge> > mEdges;
    QBrush mBrush;
    QPen mPen;
    bool mFilled;
  };

  class ShapePrivate {
  public:
    ShapePrivate(Sprite *parent) : mParent(parent), mFlags(0), mFillBits(0), mLineBits(0) {
    }

    void parseStyles(BitStream &bitstream, int version);
    void parseShapeRecords(BitStream &bitstream, int version);
    QPen *toPen(LineStyle *lineStyle);

    Sprite *mParent;
    QVector<QPainterPath> mPaths;
    QVector<QSharedPointer<QBrush> > mBrushes;
    QVector<QSharedPointer<QPen> > mPens;
    QRect mBounds;
    QRect mEdgeBounds;
    u8 mFlags;
    u8 mFillBits;
    u8 mLineBits;
  };
}

#endif
