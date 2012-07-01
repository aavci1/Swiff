#include "SwiffWidget.h"

#include "SwiffBitStream.h"
#include "SwiffDocument.h"
#include "SwiffSprite.h"

#include <QDataStream>
#include <QFile>
#include <QMouseEvent>
#include <QPainter>

namespace Swiff {
  Widget::Widget(QWidget *parent)
    : QWidget(parent), mDocument(0) {
    // enable mouse tracking
    setMouseTracking(true);
  }

  Widget::~Widget() {
    close();
  }

  bool Widget::open(QString path) {
    // close if open
    close();
    // create the document
    mDocument = new Document(path);
    // check if file could be opened
    if (mDocument->open() == false) {
      // clean up
      close();
      // failure
      return false;
    }
    // connect update signal
    QObject::connect(mDocument->sprite(), SIGNAL(update()), this, SLOT(update()));
    foreach(Item * item, mDocument->dictionary()) {
      if (item != 0 && item->mItemType == Swiff::ItemSprite) {
        Sprite *sprite = static_cast<Sprite *>(item);
        QObject::connect(sprite, SIGNAL(update()), this, SLOT(update()));
      }
    }
    // initialize view matrix
    resizeEvent(0);
    // success
    return true;
  }

  void Widget::close() {
    // clean sprite
    delete mDocument;
    // nullify
    mDocument = 0;
  }

  QSize Widget::sizeHint() const {
    if (mDocument == 0) {
      return QSize();
    }
    return QSize((int)(mDocument->frameSize().width() * 0.05f), (int)(mDocument->frameSize().height() * 0.05f));
  }

  void Widget::rewind() {
    if (mDocument == 0) {
      return;
    }
    // position to the start
    mDocument->sprite()->rewind();
  }

  void Widget::play() {
    if (mDocument == 0) {
      return;
    }
    mDocument->sprite()->play();
  }

  void Widget::pause() {
    if (mDocument == 0) {
      return;
    }
    mDocument->sprite()->pause();
  }

  void Widget::stop() {
    if (mDocument == 0) {
      return;
    }
    mDocument->sprite()->stop();
  }

  void Widget::mouseMoveEvent(QMouseEvent *event) {
    if (mDocument == 0) {
      return;
    }
    float x = (float)(event->x() - mViewMatrix.dx()) / mViewMatrix.m11();
    float y = (float)(event->y() - mViewMatrix.dy()) / mViewMatrix.m22();
    bool overButton = false, sceneChanged = false;
    // inject event to the sprite
    mDocument->sprite()->mouseMoved(QPointF(x, y), overButton, sceneChanged);
    // update mouse cursor
    setCursor(overButton ? Qt::PointingHandCursor : Qt::ArrowCursor);
    // update scene if needed
    if (sceneChanged)
      update();
  }

  void Widget::mousePressEvent(QMouseEvent *event) {
    if (mDocument == 0) {
      return;
    }
    float x = (float)(event->x() - mViewMatrix.dx()) / mViewMatrix.m11();
    float y = (float)(event->y() - mViewMatrix.dy()) / mViewMatrix.m22();
    bool sceneChanged = false;
    // inject event to the sprite
    mDocument->sprite()->mousePressed(QPointF(x, y), sceneChanged);
    if (sceneChanged)
      update();
  }

  void Widget::mouseReleaseEvent(QMouseEvent *event) {
    if (mDocument == 0) {
      return;
    }
    float x = (float)(event->x() - mViewMatrix.dx()) / mViewMatrix.m11();
    float y = (float)(event->y() - mViewMatrix.dy()) / mViewMatrix.m22();
    bool sceneChanged = false;
    // inject event to the sprite
    mDocument->sprite()->mouseReleased(QPointF(x, y), sceneChanged);
    if (sceneChanged)
      update();
  }

  void Widget::paintEvent(QPaintEvent *event) {
    if (mDocument == 0) {
      return;
    }
    Q_UNUSED(event);
    // create the painter instances
    QPainter painter(this);
    // set high quality rendering
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    // fill background
    painter.fillRect(this->rect(), mDocument->sprite()->backgroundBrush());
    // set matrix
    painter.setMatrix(mViewMatrix);
    // paint current frame
    mDocument->sprite()->paint(painter);
  }

  void Widget::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);
    if (mDocument == 0) {
      return;
    }
    float zoom = qMin((float)width() / mDocument->frameSize().width(), (float)height() / mDocument->frameSize().height());
    float left = qMax((width() - mDocument->frameSize().width() * zoom) * 0.5f, 0.0f);
    float top = qMax((height() - mDocument->frameSize().height() * zoom) * 0.5f, 0.0f);
    mViewMatrix.setMatrix(zoom, 0, 0, zoom, left, top);
  }
}
