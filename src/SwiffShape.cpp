#include "SwiffShape.h"

#include "SwiffShapePrivate.h"

namespace Swiff {
  Shape::Shape(Sprite *parent) : Item(Swiff::ItemShape), d(new ShapePrivate(parent)) {
  }

  Shape::~Shape() {
    delete d;
  }

  void Shape::parse(BitStream &bitstream, int version) {
    bitstream >> mCharacterId >> d->mBounds;
    if (version == 4) {
      bitstream >> d->mEdgeBounds >> d->mFlags;
    }
    d->parseStyles(bitstream, version);
    d->parseShapeRecords(bitstream, version);
  }

  void Shape::paint(QPainter &painter) {
    for (int i = 0; i < d->mPaths.count(); ++i) {
      if (d->mBrushes.at(i))
        painter.fillPath(d->mPaths.at(i), *d->mBrushes.at(i));
      if (d->mPens.at(i))
        painter.strokePath(d->mPaths.at(i), *d->mPens.at(i));
    }
  }

  QRect Shape::bounds() const {
    return d->mBounds;
  }

  QVector<QPair<QString, QString> > Shape::properties() const {
    QVector<QPair<QString, QString> > p;

    p.append(QPair<QString, QString>(QString("Width"), QString::number(int(d->mBounds.width() * 0.05f))));
    p.append(QPair<QString, QString>(QString("Height"), QString::number(int(d->mBounds.height() * 0.05f))));

    return p;
  }

  u8 Shape::usesFillWindingRule() const {
    return d->mFlags & 0x04;
  }

  u8 Shape::usesNonScalingStrokes() const {
    return d->mFlags & 0x01;
  }

  u8 Shape::usesScalingStrokes() const {
    return d->mFlags & 0x01;
  }

  QPainterPath Shape::combined() const {
    QPainterPath path;
    for (u16 k = 0; k < d->mPaths.size(); ++k) {
      path.addPath(d->mPaths.at(k));
    }
    return path;
  }
}
