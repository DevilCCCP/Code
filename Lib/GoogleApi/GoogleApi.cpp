#include <Lib/Log/Log.h>

#include "GoogleApi.h"
#include "ServiceAccounts.h"
#include "RestSender.h"
#include "GDrive.h"
#include "GSheet.h"


void GoogleApi::SetDebug(bool debug)
{
  mDebug = debug;
  if (mRestSender) {
    mRestSender->setDebug(mDebug);
  }
}

void GoogleApi::SetScopes(GoogleApi::Scopes scopes)
{
  mScopes = scopes;
}

const GDriveS& GoogleApi::Drive()
{
  if (!mDrive) {
    mScopes.setFlag(DriveScope);
    mDrive.reset(new GDrive(getServiceAccount(), getRestSender()));
  }

  return mDrive;
}

const GSheetS& GoogleApi::Sheet()
{
  if (!mSheet) {
    mScopes.setFlag(SheetScope);
    mSheet.reset(new GSheet(getServiceAccount(), getRestSender()));
  }

  return mSheet;
}

ServiceAccount* GoogleApi::getServiceAccount()
{
  return mServiceAccounts.data();
}

RestSender* GoogleApi::getRestSender()
{
  if (!mRestSender) {
    mRestSender.reset(new RestSender());
    mRestSender->setDebug(mDebug);
  }

  return mRestSender.data();
}

void GoogleApi::SetClientEmail(const QByteArray& clientEmail)
{
  return mServiceAccounts->SetClientEmail(clientEmail);
}

bool GoogleApi::LoadPrivateKeyPem(const QByteArray& privateKeyPem)
{
  return mServiceAccounts->LoadPrivateKeyPem(privateKeyPem);
}

bool GoogleApi::LoadFromJson(const QString& filePath)
{
  return mServiceAccounts->LoadFromJson(filePath);
}

bool GoogleApi::Authorize(const QString& filePath)
{
  if (!LoadFromJson(filePath)) {
    return false;
  }

  return Authorize();
}

bool GoogleApi::Authorize()
{
  if (mScopes == NoScope) {
    Log.Warning(QString("No scope to authorize to"));
    return false;
  }

  if (mServiceAccounts->IsValid((GoogleApi::Scope)(GoogleApi::Scopes::Int)mScopes)) {
    return true;
  }

  mServiceAccounts->SetRestSender(getRestSender());
  return mServiceAccounts->Acquire(mScopes);
}


GoogleApi::GoogleApi()
  : mServiceAccounts(new ServiceAccount()), mDebug(false)
{
}

GoogleApi::~GoogleApi()
{
}
