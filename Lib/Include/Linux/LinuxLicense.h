#pragma once

#include <QThread>
#include <QFile>
#include <QByteArray>
#include <QString>

#include "../License_h.h"


inline void AppendFile(QFile& file, QByteArray& result)
{
#ifdef __arm__
  for (int zeroLine = 0; zeroLine < 3; ) {
    QByteArray line = file.readLine();
    if (!line.startsWith("BogoMIPS")) {
      result.append(line);
    }
    if (line.isEmpty()) {
      zeroLine++;
    }
  }
#else
  result.append(file.readAll());
#endif
}

inline QByteArray GetSn(bool gui)
{
  Q_UNUSED(gui);
#ifdef __arm__
  const char kPath[] = "/proc/";
  const char* kVars[] = { "cpuinfo"
                          , ""
                        };
#else
  const char kPath[] = "/sys/class/dmi/id/";
  const char* kVars[] = { "product_uuid"
                          , "product_name"
                          , "product_serial"
                          , "bios_vendor"
                          , "bios_version"
                          , "board_name"
                          , "board_serial"
                          , "board_vendor"
                          , "board_version"
                          , "modalias"
                          , ""
                        };
#endif

  QByteArray result("result:\n");
  result.append(LICENSE_PREFIX);

  for (int i = 0; *kVars[i]; i++) {
    QString name = QString(kVars[i]);
    QFile file(QString(kPath) + name);
    for (int j = 0; j < 100; j++) {
      if (file.open(QFile::ReadOnly)) {
        AppendFile(file, result);
        break;
      }
      QThread::msleep(1);
    }
  }
  return result;
}

inline QByteArray GetSnShort(int iv)
{
#ifdef __arm__
  const char kPath[] = "/opl_*";
  const char* kVars[] = { "cosfjai"
                          , ""
                        };
#else
  const char kPath[] = "/rwp+^fZkj/ckf+d^(";
  const char* kVars[] = { "pqmaq^nXml_Y"
                          , "pqmaq^nXfXcZ"
                          , "pqmaq^nXk\\h^U_"
                          , "bhmp[q_g\\fh"
                          , "bhmp[q_kk`ec"
                          , "bn_o`ZhZe\\"
                          , "bn_o`Zm^j`Wa"
                          , "bn_o`Zp^f[eg"
                          , "bn_o`Zp^jj_db"
                          , "mnb^hd[l"
                          , ""
                        };
#endif

  QByteArray result("result:\n");
  result.append(LICENSE_PREFIX);

  for (int i = 0; *kVars[i]; i++) {
    QByteArray n(kVars[i]);
    for (int j = 0; j < n.size(); j++) {
      n[j] = (char)(n[j] + j);
    }
    QByteArray p(kPath);
    for (int j = 0; j < p.size(); j++) {
      p[j] = (char)(p[j] + (j % 10));
    }
    QFile file(QString(p) + QString(n));
    if (file.open(QFile::ReadOnly)) {
      AppendFile(file, result);
    }
  }

  int line = ((iv & 0xffff) % kLcCount) + 1;
  result.append((char*)&line, 4);

  return result;
}
