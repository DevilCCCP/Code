#pragma once

#include <QWidget>
#include <QStandardItemModel>
#include <QLabel>
#include <QSpacerItem>

#include <Lib/Db/Db.h>

#include "LayoutLabel.h"


namespace Ui {
class MonitorForm;
}

class MonitorForm: public QWidget
{
  Ui::MonitorForm*        ui;

  const Db&               mDb;
  ObjectTableS            mObjectTable;
  ArmMonitorsTableS       mArmMonitorsTable;
  MonitorLayoutsTableS    mMonitorLayoutsTable;
  AmlCamMapTableS         mAmlCamMapTable;
  ObjectItemS             mObject;
  int                     mCameraTypeId;
  int                     mCamera2TypeId;

  QStandardItemModel*     mMonitorsModel;
  QIcon                   mMonitorIcon;
  QIcon                   mMonitorIconGray;
  QStandardItemModel*     mCameraModel;
  QIcon                   mCameraIcon;
  QIcon                   mLayoutIcon;
  QPixmap                 mLayoutPixmap;

  QMap<int, ArmMonitorsS> mMonitors;
  ArmMonitorsS            mCurrentMonitor;

  LayoutMatrix            mLayoutFrames;
  QRect                   mLayoutSelectRect;
  QPoint                  mLayoutSelectPoint;
  QList<MonitorLayoutsS>  mLayoutsList;
  MonitorLayoutsS         mCurrentLayout;

  QMap<int, int>          mCameraIndex;

  int                     mLastUsedNumber;
  bool                    mInit;
  bool                    mClearCameraSelection;

  Q_OBJECT

public:
  explicit MonitorForm(const Db& _Db, const ObjectItemS& _Object, int _CameraTypeId, int _Camera2TypeId, QWidget* parent = 0);
  ~MonitorForm();

protected:
  /*override */virtual void showEvent(QShowEvent* event) Q_DECL_OVERRIDE;

private:
  void Init();
  void AddMonitor(const ArmMonitorsS& monitor);
  ArmMonitorsS AddNewMonitor(int num);
  void LoadMonitorLayouts();

  LayoutLabel* CreateLayoutLabel(int j, int i);
  bool FrameInLayouts(int j, int i);

  void ClearSelection();
  void ClearCamerasSelection();
  void UpdateCurrentMonitor();
  void LoadCameras(bool useFilter);
  void UpdateLayoutCameras();
  void ResizeMonitorLayouts(int count, bool horz);
  void FixLayoutBounds();
  void DiscardLayouts();
  void SetLayoutCameras(const QList<int>& cameras);
  void DrawLayoutCameras();
  void DrawLayoutCamerasOne(const MonitorLayoutsS& layout);

private:
  void OnMonitorSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
  void OnCamerasSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

private slots:
  void OnLayoutClicked();
  void on_pushButtonMonitorAdd_clicked();
  void on_pushButtonMonitorRemove_clicked();
  void on_pushButtonMonitorUse_clicked();
  void on_pushButtonMonitorUnuse_clicked();
  void on_pushButtonCameraFilter_clicked(bool checked);
  void on_lineEditCameraFilter_returnPressed();
  void on_lineEditCameraFilter_textChanged(const QString&);
  void on_spinBoxLayoutHorz_valueChanged(int count);
  void on_spinBoxLayoutVert_valueChanged(int count);
};

