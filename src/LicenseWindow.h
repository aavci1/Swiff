#ifndef LICENSEWINDOW_H
#define LICENSEWINDOW_H

#include "ui_LicenseWindow.h"

#include <QDate>

class QNetworkReply;

class LicenseWindow : public QMainWindow, private Ui::LicenseWindow {
  Q_OBJECT
public:
  LicenseWindow(QWidget *parent = 0);

  /** \returns true if we have a valid commercial license, false otherwise. */
  bool hasLicense();
  /** \returns true if we have a valid trial license, false otherwise. */
  bool hasTrial();

signals:
  /** Emitted on close if current license is ok to continue with. */
  void licenseOk();
private slots:
  /** Checks the license file and if doesnt exist or not valid, creates a new trial license. */
  void check();
  /** Emits the licenseOk signal if needed then closes the window. */
  void evaluate();
  /** Opens a new web page to buy a new commercial license. */
  void purchase();
  /** Switches to the activate page. */
  void activate();
  /** Does the actual activation and license creation. */
  void doActivate();
  /** Generates the license and saves on disk. */
  void doActivateFinished(QNetworkReply *reply);
private:
  /** \returns true if \p license is a valid commercial license string. */
  bool isLicenseValid(QString license);
  /** Creates a new 30-day trial license. */
  QString createTrial();
  /** Creates a new license based on given parameters. */
  QString createLicense(QString license_key);
  QByteArray calculateData(QString action, QString major_version, QString minor_version, QString license_key);
  /** Hashes given secret key with secret key to produce 40 byte hash. */
  QString hash(QString data);

  int mMajorVersion;
  int mMinorVersion;
  QDate mFirstRun;
  bool mHasLicense;
  bool mHasTrial;
};

#endif // LICENSEWINDOW_H
