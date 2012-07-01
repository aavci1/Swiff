#include "PreviewWidget.h"

#include <QDebug>

#include <QPainter>

PreviewWidget::PreviewWidget(QWidget *parent) : QWidget(parent) {
}

void PreviewWidget::setPixmap(const QPixmap &pixmap) {
  mPixmap = pixmap;
  // update scaled pixmap
  resizeEvent(0);
}

void PreviewWidget::paintEvent(QPaintEvent *) {
  // TODO: zooming
  // TODO: panning or scrollbars
  QPainter painter(this);
  // fill background
  painter.fillRect(0, 0, width(), height(), QColor(99, 99, 99));
  if (mScaledPixmap.isNull())
    return;
  // draw the checker board pattern
  painter.fillRect(mScaledRect, QBrush(QPixmap(":/icons/checker.png")));
  // draw the scaled pixmap at the center
  painter.drawPixmap(mScaledRect, mScaledPixmap);
}

void PreviewWidget::resizeEvent(QResizeEvent *) {
  // scale the pixmap to fit in
  if (mPixmap.width() > width() || mPixmap.height() > height())
    mScaledPixmap = mPixmap.scaled(size(), Qt::KeepAspectRatio);
  else
    mScaledPixmap = mPixmap;
  // calculate position
  int x = (width() - mScaledPixmap.width()) * 0.5f;
  int y = (height() - mScaledPixmap.height()) * 0.5f;
  // calculate scaled rect
  mScaledRect = QRect(x, y, mScaledPixmap.width(), mScaledPixmap.height());
  // update view
  update();
}
