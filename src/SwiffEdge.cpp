#include "SwiffEdge.h"

namespace Swiff {
  Edge *StraightEdge::copy() const {
    StraightEdge *edge = new StraightEdge();
    edge->mStartPoint.setX(mStartPoint.x());
    edge->mStartPoint.setY(mStartPoint.y());
    edge->mEndPoint.setX(mEndPoint.x());
    edge->mEndPoint.setY(mEndPoint.y());
    return edge;
  }

  Edge *StraightEdge::reversedCopy() const {
    StraightEdge *edge = new StraightEdge();
    edge->mStartPoint.setX(mEndPoint.x());
    edge->mStartPoint.setY(mEndPoint.y());
    edge->mEndPoint.setX(mStartPoint.x());
    edge->mEndPoint.setY(mStartPoint.y());
    return edge;
  }

  Edge *CurvedEdge::copy() const {
    CurvedEdge *edge = new CurvedEdge();
    edge->mStartPoint.setX(mStartPoint.x());
    edge->mStartPoint.setY(mStartPoint.y());
    edge->mControlPoint.setX(mControlPoint.x());
    edge->mControlPoint.setY(mControlPoint.y());
    edge->mEndPoint.setX(mEndPoint.x());
    edge->mEndPoint.setY(mEndPoint.y());
    return edge;
  }

  Edge *CurvedEdge::reversedCopy() const {
    CurvedEdge *edge = new CurvedEdge();
    edge->mStartPoint.setX(mEndPoint.x());
    edge->mStartPoint.setY(mEndPoint.y());
    edge->mControlPoint.setX(mControlPoint.x());
    edge->mControlPoint.setY(mControlPoint.y());
    edge->mEndPoint.setX(mStartPoint.x());
    edge->mEndPoint.setY(mStartPoint.y());
    return edge;
  }
}
