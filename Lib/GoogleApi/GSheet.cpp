#include <QJsonDocument>
#include <QJsonObject>

#include <Lib/Log/Log.h>

#include "GSheet.h"
#include "RestSender.h"
#include "ServiceAccounts.h"



bool GSheet::CreateSimple(const QString& fileName, QString* fileId)
{
  if (!Authorize()) {
    return false;
  }

  QByteArray spreadsheetData("{}");
  GenerateSimpleSheet(fileName, spreadsheetData);
  if (!mRestSender->SendFile("https://sheets.googleapis.com/v4/spreadsheets", spreadsheetData)) {
    return false;
  }

  const QByteArray& respondData = mRestSender->RespondData();
  if (respondData.isEmpty()) {
    Log.Error(QString("Sheet create fail, empty answer"));
    return false;
  }

  QJsonParseError errJson;
  QJsonDocument doc = QJsonDocument::fromJson(respondData, &errJson);
  if (errJson.error != QJsonParseError::NoError) {
    Log.Error(QString("Sheet create fail, parse respond json fail (err: '%1')").arg(errJson.errorString()));
    return false;
  }

  QJsonObject root = doc.object();
  QString id = root.value("spreadsheetId").toString();
  if (id.isEmpty()) {
    Log.Error(QString("Sheet create fail, empty file id"));
    return false;
  }
  if (fileId) {
    *fileId = id;
  }
  return true;
}

bool GSheet::AppendRow(const QString& fileId, const QStringList& row)
{
//  PUT https://sheets.googleapis.com/v4/spreadsheets/spreadsheetId/values/Sheet1!A1:D5?valueInputOption=USER_ENTERED
  if (!Authorize()) {
    return false;
  }

  QByteArray spreadsheetData("{\n"
                             "\"range\": \"Sheet1!A1:D5\",\n"
                             "\"majorDimension\": \"ROWS\",\n"
                             "\"values\": [\n"
                             "[\"Item\", \"Cost\", \"Stocked\", \"Ship Date\"],\n"
                             "[\"Wheel\", \"$20.50\", \"4\", \"3/1/2016\"],\n"
                             "[\"Door\", \"$15\", \"2\", \"3/15/2016\"],\n"
                             "[\"Engine\", \"$100\", \"1\", \"3/20/2016\"],\n"
                             "[\"Totals\", \"=SUM(B2:B4)\", \"=SUM(C2:C4)\", \"=MAX(D2:D4)\"]\n"
                             "],\n"
                             "}");

  if (!mRestSender->SendJson2(QString("https://sheets.googleapis.com/v4/spreadsheets/%1/values/Sheet1!A1:D5:append?valueInputOption=USER_ENTERED").arg(fileId), spreadsheetData)) {
    return false;
  }

//  const QByteArray& respondData = mRestSender->RespondData();
//  if (respondData.isEmpty()) {
//    Log.Error(QString("Sheet create fail, empty answer"));
//    return false;
//  }

//  QJsonParseError errJson;
//  QJsonDocument doc = QJsonDocument::fromJson(respondData, &errJson);
//  if (errJson.error != QJsonParseError::NoError) {
//    Log.Error(QString("Sheet create fail, parse respond json fail (err: '%1')").arg(errJson.errorString()));
//    return false;
//  }

//  QJsonObject root = doc.object();
//  QString id = root.value("spreadsheetId").toString();
//  if (id.isEmpty()) {
//    Log.Error(QString("Sheet create fail, empty file id"));
//    return false;
//  }
//  if (fileId) {
//    *fileId = id;
//  }
  return true;
}

bool GSheet::Download(const QString& fileId, QByteArray* fileData)
{
  if (!Authorize()) {
    return false;
  }

  if (!mRestSender->SendRequest(QString("https://sheets.googleapis.com/v4/spreadsheets/%0").arg(fileId))) {
    return false;
  }

  if (fileData) {
    *fileData = mRestSender->RespondData();
  }
  return true;
}

bool GSheet::Authorize()
{
  if (!mServiceAccount->IsValid(GoogleApi::SheetScope)) {
    if (!mServiceAccount->Acquire(GoogleApi::SheetScope)) {
      return false;
    }
  }

  mRestSender->SetBearer(mServiceAccount->Token());
  return true;
}

void GSheet::GenerateSimpleSheet(const QString& fileName, QByteArray& sheetData)
{
  QJsonDocument doc;
  QJsonObject root;
  QJsonObject properties;
  properties.insert("title", fileName);
  root.insert("properties", properties);
  doc.setObject(root);
  sheetData = doc.toJson();
}


GSheet::GSheet(ServiceAccount* _ServiceAccount, RestSender* _RestSender)
  : mServiceAccount(_ServiceAccount), mRestSender(_RestSender)
{
}
