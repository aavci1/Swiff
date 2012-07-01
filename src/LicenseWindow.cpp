#include "LicenseWindow.h"

#include <QCryptographicHash>
#include <QDate>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include <QUrl>

LicenseWindow::LicenseWindow(QWidget *parent) : QMainWindow(parent), mMajorVersion(1), mMinorVersion(0), mHasLicense(false), mHasTrial(false) {
  setupUi(this);
  // move window to desktop center
  move (QApplication::desktop()->geometry().center() - rect().center());
  // check license status
  check();
  // check trial license
  if (hasTrial()) {
    // we have a valid trial license
    lblRemainingDays->setText(tr("You have <b>%1</b> days left in your evaluation period.").arg(30 - mFirstRun.daysTo(QDate::currentDate())));
  } else {
    pbEvaluate->setEnabled(false);
    lblRemainingDays->setText(tr("Your evaluation period has expired!"));
  }
  // connect button handlers
  connect(pbEvaluate, SIGNAL(clicked()), this, SLOT(evaluate()));
  connect(pbPurchase, SIGNAL(clicked()), this, SLOT(purchase()));
  connect(pbActivate, SIGNAL(clicked()), this, SLOT(activate()));
  connect(pbDoActivate, SIGNAL(clicked()), this, SLOT(doActivate()));
  connect(pbExit, SIGNAL(clicked()), this, SLOT(evaluate()));
}

bool LicenseWindow::hasLicense() {
  return mHasLicense;
}

bool LicenseWindow::hasTrial() {
  return mHasTrial;
}

void LicenseWindow::check() {
  QString today = QDate::currentDate().toString(QLatin1String("yyyyMMdd"));
  // read current values of first run date and license from disk
  QSettings settings(QSettings::NativeFormat, QSettings::UserScope, QLatin1String("Swiff Solutions"), QLatin1String("Swiff Spy"));
  QString firstRun = settings.value(QLatin1String("FirstRun"), today).toString();
  QString license = settings.value(QLatin1String("License"), QString()).toString();
  // set first run date
  mFirstRun = QDate::fromString(firstRun, QLatin1String("yyyyMMdd"));
  // if license file doesnt exist
  if (license.isEmpty()) {
    // and this is the first run, create a 30-day trial license
    if (firstRun == today)
      license = createTrial();
  } else if (isLicenseValid(license)) {
    mHasLicense = true;
  } else if (mFirstRun.daysTo(QDate::currentDate()) >= 0 && mFirstRun.daysTo(QDate::currentDate()) <= 30) {
    mHasTrial = true;
  }
  // write last values of first run and license to disk
  settings.setValue(QLatin1String("FirstRun"), firstRun);
  settings.setValue(QLatin1String("License"), license);
  settings.sync();
}

QString LicenseWindow::createTrial() {
  // check if the trial period from the first run has ended
  if (mFirstRun.daysTo(QDate::currentDate()) > 30)
    return QString();
  // check if the system date has been taken back
  if (QDate::currentDate() < QDate(2010, 7, 22))
    return QString();
  // set has trial flag
  mHasTrial = true;
  // create a 30-day trial license
  return createLicense("1111-2222-3333-4444");
}

void LicenseWindow::evaluate() {
  if (hasLicense() || hasTrial())
    emit licenseOk();
  close();
}

void LicenseWindow::purchase() {
  QDesktopServices::openUrl(QUrl("http://www.swiffsolutions.com/spy/buy.php"));
}

void LicenseWindow::activate() {
  // switch to the activation page
  swMain->setCurrentIndex(1);
}

void LicenseWindow::doActivate() {
  // create request object
  QNetworkRequest request(QUrl("http://www.swiffsolutions.com/spy/register.php"));
  request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
  // create parameters data
  QByteArray data = calculateData(QString("RGSTR"), QString::number(mMajorVersion), QString::number(mMinorVersion), leLicenseKey->text());
  // do the actual request
  QNetworkAccessManager *manager = new QNetworkAccessManager(this);
  connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(doActivateFinished(QNetworkReply*)));
  manager->post(request, data);
}

void LicenseWindow::doActivateFinished(QNetworkReply *reply) {
  QString failedMessage = tr("Activation has failed. Please, try activating again. If you have already experienced this error multiple times, please send an email containing your license key, to <a href=\'mailto:licensing@swiffsolutions.com\'><span style=\'text-decoration: underline; color:#0057ae;\'>licensing@swiffsolutions.com</span></a> and explain the situation. We will be more than happy to help you.");
  QString failedHash = hash(QString("RGSTR|%1|FAIL").arg(leLicenseKey->text()));
  QString successMessage = tr("Activation has succeeded. You can now close this window and start using your application.");
  QString successHash = hash(QString("RGSTR|%1|SUCCESS").arg(leLicenseKey->text()));
  // switch to the result page
  swMain->setCurrentIndex(2);
  // check reply
  QString result = reply->readAll().left(40);
  // set message
  if (result == successHash) {
    lblActivationResult->setText(successMessage);
    // create the actual license
    QSettings settings(QSettings::NativeFormat, QSettings::UserScope, QLatin1String("Swiff Solutions"), QLatin1String("Swiff Spy"));
    settings.setValue("License", createLicense(leLicenseKey->text()));
    settings.sync();
    mHasLicense = true;
  } else {
    lblActivationResult->setText(failedMessage);
  }
  // clean up
  reply->deleteLater();
}

bool LicenseWindow::isLicenseValid(QString license) {
  // check hash
  if (hash(license.left(26)) != license.right(40))
    return false;
  // check that license key is not trial key
  if (license.mid(2, 16) == QLatin1String("1111222233334444"))
    return false;
  return true;
}

QString LicenseWindow::createLicense(QString license_key) {
  // License format
  // 1 byte -> major version
  // 1 byte -> minor version
  // 16 bytes -> license key without the dashes
  // 8 bytes -> current date
  // 40 bytes -> hash of the secret key | previous 26 bytes
  QString s = QString("%1%2%3%4").arg(QString::number(mMajorVersion)).arg(QString::number(mMinorVersion)).arg(license_key.remove("-")).arg(QDate::currentDate().toString("yyyyMMdd"));
  return QString("%1%2").arg(s).arg(hash(s));
}

QByteArray LicenseWindow::calculateData(QString action, QString major_version, QString minor_version, QString license_key) {
  // register parameters
  QUrl parameters;
  parameters.addQueryItem("action",action);
  parameters.addQueryItem("major_version", major_version);
  parameters.addQueryItem("minor_version", minor_version);
  parameters.addQueryItem("license_key", license_key);
  parameters.addQueryItem("verify", hash(QString("%1|%2|%3|%4").arg(action).arg(major_version).arg(minor_version).arg(license_key)).left(8));
  // parameters data
  QByteArray data;
  data.append(parameters.toString());
  data.remove(0,1);
  // return the result
  return data;
}

QString LicenseWindow::hash(QString data) {
  // calculate the verify string
  QCryptographicHash sha1(QCryptographicHash::Sha1);
  sha1.addData(QString("B2F2F8F60C2429B3|%1").arg(data).toLatin1());
  // return the result
  return sha1.result().toHex().toUpper();
}
