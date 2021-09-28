#include <QHostInfo>
#include <QElapsedTimer>
#include <QRegExp>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
#include <QRandomGenerator>
#endif

#include <Lib/Log/Log.h>
#include <Lib/Net/QSslSocket2.h>

#include "Smtp.h"


const int kWorkCircleMs = 500;
const int kDefaultConnectTimeout = 1 * 60 * 1000;
const int kDefaultRespondTimeout = 1 * 60 * 1000;

bool Smtp::SendMail(const SmtpMail& mail, QString* errorMsg)
{
  mErrorMsg.clear();

  if (Prepare() && Handshake() && Login() && Send(mail)) {
    return true;
  }

  if (errorMsg) {
    *errorMsg = mErrorMsg;
  }
  if (!mErrorMsg.isEmpty()) {
    Log.Warning(QString("Smtp send fail (%1)").arg(mErrorMsg));
  }
  return false;
}

bool Smtp::SendMailTo(const QString& userName, const QString& userPass, const SmtpMail& mail)
{
  setUserName(userName);
  setUserPass(userPass);

  return SendMail(mail);
}

bool Smtp::SaveMail(const SmtpMail& mail, QByteArray& output, QString* errorMsg)
{
  if (!MakeMime(mail, output)) {
    Log.Warning(QString("SMTP save fail: make mime message"));
    if (errorMsg) {
      *errorMsg = "Make mime fail";
    }
    return false;
  }

  return true;
}

void Smtp::SetDefaults()
{
  mSmtpType = eSsl;
  mSmtpEhlo = "[:Ip:]";

#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
  qsrand(QDateTime::currentMSecsSinceEpoch());
#endif

  setConnectTimeout(kDefaultConnectTimeout);
  setRespondTimeout(kDefaultRespondTimeout);

  SetReadCircleMs(kWorkCircleMs);
}

bool Smtp::Prepare()
{
  if (mSmtpServer.isEmpty()) {
    mErrorMsg = "server not set";
    return false;
  }
  if (!mSmtpPort) {
    mErrorMsg = "server port not set";
    return false;
  }

  if (mSmtpType == eSsl) {
    QSslSocket2* socket;
    mSocket.reset(socket = new QSslSocket2());
    socket->connectToHostEncrypted(mSmtpServer, mSmtpPort);
  } else {
    mSocket.reset(new QTcpSocket());
    mSocket->connectToHost(mSmtpServer, mSmtpPort);
  }
  QElapsedTimer timer;
  timer.start();
  while (!mSocket->waitForConnected(10000)) {
    if (mSocket->state() == QAbstractSocket::UnconnectedState) {
      if (!mConnectWarning) {
        mErrorMsg = QString("Connect fail: '%1' (%2)").arg(mSocket->errorString()).arg(mSocket->error());
        mConnectWarning = true;
      }
      return false;
    }
    if (!SayWork()) {
      return false;
    }
    if (timer.elapsed() >= mConnectTimeout) {
      if (!mConnectWarning) {
        mErrorMsg = QString("SMTP send fail: connect to server timeout");
        mConnectWarning = true;
      }
      return false;
    }
  }

  SetTcpSocket(mSocket);
  if (!GetRespond()) {
    return false;
  }
  if (!ValidateAnswer("init", 220)) {
    return false;
  }
  return true;
}

bool Smtp::Handshake()
{
  QString clientAddr = mSmtpEhlo.toUtf8();
  if (clientAddr.contains(":Ip:")) {
    clientAddr.replace(":Ip:", mSocket->localAddress().toString());
  }
  if (clientAddr.contains(":Hostname:")) {
    clientAddr.replace(":Hostname:", QHostInfo::localHostName());
  }
  if (!SendRequest(QByteArray("EHLO ") + clientAddr.toUtf8() + "\r\n")) {
    return false;
  }
  if (!GetRespond() || !ValidateAnswer("handshake", 250)) {
    return false;
  }
  while (GetRespond(false)) {
    if (!ValidateAnswer("handshake", 250)) {
      return false;
    }
  }
  return true;
}

bool Smtp::Login()
{
  if (mUserName.isEmpty() || mUserPass.isEmpty()) {
    return true;
  }

  if (!SendRequest("AUTH LOGIN\r\n")) {
    return false;
  }
  if (!GetRespond() || !ValidateAnswer("auth init", 334)) {
    return false;
  }

  if (!SendRequest(mUserName.toUtf8().toBase64() + "\r\n")) {
    return false;
  }
  if (!GetRespond() || !ValidateAnswer("auth user", 334)) {
    return false;
  }

  if (!SendRequest(mUserPass.toUtf8().toBase64() + "\r\n")) {
    return false;
  }
  if (!GetRespond() || !ValidateAnswer("auth pass", 235)) {
    return false;
  }

  return true;
}

bool Smtp::Send(const SmtpMail& mail)
{
  if (!SendRequest(QByteArray("MAIL FROM:") + MimeMail8Bit(mail.getFrom()) + "\r\n")) {
    return false;
  }
  if (!GetRespond() || !ValidateAnswer("mail from", 250)) {
    return false;
  }

  QStringList rcpts = QStringList() << mail.getTo() << mail.getCc() << mail.getBcc();
  for (auto itr = rcpts.begin(); itr != rcpts.end(); itr++) {
    const QString& rcpt = *itr;
    if (!SendRequest(QByteArray("RCPT TO:") + MimeMail8Bit(rcpt) + "\r\n")) {
      return false;
    }
    if (!GetRespond() || !ValidateAnswer("mail rcpt", 250)) {
      return false;
    }
  }

  if (!SendRequest("DATA\r\n")) {
    return false;
  }
  if (!GetRespond() || !ValidateAnswer("data init", 354)) {
    return false;
  }

  QByteArray mimeMail;
  if (!MakeMime(mail, mimeMail)) {
    Log.Warning(QString("SMTP send fail: make mime message"));
    return false;
  }
  mimeMail.append("\r\n.\r\n");
  if (!SendRequest(mimeMail)) {
    return false;
  }
  if (!GetRespond() || !ValidateAnswer("data done", 250)) {
    return false;
  }

  if (!SendRequest("QUIT\r\n")) {
    return false;
  }
  if (!GetRespond() || !ValidateAnswer("close", 221)) {
    return false;
  }

  mConnectWarning = false;
  return true;
}

bool Smtp::GetRespond(bool required)
{
  int index = 0;
  if (GetRespondLine(index)) {
    return true;
  } else if (!required) {
    return false;
  }

  QElapsedTimer timer;
  timer.start();

  forever {
    while (!ReadyRead()) {
      if (timer.elapsed() > mRespondTimeout) {
        mErrorMsg = QString("wait respond timeout");
        return false;
      }
    }
    if (!ReadData(mRespond)) {
      mErrorMsg = QString("Read data fail, error: '%1')").arg(SocketErrorString());
      return false;
    }
    if (GetRespondLine(index)) {
      return true;
    }
  }
}

bool Smtp::GetRespondLine(int& index)
{
  int indEol = mRespond.indexOf("\r\n", index);
  if (indEol >= 0) {
    GetRespondCode();
    Log.Trace(QString("SMTP S: ") + QString::fromUtf8(mRespond.left(indEol)));
    if (mRespond.startsWith("334")) {
      Log.Trace(QString::fromUtf8(QByteArray::fromBase64(mRespond.mid(4, indEol - 4))));
    }
    mRespondLine = mRespond.left(indEol);
    mRespond = mRespond.mid(indEol + 2);
    return true;
  } else {
    index = mRespond.size();
    if (index > 0) {
      index--;
    }
    return false;
  }
}

void Smtp::GetRespondCode()
{
  QByteArray code;
  for (int i = 0; i < mRespond.size(); i++) {
    char ch = mRespond[i];
    if (ch >= '0' && ch <= '9') {
      code.append(ch);
    } else {
      break;
    }
  }
  if (code.size() > 0) {
    mRespondCode = code.toInt();
  } else {
    mRespondCode = 0;
  }
}

bool Smtp::SendRequest(const QByteArray& request)
{
  Log.Trace(QString("SMTP C: ") + QString::fromUtf8(request));
  return WriteData(request);
}

bool Smtp::MakeMime(const SmtpMail& mail, QByteArray& mimeData)
{
  QByteArray boundary = GenerateBoundary();
  mimeData = QByteArray("From: ") + MimeNameUtf8(mail.getFrom().toUtf8()) + "\r\n";

  MakeMimeTo("To", mail.getTo(), mimeData);
  MakeMimeTo("Cc", mail.getCc(), mimeData);

  mimeData += QByteArray("Subject: ") + ToMimeUtf8(mail.getSubject()) + "\r\n";
  mimeData += "Content-Type: multipart/mixed;\r\n"
              " boundary=\"" + boundary + "\"\r\n";
  mimeData += "MIME-Version: 1.0\r\n\r\n";

  mimeData += "--" + boundary + "\r\n";
  mimeData += (mail.getBodyHtml())? "Content-Type: text/html; charset=utf-8\r\n\r\n"
                                  : "Content-Type: text/plain; charset=utf-8\r\n\r\n";
  mimeData += mail.getBody().toUtf8();
  mimeData += "\r\n";

  const SmtpMail::AttachList& attaches = mail.getAttaches();
  for (auto itr = attaches.begin(); itr != attaches.end(); itr++) {
    const SmtpMail::Attach& attach = *itr;

    mimeData += "\r\n--" + boundary + "\r\n";
    mimeData += "Content-Type: ";
    mimeData += (attach.getMimeType().isEmpty()? QByteArray("application/octet-stream"): attach.getMimeType().toLatin1());
    mimeData += "; name=\"" + ToMimeUtf8(attach.getName()) + "\"\r\n";
    mimeData += "Content-Description: " + ToMimeUtf8(attach.getName()) + "\r\n";
    mimeData += "Content-Disposition: attachment; filename=\"" + ToMimeUtf8(attach.getName())
        + "\"; size=" + QByteArray::number(attach.getData().size()) + ";\r\n";
    mimeData += "Content-Transfer-Encoding: base64\r\n\r\n";
    mimeData += attach.getData().toBase64();
    mimeData += "\r\n";
  }

  mimeData += "\r\n--" + boundary + "--\r\n";
  return true;
}

void Smtp::MakeMimeTo(const char* toText, const QStringList& toList, QByteArray& mimeData)
{
  if (toList.isEmpty()) {
    return;
  }

  mimeData += QByteArray(toText) + ": ";
  bool first = true;
  for (auto itr = toList.begin(); itr != toList.end(); itr++) {
    const QString& toOne = *itr;
    if (first) {
      first = false;
    } else {
      mimeData += ", ";
    }
    mimeData += MimeNameUtf8(toOne);
  }
  mimeData += "\r\n";
}

QByteArray Smtp::GenerateBoundary()
{
  QByteArray boundary;
  for (int i = 0; i < 16; i += 2) {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    static QRandomGenerator gRandomGenerator(QDateTime::currentMSecsSinceEpoch());
    int r = gRandomGenerator.bounded(1 << 16);
#else
    int r = qrand();
#endif
    boundary.append((char)(uchar)(r));
    boundary.append((char)(uchar)(r >> 8));
  }
  return boundary.toHex();
}

QByteArray Smtp::ToMimeUtf8(const QString& text)
{
  QByteArray latin1;
  for (int i = 0; i < text.size(); i++) {
    if (char ch = text[i].toLatin1()) {
      latin1.append(ch);
    } else {
      return QByteArray("=?utf-8?B?") + text.toUtf8().toBase64() + "?=";
    }
  }
  return latin1;
}

QByteArray Smtp::MimeNameUtf8(const QString& email)
{
  if (email.contains(QRegExp("*<*>*", Qt::CaseSensitive, QRegExp::Wildcard))) {
    int ind1 = email.indexOf('<');
    if (ind1 > 0) {
      return ToMimeUtf8(email.left(ind1).trimmed()) + " " + email.mid(ind1).toUtf8();
    } else {
      return ToMimeUtf8(email);
    }
  } else {
    return QByteArray("<") + email.toUtf8() + QByteArray(">");
  }
}

QByteArray Smtp::MimeMail8Bit(const QString& email)
{
  if (email.startsWith('<') && email.endsWith('>')) {
    return email.toLatin1();
  }

  int ind1 = email.indexOf('<');
  int ind2 = email.indexOf('>');
  if (ind1 < 0 || ind2 < 0) {
    return QByteArray("<") + email.toUtf8() + QByteArray(">");
  }
  QString mail = email.mid(ind1, ind2 - ind1 + 1);
  return mail.toLatin1();
}

bool Smtp::ValidateAnswer(const char* stateText, int expectedCode)
{
  if (mRespondCode == expectedCode) {
    return true;
  }
  mErrorMsg = QString("Wrong answer in %1 state; expected: %2, result: '%3'").arg(stateText).arg(expectedCode).arg(mRespondLine.constData());
  return false;
}


Smtp::Smtp(const QString& _SmtpServer, int _SmtpPort)
  : mSmtpServer(_SmtpServer), mSmtpPort(_SmtpPort)
{
  SetDefaults();
}

Smtp::Smtp()
  : mSmtpPort(0)
  , mConnectWarning(false)
{
  SetDefaults();
}

Smtp::~Smtp()
{
}
