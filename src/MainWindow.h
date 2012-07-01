#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QList>
#include <QMainWindow>

namespace Ui {
  class MainWindow;
}

namespace Swiff {
  class Document;
  class Item;
  class Frame;
}

class QDragEvent;
class QDropEvent;
class QTreeWidgetItem;

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = 0);
  ~MainWindow();

public slots:
  void open();
  void open(QString file);
  void clear();
  void exportSelected();
  void exportAll();
  void settings();
  void about();

protected:
  void dragEnterEvent(QDragEnterEvent *event);
  void dropEvent(QDropEvent *event);

  QTreeWidgetItem *createTreeWidgetItem(Swiff::Item *item, QString name);
  QTreeWidgetItem *createTreeWidgetItem(Swiff::Frame *frame, QString name);

  void exportItem(QTreeWidgetItem *twi, QString folder);
  void exportFrame(QTreeWidgetItem *twi, QString folder);

protected slots:
  void currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*);

private:
  Ui::MainWindow *ui;
  Swiff::Document *mDocument;
  QTreeWidgetItem *twiFrames;
  QTreeWidgetItem *twiBitmaps;
  QTreeWidgetItem *twiShapes;
  QTreeWidgetItem *twiButtons;
  QTreeWidgetItem *twiText;
  QTreeWidgetItem *twiFonts;
  QTreeWidgetItem *twiSounds;
  QTreeWidgetItem *twiVideos;
  QIcon frameIcon;
  QIcon bitmapIcon;
  QIcon shapeIcon;
  QIcon buttonIcon;
  QIcon textIcon;
  QIcon fontIcon;
  QIcon soundIcon;
  QIcon videoIcon;
  QList<QWidget *> lstPropertyWidgets;
};

#endif // MAINWINDOW_H
