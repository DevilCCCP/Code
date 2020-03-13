#include <QDateTime>

#include <Lib/Db/ObjectType.h>
#include <Lib/Db/ObjectState.h>
#include <Lib/Db/Event.h>
#include <Lib/Db/Files.h>
#include <Lib/Log/Log.h>
#include <Lib/Common/Format.h>
#include <Lib/Ui/UpWaiter.h>

#include "FormSystem.h"
#include "ui_FormSystem.h"


const int kNotAvailableSec = 5;

void FormSystem::Init(Core* _Core)
{
  mCore = _Core;
}

void FormSystem::ReloadState()
{
  mStateModel->clear();
  mStateMap.clear();
  mStateModel->setColumnCount(3);
  mStateModel->setHorizontalHeaderItem(0, new QStandardItem(QString::fromUtf8("Сервера")));
  mStateModel->setHorizontalHeaderItem(1, new QStandardItem(QString::fromUtf8("Состояние")));
  mStateModel->setHorizontalHeaderItem(2, new QStandardItem());
  QSet<int> childs = mCore->getObjectTable()->MasterConnection().keys().toSet();
  const QMap<int, TableItemS>& items = mCore->getObjectTable()->GetItems();
  for (auto itr = items.begin(); itr != items.end(); itr++) {
    const ObjectItem& item = static_cast<const ObjectItem&>(*itr.value());
    if (item.Type == mCore->getServerTypeId() && !childs.contains(item.Id)) {
      QStandardItem* modelItem = new QStandardItem(QString("%1").arg(item.Name));
      modelItem->setIcon(QIcon(":/ObjTree/srv"));
      const QMap<int, int>& slaves = mCore->getObjectTable()->SlaveConnection();
      for (auto itr2 = slaves.find(item.Id); itr2 != slaves.end() && itr2.key() == item.Id; itr2++) {
        int slaveId = itr2.value();
        const ObjectItem* item2 = static_cast<const ObjectItem*>(mCore->getObjectTable()->GetItem(slaveId).data());
        if (item2 && item2->Type != mCore->getScheduleTypeId()) {
          QStandardItem* modelItem2 = new QStandardItem(QString("%1").arg(item2->Name));
          const ObjectTypeItem* item2Type = static_cast<const ObjectTypeItem*>(mCore->getObjectTypeTable()->GetItem(item2->Type).data());
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

bool FormSystem::UpdateState()
{
  mCore->getObjectStateTable()->Reload();
  if (!mCore->getObjectStateTable()->Open()) {
    return false;
  }

  const QMap<int, TableItemS>& items = mCore->getObjectStateTable()->GetItems();
  for (auto itr = items.begin(); itr != items.end(); itr++) {
    const ObjectStateItem& item = static_cast<const ObjectStateItem&>(*itr.value());
    auto itr2 = mStateMap.find(item.ObjectId);
    if (itr2 != mStateMap.end()) {
      QStandardItem* modelState = itr2.value();

      if (item.ChangeSec > kNotAvailableSec) {
        modelState->setText(QString::fromUtf8("<Недоступен>"));
        modelState->setForeground(mItemDisabled.first);
        modelState->setBackground(mItemDisabled.second);
      } else if (const ObjectStateValuesItem* itemValue = mCore->getObjectStateValuesTable()->GetItemByTypeState(item.ObjectStateTypeId, item.State)) {
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

QStandardItem* FormSystem::CreateBadStateItem()
{
  QStandardItem* modelState = new QStandardItem(QString::fromUtf8("<Отсутствует>"));
  modelState->setForeground(mItemNotExists.first);
  modelState->setBackground(mItemNotExists.second);
  return modelState;
}


FormSystem::FormSystem(QWidget *parent)
  : QWidget(parent), ui(new Ui::FormSystem)
  , mCore(nullptr)
{
  ui->setupUi(this);

  QColor baseFront = ui->treeViewState->palette().color(QPalette::Foreground);
  QColor baseBack = ui->treeViewState->palette().color(QPalette::Base);
  QColor lighter;
  lighter.setHsv(baseFront.hue(), baseFront.saturation(), (baseFront.value() + baseBack.value())/2);

  mItemEnabled = qMakePair(QBrush(baseFront), QBrush(baseBack));
  mItemDisabled = qMakePair(QBrush(lighter), QBrush(baseBack));
  mItemNotExists = qMakePair(QBrush(lighter), QBrush(baseBack.darker(120)));

  mStateModel = new QStandardItemModel(this);
  ui->treeViewState->setModel(mStateModel);
}

FormSystem::~FormSystem()
{
  delete ui;
}

