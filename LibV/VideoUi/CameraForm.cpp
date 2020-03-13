#include <QCoreApplication>
#include <QShowEvent>
#include <QPainter>
#include <qsystemdetection.h>

#include <Lib/Log/Log.h>
#include <Lib/Db/ObjectType.h>
#include <Lib/Net/Chater.h>
#include <Lib/Ctrl/CtrlManager.h>
#include <LibV/Include/VideoMsg.h>
#include <LibV/Decoder/Decoder.h>
#include <LibV/Include/ModuleNames.h>

#include "CameraForm.h"
#include "ThumbnailReceiver.h"
#include "DecodeReceiver.h"
#include "ui_CameraForm.h"


const int kAutoRefreshMax = 10;
const int kAutoRefreshPeriodMs = 500;

CameraForm::CameraForm(CtrlManager* _Manager, const ObjectItemS& _Object, int _ArmObjectId, QWidget *parent)
  : QWidget(parent)
  , ui(new Ui::CameraForm)
  , mManager(_Manager), mCameraObject(_Object), mArmObjectId(_ArmObjectId)
  , mStatusFrame(false), mPlayProcess(nullptr)
{
  Init();
}

CameraForm::CameraForm(CtrlManager* _Manager, const ObjectItemS& _CameraObject, const ObjectItemS& _AnalObject
                       , const Db& _Db, EPointsType _PointsType, QWidget* parent)
  : QWidget(parent)
  , ui(new Ui::CameraForm)
  , mManager(_Manager), mCameraObject(_CameraObject), mAnalObject(_AnalObject), mArmObjectId(0), mPointsType(_PointsType)
{
  Init();

  ui->labelCameraPreview->SetPointsType(_Db, mAnalObject->Id, mPointsType);
  connect(ui->labelCameraPreview, &ImageWithPoints::NeedSave, this, &CameraForm::OnNeedSave);
}

CameraForm::~CameraForm()
{
  if (mChater) {
    mChater->Close();
    mChater.clear();
  }
  if (mDecoder) {
    mDecoder->DisconnectModule(mDecodeReceiver.data());
    mDecoder->disconnect(this);
    mDecoder->Stop();

    mDecodeReceiver->disconnect(this);
    mDecodeReceiver->Stop();
    mDecoder->WaitFinish();
  }
  delete ui;
}

void CameraForm::Init()
{
  Q_INIT_RESOURCE(Ui);
  Q_INIT_RESOURCE(VideoUi);

  ui->setupUi(this);
  ui->toolButtonReload->setDefaultAction(ui->actionRefresh);
  ui->toolButtonScale->setDefaultAction(ui->actionScale);
  ui->toolButtonSave->setDefaultAction(ui->actionSave);
  ui->toolButtonCancel->setDefaultAction(ui->actionCancel);
  ui->toolButtonPlay->setDefaultAction(ui->actionPlay);
  ui->toolButtonStop->setDefaultAction(ui->actionStop);
  ui->frameControls->setVisible(mNeedSave  = false);
  ui->actionScale->setChecked(mScaled = true);

  ui->stackedWidgetCameraPreview->setCurrentIndex(0);

//#ifndef Q_OS_WIN32
//  ui->actionPlay->setEnabled(false);
//  ui->actionStop->setEnabled(false);
//#endif

  mAutoRefresh = new QTimer(this);
  mAutoRefresh->setSingleShot(true);
  mAutoRefreshCount = kAutoRefreshMax;
  connect(mAutoRefresh, &QTimer::timeout, this, &CameraForm::OnAutoRefreshTimer);

  mPlayConfirmTimer = new QTimer(this);
  connect(mPlayConfirmTimer, &QTimer::timeout, this, &CameraForm::OnPlayContinue);

  SetPlay(false);
}


void CameraForm::closeEvent(QCloseEvent* event)
{
  if (mNeedSave) {
    ui->labelCameraPreview->SavePoints();
  }

  QWidget::closeEvent(event);
}

void CameraForm::showEvent(QShowEvent* event)
{
  Q_UNUSED(event);

  if (!mChater) {
    mAutoRefresh->start(0);
  }
}

void CameraForm::CreateChat()
{
  if (!mDecoder) {
    CreateDecoder();
  }
  ThumbnailReceiver* thumb;
  ReceiverS receiver = ReceiverS(thumb = new ThumbnailReceiver(mDecoder.data()));
  mChater = Chater::CreateChater(mManager, Uri::FromString(mCameraObject->Uri), receiver);
  connect(thumb, &ThumbnailReceiver::OnThumbnailOk, this, &CameraForm::OnThumbnailImage, Qt::QueuedConnection);
  connect(thumb, &ThumbnailReceiver::OnThumbnailFail, this, &CameraForm::OnThumbnailFail, Qt::QueuedConnection);
  connect(thumb, &ThumbnailReceiver::OnThumbnailFrame, this, &CameraForm::OnThumbnailFrame, Qt::QueuedConnection);
  connect(thumb, &ThumbnailReceiver::Disconnected, this, &CameraForm::Disconnected, Qt::QueuedConnection);
}

void CameraForm::CreateDecoder()
{
  mDecoder.reset(new Decoder(false, false, eRawRgba));
  mManager->RegisterWorker(mDecoder);

  DecodeReceiver* dr;
  mDecodeReceiver.reset(dr = new DecodeReceiver());
  connect(dr, &DecodeReceiver::OnDecoded, this, &CameraForm::OnThumbnailDecoded, Qt::QueuedConnection);
  mManager->RegisterWorker(mDecodeReceiver);

  mDecoder->ConnectModule(mDecodeReceiver.data());
}

void CameraForm::Disconnected()
{
  mChater.clear();
  SetPlay(false);
}

void CameraForm::SetPlay(bool playing)
{
  mPlaying = playing;

  ui->toolButtonPlay->setVisible(!mPlaying);
  ui->toolButtonStop->setVisible(mPlaying);
}

void CameraForm::LoadThumbnail()
{
  mStatusFrame = false;
  ui->lineEditCamerStatus->setText(QString::fromUtf8("Соединение"));
  ui->lineEditCamerStatus->setStyleSheet("background-color: lightgray;\ncolor: blue;");

  if (!mChater) {
    CreateChat();
  }

  if (!mChater->SendSimpleMessage(eMsgThumbnail)) {
    OnThumbnailFail("Отправка неудачна");
  }
}

bool CameraForm::DrawFrame()
{
  if (!mSourceFrame) {
    return false;
  }
  //if (mPlaying) {
  //  ui->stackedWidgetCameraPreview->setCurrentIndex(1);
  //  ui->frameLabelCameraPreview->SetFrame(mSourceFrame);
  //  return true;
  //} else {
    ui->stackedWidgetCameraPreview->setCurrentIndex(0);
    const char* data = mSourceFrame->VideoData();
    int     dataSize = mSourceFrame->VideoDataSize();
    int     width    = mSourceFrame->GetHeader()->Width;
    int     height   = mSourceFrame->GetHeader()->Height;
    int     stride   = dataSize / height;

    mSourceImg = QImage(width, height, QImage::Format_RGB32);
    for (int j = 0; j < height; j++) {
      char* line = (char*)mSourceImg.scanLine(j);
      const char* dataLine = data + j * stride;
      memcpy(line, dataLine, width * 4);
    }
    return DrawImage();
  //}
}

bool CameraForm::DrawImage()
{
  if (mSourceImg.isNull()) {
    ui->lineEditCamerStatus->setText("Ошибка изображения");
    ui->lineEditCamerStatus->setStyleSheet("background-color: lightgray;\ncolor: red;");
    return false;
  }

  if (!mStatusFrame) {
    if (mPlaying) {
      ui->lineEditCamerStatus->setText(QString::fromUtf8("Получение потока"));
      ui->lineEditCamerStatus->setStyleSheet("background-color: lightgray;\ncolor: green;");
    } else {
      ui->lineEditCamerStatus->setText(QString::fromUtf8("Изображение получено"));
      ui->lineEditCamerStatus->setStyleSheet("background-color: lightgray;\ncolor: green;");
    }
    ui->labelCameraPreview->SetImage(mSourceImg);
    ResizeImage();
  } else {
    ui->lineEditCamerStatus->setText("Камера не доступна");
    ui->lineEditCamerStatus->setStyleSheet("background-color: lightgray;\ncolor: red;");
    DrawStatusFrame();
  }
  return true;
}

void CameraForm::ResizeImage()
{
  QSize size;
  if (mScaled) {
    int destWidth = ui->scrollAreaContent->contentsRect().width();
    int destHeight = ui->scrollAreaContent->contentsRect().height();
    int srcWidth = mSourceImg.width();
    int srcHeight = mSourceImg.height();
    qreal k1 = (qreal)destWidth / srcWidth;
    qreal k2 = (qreal)destHeight / srcHeight;
    qreal k = qMin(k1, k2);
    size.setWidth(k * srcWidth);
    size.setHeight(k * srcHeight);
    ui->scrollAreaContent->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollAreaContent->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  } else {
    size = mSourceImg.size();
    ui->scrollAreaContent->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->scrollAreaContent->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  }

  ui->labelCameraPreview->setScaledContents(true);
  ui->scrollAreaWidgetContents->setMinimumSize(size);
  ui->scrollAreaWidgetContents->setMaximumSize(size);
}

void CameraForm::DrawStatusFrame()
{
  if (mSourceImg.isNull()) {
    return;
  }

  if (mNoVideo.isNull()) {
    mNoVideo = QIcon(":/Icons/No Video.png");
  }

  QSize iconSize = mNoVideo.availableSizes().value(0);
  //  if (mSourceImg.isNull() || mSourceImg.width() < iconSize.width() || mSourceImg.height() < iconSize.height()) {
  //    mSourceImg = QImage(iconSize, QImage::Format_ARGB32);
  //    mSourceImg.fill(Qt::black);
  //  }
  if (iconSize.isValid()) {
    QSize imgSize = mSourceImg.size();
    QPainter painter(&mSourceImg);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    int d = qMin(qMin(imgSize.width()/2, imgSize.height()/2), qMin(iconSize.width(), iconSize.height()));
    painter.drawPixmap(imgSize.width()/2 - d/2, imgSize.height()/2 - d/2, d, d, mNoVideo.pixmap(iconSize));
  }
  ui->labelCameraPreview->SetImage(mSourceImg);
  ResizeImage();
}

void CameraForm::OnAutoRefreshTimer()
{
  LoadThumbnail();
}

void CameraForm::OnPlayStarted()
{
  SetPlay(true);
}

void CameraForm::OnPlayFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  Q_UNUSED(exitCode);
  Q_UNUSED(exitStatus);

//  mPlayProcess->deleteLater();
//  mPlayProcess = nullptr;

  SetPlay(false);
}

void CameraForm::OnPlayContinue()
{
  if (mChater) {
    mChater->SendSimpleMessage(eMsgContinue);
  }
}

void CameraForm::OnThumbnailImage(QImage image)
{
  mSourceImg = image;

  if (!DrawImage()) {
    ui->lineEditCamerStatus->setText("Ошибка изображения");
    ui->lineEditCamerStatus->setStyleSheet("background-color: lightgray;\ncolor: red;");
  }
}

void CameraForm::OnThumbnailDecoded()
{
  mSourceFrame = mDecodeReceiver->LastFrame();
  if (!mSourceFrame) {
    return;
  }

  switch (mSourceFrame->GetHeader()->Compression) {
  case eRawRgba:
    if (DrawFrame()) {
      return;
    }
    break;
  default:
    break;
  }

  ui->lineEditCamerStatus->setText("Ошибка изображения");
  ui->lineEditCamerStatus->setStyleSheet("background-color: lightgray;\ncolor: red;");
}

void CameraForm::OnThumbnailFail(const QString& msg)
{
  ui->lineEditCamerStatus->setText(msg);
  ui->lineEditCamerStatus->setStyleSheet("background-color: lightgray;\ncolor: red;");
  if (--mAutoRefreshCount > 0) {
    mAutoRefresh->start(kAutoRefreshPeriodMs);
  }
}

void CameraForm::OnThumbnailFrame(bool hasVideo)
{
  if (!hasVideo) {
    mStatusFrame = true;
    DrawStatusFrame();
    ui->lineEditCamerStatus->setText("Камера не доступна");
    ui->lineEditCamerStatus->setStyleSheet("background-color: lightgray;\ncolor: red;");
    return;
  } else if (mPlaying) {
    mStatusFrame = false;
  }

  if (!mPlaying) {
    ui->lineEditCamerStatus->setText(QString::fromUtf8("Декодирование"));
    ui->lineEditCamerStatus->setStyleSheet("background-color: lightgray;\ncolor: blue;");
  }
}

void CameraForm::OnNeedSave()
{
  ui->frameControls->setVisible(true);
  mNeedSave = true;
}

void CameraForm::on_actionRefresh_triggered()
{
  LoadThumbnail();
}

void CameraForm::on_actionScale_triggered(bool checked)
{
  mScaled = checked;
  if (!mSourceImg.isNull()) {
    ResizeImage();
  }
}

void CameraForm::on_actionSave_triggered()
{
  ui->frameControls->setVisible(false);
  ui->labelCameraPreview->SavePoints();
  mNeedSave = false;
}

void CameraForm::on_actionCancel_triggered()
{
  ui->frameControls->setVisible(false);
  ui->labelCameraPreview->LoadPoints(true);
  mNeedSave = false;
}

void CameraForm::on_actionPlay_triggered()
{
  if (!mChater) {
    CreateChat();
  }

  LiveRequest* req;
  if (mChater->PrepareMessage(eMsgLiveRequest, req)) {
    req->CameraId = mCameraObject->Id;
    req->Priority = 0;
    if (mChater->SendMessage()) {
      SetPlay(true);
      mPlayConfirmTimer->start(kConfirmFramesMs/2);
    }
  }

//  if (mPlayProcess) {
//    return;
//  }

//  mPlayProcess = new QProcess(this->parentWidget());
//  mPlayProcess->setProgram(QDir(QCoreApplication::applicationDirPath()).filePath(kPlayerExe));
//  QString params = QString("-psingle;cam=%1;arm=%2").arg(mCameraObject->Id).arg(mArmObjectId);
//  mPlayProcess->setArguments(QStringList() << "--id=-1" << "-t" << params);

//  connect(mPlayProcess, &QProcess::started, this, &CameraForm::OnPlayStarted);
//  connect(mPlayProcess, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished)
//          , this, &CameraForm::OnPlayFinished);
//  connect(this, &CameraForm::destroyed, mPlayProcess, &QProcess::kill);

//  mPlayProcess->start();
}

void CameraForm::on_actionStop_triggered()
{
  if (!mChater) {
    return;
  }

  mChater->SendSimpleMessage(eMsgStop);
  mPlayConfirmTimer->stop();
  SetPlay(false);
  ui->stackedWidgetCameraPreview->setCurrentIndex(0);
  DrawFrame();
//  if (!mPlayProcess) {
//    return;
//  }

//  mPlayProcess->kill();
}
