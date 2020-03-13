#pragma once

#include <QString>


struct Column {
  enum EType {
    eInt,
    eLong,
    eReal,
    eBool,
    eTimestamp,
    eText,
    eChar,
    eByteArray,
    ePoint,
    eRect,
    eIllegal
  };

  EType   Type;
  QString ColumnName;
  QString ColumnText;
  QString VariableName;
  bool    Key;

  const char* TypeToString() const
  {
    switch (Type) {
    case eInt:       return "int";
    case eLong:      return "qint64";
    case eReal:      return "qreal";
    case eBool:      return "bool";
    case eTimestamp: return "QDateTime";
    case eText:      return "QString";
    case eChar:      return "QChar";
    case eByteArray: return "QByteArray";
    case ePoint:     return "QPoint";
    case eRect:      return "QRect";
    case eIllegal:   return "illegal";
    }
    return "error";
  }

  const char* TypeToStringToString() const
  {
    switch (Type) {
    case eInt:       return "QString::number(%0)";
    case eLong:      return "QString::number(%0)";
    case eReal:      return "QString::number(%0)";
    case eBool:      return "%0? QString(\"true\"): QString(\"false\")";
    case eTimestamp: return "%0.toString()";
    case eText:      return "%0";
    case eChar:      return "%0";
    case eByteArray: return "TextFromData(%0)";
    case ePoint:     return "QString(\"(\") + QString::number(%0.x()) + QString(\", \") + QString::number(%0.y()) + QString(\")\")";
    case eRect:      return "QString(\"(\") + QString::number(%0.left()) + QString(\", \") + QString::number(%0.top())"
                            " + QString(\", \") + QString::number(%0.right()) + QString(\", \") + QString::number(%0.bottom()) + QString(\")\")";
    case eIllegal:   return "illegal";
    }
    return "error";
  }

  const char* TypeFromStringToString() const
  {
    switch (Type) {
    case eInt:       return "%0.toInt()";
    case eLong:      return "%0.toLongLong()";
    case eReal:      return "%0.toDouble()";
    case eBool:      return "(%0.toLower() == \"true\" || %0 == \"1\")";
    case eTimestamp: return "QDateTime::fromString(%0)";
    case eText:      return "%0";
    case eChar:      return "!%0.isEmpty()? %0.first(): QChar('0')";
    case eByteArray: return "QByteArray::fromHex(text.toLatin1())";
    case ePoint:     return "false";
    case eRect:      return "false";
    case eIllegal:   return "illegal";
    }
    return "error";
  }

  const char* TypeToVariantToString() const
  {
    switch (Type) {
    case eInt:       return "QVariant(%0)";
    case eLong:      return "QVariant(%0)";
    case eReal:      return "QVariant(%0)";
    case eBool:      return "QVariant(%0)";
    case eTimestamp: return "QVariant(%0)";
    case eText:      return "QVariant(%0)";
    case eChar:      return "QVariant(%0)";
    case eByteArray: return "QVariant(%0)";
    case ePoint:     return "QVariant(%0)";
    case eRect:      return "QVariant(%0)";
    case eIllegal:   return "illegal";
    }
    return "error";
  }

  const char* TypeFromVariantToString() const
  {
    switch (Type) {
    case eInt:       return "%0.toInt()";
    case eLong:      return "%0.toLongLong()";
    case eReal:      return "%0.toDouble()";
    case eBool:      return "%0.toBool()";
    case eTimestamp: return "%0.toDateTime()";
    case eText:      return "%0.toString()";
    case eChar:      return "%0.toChar()";
    case eByteArray: return "%0.toByteArray()";
    case ePoint:     return "%0.toPoint()";
    case eRect:      return "%0.toRect()";
    case eIllegal:   return "illegal";
    }
    return "error";
  }
};

class Core
{
  QString       mSource;
  int           mIndex;
  bool          mUse64bit;

  QString       mPrefix;
  QString       mTable;
  QString       mClassName;
  QString       mValueName;
  QString       mDeclare;
  QString       mEquality;
  QString       mCtorInit;
  QString       mRowRead;
  QString       mRowWrite;
  QString       mColumns;
  QString       mColumnText;
  QString       mSwitchText;
  QString       mSwitchGetText;
  QString       mSwitchSetText;
  QString       mSwitchGetData;
  QString       mSwitchSetData;
  QString       mSwitchKeyText;
  QString       mSwitchKeySetText;
  bool          mHasName;
  QList<Column> mColumnsList;

public:
  bool Generate(const QString& script, QString prefix = QString());
  QString GetClassName();
  QString GetClasses();
  QString GetHeader();
  QString GetSource();
  QString GetModelHeader();
  QString GetModelSource();

private:
  bool MoveToAnyOf(const QString& variants);
  bool MoveAfter(const QString& key);
  void SkipSpaces();
  bool IsNextIs(const QString& value);
  bool IsNextIsSkip(const QString& value);
  QString TakeName();

  QString UnderscopeToBig(const QString& source);
  QString UnderscopeToSpace(const QString& source);

public:
  Core();
};

