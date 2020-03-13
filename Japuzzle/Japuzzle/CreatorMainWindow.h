#pragma once

#include <QMainWindow>

#include <Lib/CoreUi/MainWindow2.h>


DefineClassS(CreatorMainWindow);
DefineClassS(Puzzle);
DefineClassS(Editing);
DefineClassS(Decoration);
class QFileDialog;

namespace Ui {
class CreatorMainWindow;
}

class CreatorMainWindow: public QMainWindow
{
  Ui::CreatorMainWindow* ui;
  PuzzleS                mPuzzle;
  EditingS               mEditing;
  DecorationS            mDecoration;

  QFileDialog*           mFileDialog;
  int                    mBrickSize;
  int                    mBrickSizeLast;
  QString                mFilename;

  Q_OBJECT

public:
  explicit CreatorMainWindow(QWidget* parent = 0);
  ~CreatorMainWindow();

protected:
  /*override */virtual void moveEvent(QMoveEvent* event) Q_DECL_OVERRIDE;
  /*override */virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
  /*override */void closeEvent(QCloseEvent* event) Q_DECL_OVERRIDE;

public:
  bool ConfirmQuit();

private:
  void CreateNew();
  bool SaveCurrent();
  bool Save();
  bool SaveAs(const QString& filename);
  bool QueryFilename(QString& filename);

  void SetBrickSizeAction(int size, bool set);
  void UpdateZoom();
  void PlaceWidgets();
  void Resize();
  void ApplyNewPuzzle();

signals:
  void WindowChanged();

private:
  void OnUpdateHasUndo();
  void OnUpdateHasRedo();
  void OnZoomChanged();

private slots:
  void on_actionBrick1_triggered(bool checked);
  void on_actionBrick5_triggered(bool checked);
  void on_actionBrick9_triggered(bool checked);
  void on_actionUndo_triggered();
  void on_actionRedo_triggered();
  void on_actionExit_triggered();
  void on_actionOpen_triggered();
  void on_actionSave_triggered();
  void on_horizontalSliderZoom_valueChanged(int value);
  void on_actionZoomIn_triggered();
  void on_actionZoomOut_triggered();
  void on_actionNew_triggered();
  void on_spinBoxWidth_editingFinished();
  void on_spinBoxHeight_editingFinished();
  void on_actionResize_triggered();
  void on_actionSaveAs_triggered();
};
