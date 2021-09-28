#pragma once

#include <QString>


class ServiceAccount;
class RestSender;

class GSheet
{
  ServiceAccount*  mServiceAccount;
  RestSender*      mRestSender;

public:
  bool CreateSimple(const QString& fileName, QString* fileId);
  bool AppendRow(const QString& fileId, const QStringList& row);
  bool Download(const QString& fileId, QByteArray* fileData);

private:
  bool Authorize();
  void GenerateSimpleSheet(const QString& fileName, QByteArray& sheetData);

public:
  GSheet(ServiceAccount* _ServiceAccount, RestSender*  _RestSender);
};
