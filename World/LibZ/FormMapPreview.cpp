#include "FormMapPreview.h"
#include "ui_FormMapPreview.h"
#include "MapGenerator.h"


FormMapPreview::FormMapPreview(QWidget* parent)
  : QWidget(parent), ui(new Ui::FormMapPreview)
  , mMapGenerator(new MapGenerator(this))
{
  Q_INIT_RESOURCE(LibZ);
  Q_INIT_RESOURCE(Ui);

  ui->setupUi(this);

  UpdateButtons();

  ui->progressBarStageProgress->setVisible(false);
  ui->pushButtonCancel->setVisible(false);

  connect(mMapGenerator, &MapGenerator::Started, this, &FormMapPreview::OnGenerateStarted);
  connect(mMapGenerator, &MapGenerator::Finished, this, &FormMapPreview::OnGenerateFinished);
  connect(mMapGenerator, &MapGenerator::Percent, this, &FormMapPreview::OnGeneratePercent);
}

FormMapPreview::~FormMapPreview()
{
  delete ui;
}


void FormMapPreview::SetParameters(const MapParameters& mapParameters)
{
  mMapGenerator->SetParameters(mapParameters);
}

void FormMapPreview::Start(const MapParameters& mapParameters)
{
  mMapGenerator->Start(mapParameters, false);
}

void FormMapPreview::UpdatePreview()
{
  ui->labelStage->setText(mMapGenerator->GetStageName());
  ui->progressBarStage->setMaximum(mMapGenerator->getStageMax());
  ui->progressBarStage->setValue(mMapGenerator->GetCurrentStage());
  ui->formEarth->SetLandscape(mMapGenerator->GetEarthLandscape());
}

void FormMapPreview::UpdateButtons()
{
  ui->pushButtonBack->setEnabled(mMapGenerator->GetCurrentStage() > 1);
  ui->pushButtonForward->setEnabled(mMapGenerator->GetCompleteStage() >= mMapGenerator->GetCurrentStage());
  ui->pushButtonForward->setVisible(mMapGenerator->GetCurrentStage() < mMapGenerator->getStageMax());
  ui->pushButtonDone->setEnabled(mMapGenerator->GetCompleteStage() >= mMapGenerator->getStageMax());
  ui->pushButtonDone->setVisible(mMapGenerator->GetCurrentStage() >= mMapGenerator->getStageMax());
}

void FormMapPreview::OnGeneratePercent(int stage, int percent)
{
  ui->progressBarStage->setValue(stage);
  ui->progressBarStageProgress->setValue(percent);
}

void FormMapPreview::OnGenerateStarted()
{
  ui->progressBarStageProgress->setValue(0);
  ui->progressBarStageProgress->setVisible(true);
  ui->pushButtonGenerate->setVisible(false);
  ui->pushButtonCancel->setVisible(true);
}

void FormMapPreview::OnGenerateFinished()
{
  ui->progressBarStageProgress->setVisible(false);
  ui->pushButtonGenerate->setVisible(true);
  ui->pushButtonCancel->setVisible(false);

  UpdatePreview();
  UpdateButtons();
}

void FormMapPreview::on_pushButtonGenerate_clicked()
{
  mMapGenerator->Generate();
}

void FormMapPreview::on_pushButtonBack_clicked()
{
  mMapGenerator->Back();

  UpdatePreview();
  UpdateButtons();
}

void FormMapPreview::on_pushButtonForward_clicked()
{
  mMapGenerator->Forward();

  UpdatePreview();
  UpdateButtons();
}

void FormMapPreview::on_pushButtonCancel_clicked()
{
  mMapGenerator->Cancel();
}
