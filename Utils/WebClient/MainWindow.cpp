#include <QMessageBox>
#include <QDir>
#include <QStandardPaths>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkProxy>
#include <QTimer>
#include <QRegExp>

#include "MainWindow.h"
#include "ui_MainWindow.h"


const int kMaxRecentList = 10;

MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent), ui(new Ui::MainWindow)
  , mNetManager(new QNetworkAccessManager(this)), mTimer(new QTimer(this))
{
  ui->setupUi(this);

  ui->pushButtonStop->setVisible(false);

  QString iniFilePath = QDir(QStandardPaths::writableLocation(
#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
                               QStandardPaths::AppDataLocation
#else
                               QStandardPaths::DataLocation
#endif
                               )).absoluteFilePath("main_wnd.ini");
  mSettings   = new QSettings(iniFilePath, QSettings::IniFormat, this);
  mOnceList   = mSettings->value("RecentOnce").toStringList();
  mPeriodList = mSettings->value("RecentPeriod").toStringList();
  mProxyType  = mSettings->value("ProxyType", QNetworkProxy::NoProxy).toInt();
  mProxyUri   = mSettings->value("ProxyUri").toString();
  foreach (const QString& text, mOnceList) {
    ui->comboBoxOnce->addItem(text);
  }
  if (!mOnceList.isEmpty()) {
    ui->comboBoxOnce->setCurrentText(mOnceList.first());
  }
  foreach (const QString& text, mPeriodList) {
    ui->comboBoxPeriod->addItem(text);
  }
  if (!mPeriodList.isEmpty()) {
    ui->comboBoxPeriod->setCurrentText(mPeriodList.first());
  }

  ui->comboBoxProxyType->addItem("No proxy", QNetworkProxy::NoProxy);
  ui->comboBoxProxyType->addItem("SOCKSv5", QNetworkProxy::Socks5Proxy);
  ui->comboBoxProxyType->setCurrentIndex(mProxyType);
  ui->lineEditProxyUri->setText(mProxyUri);

  connect(mNetManager, &QNetworkAccessManager::finished, this, &MainWindow::OnFinished);
  connect(mTimer, &QTimer::timeout, this, &MainWindow::OnSendTime);
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::UpdateLog()
{
  ui->textEditLog->setHtml(mLog);
  QTextCursor cursor = ui->textEditLog->textCursor();
  cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
  ui->textEditLog->setTextCursor(cursor);
}

void MainWindow::AddToOnceRecent()
{
  const QString& text = ui->comboBoxOnce->currentText();
  if (!mOnceList.isEmpty() && mOnceList.first() == text) {
    return;
  }

  for (int i = 0; i < mOnceList.size(); i++) {
    if (mOnceList.at(i) == text) {
      mOnceList.removeAt(i);
      break;
    }
  }

  if (mOnceList.size() >= kMaxRecentList) {
    mOnceList.removeLast();
  }
  mOnceList.prepend(text);

  for (int i = 0; i < mOnceList.size(); i++) {
    if (i < ui->comboBoxOnce->count()) {
      ui->comboBoxOnce->setItemText(i, mOnceList.at(i));
    } else {
      ui->comboBoxOnce->addItem(mOnceList.at(i));
    }
  }
  ui->comboBoxOnce->setCurrentIndex(0);
  mSettings->setValue("RecentOnce", mOnceList);
  mSettings->sync();
}

void MainWindow::AddToPeriodRecent()
{
  const QString& text = ui->comboBoxPeriod->currentText();
  if (!mPeriodList.isEmpty() && mPeriodList.first() == text) {
    return;
  }

  for (int i = 0; i < mPeriodList.size(); i++) {
    if (mPeriodList.at(i) == text) {
      mPeriodList.removeAt(i);
      break;
    }
  }

  if (mPeriodList.size() >= kMaxRecentList) {
    mPeriodList.removeLast();
  }
  mPeriodList.prepend(text);

  for (int i = 0; i < mPeriodList.size(); i++) {
    if (i < ui->comboBoxPeriod->count()) {
      ui->comboBoxPeriod->setItemText(i, mPeriodList.at(i));
    } else {
      ui->comboBoxPeriod->addItem(mPeriodList.at(i));
    }
  }
  ui->comboBoxPeriod->setCurrentIndex(0);
  mSettings->setValue("RecentPeriod", mPeriodList);
  mSettings->sync();
}

bool MainWindow::SetProxy()
{
  mProxyType = ui->comboBoxProxyType->currentIndex();
  mProxyUri  = ui->lineEditProxyUri->text();

  QNetworkProxy::ProxyType ptype = (QNetworkProxy::ProxyType)ui->comboBoxProxyType->currentData().toInt();
  QNetworkProxy proxy(ptype);
  if (ptype == QNetworkProxy::Socks5Proxy) {
    QRegExp sock5RegExp("(.+):(\\d+)");
    if (!sock5RegExp.exactMatch(mProxyUri)) {
      QMessageBox::warning(this, windowTitle(), "Proxy URI incorrect");
      return false;
    }
    QString host = sock5RegExp.cap(1);
    int port     = sock5RegExp.cap(2).toInt();
    proxy.setHostName(host);
    proxy.setPort(port);
  }

  mSettings->setValue("ProxyType", mProxyType);
  mSettings->setValue("ProxyUri", mProxyUri);
  mSettings->sync();

  mNetManager->setProxy(proxy);
  return true;
}

void MainWindow::OnFinished(QNetworkReply* netReply)
{
  if (netReply->error() != QNetworkReply::NoError) {
    mLog.append(QString("<p style=\"color:red\">network error '%1'</p>").arg(netReply->errorString()));
  } else {
    int retCode = netReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QByteArray data = netReply->readAll();
    if (!data.isEmpty()) {
      mLog.append(QString("<p>send done %1 (%2)</p>").arg(retCode).arg(QString::fromUtf8(data)));
    } else if (netReply->header(QNetworkRequest::LocationHeader).isValid()) {
      mLog.append(QString("<p>send done %1 -> %2</p>").arg(retCode).arg(netReply->header(QNetworkRequest::LocationHeader).toString()));
    } else {
      mLog.append(QString("<p>send done %1</p>").arg(retCode));
    }
  }
  UpdateLog();
}

void MainWindow::OnSendTime()
{
  AddToPeriodRecent();
  const QString& text = ui->comboBoxPeriod->currentText();
  QNetworkRequest request = QNetworkRequest(QUrl(text));
  mNetManager->get(request);

  mLog.append(QString("<p style=\"color:blue\">send %1</p>").arg(text));
  UpdateLog();
}

void MainWindow::on_pushButtonSend_clicked()
{
  if (!SetProxy()) {
    return;
  }

  AddToOnceRecent();
  const QString& text = ui->comboBoxOnce->currentText();
  QNetworkRequest request = QNetworkRequest(QUrl(text));

  mNetManager->get(request);

  mLog.append(QString("<p style=\"color:blue\">send %1</p>").arg(text));
  UpdateLog();
}

void MainWindow::on_pushButtonStart_clicked()
{
  if (!SetProxy()) {
    return;
  }

  ui->pushButtonStart->setVisible(false);
  ui->pushButtonStop->setVisible(true);
  int periodMs = ui->doubleSpinBoxPeriod->value() * 1000;
  mTimer->setInterval(periodMs);
  mTimer->start();
}

void MainWindow::on_pushButtonStop_clicked()
{
  ui->pushButtonStart->setVisible(true);
  ui->pushButtonStop->setVisible(false);
  mTimer->stop();
}
