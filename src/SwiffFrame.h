#ifndef SWIFFFRAME_H
#define SWIFFFRAME_H

#include <QVector>

namespace Swiff {
  class Action;
  class FramePrivate;
  class Sprite;
  class Tag;

  class Frame {
  public:
    Frame(Sprite *parent);
    ~Frame();

    QString &label() const;
    QVector<Tag *> &tags() const;

    QVector<Action *> &actions() const;
    void setActions(QVector<Action *> actions);

    Sprite *parent() const;

  public:
    FramePrivate *d;
  };
}

#endif
