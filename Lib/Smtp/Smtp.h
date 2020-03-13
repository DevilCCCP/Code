#pragma once

#include <QByteArray>

#include <Lib/Net/SyncSocket.h>

#include "SmtpMail.h"


DefineClassS(Smtp);
DefineClassS(QTcpSocket);

class Smtp: public SyncSocket
{
public:
  enum EType {
    eSsl,
    ePlane
  };

private:
  PROPERTY_GET_SET(QString, SmtpServer)
  PROPERTY_GET_SET(int,     SmtpPort)
  PROPERTY_GET_SET(EType,   SmtpType)
  PROPERTY_GET_SET(QString, SmtpEhlo)

  PROPERTY_GET_SET(QString, UserDomain)
  PROPERTY_GET_SET(QString, UserName)
  PROPERTY_GET_SET(QString, UserPass)

  PROPERTY_GET_SET(int,     ConnectTimeout)
  PROPERTY_GET_SET(int,     RespondTimeout)

  QTcpSocketS  mSocket;
  bool         mConnectWarning;
  QByteArray   mRespond;
  QByteArray   mRespondLine;
  int          mRespondCode;
  QString      mErrorMsg;

public:
  /*new */virtual const char* Name() { return "Smtp"; }
  /*new */virtual const char* ShortName() { return "@"; }
protected:
  /*new */virtual bool DoCircle() { return false; }

public:
  bool SendMail(const SmtpMail& mail, QString* errorMsg = nullptr);
  bool SendMailTo(const QString& userName, const QString& userPass, const SmtpMail& mail);

  bool SaveMail(const SmtpMail& mail, QByteArray& output, QString* errorMsg = nullptr);

private:
  void SetDefaults();

  bool Prepare();
  bool Handshake();
  bool Login();
  bool Send(const SmtpMail& mail);

  bool GetRespond(bool required = true);
  bool GetRespondLine(int& index);
  void GetRespondCode();
  bool SendRequest(const QByteArray& request);

  bool MakeMime(const SmtpMail& mail, QByteArray& mimeData);
  void MakeMimeTo(const char* toText, const QStringList& toList, QByteArray& mimeData);

  static QByteArray GenerateBoundary();
  static QByteArray ToMimeUtf8(const QString& text);
  static QByteArray MimeNameUtf8(const QString& email);
  static QByteArray MimeMail8Bit(const QString& email);

  bool ValidateAnswer(const char* stateText, int expectedCode);

public:
  Smtp(const QString& _SmtpServer, int _SmtpPort);
  Smtp();
  ~Smtp();
};

