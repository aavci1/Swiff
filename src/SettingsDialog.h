#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "ui_SettingsDialog.h"

class SettingsDialog : public QDialog, public Ui::SettingsDialog {
  Q_OBJECT
public:
  SettingsDialog(QWidget *parent = 0);
};

#endif // SETTINGSDIALOG_H
