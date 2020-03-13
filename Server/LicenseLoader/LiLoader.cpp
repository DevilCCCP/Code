#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QCryptographicHash>

#include <Lib/Db/ObjectType.h>
#include <Lib/Settings/DbSettings.h>
#include <Lib/Dispatcher/Overseer.h>
#include <Lib/Log/Log.h>
#include <Lib/Include/License.h>

#include "LiLoader.h"


const int kTestDbPeriodMs = 1000;
const int kQuestPeriodSec = 120;

bool LiLoader::DoInit()
{
  mDb.reset(new Db(false));
  mObjectTypeTable.reset(new ObjectTypeTable(*mDb));
  mObjectTable.reset(new ObjectTable(*mDb));

  mNetManager = new QNetworkAccessManager();
  mEventLoop = new QEventLoop();
  return mDb->OpenDefault();
}

bool LiLoader::DoCircle()
{
  if (mDb->Connect() && InitLilo()) {
    if (DoLoad() && DoSave()) {
      GetOverseer()->Done();
    }
  }
  return true;
}

void LiLoader::DoRelease()
{
  delete mNetManager;
  delete mEventLoop;
}

bool LiLoader::InitLilo()
{
  if (mLiloType) {
    return true;
  }

  if (const NamedItem* typeItem = mObjectTypeTable->GetItemByName("lis")) {
    mLiloType = typeItem->Id;
    return true;
  } else {
    return false;
  }
}

bool LiLoader::DoLoad()
{
  QList<ObjectItemS> objects;
  if (mObjectTable->GetObjectsByType(mLiloType, objects)) {
    for (auto itr = objects.begin(); itr != objects.end(); itr++) {
      if (LoadOne(*itr)) {
        return true;
      }
    }
  }
  return false;
}

bool LiLoader::LoadOne(const ObjectItemS& obj)
{
  DbSettings settings(*mDb);
  settings.SetSilent(true);
  if (!settings.Open(QString::number(obj->Id))) {
    return false;
  }
  QString oneUri = settings.GetValue("Uri", "").toString();
  QString uri = oneUri + "/lilo";

  if (mLicInfo.isEmpty()) {
    mLicInfo = GetSn(false);
  }
  Log.Trace(QString("Connect to '%1'").arg(uri));
  QNetworkRequest request = QNetworkRequest(uri);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");
  request.setHeader(QNetworkRequest::ContentLengthHeader, mLicInfo.size());
  QNetworkReply* netReply = mNetManager->post(request, mLicInfo);
  QObject::connect(netReply, &QNetworkReply::finished, mEventLoop, &QEventLoop::quit);
  mEventLoop->exec();

  int retCode = netReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  QByteArray body = netReply->readAll();
  if (retCode != 200 || body.isEmpty()) {
    Log.Warning(QString("Get license fail (code: %1, body: '%2')").arg(retCode).arg(body.constData()));
    return false;
  }

  mLicData = body;
  if (!ValidateLicense()) {
    LOG_ERROR_ONCE(QString("Get invalid license (body: '%1')").arg(mLicData.constData()));
    return false;
  }
  return true;
}

bool LiLoader::ValidateLicense()
{
  QList<QByteArray> parts = mLicData.split(':');
  return parts.size() > 9 && (parts.first().size() * parts.size() + parts.size() - 1) == mLicData.size();
}

bool LiLoader::DoSave()
{
  QFile file(QCoreApplication::applicationDirPath().append('/').append('.').append('k').append('e').append('y'));
  if (file.open(QFile::WriteOnly)) {
    if (file.write(mLicData) == mLicData.size() && file.flush()) {
      file.close();
      file.setPermissions(QFile::ReadOwner | QFile::ReadUser | QFile::ReadGroup | QFile::ReadOther);
      return true;
    }
  }
  return false;
}


LiLoader::LiLoader()
  : Imp(kTestDbPeriodMs)
  , mLiloType(0)
  , mNetManager(nullptr), mEventLoop(nullptr)
{
}

LiLoader::~LiLoader()
{
}

