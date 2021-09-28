#pragma once

#include <QLabel>
#include <QFormLayout>

#include <Lib/Db/DbTable.h>
#include <Lib/DbUi/DbTableModel.h>
#include <Lib/Common/CsvWriter.h>
#include <Lib/Common/CsvReader.h>

#include "TableEditSchema.h"


DefineClassS(FormTableEditAdapterA);
DefineClassS(CsvWriter);
DefineClassS(CsvReader);

class FormTableEditAdapterA {
public:
  /*new */virtual TableEditSchema* GetTableSchema() = 0;
  /*new */virtual QAbstractTableModel* Model() = 0;
  /*new */virtual DbItemAS GetItemA(int index) = 0;
  /*new */virtual bool Load() = 0;
  /*new */virtual bool Clone(int index) = 0;
  /*new */virtual bool Delete(int index) = 0;
  /*new */virtual bool ExportAll(CsvWriter* writer) = 0;
  /*new */virtual bool ImportAll(CsvReader* reader, QString* info = nullptr) = 0;

  /*new */virtual void CreateEditControls(QWidget* parent) = 0;
  /*new */virtual bool NewEditItem() = 0;
  /*new */virtual bool LoadEditItem(int index) = 0;
  /*new */virtual bool SaveEditItem() = 0;
};

template <typename IntT, typename DbItemT>
class FormTableEditAdapterT: public FormTableEditAdapterA {
protected:
  typedef QSharedPointer<DbItemT>                  DbItemTS;
  typedef DbTableModel<DbItemT>                    DbTableModelT;
  typedef QSharedPointer<DbTableModel<DbItemT> >   DbTableModelTS;
  typedef QSharedPointer<DbTableT<IntT, DbItemT> > DbTableTS;

  const Db&                     mDb;
  PROPERTY_GET(DbTableTS,       Table)
  PROPERTY_GET(DbTableModelTS,  Model)
  PROPERTY_GET(QList<DbItemTS>, Items)

  PROPERTY_GET(int,             LoadIndex)

public:
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

  /*override */virtual bool Load() override
  {
    mItems.clear();
    if (!mTable->Select(Select(), mItems)) {
      return false;
    }
    mModel->SetList(mItems);
    return true;
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
        *info = QString("Format of file is wrong. Expecting %1 columns, found %2").arg(headersCount).arg(header.size());
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
          *info = QString("Format of file is wrong. Column '%1' not identify").arg(head);
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
          *info = QString("Format record %1 (zero based) is wrong"
                          " columns count not equal cells count at this record"
                          " (columns: %2, cells: %3).\nRecord:\n")
              .arg(total).arg(columnMap.size()).arg(line.size()) + line.join(";");
          if (total > 0) {
            *info += QString("\n\nImport %1 entries").arg(total);
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
          *info = QString("Import record %1 (zero based) fail.\nRecord:\n").arg(total) + line.join(";");
          if (total > 0) {
            *info += QString("\n\nImport %1 entries").arg(total);
          }
        }
        return false;
      }
      total++;
    }

    if (info) {
      *info = QString("Import %1 entries").arg(total);
    }
    UpdateModels();
    return true;
  }

  /*override */virtual void CreateEditControls(QWidget* parent) override
  {
    QFormLayout* formLayoutEdit = new QFormLayout(parent);
    formLayoutEdit->setObjectName(QStringLiteral("formLayoutEdit"));
    formLayoutEdit->setContentsMargins(0, 0, 0, 0);

    TableEditSchema* schema = GetTableSchema();
    foreach (const TableEditSchema::ColumnSchema& columnSchema, schema->Columns) {
      formLayoutEdit->addRow(columnSchema.Lable, columnSchema.ColumnEdit->CreateControl(parent));
    }
  }

  /*override */virtual bool NewEditItem() override
  {
    UpdateModels();

    mLoadIndex = -1;
    TableEditSchema* schema = GetTableSchema();
    for (int i = 0; i < schema->Columns.size(); i++) {
      if (!schema->Columns[i].ColumnEdit->LoadValue(QVariant())) {
        return false;
      }
    }

    return true;
  }

  /*override */virtual bool LoadEditItem(int index) override
  {
    mLoadIndex = index;
    QVariantList values;
    if (!LoadEditValues(mLoadIndex, values)) {
      return false;
    }

    TableEditSchema* schema = GetTableSchema();
    if (values.size() != schema->Columns.size()) {
      return false;
    }
    UpdateModels();
    for (int i = 0; i < schema->Columns.size(); i++) {
      if (!schema->Columns[i].ColumnEdit->LoadValue(values.at(i))) {
        return false;
      }
    }

    return true;
  }

  /*override */virtual bool SaveEditItem() override
  {
    QVariantList values;
    TableEditSchema* schema = GetTableSchema();
    for (int i = 0; i < schema->Columns.size(); i++) {
      QVariant value;
      if (!schema->Columns[i].ColumnEdit->SaveValue(value)) {
        return false;
      }
      values.append(value);
    }

    if (!SaveEditValues(mLoadIndex, values)) {
      return false;
    }

    UpdateModels();
    return true;
  }

  /*new */virtual QString Select() { return QString(); }

  /*new */virtual bool ImportItem(QVector<QString>& items) = 0;
  /*new */virtual bool LoadEditValues(int index, QVariantList& values) = 0;
  /*new */virtual bool SaveEditValues(int index, const QVariantList& values) = 0;

  /*new */virtual void UpdateModels() = 0;

public:
  DbItemTS GetItem(int index)
  {
    if (index >= 0 && index < mItems.size()) {
      return mItems.at(index);
    }
    return DbItemTS();
  }


public:
  FormTableEditAdapterT(const Db& _Db, const DbTableTS& _Table, const DbTableModelTS& _Model)
    : mDb(_Db), mTable(_Table), mModel(_Model)
  {
    Load();
  }
};
