#include <QMessageBox>
#include <QFile>
#include <QScopedPointer>

#include <Lib/Common/Icon.h>
#include <Lib/Common/CsvReader.h>
#include <Lib/Common/CsvWriter.h>
#include <Lib/Db/ObjectType.h>
#include <Lib/Db/ObjectSettings.h>
#include <Lib/Db/ObjectSettingsType.h>
#include <Lib/Log/Log.h>

#include "FormScript.h"
#include "ui_FormScript.h"
#include "ObjectModel.h"
#include "ToolForm.h"
#include "PropertyForm.h"


// Defined in QtAppGui.h
const QString GetProgramName();

enum EHeader {
  eUin       = 0,
  eId        = 1,
  eTemplate  = 2,
  eName      = 3,
  eDescr     = 4,
  eParent    = 5,
  eLinks     = 6,
  eState     = 7,
  eCustom    = 8
};

//QStringList kDefaultHeaders = QStringList() << "Id" << "Template" << "Name" << "Parent" << "Links" << "Status";
const QStringList kDefaultHeaders = QStringList() << "Уин" << "Ид" << "Шаблон" << "Название" << "Описание" << "Родитель" << "Подключения" << "Состояние";
const QString kDefaultEmpty = "<пусто>";

const QString kReportLineOk("<span style=\"color:#008300;\">Успешно %1</span> строк (<span style=\"color:#008300;\">%2</span> действий)");
const QString kReportLinePart("<span style=\"color:#ce4400;\">Частично %1</span> строк"
                              " (<span style=\"color:#008300;\">%2</span> действий <span style=\"color:#008300;\">успешно</span>"
                              ", <span style=\"color:#a10000;\">%3</span> действий <span style=\"color:#a10000;\">ошибки</span>)");
const QString kReportLineFail("<span style=\"color:#a10000;\">Неуспешно %1</span> строк");

FormScript::FormScript(QWidget* parent)
  : QWidget(parent)
  , ui(new Ui::FormScript)
  , mScriptModel(new QStandardItemModel(this))
  , mFileDialog(new QFileDialog(this))
{
  ui->setupUi(this);

  ui->tableViewScript->setModel(mScriptModel);
  ui->textEditLog->setVisible(false);

  ui->actionScriptCreate->setIcon(IconFromImage(":/ObjTree/Icons/System.png", ":/ObjTree/Icons/Script.png"));
  ui->actionScriptCreateUp->setIcon(IconFromImage(":/ObjTree/Icons/Refresh.png", ":/ObjTree/Icons/Script.png"));
  ui->actionScriptOpen->setIcon(IconFromImage(":/ObjTree/Icons/Open.png", ":/ObjTree/Icons/Script.png"));
  ui->actionScriptSave->setIcon(IconFromImage(":/ObjTree/Icons/Save.png", ":/ObjTree/Icons/Script.png"));

  mFileDialog.setDirectory(QApplication::instance()->applicationDirPath());
  mFileDialog.setNameFilter("Файлы csv (*.csv *.xls);;Текстовые файлы (*.txt);;Все файлы (*.*)");
}

FormScript::~FormScript()
{
  delete ui;
}

void FormScript::Init(ObjectModel* _ObjectModel, ToolForm* _ToolForm, PropertyForm* _PropertyForm)
{
  mObjectModel  = _ObjectModel;
  mToolForm     = _ToolForm;
  mPropertyForm = _PropertyForm;

  mScriptModel->setColumnCount(kDefaultHeaders.size());
  for (int i = 0; i < kDefaultHeaders.size(); i++) {
    mScriptModel->setHorizontalHeaderItem(i, new QStandardItem(kDefaultHeaders.at(i)));
  }
}

void FormScript::Activated()
{
  mToolForm->AddCustomAction(ui->actionScriptCreate);
  mToolForm->AddCustomAction(ui->actionScriptCreateUp);
  mToolForm->AddCustomAction(ui->actionScriptOpen);
  mToolForm->AddCustomAction(ui->actionScriptSave);
  mToolForm->AddCustomAction(ui->actionUseScript);
}

bool FormScript::GenerateScript(bool update)
{
  mIsUpdateScript = update;
  mCustomHeaders.clear();
  ui->tableViewScript->setModel(nullptr);
  mScriptModel->setColumnCount(kDefaultHeaders.size());
  mScriptModel->removeRows(0, mScriptModel->rowCount());

  mPropertyForm->GetObjectSettingsTable()->Reload();
  mTemplateObjects.clear();
  if (!LoadTemplates()) {
    mTemplateObjects.clear();
    return false;
  }

  const QMap<int, TableItemS>& allItems = mObjectModel->GetObjectTable()->GetItems();
  for (auto itr = allItems.begin(); itr != allItems.end(); itr++) {
    int         id   = itr.key();
    ObjectItemS item = itr.value().dynamicCast<ObjectItem>();
    if (mObjectModel->GetObjectTable()->IsDefault(id) && id >= mObjectModel->MinId()) {
      GenerateObjectOne(id, item, true);
    }
  }

  for (auto itr = allItems.begin(); itr != allItems.end(); itr++) {
    int         id   = itr.key();
    ObjectItemS item = itr.value().dynamicCast<ObjectItem>();
    if (!mObjectModel->GetObjectTable()->IsDefault(id)) {
      if (mIsUpdateScript || mToolForm->GetToolSchema()->IsEditable(item->Type)) {
        GenerateObjectOne(id, item, false);
      } else {
        GenerateObjectIdent(id, item);
      }
    }
  }
  ui->tableViewScript->setModel(mScriptModel);
  mPropertyForm->GetObjectSettingsTable()->Clear();
  return true;
}

void FormScript::GenerateObjectOne(int id, const ObjectItemS& item, bool isTemplate)
{
  QList<int> parentLinks = mObjectModel->GetObjectTable()->GetItemParents(id);
  if (item->ParentId) {
    parentLinks.removeOne(item->ParentId);
  }

  DefaultObject* templateObj;
  QMap<QString, QString> diffProperties;
  if (!GenerateProperties(id, item->Type, isTemplate, templateObj, diffProperties)) {
    return;
  }
  ObjectItemS templItem = templateObj->TemplItem;

  QList<QStandardItem*> row;
  bool hasId = mIsUpdateScript;
  row << new QStandardItem(QString::number(id));
  row << new QStandardItem(hasId? QString::number(id): QString("new"));
  row << new QStandardItem(mObjectModel->GetIcon(item->Type, item->Status), templItem->Name);
  row << new QStandardItem(item->Name);
  row << new QStandardItem(item->Descr);
  row << new QStandardItem(item->ParentId? QString::number(item->ParentId): QString());
  QStringList parentsText;
  if (parentLinks.contains(0)) {
    parentsText.append("0");
  } else if (!parentLinks.isEmpty()) {
    std::sort(parentLinks.begin(), parentLinks.end());

    int lastIdStart = -2;
    int lastIdFinish = -2;
    foreach (int id, parentLinks) {
      if (id == lastIdFinish + 1) {
        if (lastIdStart < 0) {
          lastIdStart = id;
        }
        lastIdFinish = id;
      } else {
        if (lastIdStart >= 0) {
          if (lastIdFinish - lastIdStart > 2) {
            parentsText.append(QString("%1-%2").arg(lastIdStart).arg(lastIdFinish));
          } else {
            parentsText.append(QString::number(lastIdStart));
            if (lastIdFinish != lastIdStart) {
              parentsText.append(QString::number(lastIdFinish));
            }
          }
        }
        lastIdStart = lastIdFinish = id;
      }
    }
    if (lastIdStart > 0) {
      if (lastIdFinish - lastIdStart > 2) {
        parentsText.append(QString("%1-%2").arg(lastIdStart).arg(lastIdFinish));
      } else {
        parentsText.append(QString::number(lastIdStart));
        if (lastIdFinish != lastIdStart) {
          parentsText.append(QString::number(lastIdFinish));
        }
      }
    }
  }

  row << new QStandardItem(parentsText.join(", "));
  if (!mIsUpdateScript && item->Status != templItem->Status) {
    int status = mToolForm->GetToolSchema()->AutoChangeCreateState(item->Type, item->Status);
    row << new QStandardItem(QString::number(status));
  } else {
    row << new QStandardItem(QString());
  }

  for (int i = 0; i < mCustomHeaders.size(); i++) {
    const QString& header = mCustomHeaders.at(i);
    auto itr = diffProperties.find(header);
    QString value = (itr != diffProperties.end())? (!itr.value().isEmpty()? itr.value(): kDefaultEmpty): QString();
    row << new QStandardItem(value);
  }

  mScriptModel->appendRow(row);
}

void FormScript::GenerateObjectIdent(int id, const ObjectItemS& item)
{
  if (!mObjectModel->GetObjectTable()->HasChilds(id)) {
    return;
  }
  auto itr = mTemplateObjects.find(item->Type);
  if (itr == mTemplateObjects.end()) {
    return;
  }
  const ObjectItemS& templItem = itr.value()->TemplItem;
  QList<QStandardItem*> row;
  bool hasId = true;
  row << new QStandardItem(QString::number(id));
  row << new QStandardItem(hasId? QString::number(id): QString("new"));
  row << new QStandardItem(mObjectModel->GetIcon(item->Type, item->Status), templItem->Name);
  row << new QStandardItem(item->Name);
  row << new QStandardItem(item->Descr);
  row << new QStandardItem(item->ParentId? QString::number(item->ParentId): QString());
  mScriptModel->appendRow(row);
}

bool FormScript::GenerateProperties(int id, int typeId, bool isTemplate, DefaultObject*& templateObj, QMap<QString, QString>& diffProperties)
{
  QList<ObjectSettingsS> settings;
  if (!mPropertyForm->GetObjectSettingsTable()->GetObjectSettings(id, settings, true)) {
    return false;
  }

  templateObj = nullptr;
  int diffCount = settings.size() + 1;
  for (auto itr = mTemplateObjects.find(typeId); itr != mTemplateObjects.end() && itr.key() == typeId; itr++) {
    const DefaultObjectS& defaultObj = itr.value();
    if (isTemplate && defaultObj->TemplItem->Id >= mObjectModel->MinId()) {
      continue;
    }
    int diffCountCurrent = settings.size();
    for (auto itr = settings.begin(); itr != settings.end(); itr++) {
      const ObjectSettingsS& setting = *itr;
      auto itr_ = defaultObj->Properties.find(setting->Key);
      if (itr_ != defaultObj->Properties.end() && itr_.value() == setting->Value) {
        diffCountCurrent--;
      }
    }
    if (diffCountCurrent < diffCount || (diffCountCurrent == diffCount && defaultObj->TemplItem->Id < templateObj->TemplItem->Id)) {
      diffCount = diffCountCurrent;
      templateObj = defaultObj.data();
    }
  }

  if (!templateObj) {
    return false;
  }

  for (auto itr = settings.begin(); itr != settings.end(); itr++) {
    const ObjectSettingsS& setting = *itr;
    auto itr_ = templateObj->Properties.find(setting->Key);
    if (itr_ != templateObj->Properties.end() && itr_.value() != setting->Value) {
      const QString& keyName = templateObj->PropKeyNameMap[setting->Key];
      diffProperties[keyName] = setting->Value;
      if (!mCustomHeaders.contains(keyName)) {
        mCustomHeaders.append(keyName);
        int newSize = kDefaultHeaders.size() + mCustomHeaders.size();
        mScriptModel->setColumnCount(newSize);
        mScriptModel->setHorizontalHeaderItem(newSize - 1, new QStandardItem(mCustomHeaders.last()));
      }
    }
  }

  return true;
}

bool FormScript::LoadTemplates()
{
  const QMap<int, TableItemS>& allItems = mObjectModel->GetObjectTable()->GetItems();
  for (auto itr = allItems.begin(); itr != allItems.end(); itr++) {
    int         id   = itr.key();
    ObjectItemS item = itr.value().dynamicCast<ObjectItem>();

    if (mObjectModel->GetObjectTable()->IsDefault(id)) {
      DefaultObjectS defaultObj(new DefaultObject(item));

      QList<ObjectSettingsS> settings;
      if (!mPropertyForm->GetObjectSettingsTable()->GetObjectSettings(id, settings, true)) {
        return false;
      }

      int typeId = item->Type;
      for (auto itr = settings.begin(); itr != settings.end(); itr++) {
        const ObjectSettingsS& item = *itr;
        defaultObj->Properties[item->Key] = item->Value;
        const ObjectSettingsType* typeItem = mPropertyForm->GetObjectSettingsTypeTable()->GetObjectTypeSettingsType(typeId, item->Key);
        if (typeItem) {
          defaultObj->PropKeyNameMap[typeItem->Key]  = typeItem->Name;
          defaultObj->PropNameKeyMap[typeItem->Name] = typeItem->Key;
        }
      }

      mTemplateObjects.insert(item->Type, defaultObj);
      mTemplateNamesMap[item->Name] = defaultObj;
    }
  }
  return true;
}

bool FormScript::LoadTypeNames()
{
  const QMap<int, TableItemS>& allItems = mObjectModel->GetObjectTypeTable()->GetItems();
  for (auto itr = allItems.begin(); itr != allItems.end(); itr++) {
    int             id   = itr.key();
    ObjectTypeItemS item = itr.value().dynamicCast<ObjectTypeItem>();
    mTypeNames[id] = item->Descr;
  }
  return true;
}

void FormScript::Create(bool update)
{
  ui->textEditLog->setVisible(false);

  if (!GenerateScript(update)) {
    mScriptModel->removeRows(0, mScriptModel->rowCount());
    QMessageBox::warning(this, GetProgramName(), QString("Внутренняя ошибка:\nНе удалось создать скрипт\n"));
  }
}

bool FormScript::SaveFile(CsvWriter* writer)
{
  for (int i = 0; i < mScriptModel->columnCount(); i++) {
    QString text = mScriptModel->horizontalHeaderItem(i)->text();
    if (!writer->WriteValue(text.toUtf8())) {
      return false;
    }
  }
  if (!writer->WriteEndLine()) {
    return false;
  }

  for (int j = 0; j < mScriptModel->rowCount(); j++) {
    for (int i = 0; i < mScriptModel->columnCount(); i++) {
      QString text = mScriptModel->index(j, i).data().toString();
      if (!writer->WriteValue(text.toUtf8())) {
        return false;
      }
    }
    if (!writer->WriteEndLine()) {
      return false;
    }
  }
  return true;
}

bool FormScript::LoadFile(CsvReader* reader)
{
  mCustomHeaders.clear();
  mScriptModel->setColumnCount(kDefaultHeaders.size());
  mScriptModel->removeRows(0, mScriptModel->rowCount());

  QStringList headers;
  if (!reader->ReadLine(headers)) {
    return false;
  }

  QVector<bool> usedDefault(kDefaultHeaders.size(), false);
  QVector<int> indexHeaders;
  indexHeaders.fill(-1, kDefaultHeaders.size());
  for (int i = 0; i < headers.size(); i++) {
    bool found = false;
    for (int j = 0; j < kDefaultHeaders.size(); j++) {
      if (headers.at(i) == kDefaultHeaders.at(j) && !usedDefault[j]) {
        usedDefault[j] = true;
        indexHeaders[j] = i;
        found = true;
        break;
      }
    }
    if (!found) {
      const QString& keyName = headers.at(i);
      mCustomHeaders.append(keyName);
      int newSize = kDefaultHeaders.size() + mCustomHeaders.size();
      mScriptModel->setColumnCount(newSize);
      mScriptModel->setHorizontalHeaderItem(newSize - 1, new QStandardItem(mCustomHeaders.last()));
      indexHeaders.append(i);
    }
  }

  forever {
    QStringList values;
    if (!reader->ReadLine(values)) {
      return false;
    }
    if (values.isEmpty() && reader->AtEnd()) {
      break;
    }

    QList<QStandardItem*> row;
    for (int i = 0; i < indexHeaders.size(); i++) {
      int index = indexHeaders.at(i);
      row << new QStandardItem(index >= 0 && index < values.size()? values.at(index): QString());
    }
    mScriptModel->appendRow(row);
  }
  return true;
}

bool FormScript::UseScript()
{
  mScriptIdMap.clear();
  mScriptSkipLines.clear();
  mUseLog.clear();
  mObjectOk = mObjectOkOne = mObjectPart = mObjectPartOneOk = mObjectPartOneFail = mObjectFail = 0;

  mPropertyForm->GetObjectSettingsTable()->Reload();
  mTemplateObjects.clear();
  if (!LoadTemplates()) {
    return false;
  }

  mScriptSkipLines.clear();
  mScriptSkipLines.reserve(mScriptModel->rowCount());
  for (int j = 0; j < mScriptModel->rowCount(); j++) {
    mScriptSkipLines.append(j);
  }

  for (int stage = 1; !mScriptSkipLines.isEmpty(); stage++) {
    QVector<int> retryLines;
    mScriptSkipLines.swap(retryLines);

    if (stage > 1) {
      QStringList skipLinesText;
      foreach (int skipId, retryLines) {
        skipLinesText.append(QString::number(skipId));
      }
      mUseLog.append(QString("Возвращаемся к отложенным строкам (%1)").arg(skipLinesText.join(", ")));
    }

    for (int j = 0; j < retryLines.size(); j++) {
      UseScriptLine(retryLines[j]);
    }
    if (mScriptSkipLines.isEmpty()) {
      break;
    }
    if (mScriptSkipLines.size() >= retryLines.size()) {
      mObjectFail += mScriptSkipLines.size();
      mUseLog.append(QString("<span style=\"color:#a10000;\">Не создано ни одного нового объекта, прерываем выполнение (отложено %1)</span>").arg(mScriptSkipLines.size()));
      break;
    }
  }

  mPropertyForm->GetObjectSettingsTable()->Clear();
  emit OnScriptDone();
  return true;
}

bool FormScript::UseScriptLine(int j)
{
  mCurrentLine = j;
  int oneOk = 0;
  int oneFail = 0;

  bool ok;
  int parent = mScriptModel->index(mCurrentLine, eParent).data().toInt(&ok);
  if (ok) {
    auto itr = mScriptIdMap.find(parent);
    if (itr == mScriptIdMap.end()) {
      AddLineLogInfo(QString("Родитель не существует (Уин: %1), откладывается").arg(parent));
      mScriptSkipLines.append(mCurrentLine);
      return false;
    } else {
      parent = itr.value();
    }
  }
  QStringList linksText = mScriptModel->index(mCurrentLine, eLinks).data().toString().split(",", Qt::SkipEmptyParts);
  QList<int> links;
  for (int i = 0; i < linksText.size(); i++) {
    QString text = linksText.at(i).trimmed();
    if (!text.isEmpty()) {
      QStringList range = text.split('-');
      int idStart, idFinish;
      bool ok = false;
      if (range.size() == 2) {
        idStart = range.at(0).toInt(&ok);
        if (ok) {
          idFinish = range.at(1).toInt(&ok);
        }
      } else {
        idStart = idFinish = text.toInt(&ok);
      }

      if (!ok) {
        AddLineLogWarning(QString("Неудалось конвертировать выражение '%1' в число или диапазон").arg(text));
        oneFail++;
        continue;
      }
      if (idStart == 0) {
        links = QList<int>() << 0;
        break;
      }
      for (int id = idStart; id <= idFinish; id++) {
        auto itr = mScriptIdMap.find(id);
        if (itr == mScriptIdMap.end()) {
          AddLineLogInfo(QString("Объект для подключения не существует (Уин: %1), откладывается").arg(id));
          mScriptSkipLines.append(mCurrentLine);
          return false;
        }
        int idDb = itr.value();
        links.append(idDb);
      }
    }
  }

  int uin = mScriptModel->index(mCurrentLine, eUin).data().toInt(&ok);
  if (!ok) {
    mUseLog.append(QString("<span style=\"color:#ce4400;\">Строка %1: Чтение Уин не удалось, объект нельзя будет связать с другими"));
  }
  int id = mScriptModel->index(mCurrentLine, eId).data().toInt();
  QString templateText = mScriptModel->index(mCurrentLine, eTemplate).data().toString().trimmed();
  DefaultObject* templateItem = nullptr;
  int templateId = 0;
  if (templateText.isEmpty()) {
    AddLineLogFatal(QString("Шаблон не задан"));
    return false;
  } else {
    auto itr = mTemplateNamesMap.find(templateText);
    if (itr == mTemplateNamesMap.end()) {
      AddLineLogFatal(QString("Шаблон '%2' не найден").arg(templateText));
      return false;
    }
    templateItem = itr.value().data();
    templateId = templateItem->TemplItem->Id;
  }

  if (mScriptIdMap.contains(uin)) {
    AddLineLogFatal(QString("Уин не уникален"));
    return false;
  }

  QString name = mScriptModel->index(mCurrentLine, eName).data().toString();
  QString descr = mScriptModel->index(mCurrentLine, eDescr).data().toString();
  int thisId;
  if (id) {
    thisId = id;
    AddLineLogOk(QString("Объект сопоставлен по Ид (Ид: %1)").arg(thisId));
    TableItemS item = mObjectModel->GetObjectTable()->GetItem(thisId);
    if (!item) {
      AddLineLogFatal(QString("Объект с указанным Ид не существует (Ид: %1)").arg(thisId));
      return false;
    }
    ObjectItem* objItem = static_cast<ObjectItem*>(item.data());
    if (objItem->Name != name || objItem->Descr != descr) {
      objItem->Name = name;
      objItem->Descr = descr;
      if (mObjectModel->GetObjectTable()->UpdateItem(item)) {
        oneOk++;
        AddLineLogOk(QString("Объект '%1' переименован (Ид: %2)").arg(name).arg(thisId));
      } else {
        AddLineLogError(QString("Изменение имени/описания не удалось"));
        oneFail++;
      }
    }
  } else if (!parent) {
    if (!mToolForm->GetToolSchema()->IsSingle(templateId)) {
      const NamedItem* item = mObjectModel->GetObjectTable()->GetItemByName(name);
      if (!item) {
        AddLineLogFatal(QString("Объект невозможно создать и "
                                "объект с указанным именем ('%1') не найден").arg(name));
        return false;
      }
      AddLineLogOk(QString("Объект '%1' сопоставлен (Ид: %2)").arg(name).arg(item->Id));
      thisId = item->Id;
    } else {
      if (!mObjectModel->GetObjectTable()->CreateObject(templateId, name, &thisId)) {
        return false;
      }
      oneOk++;
      AddLineLogOk(QString("Объект '%1' создан (Ид: %2)").arg(name).arg(thisId));
    }
  } else {
    if (!mObjectModel->GetObjectTable()->CreateSlaveObject(parent, templateId, name, &thisId)) {
      return false;
    }
    oneOk++;
    AddLineLogOk(QString("Объект '%1' создан (Ид: %2, Родитель: %3)")
                 .arg(name).arg(thisId).arg(parent));
  }
  mScriptIdMap[uin] = thisId;

  QList<ObjectSettingsS> settings;
  if (mPropertyForm->GetObjectSettingsTable()->GetObjectSettings(thisId, settings)) {
    for (int i = 0; i < mCustomHeaders.size(); i++) {
      QString text = mScriptModel->index(mCurrentLine, eCustom + i).data().toString();
      if (text.isEmpty()) {
        continue;
      }
      if (text == kDefaultEmpty) {
        text.clear();
      }
      const QString& name = mCustomHeaders.at(i);
      auto itr = templateItem->PropNameKeyMap.find(name);
      if (itr == templateItem->PropNameKeyMap.end()) {
        AddLineLogError(QString("Свойство '%1' не найдено для шаблона объекта, игнорируется").arg(name));
        oneFail++;
        continue;
      }
      const QString& key = itr.value();

      bool found = false;
      for (auto itr = settings.begin(); itr != settings.end(); itr++) {
        const ObjectSettingsS& sett = *itr;
        if (sett->Key == key) {
          if (id && sett->Value == text) {
            found = true;
            break;
          }
          sett->Value = text;
          if (mPropertyForm->GetObjectSettingsTable()->UpdateItem(*sett)) {
            AddLineLogOk(QString("Свойство '%1' установлено в '%2'").arg(name).arg(text));
            oneOk++;
          } else {
            AddLineLogError(QString("Свойство '%1' не удалось изменить").arg(name));
            oneFail++;
          }
          found = true;
          break;
        }
      }
      if (!found) {
        AddLineLogError(QString("Свойство '%1' не найдено для объекта, игнорируется").arg(name));
        oneFail++;
        continue;
      }
    }
  } else {
    AddLineLogError(QString("Свойства для объекта не загружены, настройка игнорируется"));
    oneFail++;
  }

  do {
    QList<int> linksUsed;
    if (!mObjectModel->GetObjectTable()->LoadMasters(thisId, linksUsed)) {
      AddLineLogError(QString("Загрузка подключений для объекта неудачна, строка игнорируется"));
      oneFail++;
      break;
    }
    foreach (int linkId, links) {
      if (linkId && linkId == parent) {
        continue;
      }
      int ind = linksUsed.indexOf(linkId);
      if (ind >= 0) {
        linksUsed.removeAt(ind);
      } else if (mObjectModel->GetObjectTable()->CreateLink(linkId, thisId)) {
        if (linkId) {
          AddLineLogOk(QString("Объект %1 подключён к %2").arg(thisId).arg(linkId));
        } else {
          AddLineLogOk(QString("Объект %1 подключён к шаблонам").arg(thisId));
        }
        oneOk++;
      } else {
        AddLineLogError(QString("Подключение не удалось (%1 к %2)").arg(thisId).arg(linkId));
        oneFail++;
      }
    }
    foreach (int linkId, linksUsed) {
      if (linkId == parent) {
        continue;
      }
      if (mObjectModel->GetObjectTable()->RemoveLink(linkId, thisId)) {
        AddLineLogOk(QString("Объект %1 отключён от %2").arg(thisId).arg(linkId));
        oneOk++;
      } else {
        AddLineLogError(QString("Отключение не удалось (%1 от %2)").arg(thisId).arg(linkId));
        oneFail++;
      }
    }
  } while (false);

  int status = mScriptModel->index(mCurrentLine, eState).data().toInt(&ok);
  if (ok) {
    if (mObjectModel->GetObjectTable()->UpdateState(thisId, status)) {
      AddLineLogOk(QString("Статус объекта %1 установлен в %2").arg(thisId).arg(status));
      oneOk++;
    } else {
      AddLineLogError(QString("Изменение статуса объекта %1 не удалось").arg(thisId));
      oneFail++;
    }
  }

  if (oneFail) {
    mObjectPart++;
    mObjectPartOneOk += oneOk;
    mObjectPartOneFail += oneFail;
  } else if (oneOk) {
    mObjectOk++;
    mObjectOkOne += oneOk;
  }
  return true;
}

void FormScript::AddLineLogOk(const QString& text)
{
  mUseLog.append(QString("<span style=\"color:#008300;\">Строка %1: %2</span>").arg(mCurrentLine + 1).arg(text));
  Log.Info(QString("Script line %1: %2").arg(mCurrentLine + 1).arg(text));
}

void FormScript::AddLineLogWarning(const QString& text)
{
  mUseLog.append(QString("<span style=\"color:#ce4400;\">Строка %1: %2</span>").arg(mCurrentLine + 1).arg(text));
  Log.Warning(QString("Script line %1: %2").arg(mCurrentLine + 1).arg(text));
}

void FormScript::AddLineLogError(const QString& text)
{
  mUseLog.append(QString("<span style=\"color:#a10000;\">Строка %1: %2</span>").arg(mCurrentLine + 1).arg(text));
  Log.Warning(QString("Script line %1: %2").arg(mCurrentLine + 1).arg(text));
}

void FormScript::AddLineLogFatal(const QString& text)
{
  mUseLog.append(QString("<span style=\"color:#a10000;\">Строка %1: %2, строка игнорируется</span>").arg(mCurrentLine + 1).arg(text));
  Log.Error(QString("Script line %1: %2").arg(mCurrentLine + 1).arg(text));
  mObjectFail++;
}

void FormScript::AddLineLogInfo(const QString& text)
{
  mUseLog.append(QString("Строка %1: %2").arg(mCurrentLine + 1).arg(text));
  Log.Info(QString("Script line %1: %2").arg(mCurrentLine + 1).arg(text));
}

void FormScript::on_actionScriptCreate_triggered()
{
  Create(false);
}

void FormScript::on_actionScriptSave_triggered()
{
  ui->textEditLog->setVisible(false);

  mFileDialog.setAcceptMode(QFileDialog::AcceptSave);
  if (mFileDialog.exec() != QFileDialog::Accepted) {
    return;
  }

  if (mFileDialog.selectedFiles().size() != 1) {
    QMessageBox::warning(this, GetProgramName(), QString("Внутренняя ошибка:\nНе удалось получить выбранный файл\n"));
    return;
  }

  QFile file(mFileDialog.selectedFiles().first());
  if (!file.open(QFile::WriteOnly)) {
    QMessageBox::warning(this, GetProgramName(), QString("Не удалось открыть файл на запись '%1'\n").arg(file.fileName()));
    return;
  }

  CsvWriter writer(&file);
  if (!SaveFile(&writer)) {
    QMessageBox::warning(this, GetProgramName(), QString("Не удалось сохранить данные в файл '%1'\n").arg(file.fileName()));
    return;
  }
}

void FormScript::on_actionScriptOpen_triggered()
{
  ui->textEditLog->setVisible(false);

  mFileDialog.setAcceptMode(QFileDialog::AcceptOpen);
  if (mFileDialog.exec() != QFileDialog::Accepted) {
    return;
  }

  if (mFileDialog.selectedFiles().size() != 1) {
    QMessageBox::warning(this, GetProgramName(), QString("Внутренняя ошибка:\nНе удалось получить выбранный файл\n"));
    return;
  }

  QFile file(mFileDialog.selectedFiles().first());
  if (!file.open(QFile::ReadOnly)) {
    QMessageBox::warning(this, GetProgramName(), QString("Не удалось открыть файл на чтение '%1'\n").arg(file.fileName()));
    return;
  }

  CsvReader reader(&file);
  if (!LoadFile(&reader)) {
    QMessageBox::warning(this, GetProgramName(), QString("Не удалось загрузить данные из файла '%1'\n").arg(file.fileName()));
    return;
  }
}

void FormScript::on_actionUseScript_triggered()
{
  ui->textEditLog->setHtml("");
  ui->textEditLog->setVisible(true);

  if (!UseScript()) {
    QMessageBox::warning(this, GetProgramName(), QString("Не удалось выполнить скрипт\n"));
    return;
  }

  mUseLog.prepend("<hr>");
  mUseLog.prepend(kReportLineFail.arg(mObjectFail));
  mUseLog.prepend(kReportLinePart.arg(mObjectPart).arg(mObjectPartOneOk).arg(mObjectPartOneFail));
  mUseLog.prepend(kReportLineOk.arg(mObjectOk).arg(mObjectOkOne));
  ui->textEditLog->setHtml(mUseLog.join("\n<br>"));
}

void FormScript::on_actionScriptCreateUp_triggered()
{
  Create(true);
}
