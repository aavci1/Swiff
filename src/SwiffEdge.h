#ifndef SWIFFEDGE_H
#define SWIFFEDGE_H

#include "SwiffTypes.h"

#include <QPointF>

namespace Swiff {
  class Edge {
  public:
    static const u8 Straight = 0x00;
    static const u8 Curved = 0x01;

    Edge() : mType(Straight) {
    }

    u8 type() const {
      return mType;
    }

    virtual Edge *copy() const = 0;
    virtual Edge *reversedCopy() const = 0;
  public:
    u8 mType;
    QPointF mStartPoint;
    QPointF mEndPoint;
  };

  class StraightEdge : public Edge {
  public:

    StraightEdge() {
      mType = Straight;
    }

    Edge *copy() const;
    Edge *reversedCopy() const;
  };

  class CurvedEdge : public Edge {
  public:
    QPointF mControlPoint;

    CurvedEdge() {
      mType = Curved;
    }

    Edge *copy() const;
    Edge *reversedCopy() const;
  };
}

#endif
