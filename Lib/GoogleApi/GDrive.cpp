#include <QJsonDocument>
#include <QJsonObject>

#include <Lib/Log/Log.h>

#include "GDrive.h"
#include "RestSender.h"
#include "ServiceAccounts.h"



bool GDrive::UploadSimple(const QByteArray& fileData, QString* fileId)
{
  if (!Authorize()) {
    return false;
  }

  if (!mRestSender->SendFile("https://www.googleapis.com/upload/drive/v2/files?uploadType=media", fileData)) {
    return false;
  }

  const QByteArray& respondData = mRestSender->RespondData();
  if (respondData.isEmpty()) {
    Log.Error(QString("Drive upload simple fail, empty answer"));
    return false;
  }

  QJsonParseError errJson;
  QJsonDocument doc = QJsonDocument::fromJson(respondData, &errJson);
  if (errJson.error != QJsonParseError::NoError) {
    Log.Error(QString("Drive upload simple fail, parse respond json fail (err: '%1')").arg(errJson.errorString()));
    return false;
  }

  QJsonObject root = doc.object();
  QString id = root.value("id").toString();
  if (id.isEmpty()) {
    Log.Error(QString("Drive upload simple fail, empty file id"));
    return false;
  }
  if (fileId) {
    *fileId = id;
  }
  return true;
}

bool GDrive::DownloadSimple(const QString& fileId, QByteArray* fileData)
{
  if (!Authorize()) {
    return false;
  }

  if (!mRestSender->SendRequest(QString("https://www.googleapis.com/drive/v2/files/%0?alt=media").arg(fileId))) {
    return false;
  }

  if (fileData) {
    *fileData = mRestSender->RespondData();
  }
  return true;
}

bool GDrive::DownloadParams(const QString& fileId, QByteArray* fileData)
{
  if (!Authorize()) {
    return false;
  }

  if (!mRestSender->SendRequest(QString("https://www.googleapis.com/drive/v2/files/%0").arg(fileId))) {
    return false;
  }

  if (fileData) {
    *fileData = mRestSender->RespondData();
  }
  return true;
}

bool GDrive::Share(const QString& fileId)
{
  if (!Authorize()) {
    return false;
  }

  QByteArray permissions("{\"role\": \"reader\", \"type\": \"anyone\", \"allowFileDiscovery\": false}");
  if (!mRestSender->SendJson(QString("https://www.googleapis.com/drive/v3/files/%1/permissions").arg(fileId), permissions)) {
    return false;
  }

  return true;
}

bool GDrive::Authorize()
{
  if (!mServiceAccount->IsValid(GoogleApi::DriveScope)) {
    if (!mServiceAccount->Acquire(GoogleApi::DriveScope)) {
      return false;
    }
  }

  mRestSender->SetBearer(mServiceAccount->Token());
  return true;
}


GDrive::GDrive(ServiceAccount* _ServiceAccount, RestSender* _RestSender)
  : mServiceAccount(_ServiceAccount), mRestSender(_RestSender)
{
}
