#pragma once

#ifndef NOLICENSE

#include <qsystemdetection.h>
#include <QByteArray>
#include <QFile>
#include <QCoreApplication>
#include <QCryptographicHash>

#include "License_h.h"
#ifdef Q_OS_WIN32
#include "Win/WinLicense.h"
#else
#include "Linux/LinuxLicense.h"
#endif

#define LICENSE_MAIN(Guid) { QByteArray sn = GetSn(Guid); \
  QCryptographicHash hash(QCryptographicHash::Md5); \
  hash.addData(sn); \
  if (hash.result() == GetKey()) { \
    Log.Info("License ok"); \
  } else { \
    Log.Fatal("License fail"); \
    return -1101; \
  } \
  }

#define LICENSE_INIT(Guid) { QByteArray sn = GetSnShort(Guid); \
  QCryptographicHash hash(QCryptographicHash::Md5); \
  hash.addData(sn); \
  int line = ((Guid & 0xffff) % kLcCount) + 1; \
  if (hash.result() != GetKey(line)) { \
    return false; \
  } \
  }

#define LICENSE_CIRCLE(Guid) if (mCounterL.Test()) { QByteArray sn = GetSnShort(Guid); \
  QCryptographicHash hash(QCryptographicHash::Md5); \
  hash.addData(sn); \
  int line = ((Guid & 0xffff) % kLcCount) + 1; \
  if (hash.result() != GetKey(line)) { \
    Stop(); \
  } \
  }

inline QByteArray GetKey(int line = 0)
{
  QFile file(QCoreApplication::applicationDirPath().append('/').append('V').append('a').append('r')
             .append('/').append('k').append('e').append('y').append('.').append('i').append('n').append('i'));
  if (!file.open(QIODevice::ReadOnly)) {
    return QByteArray();
  }

  QByteArray data = file.readAll();
  QList<QByteArray> values = data.split(':');
  file.close();

  return (line < values.size())? QByteArray::fromHex(values[line]): QByteArray();
}

/*
E5724A6E 4AC6334EBF245FD1
63A0C9A6F1ABF459 B09341CA92575802
B90A86D42F8921E3 A4C9D8C4E0885495
63FB904C1BED7B43 A2EFD121C033DDD3
BD70597918806859 0BE3BF9ECFC402D0
5D07DC526B4E2CAE BC6BD96346C72E9A
F6861734B67B8309 B3D361F5AE8EEE4E
BA561F2859D9CB78 5797B7308E1F51B5
7782F0D4BD22053F 441F77B1D260294B
59DF52801A22BF69 238EA665EC13A730
54B9DB6A34EED594 C73D0761AB1E0CEA
F8713F7C082B8D6E C0B1DD3300511386
9E7C5A7EBCC8503A FF29F27F007D4CA3
CAAE8F67296F5DBE CC4323DFB3ECA366
FF36ED91D8B69DA0 DEC735DF0CF56243
7ECFA7455C2CFD65 552BC80FABF055D2
FC31AD7D873772FF 28ECABB7CD451C67
A81282CDDB96D9DF 3F89B197E231C542
E721D751CA4635A1 445A22A5E9F9A2ED
E5DDA401E58790F4 246C22FF1AED80CF
F58F5E269A95D7D2 7DB5B8BD97C8604A
4DF6707CA28483BB E8D2C6DEE5D529CB
F8F65126A7858D84 24E11F84678F10F9
903ADD681A582563 EB20695C93F3826B
86EC34433B771916 6FD41D25F1390349
205A98272B09276A 166CE02A0F2635C1
6A73E29D1859BF26 373C009890B7CB7E
A1E608A897AE0BD1 2A1A53AF9CB3AA74
EF10B3699F066093 419C066814BCF8B1
0CC2F7CBA7DE2356 B39581D75479969F
5A59904BD51AB45C 7B7559F8EC1EE80D
5D912B1D175AA81B E82224743697E744
F48B658C2FA9DD42 FBD328F978E20BAF
A75FCEB3DE20602F 424CE91D2C1738A0
CE55BBFC837DDA19 B52892CE9013E6B6
*/
#else
#define LICENSE_MAIN(Guid) { }
#define LICENSE_INIT(Guid) { }
#define LICENSE_CIRCLE(Guid) { }
#endif
