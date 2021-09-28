#pragma once

#include <QString>


class ServiceAccount;
class RestSender;

class GDrive
{
  ServiceAccount*  mServiceAccount;
  RestSender*      mRestSender;

public:
  bool UploadSimple(const QByteArray& fileData, QString* fileId);
  bool DownloadSimple(const QString& fileId, QByteArray* fileData);
  bool DownloadParams(const QString& fileId, QByteArray* fileData);
  bool Share(const QString& fileId);

private:
  bool Authorize();

public:
  GDrive(ServiceAccount* _ServiceAccount, RestSender*  _RestSender);
};
