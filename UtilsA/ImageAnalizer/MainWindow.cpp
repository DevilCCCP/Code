#include <QDesktopWidget>
#include <QStandardPaths>
#include <QClipboard>
#include <QScrollBar>
#include <QComboBox>
#include <QDebug>

#include <Lib/CoreUi/Icon.h>
#include <LibA/Analyser/Analyser.h>
#include <LibA/Analyser/SignalMark.h>
#include <LibA/Analyser/SignalMark2.h>
#include <LibA/Analyser/SignalMark3.h>

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
  , mSignalMark(new SignalMark(kSignalLengthMax)), mSignalMark2(new SignalMark2()), mSignalMark3(new SignalMark3()), mAnalyser(new Analyser(true))
{
  ui->setupUi(this);

  mDumpCombo = ui->mainToolBar->addWidget(ui->widgetFilterControl);

  mCurrentActions = -1;
  ui->mainToolBar->setContextMenuPolicy(Qt::PreventContextMenu);

  mLoadedTabs.fill(false, (int)eTabIllegal);
  mCurrentTab = eTabImage;
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
  ui->tabImage->addAction(ui->actionLineStats2);
  ui->tabImage->addAction(ui->actionAreaStats);
  ui->tabImage->addAction(ui->actionFilter);
  ui->tabImage->addAction(ui->actionFilter2);
  ui->tabImage->addAction(ui->actionFilterMk3Color);
  ui->tabImage->addAction(ui->actionErosionBlack);
  ui->tabImage->addAction(ui->actionErosionWhite);
  ui->tabImage->setContextMenuPolicy(Qt::ActionsContextMenu);

  ui->toolButtonDumpFilterPrev->setDefaultAction(ui->actionPrevFilterImage);
  ui->toolButtonDumpFilterNext->setDefaultAction(ui->actionNextFilterImage);

  if (!Restore()) {
    resize(QDesktopWidget().availableGeometry(this).size() * 0.5);
  }
  ui->mainToolBar->setVisible(true);

  QString iniFilePath = QDir(QStandardPaths::writableLocation(
#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
                               QStandardPaths::AppDataLocation
#else
                               QStandardPaths::DataLocation
#endif
                               )).absoluteFilePath("main.ini");
  mSettings = new QSettings(iniFilePath, QSettings::IniFormat, this);
  mSettings->setIniCodec("UTF-8");

  mImageSource = mSettings->value("ImageFile").toString();
  mCurrentDir = QDir(mSettings->value("FilesPath").toString());
  mFileDialog->setDirectory(mCurrentDir);
  mCurrentImage = QImage(mCurrentDir.filePath(mImageSource));
  ui->tabImage->RestoreSettings(mSettings);
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
  ui->tabImage->SetScale(0);
  mCurrentValue.clear();
  ui->tabImage->SetImage(mCurrentImage);

  if (mCurrentTab) {
    UpdateTab();
  }
  qInfo() << QString("Analize '%1'").arg(mImageSource);
  setWindowTitle(QString("Analize '%1'").arg(mImageSource));
}

bool MainWindow::PrepareData()
{
  if (mCurrentValue.isEmpty()) {
    DataFromImage();
    if (mCurrentValue.isEmpty()) {
      return false;
    }
  }
  return true;
}

void MainWindow::PrepareLineActions(const QList<MainWindow::DumpLine>& infoList, int defaultIndex)
{
  mDumpList.clear();

  ui->tabStats->setContextMenuPolicy(Qt::ActionsContextMenu);
  auto oldActionsList = ui->tabStats->actions();
  foreach (auto action, oldActionsList) {
    ui->tabStats->removeAction(action);
    action->deleteLater();
  }

  mDumpLine = infoList;
  for (int i = 0; i < mDumpLine.size(); i++) {
    const DumpLine& info = mDumpLine.at(i);
    mDumpList << info.Name;
    QAction* action = new QAction(QIcon("Icons/Stats.png"), info.Name, this);
    action->setData(i);
    ui->tabStats->addAction(action);
    connect(action, &QAction::triggered, this, &MainWindow::OnDumpLineTriggered);
  }
  PrepareDump(&MainWindow::OnDumpLineChanged, defaultIndex);
}

void MainWindow::PrepareFilterActions(const QList<MainWindow::DumpRegion>& infoList, int defaultIndex)
{
  mDumpList.clear();

  ui->tabFilter->setContextMenuPolicy(Qt::ActionsContextMenu);
  auto oldActionsList = ui->tabFilter->actions();
  foreach (auto action, oldActionsList) {
    ui->tabFilter->removeAction(action);
    action->deleteLater();
  }

  mDumpRegion = infoList;
  for (int i = 0; i < mDumpRegion.size(); i++) {
    const DumpRegion& info = mDumpRegion.at(i);
    mDumpList << info.Name;
    QAction* action = new QAction(QIcon("Icons/Stats.png"), info.Name, this);
    action->setData(i);
    ui->tabFilter->addAction(action);
    connect(action, &QAction::triggered, this, &MainWindow::OnDumpRegionTriggered);
  }
  PrepareDump(&MainWindow::OnDumpRegionChanged, defaultIndex);
}

void MainWindow::PrepareDump(OnDumpTriggerFunc onDumpTriggerFunc, int defaultIndex)
{
  ui->comboBoxFilterDump->disconnect();
  ui->comboBoxFilterDump->clear();
  ui->comboBoxFilterDump->addItems(mDumpList);
  ui->comboBoxFilterDump->setCurrentIndex(-1);
  connect(ui->comboBoxFilterDump, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged)
          , this, onDumpTriggerFunc);
  ui->comboBoxFilterDump->setCurrentIndex(defaultIndex);
}

void MainWindow::SetGrayscale()
{
  if (!PrepareData()) {
    return;
  }
  Region<uchar> sourceRegion(mCurrentValue.data(), mCurrentImage.width(), mCurrentImage.height(), mCurrentImage.width());
  ui->tabGrayscale->SetImage(ImageFromRegion(sourceRegion));
}

void MainWindow::SetDiff()
{
  if (!PrepareData()) {
    return;
  }

  mAnalyser->Init(mCurrentValue.data(), mCurrentImage.width(), mCurrentImage.height(), mCurrentImage.width());
  mAnalyser->MakeGrad();

  const Region<uchar>& markRegion = mAnalyser->Result();
  ui->tabDiff->SetImage(ImageFromRegion(markRegion));
}

void MainWindow::FilterLine()
{
  mLineValues = ui->tabImage->LineValues().toVector();
  mLineMarks = QVector<uchar>(mLineValues.size(), (uchar)0);
  mSignalMark->CalcLine(mLineValues.constData(), mLineValues.size(), 0);
  mSignalMark->FillLineMark(mLineMarks.data(), mLineMarks.size());
  ui->graphLabel->SetLineValues(mLineValues, mLineMarks);

  mLoadedTabs[eTabLineStats] = true;
}

void MainWindow::FilterLine2()
{
  mLineValues = ui->tabImage->LineValues().toVector();
  mSignalMark3->CalcLine(mLineValues.constData(), mLineValues.size());
  mLoadedTabs[eTabLineStats] = true;

  PrepareLineFilters();
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

  Region<uchar> sourceRegion(mCurrentValue.data(), mCurrentImage.width(), mCurrentImage.height(), mCurrentImage.width());
  mSignalMark->Calc(&sourceRegion, 0);

  Region<uchar> markRegion(mCurrentImage.width(), mCurrentImage.height());
  mSignalMark->FillRegionMark(&markRegion);
  ui->tabFilter->SetImage(ImageFromRegion(markRegion));
  mLoadedTabs[eTabFilter] = true;
  return;
}

void MainWindow::FilterImage2()
{
  if (!PrepareData()) {
    return;
  }

  mSourceRegion.SetSource(mCurrentValue.data(), mCurrentImage.width(), mCurrentImage.height(), mCurrentImage.width());
  int plateWidth = mCurrentImage.width() > 1000? 250: mCurrentImage.width() > 800? 150: mCurrentImage.width() > 400? 100: 60;
  mAnalyser->FindUinRu(mSourceRegion, plateWidth);
  mLoadedTabs[eTabFilter] = true;

  PrepareImageFilters();
}

void MainWindow::FilterImageMk3Color()
{
  if (!PrepareData()) {
    return;
  }

  Region<uchar> sourceRegion;
  sourceRegion.SetSource(mCurrentValue.data(), mCurrentImage.width(), mCurrentImage.height(), mCurrentImage.width());
  mSignalMark->Calc(&sourceRegion, 0);


  mAnalyser->Init(mCurrentValue.data(), mCurrentImage.width(), mCurrentImage.height(), mCurrentImage.width());
  mAnalyser->MakeHigher(1);

  Region<uchar> debugRegion = sourceRegion;
//  mUinPre->Calc3Color(sourceRegion, debugRegion);

  ui->tabFilter->SetImage(ImageFromRegion(debugRegion));
  mLoadedTabs[eTabFilter] = true;
  return;
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

  Region<uchar> sourceRegion;
  sourceRegion.SetSource(mCurrentValue.data(), mCurrentImage.width(), mCurrentImage.height(), mCurrentImage.width());
  Region<uchar> debugRegion;
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

  Region<uchar> sourceRegion;
  sourceRegion.SetSource(mCurrentValue.data(), mCurrentImage.width(), mCurrentImage.height(), mCurrentImage.width());
  Region<uchar> debugRegion;
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
  if (mCurrentActions == 1) {
    OnDumpLineChanged(mCurrentDumpIndex);
    return;
  }
  mCurrentActions = 1;

  PrepareLineActions(QList<DumpLine>()
                     << DumpLine("Extrem", &SignalMark3::DumpLineExtrem)
                     << DumpLine("Move", &SignalMark3::DumpLineMove)
                     << DumpLine("Signal", &SignalMark3::DumpLineSignal)
                     , 2
                     );
}

void MainWindow::PrepareImageFilters()
{
  if (mCurrentActions == 2) {
    OnDumpRegionChanged(mCurrentDumpIndex);
    return;
  }
  mCurrentActions = 2;

  PrepareFilterActions(QList<DumpRegion>()
                       << DumpRegion("Stat raw", &Analyser::DumpUinStatRaw, 1, 64, 8)
                       << DumpRegion("Stat raw 2", &Analyser::DumpUinStatRaw2, 1, 64, 8)
                       << DumpRegion("Stat raw 2/3", &Analyser::DumpUinStatRaw23, 1, 64, 8)
                       << DumpRegion("Stat signal", &Analyser::DumpUinStatSignal)
                       << DumpRegion("Stat signal level", &Analyser::DumpUinStatSignalLevel, 1, 40, 2)
                       << DumpRegion("Stat thickness 2", &Analyser::DumpUinStatThickness2, 1, 20, 4, 1, 10, 2)
                       << DumpRegion("Stat thickness 2/3", &Analyser::DumpUinStatThickness23, 1, 20, 4, 1, 10, 2)
                       << DumpRegion("Stat edge", &Analyser::DumpUinStatEdge)
                       << DumpRegion("Stat edge filtered", &Analyser::DumpUinStatEdgeFiltered)
                       << DumpRegion("Stat black", &Analyser::DumpUinStatBlack)
                       << DumpRegion("Stat white", &Analyser::DumpUinStatWhite)
                       << DumpRegion("Stat middle", &Analyser::DumpUinStatMiddle)
                       << DumpRegion("Stat black count", &Analyser::DumpUinStatCountBlack)
                       << DumpRegion("Stat white count", &Analyser::DumpUinStatCountWhite)
                       << DumpRegion("Stat diff", &Analyser::DumpUinStatDiff)
                       << DumpRegion("Stat white level", &Analyser::DumpUinStatWhiteLevel, 1, 15, 15, 0, 4, 1)
                       << DumpRegion("Stat both level", &Analyser::DumpUinStatBothLevel, 1, 15, 15, 1, 15, 1)
                       << DumpRegion("White level cut", &Analyser::DumpUinStatWhiteLevelCut, 1, 15, 15)
                       << DumpRegion("Cut level", &Analyser::DumpUinCutLevel, 1, 15, 15, 1, 15, 1)
                       << DumpRegion("Cut level 2", &Analyser::DumpUinCutLevel2, 1, 15, 7)
                       << DumpRegion("Color level", &Analyser::DumpUinColorLevel, 0, 15)
                       << DumpRegion("Plate", &Analyser::DumpUinStatPlate, 0, 999)
                       << DumpRegion("Plate Normal", &Analyser::DumpUinStatPlateNormal, 0, 999, 0, 0, 99, 0)

//                       << DumpRegion("Signal height", &Analyser::DumpSignalHeight)
//                       << DumpRegion("Signal pack", &Analyser::DumpSignalPack)
//                       << DumpRegion("Signal area", &Analyser::DumpSignalArea)
//                       << DumpRegion("Uin solid", &Analyser::DumpUinSolids)
//                       << DumpRegion("Uin symbol", &Analyser::DumpUinSymbols)
//                       << DumpRegion("Uin places", &Analyser::DumpUinSymbolsFixed)
//                       << DumpRegion("Uin plate", &Analyser::DumpPlate)
                       << DumpRegion("Uin test", &Analyser::DumpUinTest, 0, 999)
                       << DumpRegion("Uin digits", &Analyser::DumpUinDigits, 0, 999)
                       << DumpRegion("Uin base", &Analyser::DumpUinPrepare)
                       , 0
                       );
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

QImage MainWindow::ImageFromRegion(const Region<uchar>& region)
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

QImage MainWindow::IndexFromRegion(const Region<uchar>& region)
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

uchar MainWindow::MaxRegion(const Region<uchar>& region)
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

void MainWindow::UpdateTab(bool force)
{
  if (force || !mLoadedTabs.at(mCurrentTab)) {
    mLoadedTabs[mCurrentTab] = true;
    switch (mCurrentTab) {
    case eTabImage:
      break;

    case eTabLineStats:
      FilterLine2();
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
      FilterImage2();
      break;

    case eTabIllegal:
      break;
    }
  } else {
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

void MainWindow::UpdateDumpActionsView()
{
  ui->actionNextFilterImage->setEnabled(mCurrentDumpIndex < mDumpList.size() - 1);
  ui->actionPrevFilterImage->setEnabled(mCurrentDumpIndex > 0);
}

void MainWindow::SwitchSelect(int select, bool checked)
{
  FormImageLineView::EMode newSelect = checked? (FormImageLineView::EMode)select: FormImageLineView::eSelect;
  if (newSelect != mSelect) {
    mSelect = newSelect;
    ui->tabImage->SetMode(newSelect);
  }
  ui->actionViewSelect->blockSignals(true);
  ui->actionViewSelect->setChecked(mSelect == FormImageLineView::eSelect);
  ui->actionViewSelect->blockSignals(false);
  ui->actionViewLine->blockSignals(true);
  ui->actionViewLine->setChecked(mSelect == FormImageLineView::eLine);
  ui->actionViewLine->blockSignals(false);
  ui->actionViewRectangle->blockSignals(true);
  ui->actionViewRectangle->setChecked(mSelect == FormImageLineView::eRectangle);
  ui->actionViewRectangle->blockSignals(false);
}

void MainWindow::OnLineChanged()
{
  if (mSelect == FormImageLineView::eLine) {
    mLoadedTabs[eTabLineStats] = false;
    QVector<QPoint> points = ui->tabImage->LinePoints();
    if (points.size() >= 2) {
      ui->spinBoxLineX->blockSignals(true);
      ui->spinBoxLineY->blockSignals(true);
      int d = points.at(1).x() - points.at(0).x();
      ui->spinBoxLineX->setMinimum(d >= 0? 0: -d);
      ui->spinBoxLineX->setMaximum(d >= 0? mCurrentImage.width() - 1: mCurrentImage.width() - 1  + d);
      ui->spinBoxLineY->setRange(0, mCurrentImage.height() - 1);
      ui->spinBoxLineX->setValue(points.at(0).x());
      ui->spinBoxLineY->setValue(points.at(0).y());
      ui->spinBoxLineX->blockSignals(false);
      ui->spinBoxLineY->blockSignals(false);
      ui->spinBoxLineX->setEnabled(true);
      ui->spinBoxLineY->setEnabled(true);
    } else {
      ui->spinBoxLineX->setEnabled(false);
      ui->spinBoxLineY->setEnabled(false);
    }
  } else if (mSelect == FormImageLineView::eRectangle) {
    mLoadedTabs[eTabRectStats] = false;
  }
}

void MainWindow::OnDumpLineChanged(int index)
{
  mCurrentDumpIndex = index;
  if (index >= 0 && index < mDumpLine.size()) {
    DumpLineFunc f = mDumpLine.at(index).Function;
    ((*mSignalMark3).*f)(mLineMarks);
    ui->graphLabel->SetLineValues(mLineValues, mLineMarks);
    mLoadedTabs[eTabLineStats] = true;
  }
  UpdateDumpActionsView();

  ui->spinBoxFilterParam1->setVisible(false);
  ui->spinBoxFilterParam2->setVisible(false);
}

void MainWindow::OnDumpLineTriggered()
{
  if (QAction* action = qobject_cast<QAction*>(sender())) {
    int index = action->data().toInt();
    OnDumpLineChanged(index);
  }
}

void MainWindow::OnDumpRegionChanged(int index)
{
  mCurrentDumpIndex = index;
  if (index >= 0 && index < mDumpRegion.size()) {
    const DumpRegion& dumpRegion = mDumpRegion.at(index);
    mCurrentDumpParam1 = dumpRegion.Param1Default;
    mCurrentDumpParam2 = dumpRegion.Param2Default;
    ui->spinBoxFilterParam1->setMinimum(dumpRegion.Param1Min);
    ui->spinBoxFilterParam1->setMaximum(dumpRegion.Param1Max);
    ui->spinBoxFilterParam1->setValue(dumpRegion.Param1Default);
    ui->spinBoxFilterParam1->disconnect();
    ui->spinBoxFilterParam2->setMinimum(dumpRegion.Param2Min);
    ui->spinBoxFilterParam2->setMaximum(dumpRegion.Param2Max);
    ui->spinBoxFilterParam2->setValue(dumpRegion.Param2Default);
    ui->spinBoxFilterParam2->setVisible(dumpRegion.Param2Max > dumpRegion.Param2Min);
    ui->spinBoxFilterParam2->disconnect();
    if (dumpRegion.Param1Max > dumpRegion.Param1Min) {
      ui->spinBoxFilterParam1->setVisible(true);
      connect(ui->spinBoxFilterParam1, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged)
              , this, &MainWindow::OnDumpRegionParamsChanged);
    } else {
      ui->spinBoxFilterParam1->setVisible(false);
    }
    if (dumpRegion.Param2Max > dumpRegion.Param2Min) {
      ui->spinBoxFilterParam2->setVisible(true);
      connect(ui->spinBoxFilterParam2, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged)
              , this, &MainWindow::OnDumpRegionParamsChanged);
    } else {
      ui->spinBoxFilterParam2->setVisible(false);
    }

    OnDumpRegionParamsChanged(0);
  } else {
    ui->spinBoxFilterParam1->setVisible(false);
    ui->spinBoxFilterParam2->setVisible(false);
  }

  UpdateDumpActionsView();
}

void MainWindow::OnDumpRegionParamsChanged(int)
{
  int index = ui->comboBoxFilterDump->currentIndex();
  if (index >= 0 && index < mDumpRegion.size()) {
    const DumpRegion& dumpRegion = mDumpRegion.at(index);
    Region<uchar> markRegion;
    DumpRegionFunc f = dumpRegion.Function;
    mCurrentDumpParam1 = qBound(dumpRegion.Param1Min, ui->spinBoxFilterParam1->value(), dumpRegion.Param1Max);
    mCurrentDumpParam2 = qBound(dumpRegion.Param2Min, ui->spinBoxFilterParam2->value(), dumpRegion.Param2Max);
    ((*mAnalyser).*f)(&markRegion, mCurrentDumpParam1, mCurrentDumpParam2);
    ui->tabFilter->SetImage(ImageFromRegion(markRegion));
    mLoadedTabs[eTabFilter] = true;
  }
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

      mSettings->setValue("FilesPath", mCurrentDir.absolutePath());
      mSettings->setValue("ImageFile", mImageSource);
      mSettings->sync();
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
  UpdateTab();
}

void MainWindow::on_actionLineStats_triggered()
{
  ui->tabImage->SyncSettings(mSettings);

  FilterLine();
  ui->tabWidgetMain->setCurrentIndex(eTabLineStats);
}

void MainWindow::on_actionFilter_triggered()
{
  FilterImage();

  ui->tabWidgetMain->setCurrentIndex(eTabFilter);
}

void MainWindow::on_actionFilter2_triggered()
{
  FilterImage2();

  ui->tabWidgetMain->setCurrentIndex(eTabFilter);
}

void MainWindow::on_actionFilterMk3Color_triggered()
{
  FilterImageMk3Color();

  ui->tabWidgetMain->setCurrentIndex(eTabFilter);
}

void MainWindow::on_actionLineStats2_triggered()
{
  ui->tabImage->SyncSettings(mSettings);

  FilterLine2();
  ui->tabWidgetMain->setCurrentIndex(eTabLineStats);
}

void MainWindow::on_actionErosionBlack_triggered()
{
  FilterErosionBlack();

  ui->tabWidgetMain->setCurrentIndex(eTabFilter);
}

void MainWindow::on_actionErosionWhite_triggered()
{
  FilterErosionWhite();

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
  if (mCurrentDumpIndex < mDumpList.size() - 1) {
    mCurrentDumpIndex++;
    ui->comboBoxFilterDump->setCurrentIndex(mCurrentDumpIndex);
  }
}

void MainWindow::on_actionPrevFilterImage_triggered()
{
  if (mCurrentDumpIndex > 0) {
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
  ui->tabImage->SyncSettings(mSettings);

  FilterRect();
  ui->tabWidgetMain->setCurrentIndex(eTabRectStats);
}

void MainWindow::on_actionWhiteBallance_triggered()
{
  if (!PrepareData()) {
    return;
  }
  Region<uchar> sourceRegion(mCurrentValue.data(), mCurrentImage.width(), mCurrentImage.height(), mCurrentImage.width());
  mAnalyser->Init(sourceRegion);
  mAnalyser->MakeWhiteBallance();
  const Region<uchar>& markRegion = mAnalyser->Result();
  QImage image = ImageFromRegion(markRegion);
  SetImage(image, "White black ballanced image");
}
