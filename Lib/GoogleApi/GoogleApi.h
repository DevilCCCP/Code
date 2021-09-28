#pragma once

#include <QElapsedTimer>

#include <Lib/Include/Common.h>


DefineClassS(ServiceAccount);
DefineClassS(RestSender);
DefineClassS(GDrive);
DefineClassS(GSheet);

class GoogleApi
{
  ServiceAccountS mServiceAccounts;
  RestSenderS     mRestSender;
  bool            mDebug;

  GDriveS         mDrive;
  GSheetS         mSheet;

public:
  enum Scope {
    NoScope = 0,
    DriveScope = 1 << 0,
    SheetScope = 1 << 1
  };
  Q_DECLARE_FLAGS(Scopes, Scope)
  Scopes          mScopes;

public:
  void SetDebug(bool debug);
  void SetScopes(GoogleApi::Scopes scopes);
  const GDriveS& Drive();
  const GSheetS& Sheet();

private:
  ServiceAccount* getServiceAccount();
  RestSender* getRestSender();

public:
  void SetClientEmail(const QByteArray& clientEmail);
  bool LoadPrivateKeyPem(const QByteArray& privateKeyPem);
  bool LoadFromJson(const QString& filePath);

  bool Authorize(const QString& filePath);
  bool Authorize();

public:
  GoogleApi();
  virtual ~GoogleApi();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(GoogleApi::Scopes)
