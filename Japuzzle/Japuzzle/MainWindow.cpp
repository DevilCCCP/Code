#include <QTimer>
#include <QtConcurrentRun>
#include <QDesktopWidget>
#include <QScrollBar>
#include <QMessageBox>

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "CreatorMainWindow.h"
#include "Core.h"
#include "Ai.h"
#include "UiInformer.h"
#include "Puzzle.h"
#include "Account.h"
#include "AccountInfo.h"
#include "Style.h"
#include "Decoration.h"
#include "Editing.h"
#include "GameState.h"
#include "FormCalcPopup.h"
#include "DialogAccount.h"
#include "DialogList.h"
#include "DialogSettings.h"
#include "DialogGameState.h"
#include "DialogSolve.h"
#include "DialogAbout.h"


MainWindow::MainWindow(QWidget* parent)
  : MainWindow2(parent), ui(new Ui::MainWindow)
  , mCore(new Core()), mDecoration(new Decoration()), mEditing(new Editing()), mGameState(new GameState())
  , mFormCalcPopup(new FormCalcPopup()), mFileDialog(new QFileDialog(this)), mAutoSaveTimer(new QTimer(this))
  , mNormalState(Qt::WindowMaximized), mCentralY(-1), mPreviewMove(0), mLoadDone(false), mClosing(false)
  , mSolveDialog(new DialogSolve(this)), mLastSolveFail(false)
  , mStarYes(":/Icons/Star Yellow.png"), mStarNo(":/Icons/Star Gray.png")
{
  ui->setupUi(this);

  ui->toolButtonZoomIn->setDefaultAction(ui->actionZoomIn);
  ui->toolButtonZoomOut->setDefaultAction(ui->actionZoomOut);
  ui->toolButtonFullScreen->setDefaultAction(ui->actionFullScreen);
  ui->statusbar->addPermanentWidget(ui->widgetState);
  ui->statusbar->addPermanentWidget(ui->widgetZoom);

  mUiInformer.reset(new UiInformer(this));
  qCore->SetInformer(mUiInformer.data());
  setWindowTitle(qCore->getProgramName());
  if (!RegisterSaveWidgetA(ui->dockWidgetPic, &DockWidget2::OnWindowChanged)) {
    ui->dockWidgetPic->restoreGeometry(QByteArray::fromBase64("AdnQywACAAAAAAAFAAAAUAAAACoAAACRAAAADQAAAGoAAAAiAAAAiQAAAAAAAAAAB4A="));
  }
  mCreatorMainWindow.reset(new CreatorMainWindow(this));
  if (!RegisterSaveWidgetA(mCreatorMainWindow.data(), &CreatorMainWindow::WindowChanged)) {
    mCreatorMainWindow->resize(800, 600);
  }
  if (!Restore()) {
    this->restoreState(QByteArray::fromBase64("AAAA/wAAAAD9AAAAAgAAAAEAAAAAAAAAAPwCAAAAAfsAAAAUAGQAbwBjAGsAVwBpAGQAZwBlAHQDAAADhgAAAiAAAADIAAAAZAAAAAIAAAeAAAAAufwBAAAAAfsAAAAaAGQAbwBjAGsAVwBpAGQAZwBlAHQAUABpAGMDAAAABQAAAFAAAAAWAAAAIAAAB4AAAAQgAAAABAAAAAQAAAAIAAAACPwAAAABAAAAAgAAAAEAAAAOAHQAbwBvAGwAQgBhAHIBAAAAAP////8AAAAAAAAAAA=="));
    QDesktopWidget* desktop = qApp->desktop();
    int primaryScreen = desktop->primaryScreen();
    QRect geometry = desktop->availableGeometry(primaryScreen);
    move(geometry.topLeft());
    resize(geometry.size() - (frameSize() - size()));
    setWindowState(windowState() | Qt::WindowMaximized);
  }
  ui->actionFullScreen->setChecked(windowState() & Qt::WindowFullScreen);
  ui->dockWidgetPic->resize(80, 80);

  mFileDialog->setNameFilters(QStringList()
                              << qCore->getFileName() + " (*.ypp)"
                              << qCore->getFileNameDgt() + " (*.dgy)");
  ui->toolBar->setEnabled(false);
  mAllWidgets.append(ui->tableWidget);
  mAllWidgets.append(ui->digitsWidgetLeft);
  mAllWidgets.append(ui->digitsWidgetRight);
  mAllWidgets.append(ui->digitsWidgetTop);
  mAllWidgets.append(ui->digitsWidgetBottom);
  mSideWidgets.append(ui->digitsWidgetLeft);
  mSideWidgets.append(ui->digitsWidgetRight);
  mSideWidgets.append(ui->digitsWidgetTop);
  mSideWidgets.append(ui->digitsWidgetBottom);

  ui->digitsWidgetLeft->Init(Qt::Horizontal, Qt::AlignRight);
  ui->digitsWidgetRight->Init(Qt::Horizontal, Qt::AlignLeft);
  ui->digitsWidgetTop->Init(Qt::Vertical, Qt::AlignBottom);
  ui->digitsWidgetBottom->Init(Qt::Vertical, Qt::AlignTop);

  QTimer::singleShot(500, this, &MainWindow::OnInit);

  InitSolveAi();
  InitStarsAi();
  ShowStars();
  OnUpdateHasUndo();
  OnUpdateHasRedo();

  connect(mAutoSaveTimer, &QTimer::timeout, this, &MainWindow::OnAutoSaveTimeout);
  connect(ui->tableWidget, &TableWidget::UpdateDigits, this, &MainWindow::OnUpdateDigits);
  connect(ui->tableWidget, &TableWidget::UpdatePreview, ui->previewWidget, &PreviewWidget::UpdateFull);
  connect(ui->tableWidget, &TableWidget::ShowCalcWindow, mFormCalcPopup, &FormCalcPopup::OnShow);
  connect(ui->tableWidget, &TableWidget::HideCalcWindow, mFormCalcPopup, &FormCalcPopup::OnHide);
  if (auto scroll = ui->scrollAreaMain->horizontalScrollBar()) {
    connect(scroll, &QScrollBar::valueChanged, this, &MainWindow::OnHScrollChanged);
    connect(scroll, &QScrollBar::rangeChanged, this, &MainWindow::OnHScrollRangeChanged);
  }
  if (auto scroll = ui->scrollAreaMain->verticalScrollBar()) {
    connect(scroll, &QScrollBar::valueChanged, this, &MainWindow::OnVScrollChanged);
    connect(scroll, &QScrollBar::rangeChanged, this, &MainWindow::OnVScrollRangeChanged);
  }
  connect(mCore.data(), &Core::PuzzleChanged, this, &MainWindow::OnPuzzleChanged);
  connect(qDecoration, &Decoration::ZoomChanged, this, &MainWindow::OnZoomChanged);
  connect(qDecoration, &Decoration::CursorChanged, this, &MainWindow::OnSetCursor);
  foreach (DigitsWidget* w, mSideWidgets) {
    connect(qDecoration, &Decoration::HighlightPosChanged, w, &DigitsWidget::UpdateHighlight);
    connect(w, &DigitsWidget::ChangedDigits, this, &MainWindow::OnChangedDigits);
  }
  foreach (QWidgetB* w, mAllWidgets) {
    w->setVisible(false);
  }

  connect(qEditing, &Editing::ModeChanged, this, &MainWindow::OnSetCursor);
  connect(qEditing, &Editing::HasUndoChanged, this, &MainWindow::OnUpdateHasUndo);
  connect(qEditing, &Editing::HasRedoChanged, this, &MainWindow::OnUpdateHasRedo);
  connect(qGameState, &GameState::StateChanged, this, &MainWindow::OnGameStateChanged);
  connect(qGameState, &GameState::SolvedChanged, this, &MainWindow::OnGameSoledChanged);
  connect(this, &MainWindow::PostLoginTest, this, &MainWindow::OnLoadPuzzleTest, Qt::QueuedConnection);

  connect(ui->previewWidget, &PreviewWidget::MoveToLocation, this, &MainWindow::OnMoveToLocation);
}

MainWindow::~MainWindow()
{
  mFormCalcPopup->deleteLater();
  qCore->SetInformer(nullptr);
  delete ui;
}


void MainWindow::showEvent(QShowEvent* event)
{
  mCentralY = -1;

  MainWindow2::showEvent(event);
}

void MainWindow::moveEvent(QMoveEvent* event)
{
  if (mCentralY == -1) {
    mCentralY = ui->centralwidget->mapToGlobal(ui->centralwidget->pos()).y();
  } else if (mCentralY != ui->centralwidget->mapToGlobal(ui->centralwidget->pos()).y()) {
    int move = ui->centralwidget->mapToGlobal(ui->centralwidget->pos()).y() - mCentralY;
    ui->dockWidgetPic->move(ui->dockWidgetPic->x(), ui->dockWidgetPic->y() + move);
    mPreviewMove += move;
    mCentralY = ui->centralwidget->mapToGlobal(ui->centralwidget->pos()).y();
  }

  MainWindow2::moveEvent(event);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
  if (mCreatorMainWindow->isVisible() && !mCreatorMainWindow->ConfirmQuit()) {
    event->ignore();
    return;
  }

  mClosing = true;
  mAutoSaveTimer->stop();

  if (windowState().testFlag(Qt::WindowFullScreen)) {
    ui->dockWidgetPic->move(ui->dockWidgetPic->x(), ui->dockWidgetPic->y() - mPreviewMove);
  }
  if (qAccount) {
    if (!qAccount->getName().isEmpty()) {
      qCore->CloseAccount();
    }
  }

  MainWindow2::closeEvent(event);
}

void MainWindow::Info(const QString& text)
{
  ui->statusbar->showMessage(text, 5000);
}

void MainWindow::Warning(const QString& text)
{
  QMessageBox::warning(this, qCore->getProgramName(), text);
}

void MainWindow::Error(const QString& text)
{
  QMessageBox::warning(this, qCore->getProgramName(), text);
}

void MainWindow::InitSolveAi()
{
  mSolveAi.reset(new Ai());
  mSolvePuzzle.reset(new Puzzle());
  mSolveWatcher = new QFutureWatcher<void>(this);

  connect(mSolveWatcher, &QFutureWatcher<void>::started, this, &MainWindow::OnSolveStarted);
  connect(mSolveWatcher, &QFutureWatcher<void>::finished, this, &MainWindow::OnSolveFinished);
  connect(mSolveAi.data(), &Ai::SolveDone, this, &MainWindow::OnSolveDone);
  connect(mSolveAi.data(), &Ai::SolveInfo, this, &MainWindow::OnSolveInfo);

  connect(mSolveDialog, &DialogSolve::StartSolve, this, &MainWindow::StartSolve);
  connect(mSolveDialog, &DialogSolve::StopSolve, mSolveAi.data(), &Ai::StopSolve);
  connect(mSolveAi.data(), &Ai::SolveChanged, mSolveDialog, &DialogSolve::OnSolveChanged);
}

void MainWindow::InitStarsAi()
{
  mStarsAi.reset(new Ai());
  mStarsPuzzle.reset(new Puzzle());
  mStarsWatcher = new QFutureWatcher<void>(this);

  connect(mStarsWatcher, &QFutureWatcher<void>::started, this, &MainWindow::OnStarsStarted);
  connect(mStarsWatcher, &QFutureWatcher<void>::finished, this, &MainWindow::OnStarsFinished);
  mStarsInProgress = false;
  mLoadNextPuzzle = false;
}

void MainWindow::LoadPuzzle()
{
  if (mStarsInProgress) {
    mLoadNextPuzzle = true;
    return;
  }

  mLoadDone = false;
  DrawBack();
  UpdateTitle();
  Draw();
  if (qPuzzle) {
    int mode = qPuzzle->GetAutoPropLevel();
    mEditing->ModeChange((Editing::EMode)mode);
    SetMode(mode, true);

    if (qAccount->getAutoCalcStars() && qPuzzle->Stars() == 0) {
      CalcPuzzleStars();
    }
  }
  mLoadDone = true;
  UpdateSettings();

  if (qGameState->getState() >= GameState::eUnknownSolve) {
    emit PostLoginTest();
  }

  ShowStars();
}

void MainWindow::UpdateSettings()
{
  qAi->CalcAllDigits(qPuzzle.data());

  if (qAccount->getAutoSavePeriod()) {
    mAutoSaveTimer->start(qAccount->getAutoSavePeriod());
  } else {
    mAutoSaveTimer->stop();
  }
}

void MainWindow::DrawBack()
{
  if (!qStyle) {
    return;
  }

  QPalette palette(ui->scrollAreaWidgetContents->palette());
  palette.setColor(QPalette::Background, qStyle->getBackColor());
  ui->scrollAreaWidgetContents->setAutoFillBackground(true);
  ui->scrollAreaWidgetContents->setPalette(palette);

  foreach (QWidgetB* widget, mAllWidgets) {
    widget->SetFillStyle(QWidgetB::eCopy);
    widget->SetBackImage(qStyle->getBackground());
  }
  mFormCalcPopup->SetBackImage(qStyle->getBackground());
}

void MainWindow::Draw()
{
  bool visible = qPuzzle && qStyle;
  foreach (QWidgetB* widget, mAllWidgets) {
    widget->setVisible(visible);
  }

  ui->previewWidget->Setup();
//  ui->dockWidgetPic->setMinimumSize(ui->previewWidget->minimumSize());
//  ui->dockWidgetPic->resize(ui->previewWidget->DefaultSize());

  ui->tableWidget->Setup();
  ui->tableWidget->update();
  foreach (DigitsWidget* widget, mSideWidgets) {
    widget->Setup();
    widget->update();
  }
  mFormCalcPopup->Setup();
  ui->centralwidget->setPalette(QPalette(qStyle->getBackColor()));

  PlaceWidgets();
}

void MainWindow::ShowStars()
{
  int stars = qPuzzle? qPuzzle->Stars(): 0;
  ui->toolButtonCalcStars->setVisible(stars == 0);
  ui->labelStar1->setPixmap(stars >= 1? mStarYes: mStarNo);
  ui->labelStar2->setPixmap(stars >= 2? mStarYes: mStarNo);
  ui->labelStar3->setPixmap(stars >= 3? mStarYes: mStarNo);
  ui->labelStar4->setPixmap(stars >= 4? mStarYes: mStarNo);
  ui->labelStar5->setPixmap(stars >= 5? mStarYes: mStarNo);
}

void MainWindow::CalcVisibleRect()
{
  int y1 = 0;
  if (ui->digitsWidgetTop->height() > ui->tableWidget->y()) {
    y1 = (ui->digitsWidgetTop->height() - ui->tableWidget->y() + qDecoration->getCellHeight() - 1) / qDecoration->getCellHeight();
  } else {
    y1 = qMax(0, (ui->scrollAreaMain->verticalScrollBar()->value() - ui->tableWidget->y() + qDecoration->getCellHeight() - 1) / qDecoration->getCellHeight());
  }

  int y2 = qPuzzle->getHeight() - 1;
  if (ui->digitsWidgetBottom->y() < ui->tableWidget->y() + ui->tableWidget->height()) {
    y2 = (qPuzzle->getHeight() - 1) - (ui->tableWidget->y() + ui->tableWidget->height() - ui->digitsWidgetBottom->y() + qDecoration->getCellHeight() - 1) / qDecoration->getCellHeight();
  } else {
    int visibleHeight = ui->scrollAreaWidgetContents->height() - ui->scrollAreaMain->verticalScrollBar()->maximum();
    y2 = qBound(0, (ui->scrollAreaMain->verticalScrollBar()->value() + visibleHeight - ui->tableWidget->y()) / qDecoration->getCellHeight(), qPuzzle->getHeight() - 1);
  }

  int x1 = 0;
  if (ui->digitsWidgetLeft->width() > ui->tableWidget->x()) {
    x1 = (ui->digitsWidgetLeft->width() - ui->tableWidget->x() + qDecoration->getCellWidth() - 1) / qDecoration->getCellWidth();
  } else {
    x1 = qMax(0, (ui->scrollAreaMain->horizontalScrollBar()->value() - ui->tableWidget->x() + qDecoration->getCellWidth() - 1) / qDecoration->getCellWidth());
  }

  int x2 = qPuzzle->getWidth() - 1;
  if (ui->digitsWidgetRight->x() < ui->tableWidget->x() + ui->tableWidget->width()) {
    x2 = (qPuzzle->getWidth() - 1) - (ui->tableWidget->x() + ui->tableWidget->width() - ui->digitsWidgetRight->x() + qDecoration->getCellWidth() - 1) / qDecoration->getCellWidth();
  } else {
    int visibleWidth = ui->scrollAreaWidgetContents->width() - ui->scrollAreaMain->horizontalScrollBar()->maximum();
    x2 = qBound(0, (ui->scrollAreaMain->horizontalScrollBar()->value() + visibleWidth - ui->tableWidget->x()) / qDecoration->getCellWidth(), qPuzzle->getWidth() - 1);
  }

  ui->previewWidget->SetLocation(QRect(QPoint(x1, y1), QPoint(x2 - 1, y2 - 1)));
}

void MainWindow::Update()
{
  ui->tableWidget->update();
  OnUpdateDigits(QPoint(0, 0), QPoint(qPuzzle->getWidth() - 1, qPuzzle->getHeight() - 1));
  ui->previewWidget->UpdateFull();
}

void MainWindow::UpdateTitle()
{
  if (qAccount && !qAccount->getViewName().isEmpty()) {
    if (qPuzzle && !qPuzzle->getViewName().isEmpty()) {
      setWindowTitle(qCore->getProgramName() + QString(" [%1] - %2").arg(qAccount->getViewName(), qPuzzle->getViewName()));
    } else {
      setWindowTitle(qCore->getProgramName() + QString(" [%1]").arg(qAccount->getViewName()));
    }
  } else {
    setWindowTitle(qCore->getProgramName());
  }
}

void MainWindow::UpdateZoom()
{
  ui->actionZoomIn->setEnabled(qDecoration->HasZoomIn());
  ui->actionZoomOut->setEnabled(qDecoration->HasZoomOut());
  ui->horizontalSliderZoom->setValue(qDecoration->getZoom());
  ui->labelZoom->setText(QString("%1%").arg(qDecoration->getZoom() * 10));
}

void MainWindow::PlaceWidgets()
{
  int totalWidth  = ui->digitsWidgetLeft->getWidth() + ui->tableWidget->getWidth() + ui->digitsWidgetRight->getWidth();
  int totalHeight = ui->digitsWidgetTop->getHeight() + ui->tableWidget->getHeight() + ui->digitsWidgetBottom->getHeight();

  ui->scrollAreaWidgetContents->setMinimumSize(totalWidth, totalHeight);
  ui->scrollAreaWidgetContents->setMaximumSize(totalWidth, totalHeight);

  ui->tableWidget->resize(ui->tableWidget->getWidth(), ui->tableWidget->getHeight());
  ui->tableWidget->move(ui->digitsWidgetLeft->getWidth(), ui->digitsWidgetTop->getHeight());
  ui->tableWidget->lower();

  OnVScrollChanged(ui->scrollAreaMain->verticalScrollBar()->value());
  OnHScrollChanged(ui->scrollAreaMain->horizontalScrollBar()->value());
}

void MainWindow::SetMode(int mode, bool checked)
{
  if (checked) {
    ui->actionProp1->setChecked(mode == 1);
    ui->actionProp2->setChecked(mode == 2);
    ui->actionProp3->setChecked(mode == 3);
    ui->actionErase->setChecked(mode == 4);
    qEditing->setCurrentPropLevel(mode >= 4? 0: mode);
    switch (mode) {
    case 1: qEditing->ModeChange(Editing::eModeProp1); break;
    case 2: qEditing->ModeChange(Editing::eModeProp2); break;
    case 3: qEditing->ModeChange(Editing::eModeProp3); break;
    case 4: qEditing->ModeChange(Editing::eModeErase); break;
    }
  } else {
    qEditing->ModeChange(Editing::eModeNormal);
    qEditing->setCurrentPropLevel(0);
  }
}

void MainWindow::CalcPuzzleStars()
{
  mStarsInProgress = true;
  qCore->Info(QString("Вычисляется сложность рисунка"));

  mStarsSourcePuzzle = qPuzzle;
  mStarsSourcePuzzle->Reset();
  mStarsPuzzle->Copy(*mStarsSourcePuzzle);
  mStarsFilename = qPuzzle->getSourceName();
  auto feature = QtConcurrent::run(mStarsAi.data(), &Ai::Stars, mStarsPuzzle.data());
  mStarsWatcher->setFuture(feature);
}

void MainWindow::OnInit()
{
  ui->toolBar->setEnabled(true);

  on_actionLogin_triggered();
}

void MainWindow::OnAutoSaveTimeout()
{
  if (!mClosing) {
    qCore->AutoSavePuzzle();
  }
}

void MainWindow::OnChangedDigits(int type, const QPoint& p1, const QPoint& p2)
{
//  qPuzzle->MakeUndo();
  if (type == Qt::Horizontal) {
    ui->digitsWidgetLeft->UpdateDigits(p1, p2);
    ui->digitsWidgetRight->UpdateDigits(p1, p2);
  } else {
    ui->digitsWidgetTop->UpdateDigits(p1, p2);
    ui->digitsWidgetBottom->UpdateDigits(p1, p2);
  }
}

void MainWindow::OnUpdateDigits(const QPoint& p1, const QPoint& p2)
{
  if (qAccount->getDigitStyle() == Account::eDigitManual) {
    return;
  }

  int x1 = qMin(p1.x(), p2.x());
  int x2 = qMax(p1.x(), p2.x());
  int y1 = qMin(p1.y(), p2.y());
  int y2 = qMax(p1.y(), p2.y());

  for (int j = y1; j <= y2; j++) {
    qAi->CalcHorzDigits(qPuzzle.data(), j);
  }
  for (int i = x1; i <= x2; i++) {
    qAi->CalcVertDigits(qPuzzle.data(), i);
  }

  foreach (DigitsWidget* w, mSideWidgets) {
    w->UpdateDigits(p1, p2);
  }
}

void MainWindow::OnHScrollChanged(int value)
{
  if (qAccount->getCompactDigits()) {
    int max = ui->scrollAreaMain->horizontalScrollBar()->maximum();
    ui->digitsWidgetLeft->SetViewCut(value);
    ui->digitsWidgetRight->SetViewCut(max - value);
    if (value <= max/2) {
      int expand = ui->digitsWidgetLeft->ExpandToMinView();
      ui->digitsWidgetLeft->resize(ui->digitsWidgetLeft->getWidth() + expand, ui->digitsWidgetLeft->getHeight());
      ui->digitsWidgetRight->resize(ui->digitsWidgetRight->getWidth(), ui->digitsWidgetRight->getHeight());
    } else {
      int expand = ui->digitsWidgetRight->ExpandToMinView();
      ui->digitsWidgetLeft->resize(ui->digitsWidgetLeft->getWidth(), ui->digitsWidgetLeft->getHeight());
      ui->digitsWidgetRight->resize(ui->digitsWidgetRight->getWidth() + expand, ui->digitsWidgetRight->getHeight());
    }
  } else {
    ui->digitsWidgetLeft->resize(ui->digitsWidgetLeft->getWidth(), ui->digitsWidgetLeft->getHeight());
    ui->digitsWidgetRight->resize(ui->digitsWidgetRight->getWidth(), ui->digitsWidgetRight->getHeight());
  }
  ui->digitsWidgetLeft->move(0, ui->digitsWidgetTop->getHeight());
  ui->digitsWidgetRight->move(ui->digitsWidgetLeft->getWidth() + ui->tableWidget->getWidth() + ui->digitsWidgetRight->getWidth() - ui->digitsWidgetRight->width(), ui->digitsWidgetTop->getHeight());

  CalcVisibleRect();
}

void MainWindow::OnVScrollChanged(int value)
{
  if (qAccount->getCompactDigits()) {
    int max = ui->scrollAreaMain->verticalScrollBar()->maximum();
    ui->digitsWidgetTop->SetViewCut(value);
    ui->digitsWidgetBottom->SetViewCut(max - value);
    if (value <= max/2) {
      int expand = ui->digitsWidgetTop->ExpandToMinView();
      ui->digitsWidgetTop->resize(ui->digitsWidgetTop->getWidth(), ui->digitsWidgetTop->getHeight() + expand);
      ui->digitsWidgetBottom->resize(ui->digitsWidgetBottom->getWidth(), ui->digitsWidgetBottom->getHeight());
    } else {
      int expand = ui->digitsWidgetBottom->ExpandToMinView();
      ui->digitsWidgetTop->resize(ui->digitsWidgetTop->getWidth(), ui->digitsWidgetTop->getHeight());
      ui->digitsWidgetBottom->resize(ui->digitsWidgetBottom->getWidth(), ui->digitsWidgetBottom->getHeight() + expand);
    }
  } else {
    ui->digitsWidgetTop->resize(ui->digitsWidgetTop->getWidth(), ui->digitsWidgetTop->getHeight());
    ui->digitsWidgetBottom->resize(ui->digitsWidgetBottom->getWidth(), ui->digitsWidgetBottom->getHeight());
  }
  ui->digitsWidgetTop->move(ui->digitsWidgetLeft->getWidth(), 0);
  ui->digitsWidgetBottom->move(ui->digitsWidgetLeft->getWidth(), ui->digitsWidgetTop->getHeight() + ui->tableWidget->getHeight() + ui->digitsWidgetBottom->getHeight() - ui->digitsWidgetBottom->height());

  CalcVisibleRect();
}

void MainWindow::OnHScrollRangeChanged(int min, int max)
{
  Q_UNUSED(min);
  Q_UNUSED(max);

  OnHScrollChanged(ui->scrollAreaMain->horizontalScrollBar()->value());
}

void MainWindow::OnVScrollRangeChanged(int min, int max)
{
  Q_UNUSED(min);
  Q_UNUSED(max);

  OnVScrollChanged(ui->scrollAreaMain->verticalScrollBar()->value());
}

void MainWindow::OnZoomChanged()
{
  Draw();
}

void MainWindow::OnSetCursor()
{
  if (!qStyle) {
    return;
  }

  switch (qDecoration->getCursor()) {
  case Decoration::eCursorUndefined: break;
  case Decoration::eCursorNormalCell:
    setCursor(qEditing->getMode() == Editing::eModeErase? qStyle->getCursorErase(): qStyle->getCursorArrow());
    break;
  case Decoration::eCursorArrow:     setCursor(qStyle->getCursorArrow()); break;
  case Decoration::eCursorYesCell:   setCursor(qStyle->getCursorYes().at(0)); break;
  case Decoration::eCursorYesVLine:  setCursor(qStyle->getCursorYes().at(1)); break;
  case Decoration::eCursorYesHLine:  setCursor(qStyle->getCursorYes().at(2)); break;
  case Decoration::eCursorYesBlock:  setCursor(qStyle->getCursorYes().at(3)); break;
  case Decoration::eCursorNoCell:    setCursor(qStyle->getCursorNo().at(0)); break;
  case Decoration::eCursorNoVLine:   setCursor(qStyle->getCursorNo().at(1)); break;
  case Decoration::eCursorNoHLine:   setCursor(qStyle->getCursorNo().at(2)); break;
  case Decoration::eCursorNoBlock:   setCursor(qStyle->getCursorNo().at(3)); break;
  }
}

void MainWindow::OnUpdateHasUndo()
{
  ui->actionUndo->setEnabled(qEditing->getHasUndo());
}

void MainWindow::OnUpdateHasRedo()
{
  ui->actionRedo->setEnabled(qEditing->getHasRedo());
}

void MainWindow::OnLoadPuzzleTest()
{
  OnGameStateChanged();
  if (qGameState->IsStateSolved()) {
    OnGameSoledChanged();
  }
}

void MainWindow::OnGameStateChanged()
{
  ui->toolButtonState->setIcon(qGameState->SmallIcon());
}

void MainWindow::OnGameSoledChanged()
{
  mSolveDialog->hide();

  if (qAccount->getShowGameStateDialog()) {
    on_toolButtonState_clicked();
  }
}

void MainWindow::StartSolve(int maxProp)
{
  auto feature = QtConcurrent::run(mSolveAi.data(), &Ai::Solve, mSolvePuzzle.data(), qEditing->getCurrentPropLevel(), maxProp);
  mSolveWatcher->setFuture(feature);
}

void MainWindow::OnSolveInfo(QByteArray data)
{
  qPuzzle->FromByteArray(data);
  Update();
}

void MainWindow::OnSolveDone(int result, int prop)
{
  if (result > 0) {
    qCore->Info(QString("Сборка закончила работу (предположения: %1)").arg(prop));
  } else if (result == 0) {
    if (prop > 0) {
      qCore->Info(QString("Сборка не помогла, возможно решение не однозначно (предположения: %1)").arg(prop));
    } else {
      qCore->Info("Сборка не помогла, используйте предположения");
    }
    if (qAccount->getAutoOpenPropEx()) {
      on_actionSolveEx_triggered();
    }
  } else {
    qCore->Info("В картинке найдены ошибки");
  }
}

void MainWindow::OnSolveStarted()
{
  foreach (QWidgetB* w, mAllWidgets) {
    w->setEnabled(false);
  }
}

void MainWindow::OnSolveFinished()
{
  foreach (QWidgetB* w, mAllWidgets) {
    w->setEnabled(true);
  }

  qPuzzle->Apply(*mSolvePuzzle);
  qPuzzle->ClearPropMark();
  Update();

  if (qPuzzle->SolveTest(true)) {
    mSolveDialog->hide();
  }
}

void MainWindow::OnStarsStarted()
{
}

void MainWindow::OnStarsFinished()
{
  qCore->Info(QString("Подсчёт сложности завершён, результат: %1").arg(mStarsPuzzle->StarsText()));
  mStarsInProgress = false;

  if (!mLoadNextPuzzle) {
    mStarsSourcePuzzle->SetStars(mStarsPuzzle->Stars());
    mStarsSourcePuzzle->Save(mStarsFilename);
    qPuzzle->SetStars(mStarsPuzzle->Stars());
    qPuzzle->Save(qAccount->getLastPuzzle());
    ShowStars();
  } else {
    mLoadNextPuzzle = false;
    LoadPuzzle();
  }
}

void MainWindow::OnPuzzleChanged()
{
  mDecoration->SetPuzzle(mCore->getPuzzle());
  mCore->getPuzzle()->SetEditing(mEditing);
}

void MainWindow::OnMoveToLocation(int i, int j)
{
  if (!qPuzzle) {
    return;
  }

  int visibleWidth = ui->scrollAreaWidgetContents->width() - ui->scrollAreaMain->horizontalScrollBar()->maximum();
  int visibleHeight = ui->scrollAreaWidgetContents->height() - ui->scrollAreaMain->verticalScrollBar()->maximum();
  int posX = qBound(0, i * ui->scrollAreaWidgetContents->width() / qPuzzle->getWidth() - visibleWidth/2, ui->scrollAreaMain->horizontalScrollBar()->maximum());
  int posY = qBound(0, j * ui->scrollAreaWidgetContents->height() / qPuzzle->getHeight() - visibleHeight/2, ui->scrollAreaMain->verticalScrollBar()->maximum());
  ui->scrollAreaMain->horizontalScrollBar()->setValue(posX);
  ui->scrollAreaMain->verticalScrollBar()->setValue(posY);
}

void MainWindow::on_actionExit_triggered()
{
  close();
}

void MainWindow::on_actionOpen_triggered()
{
  mFileDialog->setAcceptMode(QFileDialog::AcceptOpen);
  mFileDialog->setFileMode(QFileDialog::ExistingFile);
  if (!mFileDialog->exec() || mFileDialog->selectedFiles().size() != 1) {
    return;
  }

  QString filePath = mFileDialog->selectedFiles().first();
  mFileDialog->setDirectory(QFileInfo(filePath).absoluteDir());
  if (!qCore->LoadPuzzle(filePath)) {
    Warning(QString("%1 не открывается").arg(qCore->getFileName()));
    return;
  }

  LoadPuzzle();
}

void MainWindow::on_actionLogin_triggered()
{
  DialogAccount dialogAccount(this);
  bool ok = true;
  if (dialogAccount.exec()) {
    const AccountInfoS& info = dialogAccount.CurrentAccount();
    bool newAccount = !info->Existed;
    ok = (newAccount)? qCore->CreateAccount(info): qCore->LoadAccount(info);
    if (ok) {
      LoadPuzzle();
      if (newAccount) {
        on_actionSettings_triggered();
      }
    }
  } else if (!qCore->getAccount()) {
    ok = qCore->LoadDefaultAccount();
  }
  if (!ok) {
    Error("Невозможно продолжить");
    close();
    return;
  }
}


void MainWindow::on_actionProp1_triggered(bool checked)
{
  SetMode(1, checked);
}

void MainWindow::on_actionProp2_triggered(bool checked)
{
  SetMode(2, checked);
}

void MainWindow::on_actionProp3_triggered(bool checked)
{
  SetMode(3, checked);
}

void MainWindow::on_actionErase_triggered(bool checked)
{
  SetMode(4, checked);
}

void MainWindow::on_actionPropClear_triggered()
{
  qPuzzle->ClearProp(qEditing->getCurrentPropLevel());
  Update();
}

void MainWindow::on_actionUndo_triggered()
{
  if (qPuzzle->DoUndo()) {
    Update();
  }
}

void MainWindow::on_actionRedo_triggered()
{
  if (qPuzzle->DoRedo()) {
    Update();
  }
}

void MainWindow::on_actionSettings_triggered()
{
  DialogSettings dialogSettings;
  dialogSettings.Load(qAccount.data());
  if (dialogSettings.exec()) {
    dialogSettings.Save(qAccount.data());
    qAccount->Save();
    UpdateSettings();
    DrawBack();
    Draw();
    OnHScrollChanged(ui->scrollAreaMain->horizontalScrollBar()->value());
    OnVScrollChanged(ui->scrollAreaMain->verticalScrollBar()->value());
  }
}

void MainWindow::on_horizontalSliderZoom_valueChanged(int value)
{
  qDecoration->ZoomChange(value);

  UpdateZoom();
}

void MainWindow::on_actionZoomIn_triggered()
{
  qDecoration->ZoomIn();

  UpdateZoom();
}

void MainWindow::on_actionZoomOut_triggered()
{
  qDecoration->ZoomOut();

  UpdateZoom();
}

void MainWindow::on_toolButtonState_clicked()
{
  if (!mLoadDone || !qAccount) {
    return;
  }

  bool solved = qGameState->IsStateSolved();
  if (qAccount->getShowGameStateDialog() || !solved) {
    DialogGameState dialogGameState;
    if (!dialogGameState.exec()) {
      return;
    }
    qCore->LoadNextPuzzle((Account::EPuzzleType)dialogGameState.getSwitchType());
  } else {
    qCore->LoadNextPuzzle(Account::eDone);
  }

  LoadPuzzle();
}

void MainWindow::on_actionClearAll_triggered()
{
  if (qPuzzle) {
    qPuzzle->Clear();

    Draw();
  }
}

void MainWindow::on_actionAbout_triggered()
{
  DialogAbout dialogAbout;
  dialogAbout.exec();
}

void MainWindow::on_actionFullScreen_triggered(bool checked)
{
  if (checked) {
    mNormalState = windowState();
    setWindowState(Qt::WindowFullScreen);
  } else {
    setWindowState(mNormalState);
  }

  Draw();
}

void MainWindow::on_actionPropApply_triggered()
{
  qPuzzle->ApplyProp(qEditing->getCurrentPropLevel());
  Update();
}

void MainWindow::on_actionHint_triggered()
{
  bool hasHint = false;
  if (!qAi->Hint(qPuzzle.data(), qEditing->getCurrentPropLevel(), hasHint)) {
    qCore->Warning("В картинке найдены ошибки");
  } else if (!hasHint) {
    qCore->Info("Подсказка не даёт результата, используйте предположения");
  }

  qPuzzle->SolveTest(true);
  Update();
}

void MainWindow::on_actionTest_triggered()
{
  bool ok = qAi->Test(qPuzzle.data());
  if (ok) {
    qCore->Info("Ошибок в картинке не найдено");
  } else {
    qCore->Warning("В картинке найдены ошибки");
  }
}

void MainWindow::on_actionSolve_triggered()
{
  if (qPuzzle->Count() == qPuzzle->Size()) {
    qCore->Info("Картинка заполнена, сборка не требуется");
    return;
  }

  mSolvePuzzle->Copy(*qPuzzle);
  StartSolve(0);
}

void MainWindow::on_actionSolveEx_triggered()
{
  if (qPuzzle->Count() == qPuzzle->Size()) {
    qCore->Info("Картинка заполнена, сборка не требуется");
    return;
  }

  mSolvePuzzle->Copy(*qPuzzle);
  mSolveDialog->Init(qAccount.data(), mSolvePuzzle.data());
  mSolveDialog->show();
}

void MainWindow::on_actionList_triggered()
{
  if (qAccount) {
    qAccount->UpdatePuzzleList();
  }
  DialogList dialogList;
  dialogList.exec();
  if (dialogList.IsRestarted()) {
    LoadPuzzle();
  }
}

void MainWindow::on_actionCreator_triggered()
{
  mCreatorMainWindow->show();
}

void MainWindow::on_toolButtonCalcStars_clicked()
{
  if (!qPuzzle) {
    return;
  }

  if (mStarsInProgress) {
    Warning("Подсчёт сложности уже запущен");
    return;
  }

  CalcPuzzleStars();
}
