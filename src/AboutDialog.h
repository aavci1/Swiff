#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QtGui/QDialog>

#include "ui_AboutDialog.h"

class AboutDialog : public QDialog {
  Q_OBJECT
public:
  AboutDialog(QWidget *parent = 0);
  ~AboutDialog();

private:
  Ui::AboutDialog *ui;
};

#endif // ABOUTDIALOG_H
