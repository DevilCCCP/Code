#pragma once

#include <QMainWindow>


namespace Ui {
class FrameWindow;
}

class FrameWindow: public QMainWindow
{
  Ui::FrameWindow* ui;

  Q_OBJECT

public:
  explicit FrameWindow(QWidget* parent = 0);
  ~FrameWindow();

public:
  void SetImage(const QImage& image);
};
