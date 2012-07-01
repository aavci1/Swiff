#ifndef SWIFFABCDATA_H
#define SWIFFABCDATA_H

#include "SwiffTypes.h"

namespace Swiff {
  class ABCDataPrivate;
  class BitStream;

  class ABCData {
  public:
    ABCData();

    /** Parses the action code and length using the \param bitstream. */
    void parse(BitStream &bitstream);
  private:
    ABCDataPrivate *d;
  };

}

#endif
