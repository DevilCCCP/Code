#include <QDesktopWidget>
#include <QStandardPaths>
#include <QClipboard>
#include <QScrollBar>
#include <QComboBox>
#include <QDebug>

#include <Lib/CoreUi/Icon.h>
#include <LibA/Analyser/Analyser.h>
#include <LibA/Analyser/ImageStatFtr.h>
#include <LibA/Analyser/SignalMarkFtr.h>

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "ImageLabel.h"
#include "GraphLabel.h"


const QString& GetProgramName();
const int kSignalLengthMax = 6;

MainWindow::MainWindow(QWidget* parent)
  : MainWindow2(parent), ui(new Ui::MainWindow)
  , mClipboard(QApplication::clipboard())
  , mFileDialog(new QFileDialog(this))
  , mAnalyser(new Analyser(true))
  , mMaxDumpIndex(0), mCurrentLineIndex(2), mCurrentDumpIndex(0), mCurrentDumpParam1(0), mCurrentDumpParam2(0), mLineChanged(false)
{
  ui->setupUi(this);

  mDumpCombo = ui->mainToolBar->addWidget(ui->widgetFilterControl);

  mCurrentActions = eTabIllegal;
  ui->mainToolBar->setContextMenuPolicy(Qt::PreventContextMenu);

  mLoadedTabs.fill(false, (int)eTabIllegal);
  mCurrentTab = eTabImage;
  PrepareTab();
  ui->tabWidgetMain->setCurrentIndex(mCurrentTab);
  ui->tabWidgetMain->setTabIcon(eTabGrayscale, GrayIcon(ui->tabWidgetMain->tabIcon(eTabImage)));
  mSelect = FormImageLineView::eLine;
  SwitchSelect(FormImageLineView::eSelect, true);

  ui->mainToolBar->insertWidget(ui->actionNextFile, ui->horizontalSliderFiles);
  ui->tabImage->addAction(ui->actionPaste);
  ui->tabImage->addAction(ui->actionWhiteBallance);
  ui->tabImage->addAction(ui->actionPrevFile);
  ui->tabImage->addAction(ui->actionNextFile);
  ui->tabImage->addAction(ui->actionLineStats);
  ui->tabImage->addAction(ui->actionAreaStats);
  ui->tabImage->addAction(ui->actionFilter);
//  ui->tabImage->addAction(ui->actionFilterMk3Color);
//  ui->tabImage->addAction(ui->actionErosionBlack);
//  ui->tabImage->addAction(ui->actionErosionWhite);
  ui->tabImage->setContextMenuPolicy(Qt::ActionsContextMenu);

  UpdateActions();

  ui->toolButtonDumpFilterPrev->setDefaultAction(ui->actionPrevFilterImage);
  ui->toolButtonDumpFilterNext->setDefaultAction(ui->actionNextFilterImage);

  if (!Restore()) {
    resize(QDesktopWidget().availableGeometry(this).size() * 0.5);
  }
  ui->mainToolBar->setVisible(true);

  mImageSource = GetSettings()->value("ImageFile").toString();
  mCurrentDir = QDir(GetSettings()->value("FilesPath").toString());
  mFileDialog->setDirectory(mCurrentDir);
  mCurrentImage = QImage(mCurrentDir.filePath(mImageSource));
  ui->tabImage->RestoreSettings(GetSettings());
  SetImageAuto();
  OnLineChanged();
  UpdateTab();

  connect(ui->tabImage, &FormImageLineView::LineChanged, this, &MainWindow::OnLineChanged);
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::SetImageAuto()
{
  ApplyDir();

  ApplyImage();
}

bool MainWindow::SetImage(const QImage& img, const QString& name)
{
  if (img.isNull()) {
    return false;
  }

  mCurrentImage = img;
  mImageSource = name;
  ApplyImage();
  return true;
}

void MainWindow::UpdateActions()
{
  bool hasLineActions = false;
  bool hasRectActions = false;
  if (mSelect == FormImageLineView::eLine) {
    hasLineActions = ui->tabImage->LinePoints().size() >= 2;
  } else if (mSelect == FormImageLineView::eRectangle) {
    hasRectActions = ui->tabImage->LinePoints().size() >= 4;
  }

  ui->actionLineStats->setEnabled(hasLineActions);
  ui->actionAreaStats->setEnabled(hasRectActions);
}

void MainWindow::UpdateLineXy()
{
  QVector<QPoint> points = ui->tabImage->LinePoints();
  bool hasPoints = points.size() >= 2;
  ui->spinBoxLineX->setEnabled(hasPoints);
  ui->spinBoxLineY->setEnabled(hasPoints);
  if (hasPoints) {
    QSignalBlocker bx(ui->spinBoxLineX);
    QSignalBlocker by(ui->spinBoxLineY);
    int d = points.at(1).x() - points.at(0).x();
    ui->spinBoxLineX->setMinimum(d >= 0? 0: -d);
    ui->spinBoxLineX->setMaximum(d >= 0? mCurrentImage.width() - 1: mCurrentImage.width() - 1  + d);
    ui->spinBoxLineY->setRange(0, mCurrentImage.height() - 1);
    ui->spinBoxLineX->setValue(points.at(0).x());
    ui->spinBoxLineY->setValue(points.at(0).y());
  }
}

void MainWindow::ApplyDir()
{
  mDirFiles = mCurrentDir.entryList(QDir::Files, QDir::Name);
  mCurrentFile = mDirFiles.indexOf(mImageSource);
  ui->horizontalSliderFiles->setMaximum(mDirFiles.size() - 1);
  ui->horizontalSliderFiles->setValue(mCurrentFile);
}

void MainWindow::ApplyImage()
{
  mLoadedTabs.fill(false, (int)eTabIllegal);
  ui->tabImage->SetBestScale();
  mCurrentValue.clear();
  ui->tabImage->SetImage(mCurrentImage);

  if (mCurrentTab) {
    UpdateTab();
  }
  qInfo() << QString("Analize '%1'").arg(mImageSource);
  setWindowTitle(QString("Analize '%1'").arg(mImageSource));
}

bool MainWindow::PrepareLine()
{
  mLineValues = ui->tabImage->LineValues().toVector();
  if (mLineValues.size() < 2) {
    return false;
  }

  mAnalyser->LineInit(mLineValues.constData(), mLineValues.size());
  return true;
}

bool MainWindow::PrepareData()
{
  if (mCurrentValue.isEmpty()) {
    DataFromImage();
    if (mCurrentValue.isEmpty()) {
      return false;
    }
  }

  ByteRegion sourceRegion(mCurrentValue.data(), mCurrentImage.width(), mCurrentImage.height(), mCurrentImage.width());
  mAnalyser->RegionInit(sourceRegion);
  return true;
}

void MainWindow::PrepareLineActions()
{
  ui->tabStats->setContextMenuPolicy(Qt::ActionsContextMenu);
  auto oldActionsList = ui->tabStats->actions();
  foreach (auto action, oldActionsList) {
    ui->tabStats->removeAction(action);
    action->deleteLater();
  }

  QStringList nameList;
  mMaxDumpIndex = mAnalyser->LineFilterCount();
  for (int i = 0; i < mMaxDumpIndex; i++) {
    QString name = mAnalyser->LineFilterName(i);
    nameList << name;
    QAction* action = new QAction(QIcon("Icons/Stats.png"), name, this);
    action->setData(i);
    ui->tabStats->addAction(action);
    connect(action, &QAction::triggered, this, &MainWindow::OnDumpLineTriggered);
  }

  PrepareDump(nameList);
  ui->comboBoxFilterDump->setCurrentIndex(mCurrentLineIndex);
  connect(ui->comboBoxFilterDump, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged)
          , this, &MainWindow::OnDumpLineChanged);
  OnDumpLineChanged(mCurrentLineIndex);
}

void MainWindow::PrepareFilterActions()
{
  ui->tabFilter->setContextMenuPolicy(Qt::ActionsContextMenu);
  auto oldActionsList = ui->tabFilter->actions();
  foreach (auto action, oldActionsList) {
    ui->tabFilter->removeAction(action);
    action->deleteLater();
  }

  QStringList nameList;
  mMaxDumpIndex = mAnalyser->RegionFilterCount();
  for (int i = 0; i < mMaxDumpIndex; i++) {
    QString name = mAnalyser->RegionFilterName(i);
    nameList << name;
    QAction* action = new QAction(QIcon("Icons/Stats.png"), name, this);
    action->setData(i);
    ui->tabFilter->addAction(action);
    connect(action, &QAction::triggered, this, &MainWindow::OnDumpRegionTriggered);
  }

  PrepareDump(nameList);
  ui->comboBoxFilterDump->setCurrentIndex(mCurrentDumpIndex);
  connect(ui->comboBoxFilterDump, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged)
          , this, &MainWindow::OnDumpRegionChanged);
  OnDumpRegionChanged(mCurrentDumpIndex);
}

void MainWindow::PrepareDump(const QStringList& nameList)
{
  ui->comboBoxFilterDump->disconnect();
  ui->comboBoxFilterDump->clear();
  ui->comboBoxFilterDump->addItems(nameList);
}

void MainWindow::SetGrayscale()
{
  if (!PrepareData()) {
    return;
  }
  ByteRegion sourceRegion(mCurrentValue.data(), mCurrentImage.width(), mCurrentImage.height(), mCurrentImage.width());
  ui->tabGrayscale->SetImage(ImageFromRegion(sourceRegion));
}

void MainWindow::SetDiff()
{
  if (!PrepareData()) {
    return;
  }

  mAnalyser->GetImageStatFtr()->MakeGrad();

  const ByteRegion& markRegion = mAnalyser->Result();
  ui->tabDiff->SetImage(ImageFromRegion(markRegion));
}

void MainWindow::FilterLine()
{
  if (!PrepareLine()) {
    return;
  }

  DumpLine();
}

void MainWindow::FilterRect()
{
  QVector<uchar> rectValues = ui->tabImage->RectValues();
  Hyst hyst;
  foreach (const uchar& ch, rectValues) {
    hyst.Inc(ch);
  }
  ui->hystLabel->SetHyst(hyst);

  mLoadedTabs[eTabRectStats] = true;
}

void MainWindow::FilterImage()
{
  if (!PrepareData()) {
    return;
  }

  DumpRegion();
}

void MainWindow::FilterImageMk3Color()
{
  if (!PrepareData()) {
    return;
  }

//  mAnalyser->MakeHigher(1);

//  ByteRegion debugRegion = sourceRegion;
////  mUinPre->Calc3Color(sourceRegion, debugRegion);

//  ui->tabFilter->SetImage(ImageFromRegion(debugRegion));
  mLoadedTabs[eTabFilter] = true;
}

void MainWindow::FilterFindNumbers()
{
}

void MainWindow::FilterResolveNumbers()
{

}

void MainWindow::FilterErosionBlack()
{
  if (!PrepareData()) {
    return;
  }

  ByteRegion sourceRegion;
  sourceRegion.SetSource(mCurrentValue.data(), mCurrentImage.width(), mCurrentImage.height(), mCurrentImage.width());
  ByteRegion debugRegion;
  debugRegion.SetSize(sourceRegion.Width() - 2, sourceRegion.Height() - 2);
  for (int j = 1; j < sourceRegion.Height() - 1; j++) {
    const uchar* src1 = sourceRegion.Data(0, j);
    const uchar* src2 = sourceRegion.Data(1, j);
    const uchar* src3 = sourceRegion.Data(2, j);
    const uchar* src4 = sourceRegion.Data(1, j-1);
    const uchar* src5 = sourceRegion.Data(1, j+1);
    uchar* dst = debugRegion.Line(j - 1);
    for (int i = 1; i < sourceRegion.Width() - 1; i++) {
      *dst = qMax(*src1, qMax(qMax(*src2, *src3), qMax(*src4, *src5)));

      dst++;
      src1++;
      src2++;
      src3++;
      src4++;
      src5++;
    }
  }

  ui->tabFilter->SetImage(ImageFromRegion(debugRegion));
  mLoadedTabs[eTabFilter] = true;
}

void MainWindow::FilterErosionWhite()
{
  if (!PrepareData()) {
    return;
  }

  ByteRegion sourceRegion;
  sourceRegion.SetSource(mCurrentValue.data(), mCurrentImage.width(), mCurrentImage.height(), mCurrentImage.width());
  ByteRegion debugRegion;
  debugRegion.SetSize(sourceRegion.Width() - 2, sourceRegion.Height() - 2);
  for (int j = 1; j < sourceRegion.Height() - 1; j++) {
    const uchar* src1 = sourceRegion.Data(0, j);
    const uchar* src2 = sourceRegion.Data(1, j);
    const uchar* src3 = sourceRegion.Data(2, j);
    const uchar* src4 = sourceRegion.Data(1, j-1);
    const uchar* src5 = sourceRegion.Data(1, j+1);
    uchar* dst = debugRegion.Line(j - 1);
    for (int i = 1; i < sourceRegion.Width() - 1; i++) {
      *dst = qMin(*src1, qMin(qMin(*src2, *src3), qMin(*src4, *src5)));

      dst++;
      src1++;
      src2++;
      src3++;
      src4++;
      src5++;
    }
  }

  ui->tabFilter->SetImage(ImageFromRegion(debugRegion));
  mLoadedTabs[eTabFilter] = true;
}

void MainWindow::DataFromImage()
{
  mCurrentValue.resize(mCurrentImage.width() * mCurrentImage.height());
  for (int j = 0; j < mCurrentImage.height(); j++) {
    const QRgb* data = reinterpret_cast<const QRgb*>(mCurrentImage.scanLine(j));
    uchar* dst = mCurrentValue.data() + mCurrentImage.width() * j;
    for (int i = 0; i < mCurrentImage.width(); i++) {
      *dst = QColor::fromRgb(*data).value();

      data++;
      dst++;
    }
  }
}

void MainWindow::PrepareLineFilters()
{
  mCurrentActions = eTabLineStats;

  PrepareLineActions();
}

void MainWindow::PrepareImageFilters()
{
  mCurrentActions = eTabFilter;

  PrepareFilterActions();
}

QImage MainWindow::ImageFromData()
{
  uchar maxValue = MaxData();

  int shift = 0;
  while (maxValue < 0x80) {
    shift++;
    maxValue <<= 1;
  }

  QImage img(mCurrentImage.size(), QImage::Format_ARGB32);
  for (int j = 0; j < mCurrentImage.height(); j++) {
    const uchar* src = mFilterValue.constData() + mCurrentImage.width() * j;
    QRgb* data = reinterpret_cast<QRgb*>(img.scanLine(j));
    for (int i = 0; i < mCurrentImage.width(); i++) {
      uchar value = *src << shift;
      *data = QColor::fromRgb(value, value, value).rgb();

      data++;
      src++;
    }
  }

  return img;
}

QImage MainWindow::IndexFromData()
{
  uchar maxValue = MaxData();
  uchar v1 = qMax(maxValue/2, 1);
  uchar v2 = qMax(maxValue/4, 1);
  uchar v3 = qMax(maxValue/8, 1);
  uchar v4 = qMax(maxValue/16, 1);

  QImage img(mCurrentImage.size(), QImage::Format_ARGB32);
  for (int j = 0; j < mCurrentImage.height(); j++) {
    const uchar* src = mFilterValue.constData() + mCurrentImage.width() * j;
    QRgb* data = reinterpret_cast<QRgb*>(img.scanLine(j));
    for (int i = 0; i < mCurrentImage.width(); i++) {
      uchar value = *src;
      if (value > v1) {
        *data = QColor::fromRgb(255, 0, 0).rgb();
      } else if (value > v2) {
        *data = QColor::fromRgb(255, 128, 0).rgb();
      } else if (value > v3) {
        *data = QColor::fromRgb(128, 255, 0).rgb();
      } else if (value > v4) {
        *data = QColor::fromRgb(0, 255, 0).rgb();
      } else if (value > 0) {
        *data = QColor::fromRgb(128, 128, 128).rgb();
      } else {
        *data = QColor::fromRgb(0, 0, 0).rgb();
      }

      data++;
      src++;
    }
  }

  return img;
}

QImage MainWindow::ImageFromRegion(const ByteRegion& region)
{
  QImage img(region.Width(), region.Height(), QImage::Format_ARGB32);
  for (int j = 0; j < region.Height(); j++) {
    const uchar* src = region.Line(j);
    QRgb* data = reinterpret_cast<QRgb*>(img.scanLine(j));
    for (int i = 0; i < region.Width(); i++) {
      uchar value = *src;
      *data = QColor::fromRgb(value, value, value).rgb();

      data++;
      src++;
    }
  }
  return img;
}

QImage MainWindow::IndexFromRegion(const ByteRegion& region)
{
  uchar maxValue = MaxRegion(region);
  uchar v1 = qMax(maxValue/2, 1);
  uchar v2 = qMax(maxValue/4, 1);
  uchar v3 = qMax(maxValue/8, 1);
  uchar v4 = qMax(maxValue/16, 1);

  QImage img(region.Width(), region.Height(), QImage::Format_ARGB32);
  for (int j = 0; j < region.Height(); j++) {
    const uchar* src = region.Line(j);
    QRgb* data = reinterpret_cast<QRgb*>(img.scanLine(j));
    for (int i = 0; i < region.Width(); i++) {
      uchar value = *src;
      if (value > v1) {
        *data = QColor::fromRgb(255, 0, 0).rgb();
      } else if (value > v2) {
        *data = QColor::fromRgb(255, 128, 0).rgb();
      } else if (value > v3) {
        *data = QColor::fromRgb(128, 255, 0).rgb();
      } else if (value > v4) {
        *data = QColor::fromRgb(0, 255, 0).rgb();
      } else if (value > 0) {
        *data = QColor::fromRgb(128, 128, 128).rgb();
      } else {
        *data = QColor::fromRgb(0, 0, 0).rgb();
      }

      data++;
      src++;
    }
  }

  return img;
}

uchar MainWindow::MaxData()
{
  uchar maxValue = 0;
  for (auto itr = mFilterValue.begin(); itr != mFilterValue.end(); itr++) {
    maxValue = qMax(maxValue, *itr);
  }

  return maxValue;
}

uchar MainWindow::MaxRegion(const ByteRegion& region)
{
  uchar maxValue = 0;
  for (int j = 0; j < region.Height(); j++) {
    const uchar* src = region.Line(j);
    for (int i = 0; i < region.Width(); i++) {
      uchar value = *src;
      maxValue = qMax(maxValue, value);

      src++;
    }
  }

  return maxValue;
}

void MainWindow::ResetPointsView()
{
  mLoadedTabs[eTabLineStats] = false;
  mLoadedTabs[eTabRectStats] = false;
}

void MainWindow::PrepareTab()
{
  switch (mCurrentTab) {
  case eTabLineStats:
    PrepareLineFilters();
    break;

  case eTabFilter:
    PrepareImageFilters();
    break;

  default:
    break;
  }

  switch (mCurrentTab) {
  case eTabImage    : mDumpCombo->setVisible(false); break;
  case eTabLineStats: mDumpCombo->setVisible(true); break;
  case eTabRectStats: mDumpCombo->setVisible(false); break;
  case eTabGrayscale: mDumpCombo->setVisible(false); break;
  case eTabDiff     : mDumpCombo->setVisible(false); break;
  case eTabFilter   : mDumpCombo->setVisible(true); break;
  case eTabIllegal  : mDumpCombo->setVisible(false); break;
  }
}

void MainWindow::UpdateTab(bool force)
{
  if (mLineChanged) {
    ui->tabImage->SyncSettings(GetSettings());
    mLineChanged = false;
  }

  if (force || !mLoadedTabs.at(mCurrentTab)) {
    mLoadedTabs[mCurrentTab] = true;
    switch (mCurrentTab) {
    case eTabImage:
      break;

    case eTabLineStats:
      FilterLine();
      break;

    case eTabRectStats:
      FilterRect();
      break;

    case eTabGrayscale:
      SetGrayscale();
      break;

    case eTabDiff:
      SetDiff();
      break;

    case eTabFilter:
      FilterImage();
      break;

    case eTabIllegal:
      break;
    }
  }
}

void MainWindow::UpdateDumpActionsView()
{
  if (ui->tabWidgetMain->currentIndex() == eTabLineStats) {
    ui->actionNextFilterImage->setEnabled(mCurrentLineIndex < mMaxDumpIndex - 1);
    ui->actionPrevFilterImage->setEnabled(mCurrentLineIndex > 0);
  } else if (ui->tabWidgetMain->currentIndex() == eTabFilter) {
    ui->actionNextFilterImage->setEnabled(mCurrentDumpIndex < mMaxDumpIndex - 1);
    ui->actionPrevFilterImage->setEnabled(mCurrentDumpIndex > 0);
  }
}

void MainWindow::SwitchSelect(int select, bool checked)
{
  FormImageLineView::EMode newSelect = checked? (FormImageLineView::EMode)select: FormImageLineView::eSelect;
  if (newSelect != mSelect) {
    mSelect = newSelect;
    ui->tabImage->SetMode(newSelect);
  }
  QSignalBlocker b1(ui->actionViewSelect);
  QSignalBlocker b2(ui->actionViewLine);
  QSignalBlocker b3(ui->actionViewRectangle);
  ui->actionViewSelect->setChecked(mSelect == FormImageLineView::eSelect);
  ui->actionViewLine->setChecked(mSelect == FormImageLineView::eLine);
  ui->actionViewRectangle->setChecked(mSelect == FormImageLineView::eRectangle);

  ResetPointsView();
  if (mSelect == FormImageLineView::eLine) {
    UpdateLineXy();
  }
  UpdateActions();
}

void MainWindow::DumpLine()
{
  if (!mAnalyser->LineFilterTest(mCurrentLineIndex, mLineMarks)) {
    return;
  }

  ui->widgetParam1->setVisible(false);
  ui->widgetParam2->setVisible(false);

  ui->graphLabel->SetLineValues(mLineValues, mLineMarks);
  mLoadedTabs[eTabLineStats] = true;

  UpdateDumpActionsView();
}

void MainWindow::DumpRegion()
{
  PrepareData();
  mAnalyser->RegionFilterTest(mCurrentDumpIndex, mCurrentDumpParam1, mCurrentDumpParam2);
  ui->tabFilter->SetImage(ImageFromRegion(mAnalyser->Result()));
  mLoadedTabs[eTabFilter] = true;
}

void MainWindow::OnLineChanged()
{
  ResetPointsView();

  if (mSelect == FormImageLineView::eLine) {
    UpdateLineXy();
  }

  mLineChanged = true;
  UpdateActions();
}

void MainWindow::OnDumpLineChanged(int index)
{
  mCurrentLineIndex = index;

  FilterLine();
}

void MainWindow::OnDumpLineTriggered()
{
  if (QAction* action = qobject_cast<QAction*>(sender())) {
    int index = action->data().toInt();
    ui->comboBoxFilterDump->setCurrentIndex(index);
  }
}

void MainWindow::OnDumpRegionChanged(int index)
{
  bool setDefault = mCurrentDumpIndex != index;
  mCurrentDumpIndex = index;

  FilterInfo filterInfo;
  if (!mAnalyser->RegionFilterInfo(index, &filterInfo)) {
    return;
  }

  if (setDefault) {
    mCurrentDumpParam1 = filterInfo.Param1Default;
    mCurrentDumpParam2 = filterInfo.Param2Default;
  } else {
    mCurrentDumpParam1 = qBound(filterInfo.Param1Min, mCurrentDumpParam1, filterInfo.Param1Max);
    mCurrentDumpParam2 = qBound(filterInfo.Param2Min, mCurrentDumpParam2, filterInfo.Param2Max);
  }

  ui->widgetParam1->setVisible(filterInfo.Param1Max > filterInfo.Param1Min);
  ui->labelParam1->setText(filterInfo.Param1Name);
  ui->spinBoxFilterParam1->disconnect();
  ui->spinBoxFilterParam1->setMinimum(filterInfo.Param1Min);
  ui->spinBoxFilterParam1->setMaximum(filterInfo.Param1Max);
  ui->spinBoxFilterParam1->setValue(mCurrentDumpParam1);
  if (filterInfo.Param1Max > filterInfo.Param1Min) {
    connect(ui->spinBoxFilterParam1, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged)
            , this, &MainWindow::OnDumpRegionParamsChanged);
  }

  ui->widgetParam2->setVisible(filterInfo.Param2Max > filterInfo.Param2Min);
  ui->labelParam2->setText(filterInfo.Param2Name);
  ui->spinBoxFilterParam2->disconnect();
  ui->spinBoxFilterParam2->setMinimum(filterInfo.Param2Min);
  ui->spinBoxFilterParam2->setMaximum(filterInfo.Param2Max);
  ui->spinBoxFilterParam2->setValue(mCurrentDumpParam2);
  if (filterInfo.Param2Max > filterInfo.Param2Min) {
    connect(ui->spinBoxFilterParam2, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged)
            , this, &MainWindow::OnDumpRegionParamsChanged);
  }

  OnDumpRegionParamsChanged(0);
  UpdateDumpActionsView();
}

void MainWindow::OnDumpRegionParamsChanged(int)
{
  mCurrentDumpParam1 = ui->spinBoxFilterParam1->value();
  mCurrentDumpParam2 = ui->spinBoxFilterParam2->value();

  FilterImage();
}

void MainWindow::OnDumpRegionTriggered()
{
  if (QAction* action = qobject_cast<QAction*>(sender())) {
    int index = action->data().toInt();
    ui->comboBoxFilterDump->setCurrentIndex(index);
//    OnDumpRegionChanged(index);
  }
}

void MainWindow::on_actionPaste_triggered()
{
  QImage img = mClipboard->image();
  SetImage(img, "Clipboard image");
}

void MainWindow::on_actionOpenFile_triggered()
{
  if (mFileDialog->exec() && !mFileDialog->selectedFiles().isEmpty()) {
    mLoadedTabs.fill(false, (int)eTabIllegal);
    QString filename = mFileDialog->selectedFiles().first();
    QImage img(filename);
    if (!img.isNull()) {
      QFileInfo info(filename);
      mCurrentDir = info.dir();
      mDirFiles = mCurrentDir.entryList(QDir::Files, QDir::Name);
      SetImage(img, info.fileName());
      ApplyDir();

      GetSettings()->setValue("FilesPath", mCurrentDir.absolutePath());
      GetSettings()->setValue("ImageFile", mImageSource);
      GetSettings()->sync();
    }
  }
}

void MainWindow::on_actionPrevFile_triggered()
{
  forever {
    mCurrentFile--;
    if (mCurrentFile < 0) {
      mCurrentFile = 0;
      return;
    }
    QString filename = mDirFiles.at(mCurrentFile);
    QImage img(mCurrentDir.filePath(filename));
    if (SetImage(img, filename)) {
      ui->horizontalSliderFiles->setValue(mCurrentFile);
      return;
    }
    mDirFiles.removeAt(mCurrentFile);
    ui->horizontalSliderFiles->setMaximum(mDirFiles.size() - 1);
    ui->horizontalSliderFiles->setValue(mCurrentFile);
  }
}

void MainWindow::on_actionNextFile_triggered()
{
  forever {
    mCurrentFile++;
    if (mCurrentFile >= mDirFiles.size()) {
      mCurrentFile = mDirFiles.size() - 1;
      return;
    }
    QString filename = mDirFiles.at(mCurrentFile);
    QImage img(mCurrentDir.filePath(filename));
    if (SetImage(img, filename)) {
      ui->horizontalSliderFiles->setValue(mCurrentFile);
      return;
    }
    mDirFiles.removeAt(mCurrentFile);
    ui->horizontalSliderFiles->setMaximum(mDirFiles.size() - 1);
    ui->horizontalSliderFiles->setValue(mCurrentFile);
  }
}

void MainWindow::on_horizontalSliderFiles_sliderMoved(int position)
{
  if (position == mCurrentFile) {
    return;
  }
  int inc = (position < mCurrentFile)? 1: -1;
  mCurrentFile = position;
  forever {
    if (mCurrentFile < 0) {
      mCurrentFile = 0;
      return;
    } if (mCurrentFile >= mDirFiles.size()) {
      mCurrentFile = mDirFiles.size() - 1;
      return;
    }

    QString filename = mDirFiles.at(mCurrentFile);
    QImage img(mCurrentDir.filePath(filename));
    if (SetImage(img, filename)) {
      return;
    }
    mDirFiles.removeAt(mCurrentFile);
    ui->horizontalSliderFiles->setMaximum(mDirFiles.size() - 1);
    ui->horizontalSliderFiles->setValue(mCurrentFile);

    mCurrentFile += inc;
  }
}

void MainWindow::on_tabWidgetMain_currentChanged(int index)
{
  mCurrentTab = (ETab)index;
  PrepareTab();
  UpdateTab();
}

void MainWindow::on_actionLineStats_triggered()
{
  ui->tabWidgetMain->setCurrentIndex(eTabLineStats);
}

void MainWindow::on_actionFilter_triggered()
{
  ui->tabWidgetMain->setCurrentIndex(eTabFilter);
}

void MainWindow::on_actionFilterMk3Color_triggered()
{
  FilterImageMk3Color();

  QSignalBlocker b(ui->tabWidgetMain);
  ui->tabWidgetMain->setCurrentIndex(eTabFilter);
}

void MainWindow::on_actionErosionBlack_triggered()
{
  FilterErosionBlack();

  QSignalBlocker b(ui->tabWidgetMain);
  ui->tabWidgetMain->setCurrentIndex(eTabFilter);
}

void MainWindow::on_actionErosionWhite_triggered()
{
  FilterErosionWhite();

  QSignalBlocker b(ui->tabWidgetMain);
  ui->tabWidgetMain->setCurrentIndex(eTabFilter);
}

void MainWindow::on_spinBoxLineX_valueChanged(int value)
{
  ui->tabImage->MoveLineX(value);
  UpdateTab(true);
}

void MainWindow::on_spinBoxLineY_valueChanged(int value)
{
  ui->tabImage->MoveLineY(value);
  UpdateTab(true);
}

void MainWindow::on_actionNextFilterImage_triggered()
{
  if (ui->tabWidgetMain->currentIndex() == eTabLineStats) {
    if (mCurrentLineIndex < mMaxDumpIndex - 1) {
      mCurrentLineIndex++;
      ui->comboBoxFilterDump->setCurrentIndex(mCurrentLineIndex);
    }
  } else if (ui->tabWidgetMain->currentIndex() == eTabFilter) {
    if (mCurrentDumpIndex < mMaxDumpIndex - 1) {
      mCurrentDumpIndex++;
      ui->comboBoxFilterDump->setCurrentIndex(mCurrentDumpIndex);
    }
  }
}

void MainWindow::on_actionPrevFilterImage_triggered()
{
  if (ui->tabWidgetMain->currentIndex() == eTabLineStats) {
    mCurrentLineIndex--;
    ui->comboBoxFilterDump->setCurrentIndex(mCurrentLineIndex);
  } else if (ui->tabWidgetMain->currentIndex() == eTabFilter) {
    mCurrentDumpIndex--;
    ui->comboBoxFilterDump->setCurrentIndex(mCurrentDumpIndex);
  }
}

void MainWindow::on_actionViewSelect_toggled(bool checked)
{
  SwitchSelect(FormImageLineView::eSelect, checked);
}

void MainWindow::on_actionViewLine_toggled(bool checked)
{
  SwitchSelect(FormImageLineView::eLine, checked);
}

void MainWindow::on_actionViewRectangle_toggled(bool checked)
{
  SwitchSelect(FormImageLineView::eRectangle, checked);
}

void MainWindow::on_actionAreaStats_triggered()
{
  ui->tabImage->SyncSettings(GetSettings());

  FilterRect();
  ui->tabWidgetMain->setCurrentIndex(eTabRectStats);
}

void MainWindow::on_actionWhiteBallance_triggered()
{
  if (!PrepareData()) {
    return;
  }

  mAnalyser->GetImageStatFtr()->MakeWhiteBallance();
  const ByteRegion& markRegion = mAnalyser->Result();
  QImage image = ImageFromRegion(markRegion);
  SetImage(image, "White black ballanced image");
}
