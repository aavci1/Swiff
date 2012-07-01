#include "SwiffABCData.h"

#include <QDebug>

#include "SwiffBitStream.h"

namespace Swiff {
  class ABCDataPrivate {
  public:
    ABCDataPrivate() {

    }

    u16 mMajorVersion;
    u16 mMinorVersion;
  };

  ABCData::ABCData() : d(new ABCDataPrivate()){
  }

  void ABCData::parse(BitStream &bitstream) {
    bitstream >> d->mMinorVersion >> d->mMajorVersion;
    qDebug() << d->mMajorVersion << "." << d->mMinorVersion;
  }
} // namespace Swiff
