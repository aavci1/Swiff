#ifndef SWIFFWIDGET_H
#define SWIFFWIDGET_H

#include "SwiffBitStream.h"

#include <QSize>
#include <QWidget>

class QMouseEvent;
class QPaintEvent;
class QResizeEvent;

namespace Swiff {
  class Document;

  class Widget : public QWidget {
    Q_OBJECT
  public:
    Widget(QWidget *parent = 0);

    ~Widget();

    QSize sizeHint() const;
  public slots:
    bool open(QString file);
    void close();
    void play();
    void pause();
    void stop();
    void rewind();

  protected:
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

  public:
    Document *mDocument;

  private:
    QMatrix mViewMatrix;
  };
}

#endif
