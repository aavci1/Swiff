#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "AboutDialog.h"
#include "SettingsDialog.h"
#include "SwiffExporter.h"

#include <QActionEvent>
#include <QDebug>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QFormLayout>
#include <QImageWriter>
#include <QLineEdit>
#include <QTextEdit>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QSettings>
#include <QUrl>

#include <SwiffBitmap.h>
#include <SwiffButton.h>
#include <SwiffDocument.h>
#include <SwiffFont.h>
#include <SwiffFrame.h>
#include <SwiffItem.h>
#include <SwiffShape.h>
#include <SwiffSound.h>
#include <SwiffSprite.h>
#include <SwiffText.h>
#include <SwiffVideo.h>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), mDocument(0) {
  ui->setupUi(this);
  // move window to desktop center
  move (QApplication::desktop()->geometry().center() - rect().center());
  // create icons
  frameIcon = QIcon(":/icons/image-x-generic");
  bitmapIcon = QIcon(":/icons/image-x-generic");
  shapeIcon = QIcon(":/icons/image-svg+xml.png");
  buttonIcon = QIcon(":/icons/button.png");
  textIcon = QIcon(":/icons/text.png");
  fontIcon = QIcon(":/icons/preferences-desktop-font.png");
  soundIcon = QIcon(":/icons/audio-x-generic.png");
  videoIcon = QIcon(":/icons/video-x-generic.png");
  // create frames item
  twiFrames = new QTreeWidgetItem();
  twiFrames->setText(0, tr("Frames"));
  twiFrames->setIcon(0, frameIcon);
  ui->twItems->insertTopLevelItem(0, twiFrames);
  // create images item
  twiBitmaps = new QTreeWidgetItem();
  twiBitmaps->setText(0, tr("Bitmaps"));
  twiBitmaps->setIcon(0, bitmapIcon);
  ui->twItems->insertTopLevelItem(1, twiBitmaps);
  // create shapes item
  twiShapes = new QTreeWidgetItem();
  twiShapes->setText(0, tr("Shapes"));
  twiShapes->setIcon(0, shapeIcon);
  ui->twItems->insertTopLevelItem(2, twiShapes);
  // create buttons item
  twiButtons = new QTreeWidgetItem();
  twiButtons->setText(0, tr("Buttons"));
  twiButtons->setIcon(0, buttonIcon);
  ui->twItems->insertTopLevelItem(3, twiButtons);
  // create fonts item
  twiFonts = new QTreeWidgetItem();
  twiFonts->setText(0, tr("Fonts"));
  twiFonts->setIcon(0, fontIcon);
  ui->twItems->insertTopLevelItem(4, twiFonts);
  // create text item
  twiText = new QTreeWidgetItem();
  twiText->setText(0, tr("Text"));
  twiText->setIcon(0, textIcon);
  ui->twItems->insertTopLevelItem(5, twiText);
  // create sounds item
  twiSounds = new QTreeWidgetItem();
  twiSounds->setText(0, tr("Sounds"));
  twiSounds->setIcon(0, soundIcon);
  ui->twItems->insertTopLevelItem(6, twiSounds);
  // create videos item
  twiVideos = new QTreeWidgetItem();
  twiVideos->setText(0, tr("Videos"));
  twiVideos->setIcon(0, videoIcon);
  ui->twItems->insertTopLevelItem(7, twiVideos);
  // hide progress bar
  ui->pbProgress->hide();
  // disable actions
  ui->actionClose->setEnabled(false);
  ui->actionExportSelected->setEnabled(false);
  ui->actionExportAll->setEnabled(false);
  // connect signal-slots
  QObject::connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(open()));
  QObject::connect(ui->actionClose, SIGNAL(triggered()), this, SLOT(clear()));
  QObject::connect(ui->actionExportSelected, SIGNAL(triggered()), this, SLOT(exportSelected()));
  QObject::connect(ui->actionExportAll, SIGNAL(triggered()), this, SLOT(exportAll()));
  QObject::connect(ui->actionSettings, SIGNAL(triggered()), this, SLOT(settings()));
  QObject::connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()));
  // connect item selection changed signals
  QObject::connect(ui->twItems, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), this, SLOT(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
}

MainWindow::~MainWindow() {
  delete ui;
}

void MainWindow::open() {
  QString path = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("SWF Files (*.swf)"));
  if (!path.isNull()) {
    open(path);
  }
}

void MainWindow::open(QString file) {
  // set status tip
  ui->lblStatus->setText(tr("Opening file..."));
  // show the progress bar
  ui->pbProgress->setValue(0);
  ui->pbProgress->show();
  // clear previous items
  clear();
  // create swiff widget
  mDocument = new Swiff::Document(file);
  // check for success
  if (mDocument->open() == false) {
    // delete swiff widget
    delete mDocument;
    mDocument = 0;
    // set status tip
    ui->lblStatus->setText(tr("Can't load document."));
    // hide progress bar
    ui->pbProgress->hide();
    return;
  }
  // update title
  this->setWindowTitle(QString("%1 - Swiff Spy").arg(QFileInfo(file).fileName()));
  // check dictionary for items
  QList<Swiff::Item *> items = mDocument->dictionary().values();
  for (int i = 0; i < items.count(); ++i) {
    // update progress bar
    ui->pbProgress->setValue(100 * i / items.count());
    QTreeWidgetItem *twi = 0;
    // create the tree widget item
    switch (items.at(i)->mItemType) {
    case Swiff::ItemBitmap:
      twi = createTreeWidgetItem(items[i], QString("Bitmap %1").arg(twiBitmaps->childCount() + 1));
      twi->setIcon(0, bitmapIcon);
      twiBitmaps->addChild(twi);
      break;
    case Swiff::ItemShape:
      twi = createTreeWidgetItem(items[i], QString("Shape %1").arg(twiShapes->childCount() + 1));
      twi->setIcon(0, shapeIcon);
      twiShapes->addChild(twi);
      break;
    case Swiff::ItemButton: {
        int buttonNo = twiButtons->childCount() * 0.25 + 1;
        // create up item
        Swiff::Button *up = static_cast<Swiff::Button *>(items[i]);
        up->setState(Swiff::Idle);
        twi = createTreeWidgetItem(up, QString("Button %1 Up").arg(buttonNo));
        twi->setIcon(0, buttonIcon);
        twiButtons->addChild(twi);
        // create over item
        Swiff::Button *over = up->clone();
        over->setState(Swiff::OverUp);
        twi = createTreeWidgetItem(over, QString("Button %1 Over").arg(buttonNo));
        twi->setIcon(0, buttonIcon);
        twiButtons->addChild(twi);
        // create down item
        Swiff::Button *down = up->clone();
        down->setState(Swiff::OverDown);
        twi = createTreeWidgetItem(down, QString("Button %1 Down").arg(buttonNo));
        twi->setIcon(0, buttonIcon);
        twiButtons->addChild(twi);
        // create hit test item
        Swiff::Button *hit = up->clone();
        hit->setState(Swiff::HitTest);
        twi = createTreeWidgetItem(hit, QString("Button %1 Hit").arg(buttonNo));
        twi->setIcon(0, buttonIcon);
        twiButtons->addChild(twi);
      }
      break;
    case Swiff::ItemFont: {
        Swiff::Font *font = static_cast<Swiff::Font *>(items[i]);
        if (font->name().length() > 0)
          twi = createTreeWidgetItem(items[i], font->name());
        else
          twi = createTreeWidgetItem(items[i], QString("Font %1").arg(twiFonts->childCount() + 1));
        twi->setIcon(0, fontIcon);
        twiFonts->addChild(twi);
      }
      break;
    case Swiff::ItemText:
      twi = createTreeWidgetItem(items[i], QString("Text %1").arg(twiText->childCount() + 1));
      twi->setIcon(0, textIcon);
      twiText->addChild(twi);
      break;
    case Swiff::ItemSound: {
        Swiff::Sound *sound = static_cast<Swiff::Sound *>(items[i]);
        if (sound->format() == 1)
          qDebug() << "WARN: ADPCM codec not implemented!";
        if (sound->format() == 3)
          qDebug() << "WARN: Uncompressed, little-endian codec not implemented!";
        if (sound->format() == 4 || sound->format() == 5 || sound->format() == 6)
          qDebug() << "WARN: Nellymoser codec not implemented!";
        if (sound->format() == 11)
          qDebug() << "WARN: Speex codec not implemented!";
        if (sound->format() != 0 && sound->format() != 2)
          break;
        twi = createTreeWidgetItem(items[i], QString("Sound %1").arg(twiSounds->childCount() + 1));
        twi->setIcon(0, soundIcon);
        twiSounds->addChild(twi);
      }
      break;
    case Swiff::ItemVideo:
      twi = createTreeWidgetItem(items[i], QString("Video %1").arg(twiVideos->childCount() + 1));
      twi->setIcon(0, videoIcon);
      twiVideos->addChild(twi);
      break;
    default:
      break;
    }
  }
  // add frames to the tree list
  for (int i = 0; i < mDocument->sprite()->frames().size(); ++i) {
    QTreeWidgetItem *twi = createTreeWidgetItem(mDocument->sprite()->frames()[i], QString("Frame %1").arg(twiFrames->childCount() + 1));
    twi->setIcon(0, frameIcon);
    twiFrames->addChild(twi);
  }
  // add playback sounds to the tree list
  for (int i = 0; i < mDocument->sprite()->streams().size(); ++i) {
    if (mDocument->sprite()->streams()[i]->dataSize() == 0)
      continue;
    QTreeWidgetItem *twi = createTreeWidgetItem(mDocument->sprite()->streams()[i], QString("Stream %1").arg(i + 1));
    twi->setIcon(0, soundIcon);
    twiSounds->addChild(twi);
  }
  // update parent item names
  twiFrames->setText(0, tr("Frames (%1)").arg(twiFrames->childCount()));
  twiBitmaps->setText(0, tr("Bitmaps (%1)").arg(twiBitmaps->childCount()));
  twiShapes->setText(0, tr("Shapes (%1)").arg(twiShapes->childCount()));
  twiButtons->setText(0, tr("Buttons (%1)").arg(twiButtons->childCount()));
  twiFonts->setText(0, tr("Fonts (%1)").arg(twiFonts->childCount()));
  twiText->setText(0, tr("Text (%1)").arg(twiText->childCount()));
  twiSounds->setText(0, tr("Sounds (%1)").arg(twiSounds->childCount()));
  twiVideos->setText(0, tr("Videos (%1)").arg(twiVideos->childCount()));
  // enable actions
  ui->actionClose->setEnabled(true);
  ui->actionExportAll->setEnabled(true);
  // set status tip
  ui->lblStatus->setText(tr("Ready."));
  // hide progress bar
  ui->pbProgress->hide();
}

void MainWindow::clear() {
  // clear current item
  ui->twItems->setCurrentItem(0);
  // update title
  this->setWindowTitle(QString("Swiff Spy"));
  // delete swiff widget
  delete mDocument;
  mDocument = 0;
  // delete all frame items
  for (int i = twiFrames->childCount(); i > 0; --i)
    delete twiFrames->child(0);
  // delete all image items
  for (int i = twiBitmaps->childCount(); i > 0; --i)
    delete twiBitmaps->child(0);
  // delete all shape items
  for (int i = twiShapes->childCount(); i > 0; --i)
    delete twiShapes->child(0);
  // delete all button items
  for (int i = twiButtons->childCount(); i > 0; --i)
    delete twiButtons->child(0);
  // delete all font items
  for (int i = twiFonts->childCount(); i > 0; --i)
    delete twiFonts->child(0);
  // delete all text items
  for (int i = twiText->childCount(); i > 0; --i)
    delete twiText->child(0);
  // delete all sound items
  for (int i = twiSounds->childCount(); i > 0; --i)
    delete twiSounds->child(0);
  // delete all video items
  for (int i = twiVideos->childCount(); i > 0; --i)
    delete twiVideos->child(0);
  // update parent item names
  twiFrames->setText(0, tr("Frames"));
  twiBitmaps->setText(0, tr("Bitmaps"));
  twiShapes->setText(0, tr("Shapes"));
  twiButtons->setText(0, tr("Buttons"));
  twiFonts->setText(0, tr("Fonts"));
  twiText->setText(0, tr("Text"));
  twiSounds->setText(0, tr("Sounds"));
  twiVideos->setText(0, tr("Videos"));
  // disable actions
  ui->actionClose->setEnabled(false);
  ui->actionExportAll->setEnabled(false);
}

void MainWindow::exportSelected() {
  // get export directory
  QString path = QFileDialog::getExistingDirectory(this, tr("Select folder to export images."));
  // check directory
  if (path.isNull())
    return;
  // export frames
  for (int i = 0; i < twiFrames->childCount(); ++i)
    if (twiFrames->child(i)->isSelected())
      exportFrame(twiFrames->child(i), path);
  // export bitmaps
  for (int i = 0; i < twiBitmaps->childCount(); ++i)
    if (twiBitmaps->child(i)->isSelected())
      exportItem(twiBitmaps->child(i), path);
  // export bitmaps
  for (int i = 0; i < twiBitmaps->childCount(); ++i)
    if (twiBitmaps->child(i)->isSelected())
      exportItem(twiBitmaps->child(i), path);
  // export shapes
  for (int i = 0; i < twiShapes->childCount(); ++i)
    if (twiShapes->child(i)->isSelected())
      exportItem(twiShapes->child(i), path);
  // export buttons
  for (int i = 0; i < twiButtons->childCount(); ++i)
    if (twiButtons->child(i)->isSelected())
      exportItem(twiButtons->child(i), path);
  // export fonts
  for (int i = 0; i < twiFonts->childCount(); ++i)
    if (twiFonts->child(i)->isSelected())
      exportItem(twiFonts->child(i), path);
  // export text
  for (int i = 0; i < twiText->childCount(); ++i)
    if (twiText->child(i)->isSelected())
      exportItem(twiText->child(i), path);
  // export sounds
  for (int i = 0; i < twiSounds->childCount(); ++i)
    if (twiSounds->child(i)->isSelected())
      exportItem(twiSounds->child(i), path);
  // export videos
  for (int i = 0; i < twiVideos->childCount(); ++i)
    if (twiVideos->child(i)->isSelected())
      exportItem(twiVideos->child(i), path);
}

void MainWindow::exportAll() {
  // get export directory
  QString path = QFileDialog::getExistingDirectory(this, tr("Select folder to export images."));
  // check directory
  if (path.isNull())
    return;
  // export frames
  for (int i = 0; i < twiFrames->childCount(); ++i)
    exportFrame(twiFrames->child(i), path);
  // export bitmaps
  for (int i = 0; i < twiBitmaps->childCount(); ++i)
    exportItem(twiBitmaps->child(i), path);
  // export shapes
  for (int i = 0; i < twiShapes->childCount(); ++i)
    exportItem(twiShapes->child(i), path);
  // export buttons
  for (int i = 0; i < twiButtons->childCount(); ++i)
    exportItem(twiButtons->child(i), path);
  // export font
  for (int i = 0; i < twiFonts->childCount(); ++i)
    exportItem(twiFonts->child(i), path);
  // export text
  for (int i = 0; i < twiText->childCount(); ++i)
    exportItem(twiText->child(i), path);
  // export sounds
  for (int i = 0; i < twiSounds->childCount(); ++i)
    exportItem(twiSounds->child(i), path);
  // export videos
  for (int i = 0; i < twiVideos->childCount(); ++i)
    exportItem(twiVideos->child(i), path);
}

void MainWindow::settings() {
  // create the settings dialog
  SettingsDialog *dialog = new SettingsDialog(this);
  // find supported image formats
  QStringList bitmapFormats;
  foreach (QByteArray data, QImageWriter::supportedImageFormats())
    bitmapFormats.append(QString(data.constData()).toUpper());
  // remove duplicates and sort
  bitmapFormats.removeDuplicates();
  bitmapFormats.sort();
  // create settings object
  QSettings settings(QSettings::NativeFormat, QSettings::UserScope, QLatin1String("Swiff Solutions"), QLatin1String("Swiff Spy"));
  // fill in the image combo
  QString bitmapFormat = settings.value(QLatin1String("BitmapFormat"), QLatin1String("PNG")).toString();
  dialog->cbBitmapFormat->addItems(bitmapFormats);
  // select current item
  for (int i = 0; i < dialog->cbBitmapFormat->count(); ++i)
    if (dialog->cbBitmapFormat->itemText(i) == bitmapFormat)
      dialog->cbBitmapFormat->setCurrentIndex(i);
  // fill in the shape combo
  QString shapeFormat = settings.value(QLatin1String("ShapeFormat"), QLatin1String("SVG")).toString();
  dialog->cbShapeFormat->addItem("SVG");
  dialog->cbShapeFormat->addItems(bitmapFormats);
  // select current item
  for (int i = 0; i < dialog->cbShapeFormat->count(); ++i)
    if (dialog->cbShapeFormat->itemText(i) == shapeFormat)
      dialog->cbShapeFormat->setCurrentIndex(i);
  // fill in the button combo
  QString buttonFormat = settings.value(QLatin1String("ButtonFormat"), QLatin1String("SVG")).toString();
  dialog->cbButtonFormat->addItem("SVG");
  dialog->cbButtonFormat->addItems(bitmapFormats);
  // select current item
  for (int i = 0; i < dialog->cbButtonFormat->count(); ++i)
    if (dialog->cbButtonFormat->itemText(i) == buttonFormat)
      dialog->cbButtonFormat->setCurrentIndex(i);
  // fill in the font combo
  QString fontFormat = settings.value(QLatin1String("FontFormat"), QLatin1String("SVG")).toString();
  dialog->cbFontFormat->addItem("SVG");
  dialog->cbFontFormat->addItems(bitmapFormats);
  // select current item
  for (int i = 0; i < dialog->cbFontFormat->count(); ++i)
    if (dialog->cbFontFormat->itemText(i) == fontFormat)
      dialog->cbFontFormat->setCurrentIndex(i);
  // fill in the text combo
  QString textFormat = settings.value(QLatin1String("TextFormat"), QLatin1String("TXT")).toString();
  dialog->cbTextFormat->addItem("TXT");
  dialog->cbTextFormat->addItem("SVG");
  dialog->cbTextFormat->addItems(bitmapFormats);
  // select current item
  for (int i = 0; i < dialog->cbTextFormat->count(); ++i)
    if (dialog->cbTextFormat->itemText(i) == textFormat)
      dialog->cbTextFormat->setCurrentIndex(i);
  // show the settings dialog
  if (dialog->exec() == QDialog::Accepted) {
    // update settings
    settings.setValue(QLatin1String("BitmapFormat"), dialog->cbBitmapFormat->currentText());
    settings.setValue(QLatin1String("ShapeFormat"), dialog->cbShapeFormat->currentText());
    settings.setValue(QLatin1String("ButtonFormat"), dialog->cbButtonFormat->currentText());
    settings.setValue(QLatin1String("FontFormat"), dialog->cbFontFormat->currentText());
    settings.setValue(QLatin1String("TextFormat"), dialog->cbTextFormat->currentText());
    // sync to disk
    settings.sync();
  }
  delete dialog;
}

void MainWindow::about() {
  AboutDialog *aboutDialog = new AboutDialog(this);
  aboutDialog->exec();
  delete aboutDialog;
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
  if (event->mimeData()->urls().count() > 0) {
    QString path = event->mimeData()->urls()[0].path();
    if (path.toLower().endsWith(".swf")) {
      event->acceptProposedAction();
    }
  }
}

void MainWindow::dropEvent(QDropEvent *event) {
  if (event->mimeData()->urls().count() > 0) {
    QString path = event->mimeData()->urls()[0].path();
    if (path.toLower().endsWith(".swf")) {
      open(path);
      event->acceptProposedAction();
    }
  }
}

void MainWindow::currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous) {
  // clear info area
  for (int i = 0; i < lstPropertyWidgets.size(); ++i)
    delete lstPropertyWidgets.at(i);
  lstPropertyWidgets.clear();
  if (current == 0 || current == twiFrames || current == twiBitmaps || current == twiShapes || current == twiButtons || current == twiText || current == twiSounds) {
    // disable export selected
    ui->actionExportSelected->setEnabled(false);
    // clear preview
    ui->pwMain->setPixmap(QPixmap());
    return;
  }
  // enable export selected
  ui->actionExportSelected->setEnabled(true);

  QPixmap preview;
  QVector<QPair<QString, QString> > properties;
  if (current->text(0).startsWith("Frame")) {
    // get swiff frame data
    Swiff::Frame *frame = 0;
    if (current->data(0, Qt::UserRole).canConvert<qulonglong>())
      frame = reinterpret_cast<Swiff::Frame *>(current->data(0, Qt::UserRole).value<qulonglong>());
    if (!frame)
      return;
    // update the current frame preview
    preview = SwiffExporter::toPixmap(frame);
    // fill properties array
    properties.append(QPair<QString, QString>(QString("Name"), current->text(0)));
    properties.append(QPair<QString, QString>(QString("Width"), QString::number(preview.width())));
    properties.append(QPair<QString, QString>(QString("Height"), QString::number(preview.height())));
  } else {
    // get swiff item data
    Swiff::Item *item = 0;
    if (current->data(0, Qt::UserRole).canConvert<qulonglong>())
      item = reinterpret_cast<Swiff::Item *>(current->data(0, Qt::UserRole).value<qulonglong>());
    if (!item)
      return;
    // update preview
    preview = SwiffExporter::toPixmap(item);
    // fill properties array
    properties = item->properties();
    properties.prepend(QPair<QString, QString>(QString("Name"), current->text(0)));
  }
  // gather item info
  QString info("<table style='width: 100%; height: 100%;'>");
  QString row("<tr><td align='right'><b>%1:</b></td><td> %2</td></tr>");
  // construct the info string
  for (int i = 0; i < properties.size(); ++i) {
    // create label
    QLabel *label = new QLabel(this);
    label->setText("<b>" + properties.at(i).first + ":</b>");
    // create the line edit
    QLineEdit *lineEdit = new QLineEdit(this);
    lineEdit->setFrame(false);
    lineEdit->setReadOnly(true);
    lineEdit->setText(properties.at(i).second);
    // add widgets to the list
    lstPropertyWidgets.append(label);
    lstPropertyWidgets.append(lineEdit);
    // add widgets to the user interface
    ui->layoutProperties->addRow(label, lineEdit);
  }
  // update preview
  ui->pwMain->setPixmap(preview);
}

QTreeWidgetItem *MainWindow::createTreeWidgetItem(Swiff::Item *item, QString name) {
  QTreeWidgetItem *twi = new QTreeWidgetItem();
  // set name
  twi->setText(0, name);
  // append item to the tree widget item
  twi->setData(0, Qt::UserRole, reinterpret_cast<qulonglong>(item));
  // return tree widget item
  return twi;
}

QTreeWidgetItem *MainWindow::createTreeWidgetItem(Swiff::Frame *frame, QString name) {
  QTreeWidgetItem *twi = new QTreeWidgetItem();
  // set name
  twi->setText(0, name);
  // append item to the tree widget item
  twi->setData(0, Qt::UserRole, reinterpret_cast<qulonglong>(frame));
  // return tree widget item
  return twi;
}

void MainWindow::exportItem(QTreeWidgetItem *twi, QString folder) {
  // if destination folder is empty, return
  if (folder.isEmpty())
    return;
  // get swiff item data
  Swiff::Item *item = 0;
  if (twi->data(0, Qt::UserRole).canConvert<qulonglong>())
    item = reinterpret_cast<Swiff::Item *>(twi->data(0, Qt::UserRole).value<qulonglong>());
  if (!item)
    return;
  // create settings object
  QSettings settings(QSettings::NativeFormat, QSettings::UserScope, QLatin1String("Swiff Solutions"), QLatin1String("Swiff Spy"));
  QString format;
  // read format preference
  switch (item->mItemType) {
  case Swiff::ItemBitmap:
    format = settings.value(QLatin1String("BitmapFormat"), QLatin1String("PNG")).toString();
    break;
  case Swiff::ItemShape:
    format = settings.value(QLatin1String("ShapeFormat"), QLatin1String("SVG")).toString();
    break;
  case Swiff::ItemButton:
    format = settings.value(QLatin1String("ButtonFormat"), QLatin1String("SVG")).toString();
    break;
  case Swiff::ItemFont:
    format = settings.value(QLatin1String("FontFormat"), QLatin1String("SVG")).toString();
    break;
  case Swiff::ItemText:
    format = settings.value(QLatin1String("TextFormat"), QLatin1String("TXT")).toString();
    break;
  case Swiff::ItemSound: {
      Swiff::Sound *sound = static_cast<Swiff::Sound *>(item);
      if (sound->format() == 0)
        format = QLatin1String("WAV");
      else if (sound->format() == 2)
        format = QLatin1String("MP3");
    }
    break;
  case Swiff::ItemVideo:
    format = settings.value(QLatin1String("VideoFormat"), QLatin1String("FLV")).toString();
    break;
  default:
    break;
  }
  // TODO: ask for confirmation with file previews to override when target file exists
  QString path = QString("%1/%2.%3").arg(folder).arg(twi->text(0)).arg(format.toLower());
  // export the item
  switch (item->mItemType) {
  case Swiff::ItemBitmap:
  case Swiff::ItemShape:
  case Swiff::ItemButton:
  case Swiff::ItemFont:
  case Swiff::ItemText:
    SwiffExporter::exportItem(item, path, format.toLatin1());
    break;
  case Swiff::ItemSound:
    SwiffExporter::exportSound(static_cast<Swiff::Sound *>(item), path, format.toLatin1());
    break;
  case Swiff::ItemVideo:
    SwiffExporter::exportVideo(static_cast<Swiff::Video *>(item), path, format.toLatin1());
    break;
  default:
    break;
  }
}

void MainWindow::exportFrame(QTreeWidgetItem *twi, QString folder) {
  // if destination folder is empty, return
  if (folder.isEmpty())
    return;
  // get swiff frame data
  Swiff::Frame *frame = 0;
  if (twi->data(0, Qt::UserRole).canConvert<qulonglong>())
    frame = reinterpret_cast<Swiff::Frame *>(twi->data(0, Qt::UserRole).value<qulonglong>());
  if (!frame)
    return;
  // create settings object
  QSettings settings(QSettings::NativeFormat, QSettings::UserScope, QLatin1String("Swiff Solutions"), QLatin1String("Swiff Spy"));
  QString format = settings.value(QLatin1String("FrameFormat"), QLatin1String("PNG")).toString();
  // TODO: ask for confirmation with file previews to override when target file exists
  QString path = QString("%1/%2.%3").arg(folder).arg(twi->text(0)).arg(format.toLower());
  // export the frame
  SwiffExporter::exportFrame(frame, path, format.toLatin1());
}
