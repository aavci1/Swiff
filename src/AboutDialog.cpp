#include "AboutDialog.h"

AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent, Qt::Dialog | Qt::WindowCloseButtonHint), ui(new Ui::AboutDialog) {
  ui->setupUi(this);
}

AboutDialog::~AboutDialog() {
  delete ui;
}
