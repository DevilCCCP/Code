#include <QDateTime>
#include <QPicture>
#include <QFileDialog>
#include <QFile>

#include <Lib/Db/ObjectType.h>
#include <Lib/Db/ObjectState.h>
#include <Lib/Db/Event.h>
#include <Lib/Log/Log.h>
#include <Lib/Common/Format.h>

#include "MonitoringWindow.h"
#include "ui_MonitoringWindow.h"


const int kNotAvailableSec = 5;
const int kImportPackSize = 1000;
const QString kFileTimeFormat("yyyy-MM-dd HH:mm:ss");

void MonitoringWindow::ChangeRefreshRate(int rateMs, QAction *refreshNew)
{
  if (rateMs > 0) {
    mRefreshTimer->start(rateMs);
  } else {
    mRefreshTimer->stop();
  }
  mRefreshLast->setChecked(false);
  refreshNew->setChecked(true);
  mRefreshLast = refreshNew;
}

void MonitoringWindow::on_actionExit_triggered()
{
  close();
}

void MonitoringWindow::onUpdate()
{
  if (!mLoadSchema) {
    mLoadSchema = ReloadSchema();
  }

  bool result = false;
  if (mLoadSchema) {
    if (ui->tabWidgetMain->currentWidget() == ui->tabState) {
      result = UpdateState();
    } else {
      return;
    }
  }

  if (result) {
    ui->statusBar->showMessage(QString::fromUtf8("Обновлено успешно %1").arg(QTime::currentTime().toString(Qt::ISODate)));
  } else {
    ui->statusBar->showMessage(QString::fromUtf8("Ошибка обновления %1").arg(QTime::currentTime().toString(Qt::ISODate)));
  }
}

bool MonitoringWindow::ReloadSchema()
{
  if (!mObjectTypeTable) {
    mObjectTypeTable.reset(new ObjectTypeTable(mDb));
  }
  if (!mObjectTable) {
    mObjectTable.reset(new ObjectTable(mDb));
  }
  if (!mObjectState) {
    mObjectState.reset(new ObjectState(mDb));
  }
  if (!mObjectStateValuesTable) {
    mObjectStateValuesTable.reset(new ObjectStateValuesTable(mDb));
  }
#ifdef USE_ANALIZER
  if (!mEventTypeTable) {
    mEventTypeTable.reset(new EventTypeTable(mDb));
  }
#endif
  if (!mEventTable) {
    mEventTable.reset(new EventTable(mDb));
  }

  mObjectTypeTable->Reload();
  if (!mObjectTypeTable->Open()) {
    if (!mLoadError) {
      Log.Warning("Load object_type fail");
      mLoadError = true;
    }
    return false;
  }

  mObjectStateValuesTable->Reload();
  if (!mObjectStateValuesTable->Open()) {
    if (!mLoadError) {
      Log.Warning("Load object_state_values fail");
      mLoadError = true;
    }
    return false;
  }

  mObjectTable->Reload();
  if (!mObjectTable->Open()) {
    if (!mLoadError) {
      Log.Warning("Load object fail");
      mLoadError = true;
    }
    return false;
  }
  if (!mObjectTable->ReloadConnections()) {
    if (!mLoadError) {
      Log.Warning("Load object_connection fail");
      mLoadError = true;
    }
    return false;
  }

  mServerTypeIds.clear();
  QStringList typeNamesList;
  foreach (const QString& serverTypeName, mServerTypeNames) {
    int typeId;
    if (!GetTypeId(serverTypeName, typeId)) {
      Log.Warning(QString("Get '%1' type fail").arg(serverTypeName));
      mLoadError = true;
      return false;
    }
    mServerTypeIds.insert(typeId);
    typeNamesList.append(QString::number(typeId));
  }
  mServerTypeIdsText = typeNamesList.join(',');
  GetTypeId("sch", mScheduleTypeId);

  ReloadState();

  if (mEventTypeTable) {
    mEventTypeTable->Reload();
    if (mEventTable->Reload()) {
      ReloadEvents();
    }
  }
  return true;
}

bool MonitoringWindow::GetTypeId(const QString& abbr, int& id, QString* name)
{
  if (const ObjectTypeItem* typeItem = static_cast<const ObjectTypeItem*>(mObjectTypeTable->GetItemByName(abbr))) {
    id = typeItem->Id;
    if (name) {
      *name = typeItem->Descr;
    }
    return true;
  } else {
    id = 0;
    return false;
  }
}

void MonitoringWindow::ReloadState()
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("SELECT DISTINCT ot._id, ot.name, ot.descr FROM object o1"
                     " INNER JOIN object_connection oc ON oc._omaster = o1._id"
                     " INNER JOIN object o ON o._id = oc._oslave"
                     " INNER JOIN object_type ot ON ot._id = o._otype"
                     " WHERE o1._otype IN (%1) OR ot._id IN (%1)"
                     " ORDER BY ot._id;").arg(mServerTypeIdsText));

  ui->comboBoxLogFilterObjectType->clear();
  ui->comboBoxLogFilterObjectType->addItem(QIcon(":/ObjTree/Icons/System.png"), QString::fromUtf8("Все"), 0);
  if (mDb.ExecuteQuery(q)) {
    while (q->next()) {
      int index = 0;
      int id = q->value(index++).toInt();
      QString abbr = q->value(index++).toString();
      QString name = q->value(index++).toString();
      ui->comboBoxLogFilterObjectType->addItem(QIcon(QString(":/ObjTree/%1").arg(abbr)), name, id);
    }
  }

  mStateModel->clear();
  mStateMap.clear();
  mStateModel->setColumnCount(3);
  mStateModel->setHorizontalHeaderItem(0, new QStandardItem(QString::fromUtf8("Сервера")));
  mStateModel->setHorizontalHeaderItem(1, new QStandardItem(QString::fromUtf8("Состояние")));
  mStateModel->setHorizontalHeaderItem(2, new QStandardItem());
  QSet<int> childs = mObjectTable->MasterConnection().keys().toSet();
  const QMap<int, TableItemS>& items = mObjectTable->GetItems();
  for (auto itr = items.begin(); itr != items.end(); itr++) {
    const ObjectItem& item = static_cast<const ObjectItem&>(*itr.value());
    if (mServerTypeIds.contains(item.Type) && !childs.contains(item.Id)) {
      QStandardItem* modelItem = new QStandardItem(QString("%1").arg(item.Name));
      if (const ObjectTypeItem* itemType = static_cast<const ObjectTypeItem*>(mObjectTypeTable->GetItem(item.Type).data())) {
        modelItem->setIcon(QIcon(QString(":/ObjTree/%1").arg(itemType->Name)));
      }
      const QMap<int, int>& slaves = mObjectTable->SlaveConnection();
      for (auto itr2 = slaves.find(item.Id); itr2 != slaves.end() && itr2.key() == item.Id; itr2++) {
        int slaveId = itr2.value();
        const ObjectItem* item2 = static_cast<const ObjectItem*>(mObjectTable->GetItem(slaveId).data());
        if (item2 && item2->Type != mScheduleTypeId) {
          QStandardItem* modelItem2 = new QStandardItem(QString("%1").arg(item2->Name));
          const ObjectTypeItem* item2Type = static_cast<const ObjectTypeItem*>(mObjectTypeTable->GetItem(item2->Type).data());
          if (item2Type) {
            modelItem2->setIcon(QIcon(QString(":/ObjTree/%1").arg(item2Type->Name)));
          }
          QStandardItem* modelState = CreateBadStateItem();
          modelItem->appendRow(QList<QStandardItem*>() << modelItem2 << modelState << new QStandardItem());
          mStateMap.insert(item2->Id, modelState);
        }
      }
      QStandardItem* modelState = CreateBadStateItem();
      mStateModel->appendRow(QList<QStandardItem*>() << modelItem << modelState << new QStandardItem());
      mStateMap.insert(item.Id, modelState);
    }
  }
  ui->treeViewState->expandAll();
  ui->treeViewState->resizeColumnToContents(0);
  ui->treeViewState->resizeColumnToContents(1);
}

QStandardItem* MonitoringWindow::CreateBadStateItem()
{
  QStandardItem* modelState = new QStandardItem(QString::fromUtf8("<Отсутствует>"));
  modelState->setForeground(mItemNotExists.first);
  modelState->setBackground(mItemNotExists.second);
  return modelState;
}

void MonitoringWindow::ReloadEvents()
{
  ui->comboBoxEventFilterObject->clear();
  ui->comboBoxEventFilterObject->addItem(QIcon(":/ObjTree/Icons/System.png"), QString::fromUtf8("Все"), 0);
  QList<int> objIds = mEventTable->GetObjects();
  for (auto itr = objIds.begin(); itr != objIds.end(); itr++) {
    int id = *itr;
    if (TableItemS objectItem = mObjectTable->GetItem(id)) {
      const ObjectItem* obj = dynamic_cast<const ObjectItem*>(objectItem.data());
      QIcon objIcon;
      if (TableItemS objectTypeItem = mObjectTypeTable->GetItem(obj->Type)) {
        const ObjectTypeItem* objType = dynamic_cast<const ObjectTypeItem*>(objectTypeItem.data());
        objIcon = QIcon(QString(":/ObjTree/%1").arg(objType->Name));
      }
      if (const ObjectItem* anal = mObjectTable->GetParent(obj->Id)) {
        if (const ObjectItem* cam = mObjectTable->GetParent(anal->Id)) {
          ui->comboBoxEventFilterObject->addItem(objIcon, QString("%1 (%2)").arg(obj->Name).arg(cam->Name), id);
          continue;
        }
      }
      ui->comboBoxEventFilterObject->addItem(objIcon, obj->Name, id);
    }
  }

  ui->comboBoxEventFilterEventType->clear();
}

bool MonitoringWindow::UpdateState()
{
  mObjectState->Reload();
  if (!mObjectState->Open()) {
    return false;
  }

  const QMap<int, TableItemS>& items = mObjectState->GetItems();
  for (auto itr = items.begin(); itr != items.end(); itr++) {
    const ObjectStateItem& item = static_cast<const ObjectStateItem&>(*itr.value());
    auto itr2 = mStateMap.find(item.ObjectId);
    if (itr2 != mStateMap.end()) {
      QStandardItem* modelState = itr2.value();

      if (item.ChangeSec > kNotAvailableSec) {
        modelState->setText(QString::fromUtf8("<Недоступен>"));
        modelState->setForeground(mItemDisabled.first);
        modelState->setBackground(mItemDisabled.second);
      } else if (const ObjectStateValuesItem* itemValue = mObjectStateValuesTable->GetItemByTypeState(item.ObjectStateTypeId, item.State)) {
        modelState->setText(itemValue->Descr);
        modelState->setForeground(QBrush(QColor(itemValue->Color)));
        modelState->setBackground(mItemEnabled.second);
      } else {
        modelState->setText(QString::fromUtf8("<Ошибка>"));
        modelState->setForeground(mItemNotExists.first);
        modelState->setBackground(mItemNotExists.second);
      }
    }
  }
  ui->treeViewState->resizeColumnToContents(0);
  ui->treeViewState->resizeColumnToContents(1);
  return true;
}

QueryS MonitoringWindow::RetriveLogFilter()
{
  int id = ui->comboBoxLogFilterObjectType->itemData(ui->comboBoxLogFilterObjectType->currentIndex()).toInt();
  QString name = ui->lineEditLogFilterName->text();
  QDateTime from = ui->dateTimeEditLogFilterFrom->dateTime();
  QDateTime to = ui->dateTimeEditLogFilterTo->dateTime();
  if (from > to) {
    return QueryS();
  }
  QString queryText;
  if (id || !name.isEmpty()) {
    queryText = "SELECT l._ostate, l.old_state, l.new_state, l.change_time FROM object_state s"
        " INNER JOIN object o ON s._object = o._id"
        " INNER JOIN object_state_log l ON l._ostate = s._id"
        " WHERE ";
    if (id) {
      queryText.append(QString("o._otype = %1 AND ").arg(id));
    }
    if (!name.isEmpty()) {
      queryText.append(QString("o.name LIKE '%%%1%%' AND ").arg(name));
    }
    queryText.append("l.change_time >= :from AND l.change_time <= :to"
                     " ORDER BY l.change_time, l._ostate, l._id;");
  } else {
    queryText = "SELECT _ostate, old_state, new_state, change_time FROM object_state_log"
        " WHERE change_time >= :from AND change_time <= :to"
        " ORDER BY change_time, _ostate, _id;";
  }
  auto q = mDb.MakeQuery();
  q->prepare(queryText);
  q->bindValue(":from", from);
  q->bindValue(":to", to);

  if (!mDb.ExecuteQuery(q)) {
    q.clear();
  }
  return q;
}

struct StateInfo {
  int       StateId;
  int       State;
  QDateTime From;

  StateInfo(int _StateId, int _State, const QDateTime& _From): StateId(_StateId), State(_State), From(_From) { }
};

bool MonitoringWindow::UpdateLog(QueryS &q)
{
  mLogModel->clear();
  mLogModel->setColumnCount(4);
  mLogModel->setHorizontalHeaderItem(0, new QStandardItem(QString::fromUtf8("Объект")));
  mLogModel->setHorizontalHeaderItem(1, new QStandardItem(QString::fromUtf8("Состояние")));
  mLogModel->setHorizontalHeaderItem(2, new QStandardItem(QString::fromUtf8("Начало")));
  mLogModel->setHorizontalHeaderItem(3, new QStandardItem(QString::fromUtf8("Конец")));
  mLogModel->setHorizontalHeaderItem(4, new QStandardItem(QString::fromUtf8("Период")));
  QMap<int, StateInfo> infoMap;
  while (q->next()) {
    int    objectStateId = q->value(0).toInt();
    int         oldState = q->value(1).toInt();
    int         newState = q->value(2).toInt();
    QDateTime changeTime = q->value(3).toDateTime();

    auto itr = infoMap.find(objectStateId);
    if (itr == infoMap.end()) {
      infoMap.insert(objectStateId, StateInfo(objectStateId, newState, changeTime));
      continue;
    }
//    int       oldState = itr.value().State;
    QDateTime fromTime = itr.value().From;
    itr.value().State = newState;
    itr.value().From  = changeTime;

    AddLogRecord(objectStateId, oldState, fromTime, changeTime);
  }

  QMap<QDateTime, StateInfo> lastInfo;
  for (auto itr = infoMap.begin(); itr != infoMap.end(); itr++) {
    const StateInfo& info = itr.value();
    lastInfo.insert(info.From, info);
  }

  for (auto itr = lastInfo.begin(); itr != lastInfo.end(); itr++) {
    const StateInfo& info = itr.value();
    AddLogRecord(info.StateId, info.State, info.From, QDateTime());
  }

  for (int i = 0; i < mLogModel->columnCount(); i++) {
    ui->treeViewLog->resizeColumnToContents(i);
  }

  return true;
}

void MonitoringWindow::AddLogRecord(int objectStateId, int stateValue, const QDateTime& fromTime, const QDateTime& toTime)
{
  static const QString kError = QString::fromUtf8("<Ошибка>");
  QString objectText = kError;
  QString objectIconText;
  QString stateText = kError;
  QString stateColor;

  if (const ObjectStateItem* state = static_cast<const ObjectStateItem*>(mObjectState->GetItem(objectStateId).data())) {
    if (const ObjectItem* objectItem = static_cast<const ObjectItem*>(
          mObjectTable->GetItem(state->ObjectId).data())) {
      objectText = objectItem->Name;
      if (const ObjectTypeItem* objectTypeItem = static_cast<const ObjectTypeItem*>(mObjectTypeTable->GetItem(objectItem->Type).data())) {
        objectIconText = QString(":/ObjTree/%1").arg(objectTypeItem->Name);
      }
    }
    if (const ObjectStateValuesItem* stateValueItem = static_cast<const ObjectStateValuesItem*>(
          mObjectStateValuesTable->GetItemByTypeState(state->ObjectStateTypeId, stateValue))) {
      stateText = stateValueItem->Descr;
      stateColor = stateValueItem->Color;
    }
  }

  QStandardItem* objectItem = new QStandardItem(QString("%1").arg(objectText));
  if (!objectIconText.isNull()) {
    objectItem->setIcon(QIcon(objectIconText));
  }
  QStandardItem* stateItem = new QStandardItem(QString("%1").arg(stateText));
  if (!stateColor.isEmpty()) {
    stateItem->setForeground(QBrush(QColor(stateColor)));
  }
  QStandardItem* fromItem = new QStandardItem(QString("%1").arg(fromTime.toString("dd MMM yyyy HH:mm:ss")));
  QStandardItem* toItem = new QStandardItem(toTime.isNull()? "Конец периода": QString("%1").arg(toTime.toString("dd MMM yyyy HH:mm:ss")));
  QDateTime endTime = toTime.isNull()? ui->dateTimeEditEventFilterTo->dateTime(): toTime;
  if (endTime > QDateTime::currentDateTime()) {
    endTime = QDateTime::currentDateTime();
  }
  QStandardItem* periodItem = new QStandardItem(FormatTimeDeltaRu(fromTime.msecsTo(endTime)));
  mLogModel->appendRow(QList<QStandardItem*>() << objectItem << stateItem << fromItem << toItem << periodItem);
}

QueryS MonitoringWindow::RetriveEventFilter(bool forceAll)
{
  int objId = ui->comboBoxEventFilterObject->itemData(ui->comboBoxEventFilterObject->currentIndex()).toInt();
  mHaveObject = objId != 0;
  QDateTime from;
  QDateTime to;
  if (ui->checkBoxEventFilterPeriod->isChecked()) {
    from = ui->dateTimeEditEventFilterFrom->dateTime();
    to = ui->dateTimeEditEventFilterTo->dateTime();
    if (from > to) {
      return QueryS();
    }
  }
  int typeId = ui->comboBoxEventFilterEventType->itemData(ui->comboBoxEventFilterEventType->currentIndex()).toInt();

  QString queryText;
  if (ui->radioButtonEventFilterOutAll->isChecked() || forceAll) {
    mHourEvents = 0;
    queryText = "SELECT l._event, l.triggered_time, l.value FROM event_log l";
  } else {
    mHourEvents = ui->radioButtonEventFilterOutHour->isChecked()? 1: 24;
    queryText = "SELECT l._event, l.triggered_hour, l.value FROM event_log_hours l";
  }
  QString ts = mHourEvents? "l.triggered_hour": "l.triggered_time";
  QStringList where;

  if (objId) {
    queryText.append(" INNER JOIN event e ON l._event = e._id");
    where << QString("e._object = %1").arg(objId);
    if (typeId) {
      queryText.append(" INNER JOIN event_type t ON t._id = e._etype");
      where << QString("t._id = %1").arg(typeId);
    }
  }
  if (from.isValid() && to.isValid()) {
    where << QString("%1 >= ? AND %1 <= ?").arg(ts);
  }

  if (!where.isEmpty()) {
    queryText.append(" WHERE " + where.join(" AND "));
  }
  queryText.append(" ORDER BY " + ts + ", l._event");
  if (ui->checkBoxResultLimit->isChecked()) {
    int limit = ui->spinBoxResultLimit->value();
    queryText.append(QString(" LIMIT %1").arg(limit));
  }

  auto q = mDb.MakeQuery();
  q->prepare(queryText);
  if (from.isValid() && to.isValid()) {
    q->bindValue(0, from);
    q->bindValue(1, to);
  }

  if (!mDb.ExecuteQuery(q)) {
    q.clear();
  }
  return q;
}

bool MonitoringWindow::UpdateEvent(QueryS& q)
{
  mEventModel->clear();
  int columns = 2;
  if (!mHaveObject) {
    columns++;
  }
  if (mHourEvents) {
    columns++;
  }
  mEventModel->setColumnCount(columns);
  int index = 0;
  if (!mHaveObject) {
    mEventModel->setHorizontalHeaderItem(index++, new QStandardItem(QString::fromUtf8("Объект")));
  }
  mEventModel->setHorizontalHeaderItem(index++, new QStandardItem(QString::fromUtf8("Событие")));
  mEventModel->setHorizontalHeaderItem(index++, new QStandardItem(QString::fromUtf8("Время")));
  if (mHourEvents) {
    mEventModel->setHorizontalHeaderItem(index++, new QStandardItem(QString::fromUtf8("Кол-во")));
  }

  while (q->next()) {
    static const QString kError = QString::fromUtf8("<Ошибка>");
    QString objectText = kError;
    QString eventIconText;
    QString eventText = kError;

    int  eventId = q->value(0).toInt();
    QDateTime ts = q->value(1).toDateTime();
    int    count = q->value(2).toInt();

    TableItemS eventItem = mEventTable->GetItem(eventId);
    const Event* event = static_cast<const Event*>(eventItem.data());
    if (event) {
      TableItemS eventTypeItem = mEventTypeTable->GetItem(event->EventTypeId);
      const EventType* eventType = static_cast<const EventType*>(eventTypeItem.data());
      if (eventType) {
        eventText = eventType->Descr;
        eventIconText = QString(":/Events/Icons/%1").arg(eventType->Icon);
      }
      if (!mHaveObject) {
        if (TableItemS objectItem = mObjectTable->GetItem(event->ObjectId)) {
          const ObjectItem* obj = static_cast<const ObjectItem*>(objectItem.data());
          if (const ObjectItem* anal = mObjectTable->GetParent(obj->Id)) {
            if (const ObjectItem* cam = mObjectTable->GetParent(anal->Id)) {
              objectText = QString("%1 (%2)").arg(obj->Name).arg(cam->Name);
            }
          }
        }
      }
    }
    QString timeFormat = (mHourEvents)? "yyyy-MM-dd HH:mm": "yyyy-MM-dd HH:mm:ss";
    QString timeText = ts.toString(timeFormat);

    QList<QStandardItem*> rowItem;
    if (!mHaveObject) {
      QStandardItem* objectItem = new QStandardItem(objectText);
      if (!eventIconText.isNull()) {
        objectItem->setIcon(QIcon(eventIconText));
      }
      rowItem << objectItem;
    }

    QStandardItem* evItem = new QStandardItem(eventText);
    if (mHaveObject && !eventIconText.isNull()) {
      evItem->setIcon(QIcon(eventIconText));
    }
    rowItem << evItem;

    rowItem << new QStandardItem(timeText);

    if (mHourEvents) {
      QStandardItem* countItem = new QStandardItem(QString::number(count));
      rowItem << countItem;
    }

    mEventModel->appendRow(rowItem);
  }

  for (int i = 0; i < columns; i++) {
    ui->treeViewEvents->resizeColumnToContents(i);
  }
  return true;
}

bool MonitoringWindow::ExportEvent(const QString& path, QueryS& q)
{
  QFile file(path);
  if (!file.open(QFile::WriteOnly)) {
    return false;
  }

  while (q->next()) {
    int eventId = q->value(0).toInt();
    QDateTime ts = q->value(1).toDateTime();
    int value = q->value(2).toInt();

    TableItemS eventItem = mEventTable->GetItem(eventId);
    const Event* event = static_cast<const Event*>(eventItem.data());
    if (!event) {
      return false;
    }
    TableItemS eventTypeItem = mEventTypeTable->GetItem(event->EventTypeId);
    const EventType* eventType = static_cast<const EventType*>(eventTypeItem.data());
    if (!eventType) {
      return false;
    }
    TableItemS objectItem = mObjectTable->GetItem(event->ObjectId);
    const ObjectItem* obj = static_cast<const ObjectItem*>(objectItem.data());
    if (!obj) {
      return false;
    }

    QString legalName = obj->Name;
    legalName.replace('"', '\'');
    QString line = QString("\"%1\";\"%2\";%3;%4\n").arg(legalName).arg(eventType->Name).arg(ts.toUTC().toString(kFileTimeFormat)).arg(value);
    if (!file.write(line.toUtf8())) {
      return false;
    }
  }
  return true;
}

bool MonitoringWindow::ImportEvent(const QString& path)
{
  QFile file(path);
  if (!file.open(QFile::ReadOnly)) {
    return false;
  }

  QVariantList eventIds;
  QVariantList timestamps;
  QVariantList values;
  int lineBegin = 1;
  int lineEnd = 0;
  while (!file.atEnd()) {
    QString line = QString::fromUtf8(file.readLine().trimmed());
    lineEnd++;
    QStringList raw = line.split(';', QString::KeepEmptyParts);
    if (raw.size() == 4) {
      QString name = raw[0].mid(1, raw[0].size() - 2);
      QString evTypeName = raw[1].mid(1, raw[1].size() - 2);
      QDateTime ts = QDateTime::fromString(raw[2], kFileTimeFormat).toLocalTime();
      ts.setTimeSpec(Qt::UTC);
      ts = ts.toLocalTime();
      int count = raw[3].toInt();

      const ObjectItem* obj = static_cast<const ObjectItem*>(mObjectTable->GetItemByName(name));
      const EventType* evType = static_cast<const EventType*>(mEventTypeTable->GetItemByName(evTypeName));
      if (!obj || !evType) {
        return false;
      }
      int evId;
      if (!mEventTable->InitEvent(obj->Id, evType->Id, &evId)) {
        return false;
      }

      eventIds.append(evId);
      timestamps.append(ts);
      values.append(count);
      if (eventIds.size() >= kImportPackSize) {
        if (!ImportEventPack(lineBegin, lineEnd, eventIds, timestamps, values)) {
          return false;
        }
        eventIds.clear();
        timestamps.clear();
        values.clear();
        lineBegin = lineEnd + 1;
      }
    }
  }
  return ImportEventPack(lineBegin, lineEnd, eventIds, timestamps, values);
}

bool MonitoringWindow::ImportEventPack(int lineBegin, int lineEnd, const QVariantList& eventIds, const QVariantList& timestamps, const QVariantList& values)
{
  Log.Info(QString("Import lines [%1, %2]").arg(lineBegin).arg(lineEnd));
  bool ok = mEventTable->TriggerEvents(eventIds, timestamps, values);
  Log.Info(QString("Import %1").arg(ok? "ok": "failed"));
  return ok;
}

//bool MonitoringWindow::CancelEvents(const QList<qint64>& ids)
//{
//  for (auto itr = ids.begin(); itr != ids.end(); itr++) {
//    qint64 id = *itr;
//    if (!mEventTable->CancelEvent(id)) {
//      return false;
//    }
//  }
//  return true;
//}

void MonitoringWindow::on_actionRefresh_triggered()
{
  mLoadSchema = false;
  onUpdate();
}

void MonitoringWindow::on_actionRefreshRate0_5_triggered()
{
  ChangeRefreshRate(500, ui->actionRefreshRate0_5);
}

void MonitoringWindow::on_actionRefreshRate1_triggered()
{
  ChangeRefreshRate(1000, ui->actionRefreshRate1);
}

void MonitoringWindow::on_actionRefreshRate2_triggered()
{
  ChangeRefreshRate(2000, ui->actionRefreshRate2);
}

void MonitoringWindow::on_actionRefreshRate5_triggered()
{
  ChangeRefreshRate(5000, ui->actionRefreshRate5);
}

void MonitoringWindow::on_actionRefreshRate30_triggered()
{
  ChangeRefreshRate(3000, ui->actionRefreshRate30);
}

void MonitoringWindow::on_actionRefreshPause_triggered()
{
  ChangeRefreshRate(0, ui->actionRefreshPause);
}

void MonitoringWindow::on_tabWidgetMain_currentChanged(int index)
{
  Q_UNUSED(index);

  if (mLoadSchema) {
    onUpdate();
  }
}

void MonitoringWindow::on_pushButtonLogFilterApply_clicked()
{
  if (QueryS q = RetriveLogFilter()) {
    if (UpdateLog(q)) {
      ui->statusBar->showMessage(QString::fromUtf8("Лог загружен успешно %1").arg(QTime::currentTime().toString(Qt::ISODate)));
    } else {
      ui->statusBar->showMessage(QString::fromUtf8("Ошибка загрузки лога %1").arg(QTime::currentTime().toString(Qt::ISODate)));
    }
  } else {
    ui->statusBar->showMessage(QString::fromUtf8("Ошибка фильтра %1").arg(QTime::currentTime().toString(Qt::ISODate)));
  }
}

void MonitoringWindow::on_pushButtonEventFilterApply_clicked()
{
  if (QueryS q = RetriveEventFilter()) {
    if (UpdateEvent(q)) {
      ui->statusBar->showMessage(QString::fromUtf8("События успешно загружены %1").arg(QTime::currentTime().toString(Qt::ISODate)));
    } else {
      ui->statusBar->showMessage(QString::fromUtf8("Ошибка загрузки событий %1").arg(QTime::currentTime().toString(Qt::ISODate)));
    }
  } else {
    ui->statusBar->showMessage(QString::fromUtf8("Ошибка фильтра %1").arg(QTime::currentTime().toString(Qt::ISODate)));
  }
}

void MonitoringWindow::on_comboBoxEventFilterObject_currentIndexChanged(int index)
{
  ui->comboBoxEventFilterEventType->clear();

  int id = ui->comboBoxEventFilterObject->itemData(index).toInt();
  if (id) {
    ui->comboBoxEventFilterEventType->setEnabled(true);
    ui->comboBoxEventFilterEventType->addItem(QString::fromUtf8("Все"));

    QList<int> typeIds = mEventTable->GetObjectEventTypes(id);
    for (auto itr = typeIds.begin(); itr != typeIds.end(); itr++) {
      int typeId = *itr;
      if (TableItemS typeItem = mEventTypeTable->GetItem(typeId)) {
        const EventType* eventType = static_cast<const EventType*>(typeItem.data());
        ui->comboBoxEventFilterEventType->addItem(QIcon(QString(":/ObjTree/%1").arg(eventType->Icon)), eventType->Descr, typeId);
      }
    }
  } else {
    ui->comboBoxEventFilterEventType->setEnabled(false);
  }
}

void MonitoringWindow::on_checkBoxEventFilterPeriod_clicked(bool checked)
{
  ui->widgetFilterEventPeriod->setEnabled(checked);
}

void MonitoringWindow::on_pushButtonEventExport_clicked()
{
  QString path = QFileDialog::getSaveFileName(this, QString::fromUtf8("Укажите файл для экспорта")
                                              , QString(), QString::fromUtf8("Файл csv (*.csv)"));
  if (path.isEmpty()) {
    return;
  }

  if (QueryS q = RetriveEventFilter(true)) {
    if (ExportEvent(path, q)) {
      ui->statusBar->showMessage(QString::fromUtf8("События успешно экспортированы %1").arg(QTime::currentTime().toString(Qt::ISODate)));
    } else {
      ui->statusBar->showMessage(QString::fromUtf8("Ошибка экспорта событий %1").arg(QTime::currentTime().toString(Qt::ISODate)));
    }
  } else {
    ui->statusBar->showMessage(QString::fromUtf8("Ошибка фильтра %1").arg(QTime::currentTime().toString(Qt::ISODate)));
  }
}

void MonitoringWindow::on_pushButtonEventImport_clicked()
{
  QString path = QFileDialog::getOpenFileName(this, QString::fromUtf8("Укажите файл для импорта")
                                              , QString(), QString::fromUtf8("Файл csv (*.csv)"));
  if (path.isEmpty()) {
    return;
  }

  if (ImportEvent(path)) {
    ui->statusBar->showMessage(QString::fromUtf8("События успешно импортированы %1").arg(QTime::currentTime().toString(Qt::ISODate)));
  } else {
    ui->statusBar->showMessage(QString::fromUtf8("Ошибка импорта событий %1").arg(QTime::currentTime().toString(Qt::ISODate)));
  }
}


MonitoringWindow::MonitoringWindow(Db &_Db, const QStringList&_ServerTypeNames, QWidget *parent)
  : MainWindow2(parent), ui(new Ui::MonitoringWindow)
  , mDb(_Db), mServerTypeNames(_ServerTypeNames)
  , mLoadSchema(false), mLoadError(false)
{
  Q_INIT_RESOURCE(DbUi);

  ui->setupUi(this);

  ui->tabWidgetMain->setCurrentIndex(0);
#ifndef USE_ANALIZER
  ui->tabWidgetMain->removeTab(3);
#endif

  QDateTime baseTime(QDateTime::currentDateTime());
  baseTime.setTime(QTime(baseTime.time().hour(), baseTime.time().minute(), 0));
  ui->dateTimeEditLogFilterFrom->setDateTime(baseTime.addDays(-1));
  ui->dateTimeEditLogFilterTo->setDateTime(baseTime.addDays(1));

  ui->dateTimeEditEventFilterFrom->setDateTime(baseTime.addDays(-1));
  ui->dateTimeEditEventFilterTo->setDateTime(baseTime.addDays(1));

  mRefreshTimer = new QTimer(this);
  mRefreshLast = ui->actionRefreshRate0_5;

  QColor baseFront = ui->treeViewState->palette().color(QPalette::Foreground);
  QColor baseBack = ui->treeViewState->palette().color(QPalette::Base);
  QColor lighter;
  lighter.setHsv(baseFront.hue(), baseFront.saturation(), (baseFront.value() + baseBack.value())/2);

  mItemEnabled = qMakePair(QBrush(baseFront), QBrush(baseBack));
  mItemDisabled = qMakePair(QBrush(lighter), QBrush(baseBack));
  mItemNotExists = qMakePair(QBrush(lighter), QBrush(baseBack.darker(120)));

  mStateModel = new QStandardItemModel(this);
  ui->treeViewState->setModel(mStateModel);

  mLogModel = new QStandardItemModel(this);
  ui->treeViewLog->setModel(mLogModel);

  mEventModel = new QStandardItemModel(this);
  ui->treeViewEvents->setModel(mEventModel);

  connect(mRefreshTimer, SIGNAL(timeout()), this, SLOT(onUpdate()));
  on_actionRefreshRate0_5_triggered();

  Restore();
}

MonitoringWindow::~MonitoringWindow()
{
  delete ui;
}

