#ifndef SWIFFSETBACKGROUNDCOLOR_H
#define SWIFFSETBACKGROUNDCOLOR_H

#include "SwiffTag.h"

#include <QColor>

namespace Swiff {
  class BitStream;

  class SetBackgroundColor : public Tag {
  public:
    SetBackgroundColor(): Tag("SetBackgroundColor") {
      mBackgroundColor.setRgba(qRgba(0, 0, 0, 255));
    }

    void parse(BitStream &bitstream);

    QColor color() const {
      return mBackgroundColor;
    }
  private:
    QColor mBackgroundColor;
  };
}

#endif
