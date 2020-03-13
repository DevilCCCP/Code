#include <QDir>
#include <QStandardPaths>

#include "MainWindow.h"
#include "ui_MainWindow.h"


const int kMaxRecentList = 10;

MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent), ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  ui->widgetWrite->setEnabled(false);
  ui->pushButtonStop->setVisible(false);

  QString iniFilePath = QDir(QStandardPaths::writableLocation(
#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
                               QStandardPaths::AppDataLocation
#else
                               QStandardPaths::DataLocation
#endif
                               )).absoluteFilePath("main_wnd.ini");
  mSettings = new QSettings(iniFilePath, QSettings::IniFormat, this);
  mReplyList = mSettings->value("RecentReply").toStringList();
  mWriteList = mSettings->value("RecentWrite").toStringList();
  foreach (const QString& text, mReplyList) {
    ui->comboBoxReply->addItem(text);
  }
  if (!mReplyList.isEmpty()) {
    ui->comboBoxReply->setCurrentText(mReplyList.first());
  }
  foreach (const QString& text, mWriteList) {
    ui->comboBoxWrite->addItem(text);
  }
  if (!mWriteList.isEmpty()) {
    ui->comboBoxWrite->setCurrentText(mWriteList.first());
  }

  mTcpServer = new QTcpServer(this);
  mCurrentConnection = nullptr;
  connect(mTcpServer, &QTcpServer::newConnection, this, &MainWindow::OnNewConnection);
}

MainWindow::~MainWindow()
{
  if (mCurrentConnection) {
    mCurrentConnection->disconnectFromHost();
    mCurrentConnection = nullptr;
  }
  delete ui;
}

void MainWindow::UpdateLog()
{
  ui->textEditLog->setHtml(ui->textEditLog->toHtml() + mLog);
  QTextCursor cursor = ui->textEditLog->textCursor();
  cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
  ui->textEditLog->setTextCursor(cursor);
  mLog.clear();
}

void MainWindow::Write(const QString& text)
{
  QString sendText = text;
  sendText.replace("\\n", "\r\n");
  bool disconnect = sendText.contains("\\d");
  if (disconnect) {
    sendText.remove("\\d");
  }
  mCurrentConnection->write(sendText.toUtf8());
  mLog.append(QString("<p style=\"color:blue\">%1</p>").arg(text));
  if (disconnect) {
    on_pushButtonDisconnect_clicked();
  } else {
    UpdateLog();
  }
}

void MainWindow::AddToReplyRecent()
{
  const QString& text = ui->comboBoxReply->currentText();
  if (!mReplyList.isEmpty() && mReplyList.first() == text) {
    return;
  }

  for (int i = 0; i < mReplyList.size(); i++) {
    if (mReplyList.at(i) == text) {
      mReplyList.removeAt(i);
      break;
    }
  }

  if (mReplyList.size() >= kMaxRecentList) {
    mReplyList.removeLast();
  }
  mReplyList.prepend(text);

  for (int i = 0; i < mReplyList.size(); i++) {
    if (i < ui->comboBoxReply->count()) {
      ui->comboBoxReply->setItemText(i, mReplyList.at(i));
    } else {
      ui->comboBoxReply->addItem(mReplyList.at(i));
    }
  }
  ui->comboBoxReply->setCurrentIndex(0);
  mSettings->setValue("RecentReply", mReplyList);
  mSettings->sync();
}

void MainWindow::AddToWriteRecent()
{
  const QString& text = ui->comboBoxWrite->currentText();
  if (!mWriteList.isEmpty() && mWriteList.first() == text) {
    return;
  }

  for (int i = 0; i < mWriteList.size(); i++) {
    if (mWriteList.at(i) == text) {
      mWriteList.removeAt(i);
      break;
    }
  }

  if (mWriteList.size() >= kMaxRecentList) {
    mWriteList.removeLast();
  }
  mWriteList.prepend(text);

  for (int i = 0; i < mWriteList.size(); i++) {
    if (i < ui->comboBoxWrite->count()) {
      ui->comboBoxWrite->setItemText(i, mWriteList.at(i));
    } else {
      ui->comboBoxWrite->addItem(mWriteList.at(i));
    }
  }
  ui->comboBoxWrite->setCurrentIndex(0);
  mSettings->setValue("RecentWrite", mWriteList);
  mSettings->sync();
}

void MainWindow::OnNewConnection()
{
  if (!mCurrentConnection) {
    mCurrentConnection = mTcpServer->nextPendingConnection();
    connect(mCurrentConnection, &QTcpSocket::readyRead, this, &MainWindow::OnReadyRead);
    connect(mCurrentConnection, &QTcpSocket::disconnected, this, &MainWindow::OnDisconnected);
    mLog.append(QString("<p style=\"color:green\">connection from %1:%2</p>")
                .arg(mCurrentConnection->peerAddress().toString()).arg(mCurrentConnection->peerPort()));

    UpdateLog();
  }

  ui->widgetWrite->setEnabled(mCurrentConnection != nullptr);
}

void MainWindow::OnReadyRead()
{
  QByteArray data = mCurrentConnection->readAll();
  mLog.append(QString("<p>%1</p>").arg(QString::fromUtf8(data)));

  if (!ui->comboBoxReply->currentText().isEmpty()) {
    AddToReplyRecent();
    Write(ui->comboBoxReply->currentText());
    if (ui->checkBoxAutoDisconnect->isChecked()) {
      on_pushButtonDisconnect_clicked();
    }
  } else {
    UpdateLog();
  }
}

void MainWindow::OnDisconnected()
{
  mLog.append(QString("<p>disconnected</p>"));
  UpdateLog();
  mCurrentConnection = nullptr;
  ui->widgetWrite->setEnabled(false);
}

void MainWindow::on_pushButtonStart_clicked()
{
  AddToReplyRecent();
  int port = ui->spinBoxPort->value();
  mTcpServer->listen(QHostAddress::Any, port);
  mLog.clear();
  UpdateLog();
  ui->pushButtonStart->setVisible(false);
  ui->pushButtonStop->setVisible(true);
}

void MainWindow::on_pushButtonStop_clicked()
{
  mCurrentConnection->disconnect();
  mCurrentConnection->disconnectFromHost();
  mCurrentConnection = nullptr;
  mTcpServer->close();
  ui->pushButtonStart->setVisible(true);
  ui->pushButtonStop->setVisible(false);
}

void MainWindow::on_pushButtonWrite_clicked()
{
  AddToWriteRecent();
  Write(ui->comboBoxWrite->currentText());
}

void MainWindow::on_pushButtonDisconnect_clicked()
{
  if (mCurrentConnection) {
    mCurrentConnection->disconnectFromHost();
    mCurrentConnection = nullptr;
    mLog.append(QString("<p style=\"color:blue\">disconnected</p>"));
    UpdateLog();
  }
  ui->widgetWrite->setEnabled(false);
}
