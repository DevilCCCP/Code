#pragma once

#include <QObject>

#include <Lib/Dispatcher/Imp.h>
#include <Lib/Db/Db.h>


DefineClassS(LiLoader);
DefineClassS(Db);
DefineClassS(QNetworkAccessManager);
DefineClassS(QEventLoop);

class LiLoader: public Imp
{
  DbS                    mDb;
  ObjectTypeTableS       mObjectTypeTable;
  ObjectTableS           mObjectTable;
  int                    mLiloType;

  QNetworkAccessManager* mNetManager;
  QEventLoop*            mEventLoop;
  QByteArray             mLicInfo;
  QByteArray             mLicData;

  Q_OBJECT

public:
  /*override */virtual const char* Name() override { return "LiLoader"; }
  /*override */virtual const char* ShortName() override { return "L"; }
protected:
  /*override */virtual bool DoInit() override;
  /*override */virtual bool DoCircle() override;
  /*override */virtual void DoRelease() override;
public:
//  /*override */virtual void Stop() override;

private:
  bool InitLilo();
  bool DoLoad();
  bool LoadOne(const ObjectItemS& obj);
  bool ValidateLicense();
  bool DoSave();

public:
  LiLoader();
  /*override */virtual ~LiLoader();
};

