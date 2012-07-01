#include "SwiffShapePrivate.h"

namespace Swiff {
  QPainterPath parseGlyph(BitStream &bitstream, u8 fillBits, u8 lineBits) {
    QPainterPath combinedPath;
    QVector<Edge *> edges;
    QPointF drawing;
    // shape records
    do {
      u8 typeFlag = 0;
      typeFlag = bitstream.readUnsignedBits(1);
      if (typeFlag == 0) {
        // save current path
        if (edges.count() > 0) {
          QPainterPath path;
          float x = 0, y = 0;
          for (int j = 0; j < edges.count(); ++j) {
            Edge *edge = edges.at(j);
            if ((x != edge->mStartPoint.x()) || (y != edge->mStartPoint.y())) {
              path.moveTo(edge->mStartPoint.x(), edge->mStartPoint.y());
            }
            if (edge->mType == Edge::Straight) {
              path.lineTo(edge->mEndPoint.x(), edge->mEndPoint.y());
            } else if (edge->mType == Edge::Curved) {
              CurvedEdge *ce = static_cast<CurvedEdge *>(edge);
              path.quadTo(ce->mControlPoint.x(), ce->mControlPoint.y(), ce->mEndPoint.x(), ce->mEndPoint.y());
            }
            x = edge->mEndPoint.x();
            y = edge->mEndPoint.y();
          }
          combinedPath.addPath(path);
        }
        // clear all edges
        edges.clear();
        // read flags
        u8 flags = 0;
        flags = bitstream.readUnsignedBits(5);
        if (flags == 0) {
          break;
        }
        // stateMoveTo
        if (flags & 0x01) {
          u8 moveBitCount = 0;
          moveBitCount = bitstream.readUnsignedBits(5);
          drawing.setX(bitstream.readSignedBits(moveBitCount));
          drawing.setY(bitstream.readSignedBits(moveBitCount));
        }
        // stateBrush0
        if (flags & 0x02)
          bitstream.skip(0, fillBits);
        // stateBrush1
        if (flags & 0x04)
          bitstream.skip(0, fillBits);
        // stateLineStyle
        if (flags & 0x08)
          bitstream.skip(0, lineBits);
        // stateNewStyles
      } else {
        u8 straightFlag = 0;
        straightFlag = bitstream.readUnsignedBits(1);
        if (straightFlag) {
          StraightEdge *edge = new StraightEdge();
          edge->mStartPoint.setX(drawing.x());
          edge->mStartPoint.setY(drawing.y());
          // get bit count
          u8 bitCount = bitstream.readUnsignedBits(4) + 2;
          u8 generalLine = bitstream.readUnsignedBits(1);
          if (generalLine) {
            drawing.setX(drawing.x() + bitstream.readSignedBits(bitCount));
            drawing.setY(drawing.y() + bitstream.readSignedBits(bitCount));
          } else {
            u8 verticalLine = bitstream.readUnsignedBits(1);
            if (verticalLine) {
              drawing.setY(drawing.y() + bitstream.readSignedBits(bitCount));
            } else {
              drawing.setX(drawing.x() + bitstream.readSignedBits(bitCount));
            }
          }
          edge->mEndPoint.setX(drawing.x());
          edge->mEndPoint.setY(drawing.y());
          edges.append(edge);
        } else {
          CurvedEdge *edge = new CurvedEdge();
          edge->mStartPoint.setX(drawing.x());
          edge->mStartPoint.setY(drawing.y());
          // get bit count
          u8 bitCount = bitstream.readUnsignedBits(4) + 2;
          drawing.setX(drawing.x() + bitstream.readSignedBits(bitCount));
          drawing.setY(drawing.y() + bitstream.readSignedBits(bitCount));
          edge->mControlPoint.setX(drawing.x());
          edge->mControlPoint.setY(drawing.y());
          drawing.setX(drawing.x() + bitstream.readSignedBits(bitCount));
          drawing.setY(drawing.y() + bitstream.readSignedBits(bitCount));
          edge->mEndPoint.setX(drawing.x());
          edge->mEndPoint.setY(drawing.y());
          edges.append(edge);
        }
      }
    } while (1);
    // return the combined path
    return combinedPath;
  }

  void ShapePrivate::parseStyles(BitStream &bitstream, int version) {
    u16 count = 0;
    u8 temp = 0;
    // clear brushes
    mBrushes.clear();
    bitstream >> temp;
    if (version >= 2 && temp == 0xff) {
      bitstream >> count;
    } else {
      count = temp;
    }
    // parse and create fill styles
    for (u16 i = 0; i < count; ++i) {
      QBrush *brush = new QBrush();
      if (version <= 2)
        brush->setColor(QColor(0, 0, 0, 255));
      else
        brush->setColor(QColor(0, 0, 0, 0));
      bitstream >> (*brush);
      mBrushes.append(QSharedPointer<QBrush>(brush));
    }
    // clear pens
    mPens.clear();
    bitstream >> temp;
    if (version >= 2 && temp == 0xff) {
      bitstream >> count;
    } else {
      count = temp;
    }
    for (u16 i = 0; i < count; ++i) {
      LineStyle *style = new LineStyle(version);
      bitstream >> (*style);
      mPens.append(QSharedPointer<QPen>(toPen(style)));
      delete style;
    }
    // parse fill bits and line bits
    mFillBits = bitstream.readUnsignedBits(4);
    mLineBits = bitstream.readUnsignedBits(4);
  }

  QPen *ShapePrivate::toPen(LineStyle *lineStyle) {
    QPen *pen = new QPen();
    if (lineStyle->endCapStyle() == 0) {
      pen->setCapStyle(Qt::RoundCap);
    }
    if (lineStyle->joinStyle() == 0) {
      pen->setJoinStyle(Qt::RoundJoin);
    } else if (lineStyle->joinStyle() == 1) {
      pen->setJoinStyle(Qt::BevelJoin);
    } else {
      pen->setJoinStyle(Qt::MiterJoin);
    }
    if (lineStyle->hasFillFlag()) {
      pen->setBrush(lineStyle->mBrush);
    } else {
      pen->setColor(lineStyle->mColor);
    }
    pen->setWidth(lineStyle->mWidth);
    return pen;
  }

  void ShapePrivate::parseShapeRecords(BitStream &bitstream, int version) {
    QVector<SubShape *> subShapes;
    QVector< QSharedPointer<Edge> > edges;
    QBrush *brush0 = 0;
    QBrush *brush1 = 0;
    QPen *pen = 0;
    QPointF drawing;
    mBrushes.append(QSharedPointer<QBrush>(new QBrush(QColor(0, 0, 0, 255))));
    // shape records
    do {
      u8 typeFlag = 0;
      typeFlag = bitstream.readUnsignedBits(1);
      if (typeFlag == 0) {
        // save current path
        if (edges.count() > 0) {
          // left fill
          if (brush0 != 0) {
            SubShape *subShape = new SubShape(QBrush(*brush0));
            for (int i = 0; i < edges.count(); ++i) {
              subShape->mEdges.append(QSharedPointer<Edge>(edges.at(i)->copy()));
            }
            subShapes.append(subShape);
          }
          // right fill
          if (brush1 != 0) {
            // reverse order of the edges so all shapes are counter-clockwise
            SubShape *subShape = new SubShape(QBrush(*brush1));
            for (int i = 0; i < edges.count(); ++i) {
              subShape->mEdges.append(QSharedPointer<Edge>(edges.at(edges.count() - 1 - i)->reversedCopy()));
            }
            subShapes.append(subShape);
          }
          if (pen != 0) {
            SubShape *subShape = new SubShape(QPen(*pen));
            for (int i = 0; i < edges.count(); ++i) {
              subShape->mEdges.append(QSharedPointer<Edge>(edges.at(i)->copy()));
            }
            subShapes.append(subShape);
          }
        }
        // clear all edges
        edges.clear();
        // read flags
        u8 flags = 0;
        flags = bitstream.readUnsignedBits(5);
        if (flags == 0) {
          break;
        }
        // stateMoveTo
        if (flags & 0x01) {
          u8 moveBitCount = 0;
          moveBitCount = bitstream.readUnsignedBits(5);
          drawing.setX(bitstream.readSignedBits(moveBitCount));
          drawing.setY(bitstream.readSignedBits(moveBitCount));
        }
        // stateBrush0
        if (flags & 0x02) {
          u16 fs = bitstream.readUnsignedBits(mFillBits);
          if (1 <= fs && fs <= mBrushes.count()) {
            brush0 = mBrushes.at(fs - 1).data();
          } else {
            brush0 = 0;
          }
        }
        // stateBrush1
        if (flags & 0x04) {
          u16 fs = bitstream.readUnsignedBits(mFillBits);
          if (1 <= fs && fs <= mBrushes.count()) {
            brush1 = mBrushes.at(fs - 1).data();
          } else {
            brush1 = 0;
          }
        }
        // stateLineStyle
        if (flags & 0x08) {
          u16 ls = bitstream.readUnsignedBits(mLineBits);
          if (1 <= ls && ls <= mPens.count()) {
            pen = mPens.at(ls - 1).data();
          } else {
            pen = 0;
          }
        }
        // stateNewStyles
        if (version >= 2 && flags & 0x10) {
          parseStyles(bitstream, version);
        }
      } else {
        u8 straightFlag = 0;
        straightFlag = bitstream.readUnsignedBits(1);
        if (straightFlag) {
          StraightEdge *edge = new StraightEdge();
          edge->mStartPoint.setX(drawing.x());
          edge->mStartPoint.setY(drawing.y());
          // get bit count
          u8 bitCount = bitstream.readUnsignedBits(4) + 2;
          u8 generalLine = bitstream.readUnsignedBits(1);
          if (generalLine) {
            drawing.setX(drawing.x() + bitstream.readSignedBits(bitCount));
            drawing.setY(drawing.y() + bitstream.readSignedBits(bitCount));
          } else {
            u8 verticalLine = bitstream.readUnsignedBits(1);
            if (verticalLine) {
              drawing.setY(drawing.y() + bitstream.readSignedBits(bitCount));
            } else {
              drawing.setX(drawing.x() + bitstream.readSignedBits(bitCount));
            }
          }
          edge->mEndPoint.setX(drawing.x());
          edge->mEndPoint.setY(drawing.y());
          edges.append(QSharedPointer<Edge>(edge));
        } else {
          CurvedEdge *edge = new CurvedEdge();
          edge->mStartPoint.setX(drawing.x());
          edge->mStartPoint.setY(drawing.y());
          // get bit count
          u8 bitCount = bitstream.readUnsignedBits(4) + 2;
          drawing.setX(drawing.x() + bitstream.readSignedBits(bitCount));
          drawing.setY(drawing.y() + bitstream.readSignedBits(bitCount));
          edge->mControlPoint.setX(drawing.x());
          edge->mControlPoint.setY(drawing.y());
          drawing.setX(drawing.x() + bitstream.readSignedBits(bitCount));
          drawing.setY(drawing.y() + bitstream.readSignedBits(bitCount));
          edge->mEndPoint.setX(drawing.x());
          edge->mEndPoint.setY(drawing.y());
          edges.append(QSharedPointer<Edge>(edge));
        }
      }
    } while (1);
    // clean up
    edges.clear();
    bool iterate = false;
    // merge shapes
    do {
      iterate = false;
      // create closed shapes by merging adjacent subshape with same fillstyle
      int size = subShapes.count();
      for (int i = 0; i < size - 1; ++i) {
        SubShape *subShape1 = subShapes.at(i);
        // skip shapes without a brush
        if (!subShape1->isFilled())
          continue;
        // skip closed shapes
        if (subShape1->mEdges.first()->mStartPoint == subShape1->mEdges.last()->mEndPoint)
          continue;
        for (int j = i + 1; j < size; ++j) {
          SubShape *subShape2 = subShapes.at(j);
          // skip shapes without a brush
          if (!subShape2->isFilled())
            continue;
          // skip closed shapes
          if (subShape2->mEdges.first()->mStartPoint == subShape2->mEdges.last()->mEndPoint)
            continue;
          // skip if subshapes dont have the same brush
          if (subShape1->mBrush != subShape2->mBrush)
            continue;
          if (subShape1->mEdges.last()->mEndPoint == subShape2->mEdges.first()->mStartPoint) {
            // append subshape2 to the end of subshape1
            subShape1->mEdges += subShape2->mEdges;
            // clean up
            delete subShapes.at(j);
            subShapes.remove(j);
            size--;
            j--;
            iterate = true;
          } else if (subShape2->mEdges.last()->mEndPoint == subShape1->mEdges.first()->mStartPoint) {
            // insert subshape2 to the beginning of subshape1
            for (int k = 0; k < subShape2->mEdges.count(); ++k) {
              subShape1->mEdges.insert(subShape1->mEdges.begin() + k,
                                       subShape2->mEdges.at(k));
            }
            // clean up
            delete subShapes.at(j);
            subShapes.remove(j);
            size--;
            j--;
            iterate = true;
          }
        }
      }
    } while (iterate);
    // merge shapes with same fill style
    int size = subShapes.count();
    for (int i = 0; i < size - 1; ++i) {
      SubShape *subShape1 = subShapes.at(i);
      // skip shapes without a brush
      if (!subShape1->isFilled()) {
        continue;
      }
      for (int j = i + 1; j < size; ++j) {
        SubShape *subShape2 = subShapes.at(j);
        // skip shapes without a brush
        if (!subShape2->isFilled()) {
          continue;
        }
        if (subShape1->mBrush == subShape2->mBrush) {
          // append subshape2 to the end of subshape1
          subShape1->mEdges += subShape2->mEdges;
          // remove subshape2 from the list
          delete subShapes.at(j);
          subShapes.remove(j);
          size--;
          j--;
        }
      }
    }
    // clear all arrays
    mBrushes.clear();
    mPens.clear();
    // convert shapes to painter paths
    for (int i = 0; i < subShapes.count(); ++i) {
      SubShape *subShape = subShapes.at(i);
      // skip empty sub shapes
      if (subShape->mEdges.count() == 0) {
        continue;
      }
      QPainterPath painterPath;
      float x = 0, y = 0;
      for (int j = 0; j < subShape->mEdges.count(); ++j) {
        Edge *edge = subShape->mEdges.at(j).data();
        if ((x != edge->mStartPoint.x()) || (y != edge->mStartPoint.y())) {
          painterPath.moveTo(edge->mStartPoint.x(), edge->mStartPoint.y());
        }
        if (edge->mType == Edge::Straight) {
          painterPath.lineTo(edge->mEndPoint.x(), edge->mEndPoint.y());
        } else if (edge->mType == Edge::Curved) {
          CurvedEdge *ce = static_cast<CurvedEdge *>(edge);
          painterPath.quadTo(ce->mControlPoint.x(),
                             ce->mControlPoint.y(),
                             ce->mEndPoint.x(),
                             ce->mEndPoint.y());
        }
        x = edge->mEndPoint.x();
        y = edge->mEndPoint.y();
      }
      mPaths.append(painterPath);
      if (subShape->isFilled()) {
        mBrushes.append(QSharedPointer<QBrush>(new QBrush(subShape->mBrush)));
        mPens.append(QSharedPointer<QPen>(0));
      } else {
        mPens.append(QSharedPointer<QPen>(new QPen(subShape->mPen)));
        mBrushes.append(QSharedPointer<QBrush>(0));
      }
    }
    // clean up
    qDeleteAll(subShapes);
  }
}
