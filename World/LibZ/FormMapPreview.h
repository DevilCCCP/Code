#pragma once

#include <QWidget>

#include <Lib/Include/Common.h>

#include "MapParameters.h"


class MapGenerator;

namespace Ui {
class FormMapPreview;
}

class FormMapPreview: public QWidget
{
  Ui::FormMapPreview* ui;
  MapGenerator*       mMapGenerator;

  Q_OBJECT

public:
  explicit FormMapPreview(QWidget* parent = 0);
  ~FormMapPreview();

public:
  void SetParameters(const MapParameters& mapParameters);
  void Start(const MapParameters& mapParameters);

private:
  void UpdatePreview();
  void UpdateButtons();

private:
  void OnGenerateStarted();
  void OnGenerateFinished();
  void OnGeneratePercent(int stage, int percent);

private slots:
  void on_pushButtonGenerate_clicked();
  void on_pushButtonBack_clicked();
  void on_pushButtonForward_clicked();
  void on_pushButtonCancel_clicked();
  void on_pushButtonDone_clicked();
};
