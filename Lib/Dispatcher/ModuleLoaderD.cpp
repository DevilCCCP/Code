#include <QSet>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include <QDir>
#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkInterface>
#include <QHostInfo>

#include <Lib/Log/Log.h>
#include <Lib/Common/Format.h>
#include <Lib/Common/Version.h>
#include <Lib/Common/Var.h>
#include <Lib/Common/Uri.h>
#include <Lib/Db/ObjectType.h>
#include <Lib/Db/ObjectState.h>
#include <Lib/Settings/DbSettings.h>
#include <Local/ModuleNames.h>

#include "ModuleLoaderD.h"
#include "ModuleDb.h"
#include "ModuleStatic.h"


const int kQuietCheckUpdate = 2000;
const int kLocalhostTestPeriodMs = 1000;
const int kIpTestPeriodMs = 5000;

bool ModuleLoaderD::DoInit()
{
  mDb->MoveToThread(this);
  while (!mDb->OpenDefault()) {
    Rest(1);
    if (IsStop()) {
      return false;
    }
  }
  return LoadSettings() && ModuleLoaderB::DoInit();
}

bool ModuleLoaderD::UpdateModules()
{
  if (!mInitialized) {
    if (DoInitialize()) {
      mInitialized = true;
    } else {
      return false;
    }
  }

  if (!CheckOffline() || !Connect()) {
    return false;
  }

  if (CheckUpdateObject()) {
    Log.Info(mMainModule->Revision? "Object reload (revision changed)": "Object load");
    ReloadObject();
  } else if (CheckUpdateState()) {
    UpdateObject();
  }

  if (!mVeryQuiet) {
    UpdateObjectState();
  }

  if (IsNeedIp()) {
    if (CheckIpChange()) {
      Log.Info("Object reload (IP changed)");
      ReloadObject();
    }
  }

  return true;
}

void ModuleLoaderD::DoRelease()
{
  if (mInitialized) {
    mObjectState->UpdateState(ObjectState::eOff);
  }

  return ModuleLoaderB::DoRelease();
}

bool ModuleLoaderD::IsNeedIp()
{
  return true;
}

void ModuleLoaderD::RegisterStatic()
{
}

bool ModuleLoaderD::InitializeExtra()
{
  return true;
}

bool ModuleLoaderD::CheckUpdateObject()
{
  if (mVeryQuiet) {
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (now < mNextQuietCheckUpdate) {
      return false;
    }
    mNextQuietCheckUpdate = now + kQuietCheckUpdate;
  }
  if (mObjectTable->GetObjectRevision(mMainModule->Id, &mCurrentRevision)) {
    return mCurrentRevision != mMainModule->Revision;
  }
  return false;
}

bool ModuleLoaderD::CheckOffline()
{
  return true;
}

bool ModuleLoaderD::CheckUpdateState()
{
  return false;
}

bool ModuleLoaderD::UpdateState()
{
  return true;
}

EModuleState ModuleLoaderD::GetObjectState()
{
  return eNormalState;
}

bool ModuleLoaderD::GetObjectTypeId(const QString& typeName, int* id)
{
  if (const NamedItem* typeItem = mObjectTypeTable->GetItemByName(typeName)) {
    if (id) {
      *id = typeItem->Id;
    }
    return true;
  } else {
    Log.Warning(QString("Object type not found (abbr: '%1')").arg(typeName));
    return false;
  }
}

void ModuleLoaderD::RegisterStaticModule(const QString& path, const QStringList& params, int stateFlag)
{
  ModuleStaticS module(new ModuleStatic());
  module->setId(-1 - mModuleStaticList.size());
  module->setPath(path);
  module->setParams(params);
  module->setStateFlag(stateFlag);
  mModuleStaticList.append(module);
}

void ModuleLoaderD::LoadModule(const ModuleDbS& module)
{
  int id = module->getId();
  bool oactive = false;
  bool active = module->getActive();
  bool update = false;

  auto itr = mModuleDbMap.find(id);
  if (itr != mModuleDbMap.end()) {
    const ModuleDbS& oldModule = itr.value();
    oactive = oldModule->getActive();
    if (*oldModule != *module) {
      itr.value() = ModuleDbS(new ModuleDb(*module));
      update = true;
    }
  } else {
    mModuleDbMap[id] = ModuleDbS(new ModuleDb(*module));
  }
  if (active) {
    mRemoveModules.remove(id);
    if (!oactive) {
      mInsertModules.insert(id);
    } else if (update) {
      mUpdateModules.insert(id);
    }
  } else {
    if (oactive) {
      mDisableModules.insert(id);
    }
    mRemoveModules.remove(id);
  }

  Uri uri = Uri::FromString(module->getUri());
  if (uri.Type() == Uri::eTcp) {
    if (uri.Host() != IpAddress()) {
      uri.Host() = IpAddress();
      QString newUri = uri.ToString();
      if (mObjectTable->UpdateUri(module->getId(), newUri)) {
        module->setUri(newUri);
      }
    }
    module->setPort(uri.Port());
  } else if (module->getUri().startsWith("tcp:")) {
    mUpdatePortModules.insert(id);
    module->setPort(0);
  }
}

void ModuleLoaderD::ReloadModule(const ModuleDbS& module)
{
  int id = module->getId();
  bool active = module->getActive();

  auto itr = mModuleDbMap.find(id);
  if (itr == mModuleDbMap.end()) {
    Log.Warning(QString("Update non loaded module (id: %1)").arg(id));
    return;
  }

  const ModuleDbS& oldModule = itr.value();
  bool oactive = oldModule->getActive();
  if (active != oactive) {
    Log.Info(QString("Module active changed (id: %1 (%2 -> %3)").arg(id).arg(oactive).arg(active));
    itr.value() = ModuleDbS(new ModuleDb(*module));
    if (active) {
      AddModule(id, module->getPath(), module->getParams(), module->getUri());
    } else {
      RemoveModule(id);
    }
  }
}

bool ModuleLoaderD::DoInitialize()
{
  if (!mDb->OpenDefault()) {
    return false;
  }
  if (!Connect()) {
    if (mRestartPostgresMs > 0 && mRestartPostgresMs < 5 * 1000) {
      Log.Warning(QString("RestartPostgresMs has too small value (%1), ignored").arg(FormatTime(mRestartPostgresMs)));
      mRestartPostgresMs = 0;
    }
    if (mRestartPostgresMs > 0 && mInitializeTimer.elapsed() > mRestartPostgresMs) {
      Log.Warning(QString("Postgres unavailable for %1, restarting...").arg(FormatTime(mRestartPostgresMs)));

      QString cmd = QString("sudo");
      QStringList args = QStringList() << "service" << "postgresql" << "restart";
      Log.Info(QString("%1 %2").arg(cmd, args.join(' ')));
      QProcess::startDetached(cmd, args);

      mInitializeTimer.restart();
      mRestartPostgresMs *= 2;
    }
    return false;
  }
  if (!DbUpdate() || !mObjectTypeTable->Load()) {
    return false;
  }

  if (!mMainModule->Id) {
    if (!RegisterModule()) {
      return false;
    }
  }

#ifndef NOLICENSE
  if (!QFile::exists(GetVarFile("key"))) {
    LoadLicense();
    return false;
  }
#endif

  if (!mDbSettings->Open(QString::number(mMainModule->Id))
      || !mDbVariables->Open(QString::number(mMainModule->Id))) {
    return false;
  }
  mObjectState->InitState(mMainModule->Id, ObjectState::ePower, ObjectState::eOff, ObjectState::eOn);

  if (!InitializeExtra()) {
    return false;
  }
  InitStaticModules();

  Log.Info(QString("Loaded '%1' (id: %2, GUID: '%3', type: '%4')")
           .arg(mMainModule->Name).arg(mMainModule->Id).arg(mMainModule->Guid).arg(mTypeName));
  return true;
}

void ModuleLoaderD::InitStaticModules()
{
  mModuleStaticList.clear();

  RegisterStatic();
}

void ModuleLoaderD::ReloadObject()
{
  if (IsNeedIp()) {
    ReloadIp();
  }

  PrepareLoad();
  if (!LoadObject()) {
    return;
  }
  EModuleState newState = GetObjectState();
  if (newState != mCurrentState) {
    Log.Info(QString("Switch object state (%1 -> %2)").arg(EModuleStateToString(mCurrentState)).arg(EModuleStateToString(newState)));
    SwitchState(newState);
  }
  CommitUpdate();
}

void ModuleLoaderD::UpdateObject()
{
  if (!UpdateState()) {
    return;
  }

  EModuleState newState = GetObjectState();
  if (newState != mCurrentState) {
    SwitchState(newState);
  }
  CommitUpdate();
}

void ModuleLoaderD::SwitchState(EModuleState state)
{
  mCurrentState = state;

  for (auto itr = mModuleStaticList.constBegin(); itr != mModuleStaticList.constEnd(); itr++) {
    const ModuleStaticS& module = *itr;
    int id = module->getId();
    bool oactive = mUsedStaticModules.contains(id);
    bool active = (module->getStateFlag() & mCurrentState) != 0;
    if (active != oactive) {
      Log.Info(QString("Static module active changed (%1 -> %2)").arg(oactive).arg(active));
      if (active) {
        AddModule(id, module->getPath(), module->getParams(), QString());
        mUsedStaticModules.insert(id);
      } else {
        RemoveModule(id);
        mUsedStaticModules.remove(id);
      }
    }
  }

  if (mCurrentState == eNormalState) {
    QList<int> moduleList = mModuleDbMap.keys();
#if QT_VERSION >= QT_VERSION_CHECK(5, 14,0)
    mInsertModules = QSet<int>(moduleList.begin(), moduleList.end());
#else
    mInsertModules = QSet<int>::fromList(moduleList);
#endif
    mUpdateModules.clear();
    mDisableModules.clear();
    mRemoveModules.clear();
  } else if (mCurrentState == eStandByState) {
    QList<int> moduleList = mModuleDbMap.keys();
    mInsertModules.clear();
    mUpdateModules.clear();
#if QT_VERSION >= QT_VERSION_CHECK(5, 14,0)
    mDisableModules = QSet<int>(moduleList.begin(), moduleList.end());
#else
    mDisableModules = QSet<int>::fromList(moduleList);
#endif
    mRemoveModules.clear();
  }
}

void ModuleLoaderD::ReloadIp()
{
  mDbSettings->SetSilent(true);
  QString newSettingIpAddress = mDbSettings->GetValue("IP").toString();
  if (mSettingIpAddress != newSettingIpAddress) {
    Log.Info("Changed settings IP");
    mSettingIpAddress = newSettingIpAddress;
    LoadHostIp();
  }
}

void ModuleLoaderD::PrepareLoad()
{
  mInsertModules.clear();
  mUpdateModules.clear();
  mDisableModules.clear();
  QList<int> moduleList = mModuleDbMap.keys();
#if QT_VERSION >= QT_VERSION_CHECK(5, 14,0)
  mRemoveModules = QSet<int>(moduleList.begin(), moduleList.end());
#else
  mRemoveModules = QSet<int>::fromList(moduleList);
#endif
}

void ModuleLoaderD::CommitUpdate()
{
  if (mCurrentState == eStandByState) {
    mInsertModules.clear();
    mUpdateModules.clear();
  }

  for (auto itr = mInsertModules.constBegin(); itr != mInsertModules.constEnd(); itr++) {
    int id = *itr;
    const ModuleDbS& module = mModuleDbMap[id];
    if (!module) {
      Log.Error(QString("Insert non existen module id %1").arg(id));
      continue;
    }
    if (module->getActive()) {
      AddModule(id, module->getPath(), module->getParams(), module->getUri());
    }
  }

  for (auto itr = mUpdateModules.constBegin(); itr != mUpdateModules.constEnd(); itr++) {
    int id = *itr;
    const ModuleDbS& module = mModuleDbMap[id];
    if (!module) {
      Log.Error(QString("Update non existen module id %1").arg(id));
      continue;
    }
    if (module->getActive()) {
      UpdateModule(id, module->getPath(), module->getParams(), module->getUri());
    } else {
      Log.Warning(QString("Update inactive module ask (id: %1)").arg(id));
    }
  }

  for (auto itr = mRemoveModules.constBegin(); itr != mRemoveModules.constEnd(); itr++) {
    int id = *itr;
    const ModuleDbS& module = mModuleDbMap[id];
    if (!module) {
      Log.Error(QString("Remove non existen module id %1").arg(id));
      continue;
    }
    if (module->getPort() > 0) {
      FreeTcpPort(module->getPort());
    }
    RemoveModule(id);
    mModuleDbMap.remove(id);
  }

  for (auto itr = mDisableModules.constBegin(); itr != mDisableModules.constEnd(); itr++) {
    int id = *itr;
    const ModuleDbS& module = mModuleDbMap[id];
    if (!module) {
      Log.Error(QString("Disable non existen module id %1").arg(id));
      continue;
    }
    RemoveModule(id);
  }

  if (!mUpdatePortModules.isEmpty()) {
    ProvideTcpPorts();
  }

  mInsertModules.clear();
  mUpdateModules.clear();
  mRemoveModules.clear();
  mDisableModules.clear();
  mMainModule->Revision = mCurrentRevision;
}

void ModuleLoaderD::UpdateObjectState()
{
  switch (mCurrentState) {
  case eNormalState:  mObjectState->UpdateState(ObjectState::eOn); break;
  case eStandByState: mObjectState->UpdateState(ObjectState::eSleep); break;
  default: break;
  }
}

bool ModuleLoaderD::Connect()
{
  if (!mDb->Connect()) {
    if (mDbConnected) {
      mDbConnected = false;
      Log.Warning("Loader disconnected");
    }
    return false;
  } else if (!mDbConnected) {
    mDbConnected = true;
    Log.Info("Loader connected");
  }
  return true;
}

bool ModuleLoaderD::DbUpdate()
{
  if (mDbUpdated) {
    return true;
  }

  QDir dir(QCoreApplication::applicationDirPath());
  dir.cd("Updates");
  if (dir.exists()) {
    QFile updateScript(dir.filePath("update.sql"));
    if (!updateScript.open(QFile::ReadOnly)) {
      Log.Warning(QString("Db update script not found, skipping"));
      return true;
    }
    QTextStream scriptStream(&updateScript);
    scriptStream.setCodec("UTF-8");
    QString scriptText = scriptStream.readAll();
    if (scriptText.isEmpty()) {
      Log.Warning(QString("Db update script can't be read, skipping"));
      return true;
    }

    Log.Info(QString("Processing Db update script"));
    auto q = mDb->MakeQuery();
    if (!q->exec(scriptText)) {
      Log.Warning(QString("Db update script fail"));
      return true;
    }

    Log.Info(QString("Db update script done"));
  }
  mDbUpdated = true;
  SettingsA* settings = GetSettings();
  settings->SetValue("Updated", mDbUpdated);
  settings->Sync();
  return true;
}

bool ModuleLoaderD::RegisterModule()
{
  ObjectItemS objItem;
  if (!mObjectTable->GetObjectByGuid(mMainModule->Guid, objItem)) {
    return false;
  }
  if (!objItem) {
    int objTemplItemId;
    if (!mObjectTable->GetDefaultTemplateObjectId(mTypeName, &objTemplItemId)) {
      return false;
    }
    if (!objTemplItemId) {
      Log.Fatal(QString("Can't find default template for type '%1'").arg(mTypeName));
      return false;
    }
    if (!mObjectTable->CreateObject(objTemplItemId, mMainModule->Name, mMainModule->Guid, &mMainModule->Id)) {
      return false;
    }
    Log.Info(QString("Object created (type: '%1', name: '%2') ").arg(mTypeName).arg(mMainModule->Name));
    if (!mObjectTable->UpdateVersion(mMainModule->Id, mMainModule->Version, false)) {
      return false;
    }
  } else {
    if (mMainModule->Version != objItem->Version) {
      if (mObjectTable->UpdateVersion(objItem->Id, mMainModule->Version, true)) {
        objItem->Version = mMainModule->Version;
        objItem->Revision++;
      } else {
        Log.Warning(QString("Update version failed"));
      }
    }
    mMainModule = objItem;
  }

  mMainModule->Revision = 0;
  return true;
}

bool ModuleLoaderD::LoadLicense()
{
  if (!mLicenseInfo) {
    AddTemporaryModule(-13, kLicenseLoaderExe, QStringList(), QString());
    Log.Info(QString("License not found, wait loading it..."));
    mLicenseInfo = true;
  }
  return true;
}

bool ModuleLoaderD::LoadSettings()
{
  SettingsA* settings = GetSettings();
  mMainModule->Name = settings->GetValue("Name", "Noname object").toString();
  mMainModule->Guid = settings->GetMandatoryValue("GUID", true).toString();
  mDbUpdated        = settings->GetValue("Updated", false).toBool();
  if (mMainModule->Guid.isEmpty()) {
    Log.Fatal("GUID is empty", true);
    return false;
  }

  Version ver;
  if (ver.LoadFromThis()) {
    mMainModule->Version = ver.ToString();
  }
  return true;
}

bool ModuleLoaderD::CheckIpChange()
{
  if (mHostIpAddress.isEmpty()) {
    if (LoadHostIp()) {
      return true;
    }
  } else if (mHostIpAddress == "127.0.0.1" || mHostIpAddress == "::1") {
    if (mIpTestTimer.elapsed() > kLocalhostTestPeriodMs) {
      if (LoadHostIp()) {
        return true;
      }
    }
  } else if (mIpTestTimer.elapsed() > kIpTestPeriodMs) {
    if (!IsHostIpValid()) {
      if (LoadHostIp()) {
        return true;
      }
    } else {
      mIpTestTimer.restart();
    }
  }
  return false;
}

bool ModuleLoaderD::IsHostIpValid()
{
  QHostAddress definedAddr(mHostIpAddress);
  QList<QHostAddress> infoHost = QHostInfo::fromName(QHostInfo::localHostName()).addresses();
  // query equal
  for (auto itr = infoHost.begin(); itr != infoHost.end(); itr++) {
    QHostAddress localAddr = *itr;
    if (localAddr == definedAddr) {
      return true;
    }
  }
  return false;
}

bool ModuleLoaderD::LoadHostIp()
{
  Log.Trace("Loading predefined IP");
  if (mSettingIpAddress.isEmpty()) {
    mDbSettings->SetSilent(true);
    mSettingIpAddress = mDbSettings->GetValue("IP").toString();
    if (mSettingIpAddress.isEmpty()) {
      mSettingIpAddress = GetSettings()->GetMandatoryValue("IP").toString();
      if (!mSettingIpAddress.isEmpty()) {
        mDbSettings->SetValue("IP", mSettingIpAddress);
        mDbSettings->Sync();
      } else {
        mSettingIpAddress = "-";
      }
    }
    Log.Info(QString("Loaded settings IP '%1'").arg(mSettingIpAddress));
  }

  bool changed = false;
  bool got = LoadHostIpEqual(changed) || LoadHostIpSubnet(changed) || LoadHostIpAnyIpv4(changed) || LoadHostIpDbConnection(changed);

  if (!got) {
    if (mHostIpAddress.isEmpty()) {
      mHostIpAddress = "127.0.0.1";
      Log.Info(QString("No better idea of IP, using '%1'").arg(mHostIpAddress));
      changed = true;
    }
  }
  mIpTestTimer.start();
  return changed;
}

bool ModuleLoaderD::LoadHostIpEqual(bool& changed)
{
  changed = false;
  QHostAddress definedAddr(mSettingIpAddress);
  QList<QHostAddress> allAddresses = QNetworkInterface::allAddresses();
  Log.Trace("IP query equal");
  for (auto itr = allAddresses.begin(); itr != allAddresses.end(); itr++) {
    const QHostAddress& localAddr = *itr;
    if (localAddr == definedAddr) {
      if (mHostIpAddress != mSettingIpAddress) {
        mHostIpAddress = mSettingIpAddress;
        Log.Info(QString("Address defined from setting '%1'").arg(mHostIpAddress));
        changed = true;
      }
      return true;
    }
  }
  return false;
}

bool ModuleLoaderD::LoadHostIpSubnet(bool& changed)
{
  changed = false;
  QHostAddress definedAddr(mSettingIpAddress);
  QList<QHostAddress> allAddresses = QNetworkInterface::allAddresses();
  Log.Trace("IP query subnet");
  for (auto itr = allAddresses.begin(); itr != allAddresses.end(); itr++) {
    QHostAddress localAddr = *itr;
    if (localAddr.isInSubnet(definedAddr, 24)) {
      if (mHostIpAddress != localAddr.toString()) {
        mHostIpAddress = localAddr.toString();
        Log.Info(QString("Address defined from setting subnet '%1'").arg(mHostIpAddress));
        changed = true;
      }
      return true;
    }
  }
  return false;
}

bool ModuleLoaderD::LoadHostIpAnyIpv4(bool& changed)
{
  changed = false;
  QList<QHostAddress> allAddresses = QNetworkInterface::allAddresses();
  Log.Trace("IP query any ipv4 non localhost");
  for (auto itr = allAddresses.begin(); itr != allAddresses.end(); itr++) {
    QHostAddress localAddr = *itr;
    if (localAddr.protocol() == QAbstractSocket::IPv4Protocol) {
      qint32 ipv4 = localAddr.toIPv4Address();
      if (ipv4 != 0x7f000001) {
        if (mHostIpAddress != localAddr.toString()) {
          mHostIpAddress = localAddr.toString();
          Log.Info(QString("Address got first in the list '%1'").arg(mHostIpAddress));
          changed = true;
        }
        return true;
      }
    }
  }
  return false;
}

bool ModuleLoaderD::LoadHostIpDbConnection(bool& changed)
{
  changed = false;
  Log.Trace("IP query db connection");
  QTcpSocket client;
  client.connectToHost(mDb->Host(), mDb->Port());
  if (client.waitForConnected()) {
    QHostAddress localAddr = client.localAddress();
    client.close();

    if (localAddr.protocol() == QAbstractSocket::IPv4Protocol) {
      if (mHostIpAddress != localAddr.toString()) {
        mHostIpAddress = localAddr.toString();
        Log.Info(QString("Address defined from Db connection '%1'").arg(mHostIpAddress));
        changed = true;
      }
      return true;
    }
  }
  return false;
}

void ModuleLoaderD::ProvideTcpPorts()
{
  for (auto itr = mModuleDbMap.begin(); itr != mModuleDbMap.end(); itr++) {
    ModuleDbS& module = itr.value();
    if (module->getPort() >= 0) {
      int port = module->getPort();
      if (port == mTcpPortBase) {
        mTcpPortBase++;
        while (mTcpPortsFree.contains(mTcpPortBase)) {
          mTcpPortsFree.remove(mTcpPortBase);
          mTcpPortBase++;
        }
      } else if (port > mTcpPortBase) {
        mTcpPortsFree.insert(port, true);
      }
    }
  }

  for (auto itr = mUpdatePortModules.begin(); itr != mUpdatePortModules.end(); itr++) {
    int id = *itr;
    auto itr_ = mModuleDbMap.find(id);
    if (itr_ == mModuleDbMap.end()) {
      Log.Warning("Internal error: ProvideTcpPorts");
      continue;
    }
    ModuleDbS& module = itr_.value();
    module->setPort(GetNextFreePort());
    Uri newUri(Uri::eTcp, IpAddress(), module->getPort());
    if (mObjectTable->UpdateUri(module->getId(), newUri.ToString())) {
      Log.Info(QString("Update uri (id: %1, old: '%2', new: '%3')").arg(id).arg(module->getUri()).arg(newUri.ToString()));
      module->setUri(newUri.ToString());
      if (mCurrentState != eStandByState) {
        UpdateModule(module->getId(), module->getPath(), module->getParams(), module->getUri());
      }
    } else {
      Log.Warning(QString("Update uri fail (id: %1, old: '%2', new: '%3')").arg(id).arg(module->getUri()).arg(newUri.ToString()));
    }
  }
  mUpdatePortModules.clear();
}

int ModuleLoaderD::GetNextFreePort()
{
  int port = 0;
  forever {
    if (!mTcpPortsFree.empty()) {
      port = mTcpPortsFree.constBegin().key();
      mTcpPortsFree.erase(mTcpPortsFree.begin());
    } else {
      port = mTcpPortBase;
      mTcpPortBase++;
    }

    QTcpServer test;
    if (test.listen(QHostAddress::AnyIPv4, port)) {
      break;
    }
  }
  return port;
}

void ModuleLoaderD::FreeTcpPort(int port)
{
  mTcpPortsFree.insert(port, true);
}


ModuleLoaderD::ModuleLoaderD(SettingsAS& _Settings, const QString& _TypeName, int _TcpPortBase)
  : ModuleLoaderB(_Settings)
  , mTypeName(_TypeName), mVeryQuiet(false), mRestartPostgresMs(0)
  , mDb(new Db(false)), mDbSettings(new DbSettings(*mDb)), mDbVariables(new DbSettings(*mDb, "variables", "_object"))
  , mObjectTypeTable(new ObjectTypeTable(*mDb)), mObjectTable(new ObjectTable(*mDb)), mObjectState(new ObjectState(*mDb))
  , mInitialized(false), mCurrentState(eUnknownState), mMainModule(new ObjectItem()), mCurrentRevision(0)
  , mDbConnected(false), mTcpPortBase(_TcpPortBase), mLicenseInfo(false)
  , mNextQuietCheckUpdate(0)
{
  mInitializeTimer.start();
}
