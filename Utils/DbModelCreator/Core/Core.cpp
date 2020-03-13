#include <QFile>
#include <QVariant>

#include "Core.h"


bool Core::Generate(const QString& script, QString prefix)
{
  mPrefix = prefix;
  mUse64bit = false;

  mSource = script.toLower().replace("now()", "");
  mIndex = 0;
  if (!MoveAfter("create")) {
    return false;
  }
  SkipSpaces();

  if (!IsNextIsSkip("table")) {
    return false;
  }

  SkipSpaces();
  if (IsNextIsSkip("if")) {
    SkipSpaces();
    if (IsNextIsSkip("not")) {
      return false;
    }
    SkipSpaces();
    if (IsNextIsSkip("exists")) {
      return false;
    }
  }

  mTable = TakeName();
  if (mTable.isEmpty()) {
    return false;
  }

  if (!IsNextIsSkip("(")) {
    return false;
  }

  mColumnsList.clear();
  do {
    SkipSpaces();
    QString name = TakeName();
    if (name == "constraint" || name == "like") {
      continue;
    }
    Column column;
    column.ColumnName   = name;
    column.ColumnText   = UnderscopeToSpace(name);
    column.VariableName = UnderscopeToBig(name);
    column.Key          = name.startsWith("_");

    SkipSpaces();
    QString type = TakeName();
    if (type == "serial") {
      mUse64bit = false;
      continue;
    } else if (type == "integer") {
      column.Type = Column::eInt;
    } else if (type == "bigserial") {
      mUse64bit = true;
      continue;
    } else if (type == "bigint") {
      column.Type = Column::eLong;
    } else if (type == "real") {
      column.Type = Column::eReal;
    } else if (type == "boolean") {
      column.Type = Column::eBool;
    } else if (type == "time") {
      column.Type = Column::eTimestamp;
    } else if (type == "timestamp") {
      column.Type = Column::eTimestamp;
    } else if (type == "text") {
      column.Type = Column::eText;
    } else if (type == "character(1)") {
      column.Type = Column::eChar;
    } else if (type.startsWith("character(")) {
      column.Type = Column::eText;
    } else if (type == "bytea") {
      column.Type = Column::eByteArray;
    } else if (type == "point") {
      column.Type = Column::ePoint;
    } else if (type == "box") {
      column.Type = Column::eRect;
    } else {
      continue;
    }
    mColumnsList.append(column);

  } while (MoveToAnyOf("),") && mSource[mIndex] == ',' && ++mIndex < mSource.size());

  mClassName = UnderscopeToBig(mTable);
  mValueName = (mUse64bit)? "qint64": "int";

  mDeclare.clear();
  mEquality.clear();
  mCtorInit.clear();
  mRowRead.clear();
  mRowWrite.clear();
  mColumns.clear();
  QStringList columnNames;
  QStringList columnTexts;
  QStringList switchTexts;
  QStringList switchGetTexts;
  QStringList switchSetTexts;
  QStringList switchGetDatas;
  QStringList switchSetDatas;
  QStringList switchKeyTexts;
  QStringList switchKeySetTexts;
  int index = 0;
  int keyIndex = 0;
  mHasName = false;
  for (auto itr = mColumnsList.begin(); itr != mColumnsList.end(); itr++) {
    const Column& column = *itr;
    mDeclare.append(QString("  %1 %2;\n").arg(column.TypeToString(), -10).arg(column.VariableName));
    mEquality.append(QString(" && %1 == vs.%1").arg(column.VariableName));
    if (column.Key) {
      mCtorInit.append(QString(", %1(0)").arg(column.VariableName));
    }

    columnNames << column.ColumnName;
    if (column.Type == Column::ePoint) {
      mRowRead.append(QString("\n  it->%1 = Db::FromPoint(q->value(index++).value<QString>());").arg(column.VariableName));
    } else {
      mRowRead.append(QString("\n  it->%1 = q->value(index++).value<%2>();").arg(column.VariableName).arg(column.TypeToString()));
    }

    if (column.Key) {
      mRowWrite.append(QString("\n  q->bindValue(index++, Db::ToKey(it.%1));").arg(column.VariableName));
      switchKeyTexts << QString("    case %1: return %2;").arg(keyIndex).arg(column.VariableName);
      switchKeySetTexts << QString("    case %1: %2 = id; break;").arg(keyIndex).arg(column.VariableName);
      keyIndex++;
    } else {
      if (column.VariableName.toLower() == "name") {
        mHasName = true;
      }
      columnTexts << QString(" << \"%1\"").arg(column.ColumnText);
      switchTexts << QString("  case %1: return %2;").arg(index).arg(QString(column.TypeToStringToString()).arg("item->" + column.VariableName));
      switchGetTexts << QString("    case %1: return %2;").arg(index).arg(QString(column.TypeToStringToString()).arg(column.VariableName));
      switchGetDatas << QString("    case %1: return %2;").arg(index).arg(QString(column.TypeToVariantToString()).arg(column.VariableName));
      switchSetTexts << QString("    case %1: %2 = %3; return true;").arg(index).arg(column.VariableName).arg(QString(column.TypeFromStringToString()).arg("text"));
      switchSetDatas << QString("    case %1: %2 = %3; return true;").arg(index).arg(column.VariableName).arg(QString(column.TypeFromVariantToString()).arg("data"));
      index++;
      if (column.Type == Column::ePoint) {
        mRowWrite.append(QString("\n  q->bindValue(index++, Db::ToPoint(it.%1));").arg(column.VariableName));
      } else {
        mRowWrite.append(QString("\n  q->bindValue(index++, it.%1);").arg(column.VariableName));
      }
    }
  }
  mColumns = columnNames.join(',');
  mColumnText = columnTexts.join("");
  mSwitchText = switchTexts.join('\n');
  mSwitchGetText = switchGetTexts.join('\n');
  mSwitchGetData = switchGetDatas.join('\n');
  mSwitchSetText = switchSetTexts.join('\n');
  mSwitchSetData = switchSetDatas.join('\n');
  mSwitchKeyText = switchKeyTexts.join('\n');
  mSwitchKeySetText = switchKeySetTexts.join('\n');
  return true;
}

QString Core::GetClassName()
{
  return mClassName;
}

QString Core::GetClasses()
{
  QString text;
  QFile file(":/Templates/TemplateClasses.h");
  if (file.open(QFile::ReadOnly)) {
    text = QString::fromUtf8(file.readAll());
    text.replace("ClassT", mClassName);
  }
  return text;
}

QString Core::GetHeader()
{
  QString text;
  QFile file(":/Templates/Template.h");
  if (file.open(QFile::ReadOnly)) {
    text = QString::fromUtf8(file.readAll());
    if (mHasName) {
      text.replace("NAME_H1", "\n"
                              "  qint64 mCounter;"
                              "\n");
      text.replace("NAME_H2", "\n"
                              "  /*override */virtual bool CreateDefaultItem(QSharedPointer<DbItemT<qint64> >& item) Q_DECL_OVERRIDE;\n"
                              "  /*override */virtual void NewDefaultItem(QSharedPointer<DbItemT<qint64> >& item) Q_DECL_OVERRIDE;\n");
    } else {
      text.remove("NAME_H1");
      text.remove("NAME_H2");
    }
    text.replace("TypeT", mValueName);
    text.replace("ClassT", mClassName);
    text.replace("DECLARE", mDeclare);
    text.replace("CTOR_INIT", mCtorInit);
  }
  return text;
}

QString Core::GetSource()
{
  QString text;
  QFile file(":/Templates/Template.cpp");
  if (file.open(QFile::ReadOnly)) {
    text = QString::fromUtf8(file.readAll());
    if (mHasName) {
      text.replace("NAME_CPP1", "  if (mCounter < 0) {\n"
                                "    if (!SelectCount(\"\", mCounter)) {\n"
                                "      return false;\n"
                                "    }\n"
                                "  }\n"
                                "  ++mCounter;\n"
                                "  it->Name = QString(\"ClassT %1\").arg(mCounter);\n"
                                "\n");
      text.replace("NAME_CPP2", "\n  , mCounter(-1)");
    } else {
      text.remove("NAME_CPP1");
      text.remove("NAME_CPP2");
    }
    text.replace("TypeT", mValueName);
    text.replace("ClassT", mClassName);
    text.replace("TABLE_NAME", mTable);
    text.replace("COLUMNS", mColumns);
    text.replace("EQUALITY", mEquality);
    text.replace("ROW_READ", mRowRead);
    text.replace("ROW_WRITE", mRowWrite);
    text.replace("SWITCH_CASES_SET", mSwitchSetText);
    text.replace("SWITCH_CASES_SDATA", mSwitchSetData);
    text.replace("SWITCH_CASES_KEY", mSwitchKeyText);
    text.replace("SWITCH_CASES_SKEY", mSwitchKeySetText);
    text.replace("SWITCH_CASES_GET", mSwitchGetText);
    text.replace("SWITCH_CASES_DATA", mSwitchGetData);
    text.replace("COLUMN_LIST", mColumnText);
  }
  return text;
}

QString Core::GetModelHeader()
{
  QString text;
  QFile file(":/Templates/TemplateModel.h");
  if (file.open(QFile::ReadOnly)) {
    text = QString::fromUtf8(file.readAll());
    text.replace("PREFIX", mPrefix);
    text.replace("ClassT", mClassName);
  }
  return text;
}

QString Core::GetModelSource()
{
  QString text;
  QFile file(":/Templates/TemplateModel.cpp");
  if (file.open(QFile::ReadOnly)) {
    text = QString::fromUtf8(file.readAll());
    text.replace("ClassT", mClassName);
    text.replace("COLUMN_LIST", mColumnText);
    text.replace("SWITCH_CASES", mSwitchText);
  }
  return text;
}

bool Core::MoveToAnyOf(const QString& variants)
{
  for (; mIndex < mSource.size(); mIndex++) {
    if (variants.contains(mSource[mIndex])) {
      return true;
    }
  }
  return false;
}

bool Core::MoveAfter(const QString& key)
{
  mIndex = mSource.indexOf(key, mIndex);
  if (mIndex < 0) {
    return false;
  }
  mIndex += key.size();
  return true;
}

void Core::SkipSpaces()
{
  while (mIndex < mSource.size() && mSource[mIndex].isSpace()) {
    mIndex++;
  }
}

bool Core::IsNextIs(const QString& value)
{
  return mSource.mid(mIndex, value.size()) == value;
}

bool Core::IsNextIsSkip(const QString& value)
{
  if (IsNextIs(value)) {
    mIndex += value.size();
    return true;
  }
  return false;
}

QString Core::TakeName()
{
  QString name;
  if (mIndex >= mSource.size() || mSource[mIndex].isSpace()) {
    return QString();
  }

  if (mSource[mIndex] == '"') {
    int indexLast = mSource.indexOf('"', mIndex + 1);
    if (indexLast < 0) {
      return QString();
    }
    name = mSource.mid(mIndex + 1, indexLast - mIndex - 1);
    mIndex = indexLast + 1;
  } else {
    do {
      name.append(mSource[mIndex++]);
    } while (mIndex < mSource.size() && (!mSource[mIndex].isSpace() && mSource[mIndex] != ','));
  }
  SkipSpaces();

  return name;
}

QString Core::UnderscopeToBig(const QString& source)
{
  QString result;
  QStringList parts = source.split('_');
  for (auto itr = parts.begin(); itr != parts.end(); itr++) {
    QString part = *itr;
    if (!part.isEmpty()) {
      part[0] = part[0].toUpper();
    }
    result.append(part);
  }
  if (source.startsWith("_") && !source.startsWith("_id")) {
    result.append("Id");
  }
  return result;
}

QString Core::UnderscopeToSpace(const QString& source)
{
  QStringList parts = source.split('_');
  if (!parts.isEmpty()) {
    QString& part = parts.first();
    if (!part.isEmpty()) {
      part[0] = part[0].toUpper();
    }
  }
  if (source.startsWith("_")) {
    parts.clear();
  }
  return parts.join(' ');
}


Core::Core()
{
  Q_INIT_RESOURCE(Core);
}
