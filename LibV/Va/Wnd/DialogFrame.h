#pragma once

#include <QDialog>


namespace Ui {
class DialogFrame;
}

class DialogFrame: public QDialog
{
  Ui::DialogFrame* ui;

  Q_OBJECT

public:
  explicit DialogFrame(QWidget* parent = 0);
  ~DialogFrame();

public:
  void SetImage(const QImage& image);
};
