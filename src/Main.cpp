#include "LicenseWindow.h"
#include "MainWindow.h"

#include <QApplication>
#include <QFile>

#ifdef Q_WS_WIN

#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR argv, INT argc) {
  QApplication ma(argc, &argv);
#else
int main(int argc, char **argv) {
  QApplication ma(argc, argv);
#endif
  LicenseWindow lw;
  MainWindow mw;
  if (ma.arguments().size() > 1) {
    mw.open(ma.arguments().at(1));
  }
  if (lw.hasLicense()) {
    mw.show();
  } else {
    // show the nag-screen
    lw.show();
    // show the main window if the license is ok
    QObject::connect(&lw, SIGNAL(licenseOk()), &mw, SLOT(show()));
  }
  // execute the main application
  return ma.exec();
}
