#include <QDateTime>
#include <QPicture>
#include <QFileDialog>
#include <QFile>

#include <Lib/Db/ObjectType.h>
#include <Lib/Db/ObjectState.h>
#include <Lib/Db/Event.h>
#include <Lib/Db/Files.h>
#include <Lib/Log/Log.h>
#include <Lib/Common/Format.h>
#include <Lib/Ui/UpWaiter.h>

#include "FormEvents.h"
#include "ui_FormEvents.h"


const int kImportPackSize = 1000;
const QString kFileTimeFormat("yyyy-MM-dd HH:mm:ss");

FormEvents::FormEvents(QWidget *parent)
  : QWidget(parent), ui(new Ui::FormEvents)
  , mCore(nullptr)
  , mHourEvents(0), mHaveObject(false)
  , mCurrentEventLog(nullptr)
{
#ifdef USE_EVENTS
  Q_INIT_RESOURCE(VideoUi);
#endif

  ui->setupUi(this);

  QDateTime baseTime(QDateTime::currentDateTime());
  baseTime.setTime(QTime(baseTime.time().hour(), baseTime.time().minute(), 0));
  ui->dateTimeEditEventFilterFrom->setDateTime(baseTime.addDays(-1));
  ui->dateTimeEditEventFilterTo->setDateTime(baseTime.addDays(1));

  mEventModel = new QStandardItemModel(this);
  ui->treeViewEvents->setModel(mEventModel);

  ui->treeViewEvents->addAction(ui->actionPreview);
  ui->treeViewEvents->setContextMenuPolicy(Qt::ActionsContextMenu);
  if (auto sm = ui->treeViewEvents->selectionModel()) {
    connect(sm, &QItemSelectionModel::currentChanged, this, &FormEvents::OnSelectEvent);
  }
  ui->actionShowEvent->setEnabled(false);
  ui->actionShowClose->setEnabled(false);
}

FormEvents::~FormEvents()
{
  delete ui;
}


QAction* FormEvents::PreviewAction()
{
  return ui->actionPreview;
}

void FormEvents::Init(Core* _Core, bool _ShowEvent)
{
  mCore      = _Core;
  mShowEvent = _ShowEvent;

  if (mShowEvent) {
    ui->treeViewEvents->addAction(ui->actionShowEvent);
    ui->treeViewEvents->addAction(ui->actionShowClose);
    ui->actionShowClose->setEnabled(true);
  }
}

void FormEvents::ReloadEvents()
{
  ui->comboBoxEventFilterObject->clear();
  ui->comboBoxEventFilterObject->addItem(QIcon(":/ObjTree/Icons/System.png"), QString::fromUtf8("Все"), 0);
  QList<int> objIds = mCore->getEventTable()->GetObjects();
  for (auto itr = objIds.begin(); itr != objIds.end(); itr++) {
    int id = *itr;
    if (TableItemS objectItem = mCore->getObjectTable()->GetItem(id)) {
      const ObjectItem* obj = dynamic_cast<const ObjectItem*>(objectItem.data());
      QIcon objIcon;
      if (TableItemS objectTypeItem = mCore->getObjectTypeTable()->GetItem(obj->Type)) {
        const ObjectTypeItem* objType = dynamic_cast<const ObjectTypeItem*>(objectTypeItem.data());
        objIcon = QIcon(QString(":/ObjTree/%1").arg(objType->Name));
      }
      if (const ObjectItem* anal = mCore->getObjectTable()->GetParent(obj->Id)) {
        if (const ObjectItem* cam = mCore->getObjectTable()->GetParent(anal->Id)) {
          ui->comboBoxEventFilterObject->addItem(objIcon, QString("%1 (%2)").arg(obj->Name).arg(cam->Name), id);
          continue;
        }
      }
      ui->comboBoxEventFilterObject->addItem(objIcon, obj->Name, id);
    }
  }

  ui->comboBoxEventFilterEventType->clear();
}

QueryS FormEvents::RetriveEventFilter(bool forceAll)
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
    queryText = "SELECT l._id, l._event, l.triggered_time, l.value, l._file FROM event_log l";
  } else {
    mHourEvents = ui->radioButtonEventFilterOutHour->isChecked()? 1: 24;
    queryText = "SELECT l._id, l._event, l.triggered_hour, l.value, 0 FROM event_log_hours l";
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

  auto q = mCore->GetDb().MakeQuery();
  q->prepare(queryText);
  if (from.isValid() && to.isValid()) {
    q->bindValue(0, from);
    q->bindValue(1, to);
  }

  if (!mCore->GetDb().ExecuteQuery(q)) {
    q.clear();
  }
  return q;
}

bool FormEvents::UpdateEvent(QueryS& q)
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

    EventLogInfo info;
    qint64 id      = q->value(0).toLongLong();
    info.EventId   = q->value(1).toInt();
    info.Timestamp = q->value(2).toDateTime();
    info.Count     = q->value(3).toInt();
    info.FileId    = q->value(4).toLongLong();
    mEventLogInfoMap[id] = info;

    TableItemS eventItem = mCore->getEventTable()->GetItem(info.EventId);
    const Event* event = static_cast<const Event*>(eventItem.data());
    if (event) {
      TableItemS eventTypeItem = mCore->getEventTypeTable()->GetItem(event->EventTypeId);
      const EventType* eventType = static_cast<const EventType*>(eventTypeItem.data());
      if (eventType) {
        eventText = eventType->Descr;
        eventIconText = QString(":/Events/Icons/%1").arg(eventType->Icon);
      }
      if (!mHaveObject) {
        if (TableItemS objectItem = mCore->getObjectTable()->GetItem(event->ObjectId)) {
          const ObjectItem* obj = static_cast<const ObjectItem*>(objectItem.data());
          if (const ObjectItem* anal = mCore->getObjectTable()->GetParent(obj->Id)) {
            if (const ObjectItem* cam = mCore->getObjectTable()->GetParent(anal->Id)) {
              objectText = QString("%1 (%2)").arg(obj->Name).arg(cam->Name);
            }
          }
        }
      }
    }
    QString timeFormat = (mHourEvents)? "yyyy-MM-dd HH:mm": "yyyy-MM-dd HH:mm:ss";
    QString timeText   = info.Timestamp.toString(timeFormat);

    QList<QStandardItem*> rowItem;
    if (!mHaveObject) {
      QStandardItem* objectItem = new QStandardItem(objectText);
      if (!eventIconText.isNull()) {
        objectItem->setIcon(QIcon(eventIconText));
      }
      objectItem->setData(id);
      rowItem << objectItem;
    }

    QStandardItem* evItem = new QStandardItem(eventText);
    if (mHaveObject && !eventIconText.isNull()) {
      evItem->setIcon(QIcon(eventIconText));
    }
    evItem->setData(id);
    rowItem << evItem;

    QStandardItem* timeItem = new QStandardItem(timeText);
    timeItem->setData(id);
    rowItem << timeItem;

    if (mHourEvents) {
      QStandardItem* countItem = new QStandardItem(QString::number(info.Count));
      countItem->setData(id);
      rowItem << countItem;
    }

    mEventModel->appendRow(rowItem);
  }

  for (int i = 0; i < columns; i++) {
    ui->treeViewEvents->resizeColumnToContents(i);
  }
  return true;
}

bool FormEvents::ExportEvent(const QString& path, QueryS& q)
{
  QFile file(path);
  if (!file.open(QFile::WriteOnly)) {
    return false;
  }

  while (q->next()) {
    int eventId = q->value(0).toInt();
    QDateTime ts = q->value(1).toDateTime();
    int value = q->value(2).toInt();

    TableItemS eventItem = mCore->getEventTable()->GetItem(eventId);
    const Event* event = static_cast<const Event*>(eventItem.data());
    if (!event) {
      return false;
    }
    TableItemS eventTypeItem = mCore->getEventTypeTable()->GetItem(event->EventTypeId);
    const EventType* eventType = static_cast<const EventType*>(eventTypeItem.data());
    if (!eventType) {
      return false;
    }
    TableItemS objectItem = mCore->getObjectTable()->GetItem(event->ObjectId);
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

bool FormEvents::ImportEvent(const QString& path)
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

      const ObjectItem* obj = static_cast<const ObjectItem*>(mCore->getObjectTable()->GetItemByName(name));
      const EventType* evType = static_cast<const EventType*>(mCore->getEventTypeTable()->GetItemByName(evTypeName));
      if (!obj || !evType) {
        return false;
      }
      int evId;
      if (!mCore->getEventTable()->InitEvent(obj->Id, evType->Id, &evId)) {
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

bool FormEvents::ImportEventPack(int lineBegin, int lineEnd, const QVariantList& eventIds, const QVariantList& timestamps, const QVariantList& values)
{
  Log.Info(QString("Import lines [%1, %2]").arg(lineBegin).arg(lineEnd));
  bool ok = mCore->getEventTable()->TriggerEvents(eventIds, timestamps, values);
  Log.Info(QString("Import %1").arg(ok? "ok": "failed"));
  return ok;
}

bool FormEvents::LoadEvent(const QModelIndex& index)
{
  mCurrentEventLog = nullptr;
  qint64 id = index.data(Qt::UserRole + 1).toLongLong();
  if (id) {
    auto itr = mEventLogInfoMap.find(id);
    if (itr != mEventLogInfoMap.end()) {
      mCurrentEventLog = &itr.value();
      emit Preview(mCurrentEventLog->FileId);
    }
  }

  return mCurrentEventLog && mCurrentEventLog->EventId;
}

void FormEvents::ShowEvent()
{
  TableItemS eventItem = mCore->getEventTable()->GetItem(mCurrentEventLog->EventId);
  if (const Event* event = static_cast<const Event*>(eventItem.data())) {
    ObjectItemS analItem;
    if (mCore->getObjectTable()->LoadMaster(event->ObjectId, analItem)) {
      ObjectItemS camItem;
      if (mCore->getObjectTable()->LoadMaster(analItem->Id, camItem)) {
        emit Show(camItem->Id, mCurrentEventLog->Timestamp.toMSecsSinceEpoch());
      }
    }
  }
}

void FormEvents::OnSelectEvent(const QModelIndex& current, const QModelIndex& previous)
{
  Q_UNUSED(previous);

  ui->actionShowEvent->setEnabled(LoadEvent(current));
}

//bool FormEvents::CancelEvents(const QList<qint64>& ids)
//{
//  for (auto itr = ids.begin(); itr != ids.end(); itr++) {
//    qint64 id = *itr;
//    if (!mCore->getEventTable()->CancelEvent(id)) {
//      return false;
//    }
//  }
//  return true;
//}

void FormEvents::on_pushButtonEventFilterApply_clicked()
{
  if (QueryS q = RetriveEventFilter()) {
    if (UpdateEvent(q)) {
      emit Info(QString::fromUtf8("События успешно загружены %1").arg(QTime::currentTime().toString(Qt::ISODate)));
    } else {
      emit Info(QString::fromUtf8("Ошибка загрузки событий %1").arg(QTime::currentTime().toString(Qt::ISODate)));
    }
  } else {
    emit Info(QString::fromUtf8("Ошибка фильтра %1").arg(QTime::currentTime().toString(Qt::ISODate)));
  }
}

void FormEvents::on_comboBoxEventFilterObject_currentIndexChanged(int index)
{
  ui->comboBoxEventFilterEventType->clear();

  int id = ui->comboBoxEventFilterObject->itemData(index).toInt();
  if (id) {
    ui->comboBoxEventFilterEventType->setEnabled(true);
    ui->comboBoxEventFilterEventType->addItem(QString::fromUtf8("Все"));

    QList<int> typeIds = mCore->getEventTable()->GetObjectEventTypes(id);
    for (auto itr = typeIds.begin(); itr != typeIds.end(); itr++) {
      int typeId = *itr;
      if (TableItemS typeItem = mCore->getEventTypeTable()->GetItem(typeId)) {
        const EventType* eventType = static_cast<const EventType*>(typeItem.data());
        ui->comboBoxEventFilterEventType->addItem(QIcon(QString(":/ObjTree/%1").arg(eventType->Icon)), eventType->Descr, typeId);
      }
    }
  } else {
    ui->comboBoxEventFilterEventType->setEnabled(false);
  }
}

void FormEvents::on_checkBoxEventFilterPeriod_clicked(bool checked)
{
  ui->widgetFilterEventPeriod->setEnabled(checked);
}

void FormEvents::on_pushButtonEventExport_clicked()
{
  QString path = QFileDialog::getSaveFileName(this, QString::fromUtf8("Укажите файл для экспорта")
                                              , QString(), QString::fromUtf8("Файл csv (*.csv)"));
  if (path.isEmpty()) {
    return;
  }

  if (QueryS q = RetriveEventFilter(true)) {
    if (ExportEvent(path, q)) {
      emit Info(QString::fromUtf8("События успешно экспортированы %1").arg(QTime::currentTime().toString(Qt::ISODate)));
    } else {
      emit Info(QString::fromUtf8("Ошибка экспорта событий %1").arg(QTime::currentTime().toString(Qt::ISODate)));
    }
  } else {
    emit Info(QString::fromUtf8("Ошибка фильтра %1").arg(QTime::currentTime().toString(Qt::ISODate)));
  }
}

void FormEvents::on_pushButtonEventImport_clicked()
{
  QString path = QFileDialog::getOpenFileName(this, QString::fromUtf8("Укажите файл для импорта")
                                              , QString(), QString::fromUtf8("Файл csv (*.csv)"));
  if (path.isEmpty()) {
    return;
  }

  if (ImportEvent(path)) {
    emit Info(QString::fromUtf8("События успешно импортированы %1").arg(QTime::currentTime().toString(Qt::ISODate)));
  } else {
    emit Info(QString::fromUtf8("Ошибка импорта событий %1").arg(QTime::currentTime().toString(Qt::ISODate)));
  }
}

void FormEvents::on_treeViewEvents_activated(const QModelIndex& index)
{
  if (mShowEvent && LoadEvent(index)) {
    ShowEvent();
  }
}

void FormEvents::on_actionShowEvent_triggered()
{
  if (mCurrentEventLog) {
    ShowEvent();
  }
}

void FormEvents::on_actionShowClose_triggered()
{
  emit ShowClose();
}
