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
  /*override */virtual const char* Name() Q_DECL_OVERRIDE { return "LiLoader"; }
  /*override */virtual const char* ShortName() Q_DECL_OVERRIDE { return "L"; }
protected:
  /*override */virtual bool DoInit() Q_DECL_OVERRIDE;
  /*override */virtual bool DoCircle() Q_DECL_OVERRIDE;
  /*override */virtual void DoRelease() Q_DECL_OVERRIDE;
public:
//  /*override */virtual void Stop() Q_DECL_OVERRIDE;

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

