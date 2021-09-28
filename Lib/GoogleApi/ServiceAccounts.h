#pragma once

#include <QString>

#include "GoogleApi.h"


DefineClassS(Rsa);
class RestSender;

class ServiceAccount
{
  GoogleApi::Scopes mScopes;
  RestSender*       mRestSender;

  QByteArray        mClientEmail;
  RsaS              mPrivateKey;
  QString           mToken;

public:
  bool IsValid(GoogleApi::Scope scope) const;
  bool IsValid(GoogleApi::Scopes scopes) const;
  const QString& Token() const;
  void SetRestSender(RestSender* _RestSender);

public:
  void SetClientEmail(const QByteArray& clientEmail);
  bool LoadPrivateKeyPem(const QByteArray& privateKeyPem);

  bool LoadFromJson(const QString& filePath);
  bool LoadFromJson(const QByteArray& jsonData);
  bool Acquire(GoogleApi::Scope scope);
  bool Acquire(GoogleApi::Scopes scopes);
  bool Acquire();

public:
  ServiceAccount();
};
