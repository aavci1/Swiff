#include "SwiffDoABC.h"

#include "SwiffABCData.h"
#include "SwiffBitStream.h"

namespace Swiff {
  DoABC::DoABC() : Tag("DoABC") {
  }

  DoABC::~DoABC() {
    delete mABCData;
  }

  void DoABC::parse(BitStream &bitstream) {
    // create abcData structure
    mABCData = new ABCData();
    // parse tag header
    Tag::parse(bitstream);
    // parse flags and name
    bitstream >> mFlags >> mName;
    // parse the abcData
    mABCData->parse(bitstream);
  }
}
