#pragma once

#include <QSharedPointer>
#include <QVector>
#include <QByteArray>
#include <QDataStream>


class HystText;
typedef QSharedPointer<HystText> HystTextS;
class QTextCodec;
class QTextEncoder;
class QTextDecoder;

class AutoCodec
{
  struct LangInfo {
    QString           LangName;
    HystTextS         LangHyst;
  };

  QStringList        mCodecList;
  QStringList        mCharSetList;
  QStringList        mLangList;
  QString            mNeytralCharSet;

  struct CodecInfo {
    QTextCodec*       Codec;
    QTextEncoder*     Encoder;
    QTextDecoder*     Decoder;
    QString           CodecName;
    HystTextS         RangesHyst;
    QVector<LangInfo> LangInfoList;
  };
  QVector<CodecInfo> mCodecInfoList;
  QVector<int>       mNeytralCharsMap;

  CodecInfo*         mCurrentCodec;
  LangInfo*          mCurrentLang;
  QString            mHintCodec;
  QString            mHintLang;
  qreal              mPrecision;

public:
  QString          CurrentCodec() const { return mCurrentCodec? mCurrentCodec->CodecName: QString(); }
  QTextEncoder*  CurrentEncoder() const { return mCurrentCodec->Encoder; }
  QTextDecoder*  CurrentDecoder() const { return mCurrentCodec->Decoder; }
  QString           CurrentLang() const { return mCurrentLang? mCurrentLang->LangName: QString(); }
  qreal        CurrentPrecision() const { return mPrecision; }

  void   SetCodecList(const QStringList& list) { mCodecList   = list; }
  void SetCharSetList(const QStringList& list) { mCharSetList = list; }
  void    SetLangList(const QStringList& list) { mLangList    = list;}
  void SetNeytralCharSet(const QString& set) { mNeytralCharSet = set;}

public:
  bool Create(const QString& dictsPath);
  bool Save(QByteArray& data) const;
  bool Load(const QByteArray& data);

  bool DefineCodecLang(const QByteArray& text);
  bool DefineCodec(const QByteArray& text, const QString& lang);
  bool DefineLang(const QByteArray& text, const QString& codec);

  bool DefineCodec(const QByteArray& text);
  bool CheckCodec(const QByteArray& text, const QString& codecName);

  QString Dump() const;

private:
  QString LoadCharset(const QString& charSetName);
  void AddLangWord(int langIndex, const QString& word, int count);

  bool DefineCurrent(const QByteArray& text);

public:
  AutoCodec();
};
