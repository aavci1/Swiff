#include "SwiffFrame.h"

#include "SwiffAction.h"
#include "SwiffSprite.h"
#include "SwiffTag.h"

namespace Swiff {
  class FramePrivate {
  public:
    FramePrivate(Sprite *parent) : mParent(parent), mLabel("") {

    }

    Sprite *mParent;
    QString mLabel;
    QVector<Tag *> mTags;
    QVector<Action *> mActions;
  };

  Frame::Frame(Sprite *parent) : d(new FramePrivate(parent)){
    d->mActions.append((char)0);
  }

  Frame::~Frame() {
    qDeleteAll(d->mTags);
  }

  QString &Frame::label() const {
    return d->mLabel;
  }

  QVector<Tag *> &Frame::tags() const {
    return d->mTags;
  }

  QVector<Action *> &Frame::actions() const {
    return d->mActions;
  }

  void Frame::setActions(QVector<Action *> actions) {
    d->mActions = actions;
  }

  Sprite *Frame::parent() const {
    return d->mParent;
  }

}
