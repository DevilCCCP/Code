#pragma once

#include <Lib/Db/DbTable.h>
#include <Lib/DbUi/DbTableModel.h>
#include <Lib/Common/CsvWriter.h>
#include <Lib/Common/CsvReader.h>


DefineClassS(FormTableAdapterA);
DefineClassS(CsvWriter);
DefineClassS(CsvReader);

const QByteArray kEsc("<>");
const QByteArray kNull("<null>");
const QByteArray kBase64("<b64>");

struct TableSchema
{
  struct TableFilter
  {
    enum EEqualType {
      eLike,
      eLikeInside,
      eByteArray,
      eEqGreater,
      eEqualKey,
      eEqual,
      eEqText,
      eTimeRange,
      eDateRange
    };
    EEqualType  EqualType;
    QString     Name;
    QString     Column;
    QIcon       Icon;
    bool        Main;

    TableFilter(EEqualType _EqualType, const QString& _Name, const QString& _Column, const QIcon& _Icon, bool _Main = true)
      : EqualType(_EqualType), Name(_Name), Column(_Column), Icon(_Icon), Main(_Main)
    { }
    TableFilter()
    { }
  };

  QString              Name;
  QList<TableFilter>   Filters;

  TableSchema(const QString& _Name): Name(_Name)
  { }
  TableSchema()
  { }
};

class FormTableAdapterA {
public:
  /*new */virtual TableSchema* GetTableSchema() = 0;
  /*new */virtual int LoadQuery(const QString& where) = 0;
  /*new */virtual int LoadLimit() { return 200; }
  /*new */virtual QAbstractTableModel* Model() = 0;
  /*new */virtual DbItemAS GetItemA(int index) = 0;
  /*new */virtual bool Clone(int index) = 0;
  /*new */virtual bool Delete(int index) = 0;
  /*new */virtual bool ExportAll(CsvWriter* writer) = 0;
  /*new */virtual bool ImportAll(CsvReader* reader, QString* info = nullptr) = 0;
  /*new */virtual bool CanBackup() { return false; }
  /*new */virtual bool Backup(CsvWriter* writer) { Q_UNUSED(writer) return false; }
  /*new */virtual bool Restore(CsvReader* reader) { Q_UNUSED(reader) return false; }

  /*new */virtual ~FormTableAdapterA() = default;
};

template <typename IntT, typename DbItemT>
class FormTableAdapterT: public FormTableAdapterA {
protected:
  typedef QSharedPointer<DbItemT>                  DbItemTS;
  typedef DbTableModel<DbItemT>                    DbTableModelT;
  typedef QSharedPointer<DbTableModel<DbItemT> >   DbTableModelTS;
  typedef QSharedPointer<DbTableT<IntT, DbItemT> > DbTableTS;

  const Db&                     mDb;
  PROPERTY_GET(DbTableTS,       Table)
  PROPERTY_GET(DbTableModelTS,  Model)
  PROPERTY_GET(QList<DbItemTS>, Items)
  PROPERTY_GET(QString,         BackupTable)

public:
  /*override */virtual int LoadQuery(const QString& where) override
  {
    mItems.clear();
    mTable->Select(where, mItems);
    mModel->SetList(mItems);
    return mItems.size();
  }

  /*override */virtual QAbstractTableModel* Model() override
  {
    return mModel.data();
  }

  /*override */virtual DbItemAS GetItemA(int index) override
  {
    if (index >= 0 && index < mItems.size()) {
      return mItems.at(index).template staticCast<DbItemA>();
    }
    return DbItemAS();
  }

  /*override */virtual bool Clone(int index) override
  {
    if (index >= 0 && index < mItems.size()) {
      return mTable->InsertCopy(mItems.at(index));
    }
    return false;
  }

  /*override */virtual bool Delete(int index) override
  {
    if (index >= 0 && index < mItems.size()) {
      return mTable->Delete(mItems.at(index)->Id);
    }
    return false;
  }

  /*override */virtual bool ExportAll(CsvWriter* writer) override
  {
    int size = mModel->columnCount();
    for (int i = 0; i < size; i++) {
      if (!writer->WriteValue(mModel->headerData(i, Qt::Horizontal, Qt::DisplayRole).toByteArray())) {
        return false;
      }
    }
    if (!writer->WriteEndLine()) {
      return false;
    }

    int rowCount = mModel->rowCount();
    for (int j = 0; j < rowCount; j++) {
      for (int i = 0; i < size; i++) {
        if (!writer->WriteValue(mModel->data(mModel->index(j, i), Qt::DisplayRole).toByteArray())) {
          return false;
        }
      }
      if (!writer->WriteEndLine()) {
        return false;
      }
    }

    return true;
  }

  /*override */virtual bool ImportAll(CsvReader* reader, QString* info = nullptr) override
  {
    QStringList header;
    if (!reader->ReadLine(header)) {
      return false;
    }
    int headersCount = mModel->columnCount();
    if (header.size() != headersCount) {
      if (info) {
        *info = QString(QObject::tr("Format of file is wrong. Expecting %1 columns, found %2")).arg(headersCount).arg(header.size());
      }
      return false;
    }
    QStringList modelHeaders;
    for (int i = 0; i < headersCount; i++) {
      modelHeaders.append(mModel->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString());
    }

    QVector<int> columnMap(header.size());
    for (int j = 0; j < header.size(); j++) {
      const QString& head = header.at(j);
      int index = -1;
      for (int i = 0; i < modelHeaders.size(); i++) {
        if (head == modelHeaders.at(i)) {
          index = i;
          break;
        }
      }
      if (index < 0) {
        if (info) {
          *info = QString(QObject::tr("Format of file is wrong. Column '%1' not identify")).arg(head);
        }
        return false;
      }
      columnMap[j] = index;
    }

    int total = 0;
    while (!reader->AtEnd()) {
      QStringList line;
      if (!reader->ReadLine(line)) {
        return false;
      }
      if (line.size() != columnMap.size()) {
        if (info) {
          *info = QString(QObject::tr("Format record %1 (zero based) is wrong columns count not equal cells count at this record (columns: %2, cells: %3).\nRecord:\n"))
              .arg(total).arg(columnMap.size()).arg(line.size()) + line.join(";");
          if (total > 0) {
            *info += QString(QObject::tr("\n\nImport %1 entries")).arg(total);
          }
        }
        return false;
      }

      QVector<QString> items;
      for (int j = 0; j < columnMap.size(); j++) {
        items.append(line.at(columnMap.at(j)));
      }
      if (!ImportItem(items)) {
        if (info) {
          *info = QString(QObject::tr("Import record %1 (zero based) fail.\nRecord:\n")).arg(total) + line.join(";");
          if (total > 0) {
            *info += QString(QObject::tr("\n\nImport %1 entries")).arg(total);
          }
        }
        return false;
      }
      total++;
    }

    if (info) {
      *info = QString(QObject::tr("Import %1 entries")).arg(total);
    }
    return true;
  }

  /*override */virtual bool CanBackup() override
  {
    return !mBackupTable.isEmpty();
  }

  /*override */virtual bool Backup(CsvWriter* writer) override
  {
    if (mBackupTable.isEmpty()) {
      Log.Warning(QString("Try backup non backupable model"));
      return false;
    }

    auto q = mDb.MakeQuery();
    q->prepare(QString("SELECT column_name, data_type FROM information_schema.columns WHERE table_name = %1").arg(ToSql(mBackupTable)));
    if (!mDb.ExecuteQuery(q)) {
      return false;
    }
    bool hasId = false;
    QStringList columns;
    while (q->next()) {
      QString name = q->value(0).toString();
      if (name != "_id") {
        columns.append(name);
      } else {
        hasId = true;
      }
    }
    if (!hasId) {
      Log.Warning(QString("Backup column '_id' not found"));
      return false;
    }

    if (!writer->WriteValue("_id")) {
      return false;
    }
    if (!writer->WriteLine(columns)) {
      return false;
    }

    q->prepare(QString("SELECT _id FROM %1 ORDER BY _id LIMIT 1").arg(mBackupTable));
    if (!mDb.ExecuteQuery(q)) {
      return false;
    }
    if (!q->next()) {
      return true;
    }

    QLocale::setDefault(QLocale::C);
    const int kRecordsAtOnce = 200;
    qint64 lastId = q->value(0).toLongLong() - 1;
    forever {
      qint64 nextId = lastId;
      q->prepare(QString("SELECT _id, %2 FROM %1 WHERE _id > %3 ORDER BY _id LIMIT %4")
                 .arg(mBackupTable).arg(columns.join(',')).arg(lastId).arg(kRecordsAtOnce));
      if (!mDb.ExecuteQuery(q)) {
        return false;
      }
      while (q->next()) {
        nextId = q->value(0).toLongLong();
        if (!writer->WriteValue(QByteArray::number(nextId))) {
          return false;
        }
        for (int i = 1; i <= columns.size(); i++) {
          QVariant value = q->value(i);
          QByteArray data;
          if (value.isNull()) {
            data = kNull;
          } else switch (value.type()) {
          case QVariant::Invalid:
            data = kNull;
            break;
          case QVariant::DateTime:
            data = value.toDateTime().toString("yyyy-MM-ddThh:mm:ss.zzz").toLatin1();
            break;
          case QVariant::Date:
            data = value.toDate().toString("yyyy-MM-dd").toLatin1();
            break;
          case QVariant::String:
            data = value.toString().toUtf8();
            if (data.startsWith(kEsc) || data.startsWith(kNull) || data.startsWith(kBase64)) {
              data.prepend(kEsc);
            }
            break;
          case QVariant::ByteArray:
            data = QByteArray(kBase64) + value.toByteArray().toBase64();
            break;
          default:
            data = value.toString().toUtf8();
            break;
          }
          if (!writer->WriteValue(data)) {
            return false;
          }
        }
        if (!writer->WriteEndLine()) {
          return false;
        }
      }
      if (nextId == lastId) {
        return true;
      }
      lastId = nextId;
    }

    return true;
  }

  /*override */virtual bool Restore(CsvReader* reader) override
  {
    if (mBackupTable.isEmpty()) {
      Log.Warning(QString("Try restore non backupable model"));
      return false;
    }

    auto q = mDb.MakeQuery();
    q->prepare(QString("SELECT column_name, data_type FROM information_schema.columns WHERE table_name = %1").arg(ToSql(mBackupTable)));
    if (!mDb.ExecuteQuery(q)) {
      return false;
    }
    QStringList columns;
    while (q->next()) {
      QString name = q->value(0).toString();
      columns.append(name);
    }
    if (columns.isEmpty()) {
      Log.Warning(QString("Restore columns not found"));
      return false;
    }

    QStringList headers;
    if (!reader->ReadLine(headers)) {
      return false;
    }
    if (headers.size() != columns.size()) {
      Log.Warning(QString("Restore columns count not equal to file header's "));
      return false;
    }

    QStringList queryList;
    foreach (const QString& header, headers) {
      if (!columns.contains(header)) {
        Log.Warning(QString("Restore columns not equal to file header's "));
        return false;
      }
      queryList << "?";
    }

    q->prepare(QString("DELETE FROM %1").arg(mBackupTable));
    if (!mDb.ExecuteNonQuery(q)) {
      return false;
    }

    QString queryLine = QString("(") + queryList.join(',') + QString(")");

    const int kRecordsAtOnce = 200;
    QList<QStringList> rows;
    qint64 topId = 1;
    while (!reader->AtEnd()) {
      while (rows.size() < kRecordsAtOnce && !reader->AtEnd()) {
        QStringList line;
        if (!reader->ReadLine(line)) {
          return false;
        }
        if (line.size() != headers.size()) {
          Log.Warning(QString("Columns count not equal cells count at this record"
                              " (columns: %1, cells: %2).\nRecord:\n")
                      .arg(headers.size()).arg(line.size()) + line.join(";"));
          return false;
        }

        rows.append(line);
      }

      if (rows.isEmpty()) {
        break;
      }

      QString query = QString("INSERT INTO %1 (%2) VALUES ").arg(mBackupTable).arg(columns.join(','));
      query.append(queryLine);
      for (int i = 1; i < rows.size(); i++) {
        query.append(QString(", ") + queryLine);
      }
      q->prepare(query);

      foreach (const QStringList& row, rows) {
        qint64 id = row.at(0).toLongLong();
        q->addBindValue(id);
        topId = qMax(topId, id);
        for (int i = 1; i < row.size(); i++) {
          const QString& cell = row.at(i);
          if (cell.startsWith(kEsc)) {
            QString newCell = cell.mid(kEsc.size());
            q->addBindValue(newCell);
          } else if (cell.startsWith(kNull)) {
            q->addBindValue(QVariant());
          } else if (cell.startsWith(kBase64)) {
            q->addBindValue(QByteArray::fromBase64(cell.mid(kBase64.size()).toLatin1()));
          } else {
            q->addBindValue(cell);
          }
        }
      }

      if (!mDb.ExecuteQuery(q)) {
        return false;
      }
      rows.clear();
    }

    q->prepare(QString("SELECT setval('%1__id_seq', %2)").arg(mBackupTable).arg(topId));
    if (!mDb.ExecuteNonQuery(q)) {
      return false;
    }

    return true;
  }

  /*new */virtual bool ImportItem(QVector<QString>& items) = 0;

public:
  DbItemTS GetItem(int index)
  {
    if (index >= 0 && index < mItems.size()) {
      return mItems.at(index);
    }
    return DbItemTS();
  }


public:
  FormTableAdapterT(const Db& _Db, const DbTableTS& _Table, const DbTableModelTS& _Model, const QString& _BackupTable = QString())
    : mDb(_Db), mTable(_Table), mModel(_Model), mBackupTable(_BackupTable)
  {
  }
};
