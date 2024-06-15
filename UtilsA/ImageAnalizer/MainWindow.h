#pragma once

#include <QMainWindow>
#include <QSettings>
#include <QDir>
#include <QFileDialog>

#include <Lib/Include/Common.h>
#include <Lib/CoreUi/MainWindow2.h>
#include <LibA/Analyser/ByteRegion.h>


namespace Ui {
class MainWindow;
}

DefineClassS(Analyser);

class MainWindow: public MainWindow2
{
  Ui::MainWindow*   ui;
  QClipboard*       mClipboard;
  QString           mImageSource;

  enum ETab {
    eTabImage  = 0,
    eTabLineStats,
    eTabRectStats,
    eTabGrayscale,
    eTabDiff,
    eTabFilter,
    eTabIllegal
  };

  ETab              mCurrentTab;
  int               mSelect;
  QVector<bool>     mLoadedTabs;
  QImage            mCurrentImage;
  QVector<uchar>    mCurrentValue;
  QVector<uchar>    mFilterValue;
  QFileDialog*      mFileDialog;
  QStringList       mDirFiles;
  QDir              mCurrentDir;
  int               mCurrentFile;

  AnalyserS         mAnalyser;

  int               mCurrentActions;
  QAction*          mDumpCombo;
  int               mMaxDumpIndex;
  int               mCurrentLineIndex;
  int               mCurrentDumpIndex;
  int               mCurrentDumpParam1;
  int               mCurrentDumpParam2;
  ByteRegion        mSourceRegion;
  QVector<uchar>    mLineValues;
  QVector<uchar>    mLineMarks;
  bool              mLineChanged;

  Q_OBJECT

public:
  explicit MainWindow(QWidget* parent = 0);
  ~MainWindow();

private:
  void SetImageAuto();
  bool SetImage(const QImage& img, const QString& name);

  void UpdateActions();
  void UpdateLineXy();

  void ApplyDir();
  void ApplyImage();
  bool PrepareLine();
  bool PrepareData();
  void PrepareLineActions();
  void PrepareFilterActions();
  void PrepareDump(const QStringList& nameList);
  void SetGrayscale();
  void SetDiff();
  void FilterLine();
  void FilterRect();
  void FilterImage();
  void FilterImageMk3Color();
  void FilterFindNumbers();
  void FilterResolveNumbers();
  void FilterErosionBlack();
  void FilterErosionWhite();
  void DataFromImage();
  void PrepareLineFilters();
  void PrepareImageFilters();
  QImage ImageFromData();
  QImage IndexFromData();
  QImage ImageFromRegion(const ByteRegion& region);
  QImage IndexFromRegion(const ByteRegion& region);
  uchar MaxData();
  uchar MaxRegion(const ByteRegion& region);

  void ResetPointsView();
  void PrepareTab();
  void UpdateTab(bool force = false);
  void UpdateDumpActionsView();

  void SwitchSelect(int select, bool checked);

  void DumpLine();
  void DumpRegion();

private:
  void OnLineChanged();
  void OnDumpLineChanged(int index);
  void OnDumpLineTriggered();
  void OnDumpRegionChanged(int index);
  void OnDumpRegionParamsChanged(int);
  void OnDumpRegionTriggered();

private slots:
  void on_actionPaste_triggered();
  void on_actionOpenFile_triggered();
  void on_actionPrevFile_triggered();
  void on_actionNextFile_triggered();
  void on_horizontalSliderFiles_sliderMoved(int position);
  void on_tabWidgetMain_currentChanged(int index);
  void on_actionLineStats_triggered();
  void on_actionFilter_triggered();
  void on_actionFilterMk3Color_triggered();
  void on_actionErosionBlack_triggered();
  void on_actionErosionWhite_triggered();
  void on_spinBoxLineX_valueChanged(int value);
  void on_spinBoxLineY_valueChanged(int value);
  void on_actionNextFilterImage_triggered();
  void on_actionPrevFilterImage_triggered();
  void on_actionViewSelect_toggled(bool checked);
  void on_actionViewLine_toggled(bool checked);
  void on_actionViewRectangle_toggled(bool checked);
  void on_actionAreaStats_triggered();
  void on_actionWhiteBallance_triggered();
};

