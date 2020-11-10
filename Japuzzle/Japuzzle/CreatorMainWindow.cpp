#include <QMessageBox>
#include <QCloseEvent>
#include <QFileDialog>

#include "CreatorMainWindow.h"
#include "ui_CreatorMainWindow.h"
#include "Puzzle.h"
#include "Editing.h"
#include "Decoration.h"
#include "Account.h"
#include "Core.h"


CreatorMainWindow::CreatorMainWindow(QWidget* parent)
  : QMainWindow(parent), ui(new Ui::CreatorMainWindow)
  , mPuzzle(new Puzzle()), mEditing(new Editing()), mDecoration(new Decoration())
  , mFileDialog(new QFileDialog(this)), mBrickSize(1), mBrickSizeLast(5)
{
  ui->setupUi(this);

  SetBrickSizeAction(1, true);
  CreateNew();

  ui->centralwidget->addAction(ui->actionUndo);
  ui->centralwidget->addAction(ui->actionRedo);
  {
    QAction* separator = new QAction(this);
    separator->setSeparator(true);
    ui->centralwidget->addAction(separator);
  }
  ui->centralwidget->addAction(ui->actionBrick1);
  ui->centralwidget->addAction(ui->actionBrick5);
  ui->centralwidget->addAction(ui->actionBrick9);
  {
    QAction* separator = new QAction(this);
    separator->setSeparator(true);
    ui->centralwidget->addAction(separator);
  }
  ui->centralwidget->addAction(ui->actionZoomIn);
  ui->centralwidget->addAction(ui->actionZoomOut);
  ui->centralwidget->setContextMenuPolicy(Qt::ActionsContextMenu);

  ui->toolButtonZoomIn->setDefaultAction(ui->actionZoomIn);
  ui->toolButtonZoomOut->setDefaultAction(ui->actionZoomOut);
  ui->statusbar->addPermanentWidget(ui->widgetZoom);

  ui->toolButtonResizeOk->setDefaultAction(ui->actionResize);
  ui->toolBar->insertWidget(ui->actionUndo, ui->widgetResize);
  {
    QAction* separator = new QAction(this);
    separator->setSeparator(true);
    ui->toolBar->insertAction(ui->actionUndo, separator);
  }

  mFileDialog->setNameFilters(QStringList()
                              << qCore->getFileName() + " (*.ypp)"
                              << qCore->getFileNameDgt() + " (*.dgy)");
  PlaceWidgets();

  connect(mEditing.data(), &Editing::HasUndoChanged, this, &CreatorMainWindow::OnUpdateHasUndo);
  connect(mEditing.data(), &Editing::HasRedoChanged, this, &CreatorMainWindow::OnUpdateHasRedo);

  connect(mDecoration.data(), &Decoration::ZoomChanged, this, &CreatorMainWindow::OnZoomChanged);
}

CreatorMainWindow::~CreatorMainWindow()
{
  delete ui;
}


void CreatorMainWindow::moveEvent(QMoveEvent* event)
{
  Q_UNUSED(event);

  emit WindowChanged();
}

void CreatorMainWindow::resizeEvent(QResizeEvent* event)
{
  Q_UNUSED(event);

  emit WindowChanged();
}

void CreatorMainWindow::closeEvent(QCloseEvent* event)
{
  event->ignore();

  on_actionExit_triggered();
}

bool CreatorMainWindow::ConfirmQuit()
{
  return SaveCurrent();
}

void CreatorMainWindow::CreateNew()
{
  const int kDefaultWidth  = 30;
  const int kDefaultHeight = 30;
  mPuzzle->New(kDefaultWidth, kDefaultHeight);
  ui->spinBoxWidth->setValue(kDefaultWidth);
  ui->spinBoxHeight->setValue(kDefaultHeight);
  mFilename.clear();

  ApplyNewPuzzle();
}

bool CreatorMainWindow::SaveCurrent()
{
  if (ui->editWidget->HasChanges() && !mPuzzle->IsBlank()) {
    auto ret = QMessageBox::information(this, windowTitle(), "Рисунок изменён, сохранить его?", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    if (ret == QMessageBox::No) {
      return true;
    } else if (ret == QMessageBox::Cancel) {
      return false;
    } else {
      return Save();
    }
  }
  return true;
}

bool CreatorMainWindow::Save()
{
  if (!mFilename.isEmpty()) {
    return SaveAs(mFilename);
  }

  QString filename;
  if (!QueryFilename(filename)) {
    return false;
  }
  if (SaveAs(filename)) {
    mFilename = filename;
    return true;
  }
  return false;
}

bool CreatorMainWindow::SaveAs(const QString& filename)
{
  return mPuzzle->Save(filename);
}

bool CreatorMainWindow::QueryFilename(QString& filename)
{
  filename = QFileDialog::getSaveFileName(this, "Сохранить файл \"Японский рисунок\""
                                          , qAccount->PuzzleDir(), qCore->getFileName() + " (*.ypp)", 0);
  return !filename.isEmpty();
}

void CreatorMainWindow::SetBrickSizeAction(int size, bool set)
{
  if (set) {
    if (size != mBrickSize) {
      mBrickSizeLast = mBrickSize;
      mBrickSize = size;
    }
  } else {
    qSwap(mBrickSize, mBrickSizeLast);
  }

  ui->actionBrick1->setChecked(mBrickSize == 1);
  ui->actionBrick5->setChecked(mBrickSize == 5);
  ui->actionBrick9->setChecked(mBrickSize == 9);
  ui->editWidget->SetBrickSize(mBrickSize);
}

void CreatorMainWindow::UpdateZoom()
{
  ui->actionZoomIn->setEnabled(mDecoration->HasZoomIn());
  ui->actionZoomOut->setEnabled(mDecoration->HasZoomOut());
  ui->horizontalSliderZoom->setValue(mDecoration->getZoom());
  ui->labelZoom->setText(QString("%1%").arg(mDecoration->getZoom() * 10));
}

void CreatorMainWindow::PlaceWidgets()
{
  int totalWidth  = ui->editWidget->getWidth();
  int totalHeight = ui->editWidget->getHeight();

  ui->scrollAreaWidgetContents->setMinimumSize(totalWidth, totalHeight);
  ui->scrollAreaWidgetContents->setMaximumSize(totalWidth, totalHeight);

  ui->editWidget->resize(ui->editWidget->getWidth(), ui->editWidget->getHeight());
  ui->editWidget->move(0, 0);
}

void CreatorMainWindow::Resize()
{
  int width = ui->spinBoxWidth->value();
  int height = ui->spinBoxHeight->value();
  QRect newRect(0, 0, width, height);
  mPuzzle->Resize(newRect);

  ApplyNewPuzzle();
  ui->editWidget->SetChanges();
}

void CreatorMainWindow::ApplyNewPuzzle()
{
  OnUpdateHasUndo();
  OnUpdateHasRedo();

  ui->editWidget->Setup(mPuzzle, mDecoration);
  mPuzzle->SetEditing(mEditing);
  PlaceWidgets();
}

void CreatorMainWindow::OnUpdateHasUndo()
{
  ui->actionUndo->setEnabled(mEditing->getHasUndo());
}

void CreatorMainWindow::OnUpdateHasRedo()
{
  ui->actionRedo->setEnabled(mEditing->getHasRedo());
}

void CreatorMainWindow::OnZoomChanged()
{
  ui->editWidget->UpdateSize();
  PlaceWidgets();
}

void CreatorMainWindow::on_actionBrick1_triggered(bool checked)
{
  SetBrickSizeAction(1, checked);
}

void CreatorMainWindow::on_actionBrick5_triggered(bool checked)
{
  SetBrickSizeAction(5, checked);
}

void CreatorMainWindow::on_actionBrick9_triggered(bool checked)
{
  SetBrickSizeAction(9, checked);
}

void CreatorMainWindow::on_actionUndo_triggered()
{
  if (mPuzzle->DoUndo()) {
    ui->editWidget->update();
  }
}

void CreatorMainWindow::on_actionRedo_triggered()
{
  if (mPuzzle->DoRedo()) {
    ui->editWidget->update();
  }
}

void CreatorMainWindow::on_actionExit_triggered()
{
  if (!ConfirmQuit()) {
    return;
  }

  hide();
}

void CreatorMainWindow::on_actionOpen_triggered()
{
  if (mFilename.isEmpty()) {
    mFileDialog->setDirectory(qAccount->PuzzleDir());
  }
  mFileDialog->setAcceptMode(QFileDialog::AcceptOpen);
  mFileDialog->setFileMode(QFileDialog::ExistingFile);
  if (!mFileDialog->exec() || mFileDialog->selectedFiles().size() != 1) {
    return;
  }

  QString filePath = mFileDialog->selectedFiles().first();
  if (!mPuzzle->Load(filePath, false)) {
    QMessageBox::warning(this, windowTitle(), QString("%1 не открывается").arg(filePath));
    return;
  }

  mPuzzle->Reset();
  mFilename = filePath;
  mFileDialog->setDirectory(QFileInfo(mFilename).absoluteDir());
  ApplyNewPuzzle();
}

void CreatorMainWindow::on_actionSave_triggered()
{
  Save();
}

void CreatorMainWindow::on_horizontalSliderZoom_valueChanged(int value)
{
  mDecoration->ZoomChange(value);

  UpdateZoom();
}

void CreatorMainWindow::on_actionZoomIn_triggered()
{
  mDecoration->ZoomIn();

  UpdateZoom();
}

void CreatorMainWindow::on_actionZoomOut_triggered()
{
  mDecoration->ZoomOut();

  UpdateZoom();
}

void CreatorMainWindow::on_actionNew_triggered()
{
  SaveCurrent();
  CreateNew();
}

void CreatorMainWindow::on_spinBoxWidth_editingFinished()
{
  ui->actionResize->setEnabled(true);
}

void CreatorMainWindow::on_spinBoxHeight_editingFinished()
{
  ui->actionResize->setEnabled(true);
}

void CreatorMainWindow::on_actionResize_triggered()
{
  Resize();
}

void CreatorMainWindow::on_actionSaveAs_triggered()
{
  QString filename;
  if (!QueryFilename(filename)) {
    return;
  }
  if (SaveAs(filename)) {
    mFilename = filename;
  }
}
