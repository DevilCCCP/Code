#pragma once

#include <QString>

#include <LibV/Include/Frame.h>
#include <Lib/Db/Db.h>
#include <Lib/Settings/SettingsA.h>


DefineClassS(Db);
DefineDbClassS(DbIndex);
DefineDbClassS(Container);
DefineDbClassS(Storage);

class Storage
{
  QString          mDbConnectionFile;

  Db               mDb;
  DbIndexS         mDbIndex;
  ContainerS       mContainer;
  int              mCapacity;

  int              mUnitId;
  int              mCurrentCell;
  int              mNextCell;

  qint64           mLastWriteTimestamp;

public:
  bool Create();
  bool Open(int _UnitId);
  bool CloseWrite();
  void Reset();

private:
  bool Prepare();

public:
  bool WriteFrame(FrameS& frame);
  bool SeekFrame(const qint64& timestamp, bool& found, int maxDifferentMs = 60000, bool useKey = true);
  bool ReadFrame(FrameS &frame, bool forward);
  bool ReadNextFrame(FrameS &frame);
  bool ReadPrevFrame(FrameS &frame);

private:

public:
  static SettingsAS CopySettings(const QString &_DbConnectionFile, SettingsA& _Settings);
  static QString ConnectionFile(int id);
  int GetCellLimit();

  Storage(SettingsA& _Settings, const QString& _DbConnectionFile = QString());
  ~Storage();
};

