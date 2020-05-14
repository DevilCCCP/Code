#pragma once

#include <QFutureWatcher>
#include <QFileDialog>
#include <QTimer>

#include <Lib/CoreUi/MainWindow2.h>


DefineClassS(Core);
DefineClassS(Decoration);
DefineClassS(Editing);
DefineClassS(Ai);
DefineClassS(Puzzle);
DefineClassS(UiInformer);
DefineClassS(CreatorMainWindow);
DefineClassS(DigitsWidget);
DefineClassS(GameState);

class FormCalcPopup;
class DialogSolve;
class QWidgetB;

namespace Ui {
class MainWindow;
}

class MainWindow: public MainWindow2
{
  Ui::MainWindow*        ui;
  CoreS                  mCore;
  DecorationS            mDecoration;
  EditingS               mEditing;
  GameStateS             mGameState;
  UiInformerS            mUiInformer;
  CreatorMainWindowS     mCreatorMainWindow;

  FormCalcPopup*         mFormCalcPopup;
  QFileDialog*           mFileDialog;
  QVector<QWidgetB*>     mAllWidgets;
  QVector<DigitsWidget*> mSideWidgets;
  QTimer*                mAutoSaveTimer;

  Qt::WindowStates       mNormalState;
  int                    mCentralY;
  int                    mPreviewMove;
  bool                   mLoadDone;
  bool                   mClosing;

  AiS                    mSolveAi;
  PuzzleS                mSolvePuzzle;
  DialogSolve*           mSolveDialog;
  QFutureWatcher<void>*  mSolveWatcher;
  bool                   mLastSolveFail;

  AiS                    mStarsAi;
  PuzzleS                mStarsSourcePuzzle;
  PuzzleS                mStarsPuzzle;
  QString                mStarsFilename;
  QFutureWatcher<void>*  mStarsWatcher;
  bool                   mStarsInProgress;
  bool                   mLoadNextPuzzle;
  QPixmap                mStarYes;
  QPixmap                mStarNo;

  Q_OBJECT

public:
  explicit MainWindow(QWidget* parent = 0);
  ~MainWindow();

protected:
  /*override */void showEvent(QShowEvent* event) Q_DECL_OVERRIDE;
  /*override */virtual void moveEvent(QMoveEvent* event) Q_DECL_OVERRIDE;
  /*override */void closeEvent(QCloseEvent* event) Q_DECL_OVERRIDE;

public:
  void Info(const QString& text);
  void Warning(const QString& text);
  void Error(const QString& text);

private:
  void InitSolveAi();
  void InitStarsAi();
  void LoadPuzzle();
  void UpdateSettings();
  void DrawBack();
  void Draw();
  void ShowStars();
  void CalcVisibleRect();
  void Update();
  void UpdateTitle();
  void UpdateZoom();
  void PlaceWidgets();
  void SetMode(int mode, bool checked);
  void CalcPuzzleStars();

private:
  void OnInit();
  void OnAutoSaveTimeout();
  void OnChangedDigits(int type, const QPoint& p1, const QPoint& p2);
  void OnUpdateDigits(const QPoint& p1, const QPoint& p2);
  void OnHScrollChanged(int value);
  void OnVScrollChanged(int value);
  void OnHScrollRangeChanged(int min, int max);
  void OnVScrollRangeChanged(int min, int max);
  void OnZoomChanged();
  void OnSetCursor();
  void OnUpdateHasUndo();
  void OnUpdateHasRedo();
  void OnLoadPuzzleTest();
  void OnGameStateChanged();
  void OnGameSoledChanged();

  void StartSolve(int maxProp);
  void OnSolveInfo(QByteArray data);
  void OnSolveDone(int result, int prop);
  void OnSolveStarted();
  void OnSolveFinished();
  void OnStarsStarted();
  void OnStarsFinished();

  void OnPuzzleChanged();

  void OnMoveToLocation(int i, int j);

signals:
  void PostLoginTest();

private slots:
  void on_actionExit_triggered();
  void on_actionOpen_triggered();
  void on_actionLogin_triggered();
  void on_actionProp1_triggered(bool checked);
  void on_actionProp2_triggered(bool checked);
  void on_actionProp3_triggered(bool checked);
  void on_actionErase_triggered(bool checked);
  void on_actionPropClear_triggered();
  void on_actionUndo_triggered();
  void on_actionRedo_triggered();
  void on_actionSettings_triggered();
  void on_horizontalSliderZoom_valueChanged(int value);
  void on_actionZoomIn_triggered();
  void on_actionZoomOut_triggered();
  void on_toolButtonState_clicked();
  void on_actionClearAll_triggered();
  void on_actionAbout_triggered();
  void on_actionFullScreen_triggered(bool checked);
  void on_actionPropApply_triggered();
  void on_actionHint_triggered();
  void on_actionTest_triggered();
  void on_actionSolve_triggered();
  void on_actionSolveEx_triggered();
  void on_actionList_triggered();
  void on_actionCreator_triggered();
  void on_toolButtonCalcStars_clicked();
};
