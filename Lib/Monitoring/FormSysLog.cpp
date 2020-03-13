#include <QDateTime>

#include <Lib/Db/ObjectType.h>
#include <Lib/Db/ObjectState.h>
#include <Lib/Db/Event.h>
#include <Lib/Db/Files.h>
#include <Lib/Log/Log.h>
#include <Lib/Common/Format.h>
#include <Lib/Ui/UpWaiter.h>

#include "FormSysLog.h"
#include "ui_FormSysLog.h"


void FormSysLog::Init(Core* _Core)
{
  mCore = _Core;
}

void FormSysLog::ReloadObjects()
{
  auto q = mCore->GetDb().MakeQuery();
  q->prepare(QString("SELECT DISTINCT ot._id, ot.name, ot.descr FROM object o1"
                     " INNER JOIN object_connection oc ON oc._omaster = o1._id"
                     " INNER JOIN object o ON o._id = oc._oslave"
                     " INNER JOIN object_type ot ON ot._id = o._otype"
                     " WHERE o1._otype = %1"
                     " ORDER BY ot._id;").arg(mCore->getServerTypeId()));

  ui->comboBoxLogFilterObjectType->clear();
  ui->comboBoxLogFilterObjectType->addItem(QIcon(":/ObjTree/Icons/System.png"), QString::fromUtf8("Все"), 0);
  ui->comboBoxLogFilterObjectType->addItem(QIcon(":/ObjTree/srv"), mCore->getServerTypeName(), mCore->getServerTypeId());
  if (mCore->GetDb().ExecuteQuery(q)) {
    while (q->next()) {
      int index = 0;
      int id = q->value(index++).toInt();
      QString abbr = q->value(index++).toString();
      QString name = q->value(index++).toString();
      ui->comboBoxLogFilterObjectType->addItem(QIcon(QString(":/ObjTree/%1").arg(abbr)), name, id);
    }
  }
}

QueryS FormSysLog::RetriveLogFilter()
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
  auto q = mCore->GetDb().MakeQuery();
  q->prepare(queryText);
  q->bindValue(":from", from);
  q->bindValue(":to", to);

  if (!mCore->GetDb().ExecuteQuery(q)) {
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

bool FormSysLog::UpdateLog(QueryS &q)
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

void FormSysLog::AddLogRecord(int objectStateId, int stateValue, const QDateTime& fromTime, const QDateTime& toTime)
{
  static const QString kError = QString::fromUtf8("<Ошибка>");
  QString objectText = kError;
  QString objectIconText;
  QString stateText = kError;
  QString stateColor;

  if (const ObjectStateItem* state = static_cast<const ObjectStateItem*>(mCore->getObjectStateTable()->GetItem(objectStateId).data())) {
    if (const ObjectItem* objectItem = static_cast<const ObjectItem*>(
          mCore->getObjectTable()->GetItem(state->ObjectId).data())) {
      objectText = objectItem->Name;
      if (const ObjectTypeItem* objectTypeItem = static_cast<const ObjectTypeItem*>(mCore->getObjectTypeTable()->GetItem(objectItem->Type).data())) {
        objectIconText = QString(":/ObjTree/%1").arg(objectTypeItem->Name);
      }
    }
    if (const ObjectStateValuesItem* stateValueItem = static_cast<const ObjectStateValuesItem*>(
          mCore->getObjectStateValuesTable()->GetItemByTypeState(state->ObjectStateTypeId, stateValue))) {
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
  QDateTime endTime = toTime.isNull()? ui->dateTimeEditLogFilterTo->dateTime(): toTime;
  if (endTime > QDateTime::currentDateTime()) {
    endTime = QDateTime::currentDateTime();
  }
  QStandardItem* periodItem = new QStandardItem(FormatTimeDeltaRu(fromTime.msecsTo(endTime)));
  mLogModel->appendRow(QList<QStandardItem*>() << objectItem << stateItem << fromItem << toItem << periodItem);
}

void FormSysLog::on_pushButtonLogFilterApply_clicked()
{
  if (QueryS q = RetriveLogFilter()) {
    if (UpdateLog(q)) {
      emit Info(QString::fromUtf8("Лог загружен успешно %1").arg(QTime::currentTime().toString(Qt::ISODate)));
    } else {
      emit Info(QString::fromUtf8("Ошибка загрузки лога %1").arg(QTime::currentTime().toString(Qt::ISODate)));
    }
  } else {
    emit Info(QString::fromUtf8("Ошибка фильтра %1").arg(QTime::currentTime().toString(Qt::ISODate)));
  }
}


FormSysLog::FormSysLog(QWidget *parent)
  : QWidget(parent), ui(new Ui::FormSysLog)
  , mCore(nullptr)
{
  ui->setupUi(this);

  QDateTime baseTime(QDateTime::currentDateTime());
  baseTime.setTime(QTime(baseTime.time().hour(), baseTime.time().minute(), 0));
  ui->dateTimeEditLogFilterFrom->setDateTime(baseTime.addDays(-1));
  ui->dateTimeEditLogFilterTo->setDateTime(baseTime.addDays(1));

  mLogModel = new QStandardItemModel(this);
  ui->treeViewLog->setModel(mLogModel);
}

FormSysLog::~FormSysLog()
{
  delete ui;
}

