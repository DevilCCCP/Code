#pragma once

#include <QString>
#include <QStringList>

#include <Lib/Include/Common.h>


DefineClassS(SmtpMail);

class SmtpMail
{
  PROPERTY_GET_SET(QString,     From)
  PROPERTY_GET_SET(QStringList, To)
  PROPERTY_GET_SET(QStringList, Cc)
  PROPERTY_GET_SET(QStringList, Bcc)

  PROPERTY_GET_SET(QString,     Subject)
  PROPERTY_GET_SET(QString,     Body)
  PROPERTY_GET_SET(bool,        BodyHtml)
  ;
public:
  class Attach
  {
  private:
    PROPERTY_GET_SET(QString,    Name)
    PROPERTY_GET_SET(QString,    MimeType)
    PROPERTY_GET_SET(QByteArray, Data)
  };
  typedef QList<Attach> AttachList;
  PROPERTY_GET_SET(AttachList, Attaches)
  ;
public:
  void AddAttach(const Attach& attach) { mAttaches.append(attach); }

  SmtpMail()
    : mBodyHtml(false)
  { }
};
