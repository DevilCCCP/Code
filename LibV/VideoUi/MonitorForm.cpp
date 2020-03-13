#include <QMessageBox>

#include <Lib/Db/ObjectType.h>
#include <Lib/Db/ArmMonitors.h>
#include <Lib/Db/MonitorLayouts.h>
#include <Lib/Db/AmlCamMap.h>
#include <Lib/Common/Icon.h>
#include <Lib/Log/Log.h>

#include "MonitorForm.h"
#include "ui_MonitorForm.h"


MonitorForm::MonitorForm(const Db& _Db, const ObjectItemS& _Object, int _CameraTypeId, int _Camera2TypeId, QWidget* parent)
  : QWidget(parent)
  , ui(new Ui::MonitorForm)
  , mDb(_Db), mObjectTable(new ObjectTable(_Db)), mArmMonitorsTable(new ArmMonitorsTable(_Db))
  , mMonitorLayoutsTable(new MonitorLayoutsTable(_Db)), mAmlCamMapTable(new AmlCamMapTable(_Db))
  , mObject(_Object), mCameraTypeId(_CameraTypeId), mCamera2TypeId(_Camera2TypeId)
  , mInit(false), mClearCameraSelection(false)
{
  Q_INIT_RESOURCE(VideoUi);

  ui->setupUi(this);

  mMonitorIcon = QIcon(":/Icons/Monitor.png");
  mMonitorIconGray = (!mMonitorIcon.availableSizes().isEmpty())
      ? QIcon(mMonitorIcon.pixmap(mMonitorIcon.availableSizes().first(), QIcon::Disabled))
      : mMonitorIcon;
  ui->pushButtonMonitorUnuse->setIcon(mMonitorIconGray);
  mCameraIcon = QIcon(":/ObjTree/cam");
  ui->pushButtonMonitorAdd->setIcon(IconFromImage(":/Icons/Monitor.png", ":/ObjTree/Icons/Add.png"));
  ui->pushButtonMonitorRemove->setIcon(IconFromImage(":/Icons/Monitor.png", ":/ObjTree/Icons/Remove.png"));
  mLayoutIcon = QIcon(":/Icons/Layout.png");
  mLayoutPixmap = (!mLayoutIcon.availableSizes().isEmpty())
      ? mLayoutIcon.pixmap(mLayoutIcon.availableSizes().first())
      : QPixmap(60, 40);

  mMonitorsModel = new QStandardItemModel(this);
  ui->listViewMonitorList->setModel(mMonitorsModel);

  mCameraModel = new QStandardItemModel(this);
  ui->listViewCameras->setModel(mCameraModel);

  if (auto sm = ui->listViewMonitorList->selectionModel()) {
    connect(sm, &QItemSelectionModel::selectionChanged, this, &MonitorForm::OnMonitorSelectionChanged);
  }
  if (auto sm = ui->listViewCameras->selectionModel()) {
    connect(sm, &QItemSelectionModel::selectionChanged, this, &MonitorForm::OnCamerasSelectionChanged);
  }
}

MonitorForm::~MonitorForm()
{
  delete ui;
}

void MonitorForm::showEvent(QShowEvent* event)
{
  Q_UNUSED(event);

  if (!mInit) {
    Init();
    ui->widgetLayouts->setVisible(false);
    ui->pushButtonMonitorUse->setVisible(false);
    ui->pushButtonMonitorUse->setMaximumWidth(16777215);
    ui->pushButtonMonitorUnuse->setVisible(false);
    ui->pushButtonMonitorUnuse->setMaximumWidth(16777215);
  }
}

void MonitorForm::Init()
{
  mMonitorsModel->clear();
  mMonitors.clear();
  QList<ArmMonitorsS> monitors;
  if (!mArmMonitorsTable->Select(QString("WHERE _object=%1 ORDER by num").arg(mObject->Id), monitors)) {
    return;
  }

  int highNum = 0;
  for (auto itr = monitors.begin(); itr != monitors.end(); itr++) {
    const ArmMonitorsS& monitor = *itr;
    mMonitors[monitor->Num] = monitor;
    highNum = qMax(highNum, monitor->Num);
  }

  mLastUsedNumber = 0;
  for (auto itr = mMonitors.begin(); itr != mMonitors.end(); itr++) {
    const ArmMonitorsS& monitor = *itr;
    for (int i = mLastUsedNumber + 1; i < monitor->Num; i++) {
      AddNewMonitor(i);
    }
    AddMonitor(monitor);
    mLastUsedNumber = monitor->Num;
  }

//  for (int i = mLastUsedNumber + 1; i <= mLastUsedNumber + 3; i++) {
//    AddNewMonitor(i);
//  }
}

void MonitorForm::AddMonitor(const ArmMonitorsS& monitor)
{
  QStandardItem* item = new QStandardItem((monitor->Used)? mMonitorIcon: mMonitorIconGray, monitor->Name);
  item->setData(monitor->Num);
  mMonitorsModel->appendRow(item);
}

ArmMonitorsS MonitorForm::AddNewMonitor(int num)
{
  ArmMonitorsS newMonitor(new ArmMonitors());
  newMonitor->Object = mObject->Id;
  newMonitor->Name = QString::fromUtf8("Монитор %1").arg(num);
  newMonitor->Descr = "";
  newMonitor->Num = num;
  newMonitor->Width = 0;
  newMonitor->Height = 0;
  newMonitor->Size = QPoint(1, 1);
  newMonitor->Used = false;
  mMonitors[num] = newMonitor;
  AddMonitor(newMonitor);
  return newMonitor;
}

void MonitorForm::LoadMonitorLayouts()
{
  ui->lineEditMonitorNumber->setText(QString::number(mCurrentMonitor->Num));
  ui->lineEditMonitorResolution->setText(QString("%1x%2").arg(mCurrentMonitor->Width).arg(mCurrentMonitor->Height));
  if (!mCurrentMonitor->Used) {
    ui->pushButtonMonitorUnuse->setVisible(false);
    ui->pushButtonMonitorUse->setVisible(true);
    ui->widgetLayouts->setVisible(false);
    return;
  }
  ui->pushButtonMonitorUse->setVisible(false);
  ui->pushButtonMonitorUnuse->setVisible(true);
  ui->widgetLayouts->setVisible(true);

  for (auto itr = mLayoutFrames.begin(); itr != mLayoutFrames.end(); itr++) {
    const LayoutList& list = *itr;
    for (auto itr = list.begin(); itr != list.end(); itr++) {
      LayoutLabel* lbl = *itr;
      ui->gridLayoutLayouts->removeWidget(lbl);
      lbl->deleteLater();
    }
  }
  mLayoutFrames.clear();
  ClearSelection();

  if (mCurrentMonitor->Size.x() <= 0 || mCurrentMonitor->Size.y() <= 0) {
    mCurrentMonitor->Size = QPoint(1, 1);
  }
  ui->spinBoxLayoutHorz->setValue(mCurrentMonitor->Size.x());
  ui->spinBoxLayoutVert->setValue(mCurrentMonitor->Size.y());

  for (int j = 0; j < mCurrentMonitor->Size.y(); j++) {
    LayoutList list;
    for (int i = 0; i < mCurrentMonitor->Size.x(); i++) {
      LayoutLabel* lbl = CreateLayoutLabel(j, i);
      list.append(lbl);
    }
    mLayoutFrames.append(list);
  }

  mMonitorLayoutsTable->Select(QString(" WHERE _amonitor = %1").arg(mCurrentMonitor->Id), mLayoutsList);

  ui->pushButtonCameraFilter->setChecked(false);
  LoadCameras(false);
}

LayoutLabel* MonitorForm::CreateLayoutLabel(int j, int i)
{
  LayoutLabel* lbl = new LayoutLabel(QPoint(i, j), this);
  lbl->setStyleSheet("background-image: url(:/Icons/Layout.png);");
  lbl->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  lbl->setMinimumWidth(mLayoutPixmap.width());
  lbl->setMinimumHeight(mLayoutPixmap.height());

  ui->gridLayoutLayouts->addWidget(lbl, j, i);
  connect(lbl, SIGNAL(Clicked()), this, SLOT(OnLayoutClicked()));
  return lbl;
}

bool MonitorForm::FrameInLayouts(int j, int i)
{
  for (auto itr = mLayoutsList.begin(); itr != mLayoutsList.end(); itr++) {
    const MonitorLayoutsS& monitorLayout = *itr;
    if (monitorLayout->Place.contains(i, j)) {
      return true;
    }
  }
  return false;
}

void MonitorForm::ClearSelection()
{
  mLayoutSelectRect = QRect(-1, -1, 0, 0);
  mLayoutSelectPoint = QPoint(-1, -1);
}

void MonitorForm::ClearCamerasSelection()
{
  if (auto sm = ui->listViewCameras->selectionModel()) {
    mClearCameraSelection = true;
    sm->select(QItemSelection(), QItemSelectionModel::Clear);
    mClearCameraSelection = false;
  }
}

void MonitorForm::UpdateCurrentMonitor()
{
  if (mCurrentMonitor->Id) {
    mArmMonitorsTable->Update(mCurrentMonitor);
  } else {
    mArmMonitorsTable->Insert(mCurrentMonitor);
  }

  QStandardItem* rootItem = mMonitorsModel->invisibleRootItem();
  for (int i = 0; i < rootItem->rowCount(); i++) {
    QStandardItem* item = rootItem->child(i);
    int num = item->data().toInt();
    if (num == mCurrentMonitor->Num) {
      item->setIcon((mCurrentMonitor->Used)? mMonitorIcon: mMonitorIconGray);
      break;
    }
  }

  LoadMonitorLayouts();
}

void MonitorForm::LoadCameras(bool useFilter)
{
  QString filter = ui->lineEditCameraFilter->text();
  QList<ObjectItemS> cameraItems;
  if (!mObjectTable->GetObjectsByType(mCameraTypeId, cameraItems)) {
    QMessageBox::warning(this, QString::fromUtf8("Внутренняя ошибка"), QString::fromUtf8("Неудалось загрузить список камер"));
    return;
  }
  if (mCamera2TypeId && !mObjectTable->GetObjectsByType(mCamera2TypeId, cameraItems)) {
    QMessageBox::warning(this, QString::fromUtf8("Внутренняя ошибка"), QString::fromUtf8("Неудалось загрузить список аналоговых камер"));
    return;
  }

  mCameraModel->clear();
  mCameraIndex.clear();
  int index = 0;
  mCameraModel->appendRow(new QStandardItem(mLayoutIcon, QString::fromUtf8("Нет камер")));
  for (auto itr = cameraItems.begin(); itr != cameraItems.end(); itr++) {
    const ObjectItemS& cameraItem = *itr;
    if (useFilter && !cameraItem->Name.contains(filter)) {
      continue;
    }
    if (mObjectTable->IsDefault(cameraItem->Id)) {
      continue;
    }

    ++index;
    QStandardItem* item = new QStandardItem(mCameraIcon, QString("%1: %2").arg(index).arg(cameraItem->Name));
    item->setData(cameraItem->Id);
    mCameraIndex[cameraItem->Id] = index;
    mCameraModel->appendRow(item);
  }

  DrawLayoutCameras();
  UpdateLayoutCameras();
}

void MonitorForm::UpdateLayoutCameras()
{
  for (int j = 0; j < mLayoutFrames.size(); j++) {
    LayoutList* list = &mLayoutFrames[j];
    for (int i = 0; i < list->size(); i++) {
      LayoutLabel* lbl = list->at(i);
      if (mLayoutSelectRect.contains(i, j)) {
        lbl->setStyleSheet("background-image: url(:/Icons/LayoutSelected.png);");
      } else if (FrameInLayouts(j, i)) {
        lbl->setStyleSheet("background-image: url(:/Icons/Layout.png);");
      } else {
        lbl->setStyleSheet("background-image: url(:/Icons/LayoutGrayed.png);");
        lbl->setText(QString());
      }
    }
  }
}

void MonitorForm::ResizeMonitorLayouts(int count, bool horz)
{
  if (mLayoutFrames.isEmpty()) {
    return;
  }
  if (!mCurrentMonitor) {
    QMessageBox::warning(this, QString::fromUtf8("Обновление раскладки"), QString::fromUtf8("Необходимо выбрать монитор"));
    return;
  }

  if (horz) {
    for (auto itr = mLayoutFrames.begin(); itr != mLayoutFrames.end(); itr++) {
      LayoutList* list = &*itr;
      for (int i = count; i < mCurrentMonitor->Size.x(); i++) {
        LayoutLabel* lbl = list->takeLast();
        ui->gridLayoutLayouts->removeWidget(lbl);
        lbl->deleteLater();
      }
    }

    for (int j = 0; j < mCurrentMonitor->Size.y(); j++) {
      LayoutList* list = &mLayoutFrames[j];
      while (list->size() > count) {
        list->removeLast();
      }
      for (int i = list->size(); i < count; i++) {
        LayoutLabel* lbl = CreateLayoutLabel(j, i);
        list->append(lbl);
      }
    }

    mCurrentMonitor->Size.setX(count);
  } else {
    for (int j = count; j < mLayoutFrames.size(); j++) {
      LayoutList list = mLayoutFrames.takeLast();
      for (auto itr = list.begin(); itr != list.end(); itr++) {
        LayoutLabel* lbl = *itr;
        ui->gridLayoutLayouts->removeWidget(lbl);
        lbl->deleteLater();
      }
    }

    for (int j = mLayoutFrames.size(); j < count; j++) {
      LayoutList list;
      for (int i = 0; i < mCurrentMonitor->Size.x(); i++) {
        LayoutLabel* lbl = CreateLayoutLabel(j, i);
        list.append(lbl);
      }
      mLayoutFrames.append(list);
    }

    mCurrentMonitor->Size.setY(count);
  }

  ClearSelection();
  FixLayoutBounds();
  UpdateLayoutCameras();

  if (!mArmMonitorsTable->Update(mCurrentMonitor)) {
    QMessageBox::warning(this, QString::fromUtf8("Внутренняя ошибка"), QString::fromUtf8("Неудалось загрузить изменения в БД"));
  }
}

void MonitorForm::FixLayoutBounds()
{
  QRect fullMonitor(0, 0, mCurrentMonitor->Size.x(), mCurrentMonitor->Size.y());
  for (auto itr = mLayoutsList.begin(); itr != mLayoutsList.end(); ) {
    const MonitorLayoutsS& ml = *itr;
    if (ml->Monitor == mCurrentMonitor->Id && ml->Place.intersected(fullMonitor) != ml->Place) {
      mMonitorLayoutsTable->Delete(ml->Id);
      itr = mLayoutsList.erase(itr);
      continue;
    }
    itr++;
  }
}

void MonitorForm::DiscardLayouts()
{
  mCurrentLayout.clear();
  for (auto itr = mLayoutsList.begin(); itr != mLayoutsList.end(); ) {
    const MonitorLayoutsS& ml = *itr;
    if (mLayoutSelectRect == ml->Place) {
      mCurrentLayout = ml;
    } else if (mLayoutSelectRect.intersects(ml->Place)) {
      mMonitorLayoutsTable->Delete(ml->Id);
      itr = mLayoutsList.erase(itr);
      continue;
    }
    itr++;
  }
}

void MonitorForm::SetLayoutCameras(const QList<int>& cameras)
{
  if (cameras.isEmpty()) {
    if (mCurrentLayout) {
      if (mMonitorLayoutsTable->Delete(*mCurrentLayout)) {
        mCurrentLayout.clear();
        mLayoutsList.removeOne(mCurrentLayout);
      }
    }

    DrawLayoutCameras();
    return;
  }
  if (mCurrentLayout) {
    mAmlCamMapTable->RemoveSlaves(mCurrentLayout->Id);
    mCurrentLayout->Cameras.clear();
  } else {
    mCurrentLayout.reset(new MonitorLayouts());
    mCurrentLayout->Monitor = mCurrentMonitor->Id;
    mCurrentLayout->Place = mLayoutSelectRect;
    if (!mMonitorLayoutsTable->Insert(mCurrentLayout)) {
      return;
    }
    mLayoutsList.append(mCurrentLayout);
  }

  for (auto itr = cameras.begin(); itr != cameras.end(); itr++) {
    int camId = *itr;
    if (mAmlCamMapTable->InsertItem(mCurrentLayout->Id, camId)) {
      mCurrentLayout->Cameras.append(camId);
    }
  }
  mCurrentLayout->CameraLoaded = true;

  DrawLayoutCameras();
}

void MonitorForm::DrawLayoutCameras()
{
  for (int j = 0; j < mLayoutFrames.size(); j++) {
    const LayoutList& list = mLayoutFrames[j];
    for (int i = 0; i < list.size(); i++) {
      LayoutLabel* lbl = list[i];
      lbl->setText("");
    }
  }

  for (auto itr = mLayoutsList.begin(); itr != mLayoutsList.end(); itr++) {
    const MonitorLayoutsS& layout = *itr;
    if (!layout->CameraLoaded) {
      mAmlCamMapTable->SelectSlaves(layout->Id, layout->Cameras);
      layout->CameraLoaded = true;
    }

    DrawLayoutCamerasOne(layout);
  }

  UpdateLayoutCameras();
}

void MonitorForm::DrawLayoutCamerasOne(const MonitorLayoutsS& layout)
{
  QStringList camList;
  for (auto itr = layout->Cameras.begin(); itr != layout->Cameras.end(); itr++) {
    int camId = *itr;
    if (camId) {
      auto itr = mCameraIndex.find(camId);
      if (itr != mCameraIndex.end()) {
        camList << QString::number(itr.value());
      } else {
        camList << QString("*");
      }
    }
  }
  QString camText = camList.join(',');

  for (int j = layout->Place.top(); j <= layout->Place.bottom(); j++) {
    if (j >= mLayoutFrames.size()) {
      break;
    }
    const LayoutList& list = mLayoutFrames[j];
    for (int i = layout->Place.left(); i <= layout->Place.right(); i++) {
      if (i >= list.size()) {
        break;
      }
      LayoutLabel* lbl = list[i];
      lbl->setText(camText);
    }
  }
}

void MonitorForm::OnMonitorSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  Q_UNUSED(selected);
  Q_UNUSED(deselected);

  QModelIndexList selection = ui->listViewMonitorList->selectionModel()->selectedIndexes();
  bool hasSelection = !selection.isEmpty();
  ui->widgetMonitorsControl->setEnabled(hasSelection);
  ui->widgetLayouts->setVisible(hasSelection);
  mCurrentMonitor.clear();
  if (hasSelection) {
    QModelIndex selIndex = selection.first();
    mCurrentMonitor = mMonitors[selIndex.data(Qt::UserRole + 1).toInt()];

    LoadMonitorLayouts();
  }
}

void MonitorForm::OnCamerasSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  Q_UNUSED(deselected);
  Q_UNUSED(selected);

  if (mClearCameraSelection) {
    return;
  }

  QModelIndex noCameraIndex;
  QList<int> cameras;
  QModelIndexList selection = ui->listViewCameras->selectionModel()->selectedIndexes();
  for (auto itr = selection.begin(); itr != selection.end(); itr++) {
    const QModelIndex& index = *itr;
    if (!index.isValid()) {
      continue;
    }

    int id = index.data(Qt::UserRole + 1).toInt();
    if (id) {
      cameras.append(id);
    } else {
      noCameraIndex = index;
    }
  }

  if (!cameras.isEmpty() && noCameraIndex.isValid()) {
    ui->listViewCameras->selectionModel()->select(noCameraIndex, QItemSelectionModel::Deselect);
    return;
  }

  DiscardLayouts();
  SetLayoutCameras(cameras);
}

void MonitorForm::OnLayoutClicked()
{
  LayoutLabel* lbl = static_cast<LayoutLabel*>(sender());
  QPoint lastSelPoint = (mLayoutSelectPoint.x() >= 0)
      ? mLayoutSelectPoint: lbl->getUserData().toPoint();
  mLayoutSelectPoint = lbl->getUserData().toPoint();

  mLayoutSelectRect.setLeft(qMin(mLayoutSelectPoint.x(), lastSelPoint.x()));
  mLayoutSelectRect.setTop(qMin(mLayoutSelectPoint.y(), lastSelPoint.y()));
  mLayoutSelectRect.setRight(qMax(mLayoutSelectPoint.x(), lastSelPoint.x()));
  mLayoutSelectRect.setBottom(qMax(mLayoutSelectPoint.y(), lastSelPoint.y()));

  UpdateLayoutCameras();
  ClearCamerasSelection();
}

void MonitorForm::on_pushButtonMonitorAdd_clicked()
{
  ArmMonitorsS newMonitor = AddNewMonitor(++mLastUsedNumber);
  mArmMonitorsTable->Insert(newMonitor);
}

void MonitorForm::on_pushButtonMonitorRemove_clicked()
{
  if (!mCurrentMonitor) {
    QMessageBox::warning(this, QString::fromUtf8("Удаление монитора"), QString::fromUtf8("Необходимо выбрать монитор"));
    return;
  }
  if (!mCurrentMonitor->Id) {
    QMessageBox::information(this, QString::fromUtf8("Удаление монитора"), QString::fromUtf8("Монитор не добавлен в АРМ"));
    return;
  }
  if (mCurrentMonitor->Width > 0 && mCurrentMonitor->Height > 0) {
    QMessageBox::information(this, QString::fromUtf8("Удаление монитора"), QString::fromUtf8("Монитор установлен на АРМ и не может быть удалён"));
    return;
  }

  if (!mArmMonitorsTable->Delete(mCurrentMonitor->Id)) {
    QMessageBox::information(this, QString::fromUtf8("Внутренняя ошибка"), QString::fromUtf8("Удаление монитора не удачно"));
    return;
  }

  QStandardItem* root = mMonitorsModel->invisibleRootItem();
  for (int i = 0; i < root->rowCount(); i++) {
    QStandardItem* child = root->child(i);
    if (child->data() == mCurrentMonitor->Num) {
      mCurrentMonitor.clear();
      mMonitorsModel->removeRow(i);
      break;
    }
  }
}

void MonitorForm::on_pushButtonMonitorUse_clicked()
{
  if (!mCurrentMonitor) {
    QMessageBox::warning(this, QString::fromUtf8("Использование монитора"), QString::fromUtf8("Необходимо выбрать монитор"));
    return;
  }
  if (mCurrentMonitor->Used) {
    QMessageBox::warning(this, QString::fromUtf8("Использование монитора"), QString::fromUtf8("Монитор уже используется"));
    return;
  }

  mCurrentMonitor->Used = true;
  UpdateCurrentMonitor();
}

void MonitorForm::on_pushButtonMonitorUnuse_clicked()
{
  if (!mCurrentMonitor) {
    QMessageBox::warning(this, QString::fromUtf8("Отключение монитора"), QString::fromUtf8("Необходимо выбрать монитор"));
    return;
  }
  if (!mCurrentMonitor->Used) {
    QMessageBox::warning(this, QString::fromUtf8("Отключение монитора"), QString::fromUtf8("Монитор и так не используется"));
    return;
  }

  mCurrentMonitor->Used = false;
  UpdateCurrentMonitor();
}

void MonitorForm::on_pushButtonCameraFilter_clicked(bool checked)
{
  LoadCameras(checked);
}

void MonitorForm::on_lineEditCameraFilter_returnPressed()
{
  ui->pushButtonCameraFilter->setChecked(true);
  LoadCameras(true);
}

void MonitorForm::on_lineEditCameraFilter_textChanged(const QString&)
{
  ui->pushButtonCameraFilter->setChecked(false);
  LoadCameras(false);
}

void MonitorForm::on_spinBoxLayoutHorz_valueChanged(int count)
{
  ResizeMonitorLayouts(count, true);
}

void MonitorForm::on_spinBoxLayoutVert_valueChanged(int count)
{
  ResizeMonitorLayouts(count, false);
}
