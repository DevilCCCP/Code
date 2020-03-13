#pragma once

#include <QMainWindow>
#include <QSettings>
#include <QDir>
#include <QFileDialog>

#include <Lib/Include/Common.h>
#include <LibV/Include/Region.h>
#include <Lib/CoreUi/MainWindow2.h>


namespace Ui {
class MainWindow;
}

DefineClassS(SignalMark);
DefineClassS(SignalMark2);
DefineClassS(SignalMark3);
DefineClassS(Analyser);

class MainWindow: public MainWindow2
{
  Ui::MainWindow*   ui;
  QSettings*        mSettings;
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

  SignalMarkS       mSignalMark;
  SignalMark2S      mSignalMark2;
  SignalMark3S      mSignalMark3;
  AnalyserS         mAnalyser;

  int               mCurrentActions;
  QAction*          mDumpCombo;
  QStringList       mDumpList;
  int               mCurrentDumpIndex;
  int               mCurrentDumpParam1;
  int               mCurrentDumpParam2;
  Region<uchar>     mSourceRegion;
  QVector<uchar>    mLineValues;
  QVector<uchar>    mLineMarks;

  typedef void (SignalMark3::*DumpLineFunc)(QVector<uchar>&);
  struct DumpLine {
    QString        Name;
    DumpLineFunc   Function;

    DumpLine(const QString& _Name, const DumpLineFunc& _Function): Name(_Name), Function(_Function) { }
  };
  typedef void (Analyser::*DumpRegionFunc)(Region<uchar>*, int, int);
  struct DumpRegion {
    QString        Name;
    DumpRegionFunc Function;
    int            Param1Min;
    int            Param1Max;
    int            Param1Default;
    int            Param2Min;
    int            Param2Max;
    int            Param2Default;

    DumpRegion(const QString& _Name, const DumpRegionFunc& _Function, int _Param1Min = 0, int _Param1Max = 0, int _Param1Default = 0, int _Param2Min = 0, int _Param2Max = 0, int _Param2Default = 0)
      : Name(_Name), Function(_Function)
      , Param1Min(_Param1Min), Param1Max(_Param1Max), Param1Default(qBound(_Param1Min, _Param1Default, _Param1Max))
      , Param2Min(_Param2Min), Param2Max(_Param2Max), Param2Default(qBound(_Param2Min, _Param2Default, _Param2Max))
    { }
  };
  typedef void (MainWindow::*OnDumpTriggerFunc)(int);

  QList<DumpLine>   mDumpLine;
  QList<DumpRegion> mDumpRegion;

  Q_OBJECT

public:
  explicit MainWindow(QWidget* parent = 0);
  ~MainWindow();

private:
  void SetImageAuto();
  bool SetImage(const QImage& img, const QString& name);
  void ApplyDir();
  void ApplyImage();
  bool PrepareData();
  void PrepareLineActions(const QList<DumpLine>& infoList, int defaultIndex);
  void PrepareFilterActions(const QList<DumpRegion>& infoList, int defaultIndex);
  void PrepareDump(OnDumpTriggerFunc onDumpTriggerFunc, int defaultIndex);
  void SetGrayscale();
  void SetDiff();
  void FilterLine();
  void FilterLine2();
  void FilterRect();
  void FilterImage();
  void FilterImage2();
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
  QImage ImageFromRegion(const Region<uchar>& region);
  QImage IndexFromRegion(const Region<uchar>& region);
  uchar MaxData();
  uchar MaxRegion(const Region<uchar>& region);

  void UpdateTab(bool force = false);
  void UpdateDumpActionsView();

  void SwitchSelect(int select, bool checked);

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
  void on_actionFilter2_triggered();
  void on_actionFilterMk3Color_triggered();
  void on_actionLineStats2_triggered();
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

