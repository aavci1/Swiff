#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

#include <QWidget>

class PreviewWidget : public QWidget {
  Q_OBJECT
public:
  explicit PreviewWidget(QWidget *parent = 0);

  void setPixmap(const QPixmap &pixmap);

protected:
  void paintEvent(QPaintEvent *);
  void resizeEvent(QResizeEvent *);

private:
  QPixmap mPixmap;
  QPixmap mScaledPixmap;
  QRect mScaledRect;
};

#endif // PREVIEWWIDGET_H
