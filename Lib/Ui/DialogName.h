#pragma once

#include <QDialog>

#include <Lib/Include/Common.h>


namespace Ui {
class DialogName;
}

class DialogName: public QDialog
{
  Ui::DialogName* ui;

  Q_OBJECT

public:
  explicit DialogName(QWidget* parent = 0);
  ~DialogName();

public:
  static QString Open(QWidget* parent, const QString& tytle, const QString& caption, QString name = QString());

  QString Tytle() const;
  QString Caption() const;
  QString Name() const;

public slots:
  void SetTytle(const QString& tytle);
  void SetCaption(const QString& caption);
  void SetName(const QString& name);

private slots:
  void on_lineEditName_textEdited(const QString& text);
  void on_buttonBoxMain_accepted();
  void on_buttonBoxMain_rejected();
};
