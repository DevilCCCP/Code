#include <QFile>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QProcess>

#include <Lib/Db/Db.h>
#include <Lib/Log/Log.h>

#include "PackageCore.h"
#include "PackLoaderFile.h"
#include "Tools.h"


bool PackageCore::Prepare(const QString& sourceBasePath, const QString& destBasePath)
{
  mInstallerPath.clear();

  QDir dir(sourceBasePath);
  if (!dir.exists()) {
    Log.Error(QString("Directory not exists ('%1')").arg(sourceBasePath));
    return false;
  }

  if (!mPackVersion.LoadFromPack(sourceBasePath)) {
    return false;
  }

  PackLoaderFile* loader;
  mPackLoader.reset(loader = new PackLoaderFile());
  loader->SetPackPath(sourceBasePath);
  QFile infoFile(sourceBasePath + "/.info");
  if (!infoFile.open(QFile::ReadOnly)) {
    Log.Error(QString("Source '.info' file not opened (error: '%1')").arg(infoFile.errorString()));
    return false;
  }
  QByteArray info = infoFile.readAll();
  if (info.isEmpty()) {
    Log.Error(QString("Source '.info' file not read (error: '%1')").arg(infoFile.errorString()));
    return false;
  }
  const QList<QByteArray>& lines = info.split('\n');
  if (!ParseInfo(lines)) {
    Log.Error(QString("Source '.info' file parse fail"));
    return false;
  }

  mMd5.reset(new QCryptographicHash(QCryptographicHash::Md5));
  QByteArray data = GetInfoData();
  mMd5->addData(data);

  mPackingMode = eModePrepare;
  if (!DeployPack(destBasePath)) {
    return false;
  }

  if (!PrepareInfo(info) || !DeployDir(".") || !WriteFile(".info", info)) {
    return false;
  }
  Log.Info(QString("Done"));
  return true;
}

bool PackageCore::Install(const QString& sourceBasePath, const QString& destBasePath, bool aggressive)
{
  if (!mPackVersion && !mPackVersion.LoadFromPack(sourceBasePath)) {
    Log.Error(QString("Version not loaded"));
    return false;
  }

  QByteArray info;
  if (!mPackLoader) {
    mPackLoader.reset(new PackLoaderFile());
    mPackLoader->setUri(sourceBasePath);
    if (!mPackLoader->LoadInfo(info)) {
      return false;
    }
  }

  mPackingMode = aggressive? eModeInstallAggressive: eModeInstallSoft;
  if (!DeployWithInfo(info, destBasePath)) {
    return false;
  }

  return true;
}

bool PackageCore::Update(const QString& sourceBasePath, const QString& destBasePath)
{
  if (!mPackVersion && !mPackVersion.LoadFromPack(sourceBasePath)) {
    Log.Error(QString("Version not loaded"));
    return false;
  }

  QByteArray info;
  if (!mPackLoader) {
    mPackLoader.reset(new PackLoaderFile());
    mPackLoader->setUri(sourceBasePath);
    if (!mPackLoader->LoadInfo(info)) {
      Log.Error(QString("Info not loaded"));
      return false;
    }
  }

  mPackingMode = eModeUpdate;
  if (!DeployWithInfo(info, destBasePath)) {
    return false;
  }

  return true;
}

bool PackageCore::Deploy(const PackLoaderAS& packLoader, const QString& destBasePath)
{
  mInstallerPath.clear();

  mPackLoader = packLoader;
  mPackLoader->SetCtrl(mCtrl);
  QByteArray ver;
  if (!mPackLoader->LoadVer(ver) || !mPackVersion.LoadFromString(QString::fromLatin1(ver))) {
    return false;
  }

  QByteArray info;
  if (!mPackLoader->LoadInfo(info)) {
    return false;
  }
  mPackingMode = eModeDeploy;
  if (!DeployWithInfo(info, destBasePath)) {
    return false;
  }

//  if (mPackLoader->LoadExternalsVer(ver) && mExternalsVersion.LoadFromString(QString::fromLatin1(ver))) {
//    if (!mPackLoader->LoadExternalsInfo(info)) {
//      return false;
//    }
//    mPackingMode = eModeDeploy;
//    if (!DeployWithInfoExt(info, destBasePath)) {
//      return false;
//    }
//  }

  return true;
}

bool PackageCore::RunUpdate(const QString& sourceBasePath, const QString& destBasePath)
{
  if (getInstallerPath().isEmpty()) {
    Log.Error(QString("Install fail, no installer in package"));
    return false;
  }

  QString installerPath = QDir(sourceBasePath).absoluteFilePath(mInstallerPath);
  if (!QFile::exists(installerPath)) {
    Log.Error(QString("Install fail, installer not exists"));
    return false;
  }

  return QProcess::startDetached(installerPath, QStringList() << "update" << sourceBasePath << destBasePath);
}

bool PackageCore::DeployWithInfo(const QByteArray& info, const QString& destBasePath)
{
  if (mCommands.isEmpty()) {
    QList<QByteArray> lines = info.split('\n');
    lines.removeFirst();
    if (!ParseInfo(lines)) {
      Log.Error(QString("Parse info fail"));
      return false;
    }
  }

  if (!DeployPack(destBasePath)) {
    if (mPackingMode == eModeInstallSoft) {
      Log.Warning(QString("Deploy pack fail"));
    } else {
      Log.Error(QString("Deploy pack fail"));
    }
    return false;
  }

  if (mPackingMode == eModeDeploy) {
    if (!DeployDir(".") || !WriteFile(".info", info)) {
      return false;
    }
  }
  if (mPackingMode == eModeInstallSoft || mPackingMode == eModeInstallAggressive || mPackingMode == eModeUpdate) {
    if (!ExecScripts()) {
      Log.Error(QString("Exec scripts fail"));
      return false;
    }
  }

  Log.Info(QString("Done"));
  return true;
}

bool PackageCore::DeployWithInfoExt(const QByteArray& info, const QString& destBasePath)
{
  if (mCommandsExt.isEmpty()) {
    QList<QByteArray> lines = info.split('\n');
    lines.removeFirst();
    if (!ParseInfoExt(lines)) {
      Log.Error(QString("Parse externals info fail"));
      return false;
    }
  }

  if (!DeployPack(destBasePath)) {
    if (mPackingMode == eModeInstallSoft) {
      Log.Warning(QString("Deploy pack fail"));
    } else {
      Log.Error(QString("Deploy pack fail"));
    }
    return false;
  }

  if (mPackingMode == eModeDeploy) {
    if (!DeployDir(".") || !WriteFile(".info", info)) {
      return false;
    }
  }
  if (mPackingMode == eModeInstallSoft || mPackingMode == eModeInstallAggressive) {
    if (!ExecScripts()) {
      Log.Error(QString("Exec scripts fail"));
      return false;
    }
  }

  Log.Info(QString("Done"));
  return true;
}

bool PackageCore::DeployPack(const QString& destBasePath)
{
  Log.Info(QString("DeployMode is '%1'").arg(ModeToString()));
  if (mPackingMode >= eModeIllegal) {
    return false;
  }
  mDestPath = destBasePath;
  mDestDir.setPath(destBasePath);

  mCurrentPath.clear();

  for (int i = 0; i < mCommands.size(); i++) {
    if (mPackingMode == eModePrepare) {
      Log.Status(QString("Done %1/%2").arg(i).arg(mCommands.size()));
    }

    PackCommand* command = &mCommands[i];
    if (!command->DeployDone) {
      if (!DeployOne(command)) {
        return false;
      }
      if (command->Type != eDir) {
        command->DeployDone = true;
      }
    }
  }

  if (mInstallerPath.isEmpty() && mPackingMode != eModeUpdate) {
    Log.Error("No installer");
    return false;
  }
  if (!QFile::exists(destBasePath + "/" + mInstallerPath) && mPackingMode != eModeUpdate) {
    Log.Error("Installer not copied with pack");
    return false;
  }
  if (mPackingMode == eModeDeploy) {
    return ValidateHash();
  }
  return true;
}

bool PackageCore::DeployOne(PackageCore::PackCommand* cmd)
{
  switch (cmd->Type) {
  case eDir:       return DeployDir(cmd->Path);
  case eFile:      return DeployFile(cmd->Path, false, false);
  case eFileExec:  return DeployFile(cmd->Path, false, true);
  case eArchExec:  return DeployFile(cmd->Path, true, true);
  case eArchFile:  return DeployFile(cmd->Path, true, false);
  case eBatScript: return DeployFile(cmd->Path, false, true);
  case eSqlScript: return DeployFile(cmd->Path, false, false);
  case eIllegal:   Log.Warning(QString("Bad cmd (path: '%1')").arg(cmd->Path)); break;
  }
  return true;
}

bool PackageCore::DeployDir(const QString& path)
{
  mCurrentPath = path;
  mDestDir.setPath(mDestPath + "/" + mCurrentPath);
  if (!mDestDir.exists() && !mDestDir.mkpath(mDestDir.path())) {
    Log.Error(QString("Create path fail ('%1')").arg(mCurrentPath));
    return false;
  }
  Log.Info(QString("Dir done ('%1')").arg(path));
  return true;
}

bool PackageCore::DeployFile(const QString& path, bool arch, bool exec)
{
  if (mPackingMode == eModeUpdate && path == mInstallerPath) {
    return true;
  }
  if (mCurrentPath.isEmpty()) {
    Log.Error(QString("Current directory not specified before file '%1'").arg(path));
    return false;
  }
  QByteArray data;
  if (!mPackLoader->LoadFile(mCurrentPath + "/" + path, data)) {
    Log.Error(QString("Read file fail (path: '%1', file: '%2')").arg(mCurrentPath, path));
    return false;
  }

  if (mPackingMode == eModePrepare) {
    mMd5->addData(data);

    if (arch) {
      data = qCompress(data, 9);
    }
  } else if (mPackingMode == eModeDeploy) {
    if (arch) {
      data = qUncompress(data);
    }
  }

  bool ok = WriteFile(path, data);
  if (!ok && mPackingMode == eModeUpdate) {
    QThread::msleep(500);
    ok = WriteFile(path, data);
    if (!ok) {
      for (int retry = 10; retry > 0 && !ok; retry--) {
        QThread::msleep(500);
        Log.Warning(QString("Write file fail (path: '%1', retry: %2)"));
        ok = WriteFile(path, data);
        if (ok) {
          Log.Info(QString("Write ok"));
        }
      }
    }
  }
  if (!ok && (mPackingMode == eModeInstallAggressive || mPackingMode == eModeUpdate)) {
#ifdef Q_OS_WIN32
    if (!KillFileOwner(mDestDir.absoluteFilePath(path))) {
      return false;
    }
#elif defined(Q_OS_UNIX)
    if (!KillFileOwner(mDestDir.absoluteFilePath(path))) {
      return false;
    }
#endif
    ok = WriteFile(path, data);
  }
  if (!ok) {
    Log.Warning(QString("Write file fail (path: '%1', size: %2)").arg(path).arg(data.size()));
    return false;
  }

  if (mPackingMode != eModePrepare && exec) {
    if (!ChmodFile(path)) {
      Log.Error(QString("chmod fail (path: '%1', file: '%2')").arg(mCurrentPath, path));
      return false;
    }
  }

  Log.Info(QString("File done ('%1', a:%2, x: %3)").arg(path).arg(arch? "yes": "no").arg(exec? "yes": "no"));
  return true;
}

bool PackageCore::ExecScripts()
{
  if (mCommands.isEmpty()) {
    return true;
  }

  for (auto itr = mCommands.begin(); itr != mCommands.end(); itr++) {
    PackCommand* command = &*itr;
    if (command->ExecDone) {
      continue;
    }
    if (command->Type == eBatScript) {
      QString scriptPath = mDestDir.absoluteFilePath(command->Path);
      if (!ExecFile(scriptPath)) {
        return false;
      }
      command->ExecDone = true;
    } else if (command->Type == eSqlScript) {
      QString scriptPath = mDestDir.absoluteFilePath(command->Path);
      if (!ExecSql(scriptPath)) {
        return false;
      }
      command->ExecDone = true;
    } else if (command->Type == eDir) {
      mCurrentPath = command->Path;
      mDestDir.setPath(mDestPath + "/" + mCurrentPath);
    }
  }
  Log.Info(QString("Exec all scripts done"));
  return true;
}

bool PackageCore::ExecFile(const QString& scriptPath)
{
  QDir::setCurrent(mDestPath);

#ifdef Q_OS_WIN32
  QProcess p;
  p.start("cmd.exe", QStringList() << "/c" << scriptPath);
  p.waitForFinished();
  int ret = p.exitCode();
#else
  int ret = QProcess::execute(scriptPath);
#endif
  if (ret == 0) {
    Log.Info(QString("Exec script done (file: '%1')").arg(scriptPath));
#ifdef Q_OS_WIN32
    Log.Info(QString("Output: \n") + QString::fromUtf8(p.readAllStandardOutput()));
#endif
    QFile::remove(scriptPath);
    return true;
  } else {
    Log.Error(QString("Exec script fail (file: '%1', code: %2)").arg(scriptPath).arg(ret));
    return false;
  }
}

bool PackageCore::ExecSql(const QString& scriptPath)
{
  Q_UNUSED(scriptPath);

  return false;
}

bool PackageCore::ConnectDb()
{
  return false;
}

bool PackageCore::WriteFile(const QString& path, const QByteArray& data)
{
  QString filePath = mDestDir.absoluteFilePath(path);
  QFile file(filePath);
  if (file.open(QFile::WriteOnly)) {
    if (file.write(data) == data.size()) {
      return file.flush();
    }
  }
  return false;
}

bool PackageCore::ChmodFile(const QString& path)
{
#ifdef Q_OS_UNIX
  QString filePath = mDestDir.absoluteFilePath(path);
  int ret = QProcess::execute(QString("chmod +x \"%1\"").arg(filePath));
  if (ret != 0) {
    Log.Error(QString("chmod fail (file: '%1', code: %2)").arg(filePath).arg(ret));
    return false;
  }
#else
  Q_UNUSED(path);
#endif
  return true;
}

bool PackageCore::HashFile(const QString& path)
{
  QString filePath = mDestDir.absoluteFilePath(path);
  QFile file(filePath);
  if (file.open(QFile::ReadOnly)) {
    return mMd5->addData(&file);
  }
  Log.Warning(QString("hash file fail '%1'").arg(path));
  return false;
}

bool PackageCore::PrepareInfo(QByteArray& info)
{
  info = mPackVersion.ToString().toLatin1() + "\n";
  for (auto itr = mCommands.begin(); itr != mCommands.end(); itr++) {
    const PackCommand& cmd = *itr;
    QByteArray prefix;
    switch (cmd.Type) {
    case eDir:       prefix = "D "; break;
    case eFile:      prefix = "F "; break;
    case eFileExec:  prefix = "E "; break;
    case eArchFile:  prefix = "A "; break;
    case eArchExec:  prefix = "X "; break;
    case eBatScript: prefix = "S "; break;
    case eSqlScript: prefix = "Q "; break;

    case eIllegal:
      Log.Error("illegal command");
      return false;
    }

    info.append(prefix + cmd.Path.toUtf8() + "\n");
  }
  info.append(QByteArray("M ") + mMd5->result().toHex() + "\n");
  info.append(QByteArray("I ") + mInstallerPath.toUtf8() + "\n");
  return true;
}

bool PackageCore::ValidateHash()
{
  mMd5.reset(new QCryptographicHash(QCryptographicHash::Md5));
  QByteArray info = GetInfoData();
  mMd5->addData(info);

  for (auto itr = mCommands.begin(); itr != mCommands.end(); itr++) {
    const PackCommand& cmd = *itr;
    switch (cmd.Type) {
    case eDir: mCurrentPath = cmd.Path; break;

    case eFile:
    case eFileExec:
    case eArchFile:
    case eArchExec:
    case eBatScript:
    case eSqlScript:
      if (!HashFile(mCurrentPath + "/" + cmd.Path)) {
        return false;
      }
      break;

    case eIllegal: break;
    }
  }

  QByteArray calcHash = mMd5->result();
  Log.Info(QString("Hash calc done (real: '%1', exp: '%2')")
           .arg(calcHash.toHex().constData()).arg(mMd5Hash.toHex().constData()));
  if (calcHash != mMd5Hash) {
    Log.Error("Hash missmatch");
    return false;
  }
  return true;
}

bool PackageCore::ParseInfo(const QList<QByteArray>& lines)
{
  for (auto itr = lines.begin(); itr != lines.end(); itr++) {
    const QByteArray& line = *itr;
    if (!ParseLine(line)) {
      return false;
    }
  }
  return true;
}

bool PackageCore::ParseInfoExt(const QList<QByteArray>& lines)
{
  for (auto itr = lines.begin(); itr != lines.end(); itr++) {
    const QByteArray& line = *itr;
    if (!ParseLineExt(line)) {
      return false;
    }
  }
  return true;
}

bool PackageCore::ParseLine(const QByteArray& line)
{
  QString lineText = QString::fromUtf8(line).trimmed();
  if (lineText.isEmpty()) {
    return true;
  }

  char command = lineText.at(0).toLatin1();
  QString cmdParams = lineText.mid(2).trimmed();
  switch (command) {
  case '-':
  case ' ':
  case '#':
  case '/':
    return true;

  case 'D': mCommands.append(PackCommand(eDir, cmdParams)); return true;
  case 'F': mCommands.append(PackCommand(eFile, cmdParams)); return true;
  case 'E': mCommands.append(PackCommand(eFileExec, cmdParams)); return true;
  case 'A': mCommands.append(PackCommand(eArchFile, cmdParams)); return true;
  case 'S': mCommands.append(PackCommand(eBatScript, cmdParams)); return true;
  case 'Q': mCommands.append(PackCommand(eSqlScript, cmdParams)); return true;
  case 'X': mCommands.append(PackCommand(eArchExec, cmdParams)); return true;

  case 'I': mInstallerPath = cmdParams; return true;
  case 'M': mMd5Hash = QByteArray::fromHex(cmdParams.toLatin1()); return true;
  }
  Log.Warning(QString("Parse info: illegal line ('%1')").arg(line.constData()));
  return true;
}

bool PackageCore::ParseLineExt(const QByteArray& line)
{
  QString lineText = QString::fromUtf8(line).trimmed();
  if (lineText.isEmpty()) {
    return true;
  }

  char command = lineText.at(0).toLatin1();
  QString cmdParams = lineText.mid(2).trimmed();
  switch (command) {
  case '-':
  case ' ':
  case '#':
  case '/':
    return true;
  case 'D': mCommands.append(PackCommand(eDir, cmdParams)); return true;
  }

  QStringList cmdParamsList = cmdParams.split(' ', Qt::KeepEmptyParts);
  if (cmdParamsList.size() >= 4) {
    QString fileHash = cmdParamsList.takeLast();
    QString fileVersion = cmdParamsList.takeLast();
    QString fileSize = cmdParamsList.takeLast();
    QByteArray hash = QByteArray::fromHex(fileHash.toLatin1());
    Version fileVer;
    bool ok1 = fileVer.LoadFromString(fileVersion);
    bool ok2;
    qint64 size = fileSize.toLongLong(&ok2);
    QString filePath = cmdParamsList.join(' ');

    if (ok1 && ok2) {
      switch (command) {
      case 'F': mCommands.append(PackCommand(eFile, filePath, size, fileVer, hash)); return true;
      case 'E': mCommands.append(PackCommand(eFileExec, filePath, size, fileVer, hash)); return true;
      case 'A': mCommands.append(PackCommand(eArchFile, filePath, size, fileVer, hash)); return true;
      case 'S': mCommands.append(PackCommand(eBatScript, filePath, size, fileVer, hash)); return true;
      case 'Q': mCommands.append(PackCommand(eSqlScript, filePath, size, fileVer, hash)); return true;
      case 'X': mCommands.append(PackCommand(eArchExec, filePath, size, fileVer, hash)); return true;
      }
    }
  }
  Log.Warning(QString("Parse info externals: illegal line ('%1')").arg(line.constData()));
  return true;
}

QByteArray PackageCore::GetInfoData() const
{
  QByteArray data;
  for (auto itr = mCommands.begin(); itr != mCommands.end(); itr++) {
    const PackCommand& cmd = *itr;
    data.append(cmd.Path.toUtf8());
    data.append("\n");
  }
  return data;
}

QString PackageCore::ModeToString()
{
  switch (mPackingMode) {
  case eModePrepare:           return "prepare";
  case eModeDeploy:            return "deploy";
  case eModeInstallSoft:       return "inst soft";
  case eModeInstallAggressive: return "inst aggr";
  case eModeUpdate:            return "update";
  case eModeIllegal:           return "illeagal";
  }
  return QString::number((int)mPackingMode);
}


PackageCore::PackageCore()
  : mCtrl(nullptr)
  , mPackingMode(eModeIllegal)
{
}

PackageCore::~PackageCore()
{
}

