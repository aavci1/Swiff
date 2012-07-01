#ifndef SWIFFOBJECT_H
#define SWIFFOBJECT_H

#include "SwiffItem.h"
#include "SwiffCxForm.h"

#include <QMatrix>

namespace Swiff {
  class Object {
  public:
    Object() : mItem(0), mColorTransform(true) {
    }

    Item *item() const {
      return mItem;
    }

    void setItem(Item *item) {
      mItem = item;
    }

    const QMatrix matrix() const {
      return mMatrix;
    }
    void setMatrix(const QMatrix &matrix) {
      mMatrix = matrix;
    }

    const CxForm colorTransform() const {
      return mColorTransform;
    }
    void setColorTransform(const CxForm &colorTransform) {
      mColorTransform = colorTransform;
    }
  private:
    Item *mItem;
    QMatrix mMatrix;
    CxForm mColorTransform;
  };

}

#endif
