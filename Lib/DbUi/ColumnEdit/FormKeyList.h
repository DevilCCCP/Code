#pragma once

#include <QWidget>


namespace Ui {
class FormKeyList;
}

class QAbstractTableModel;
class QStandardItemModel;

class FormKeyList : public QWidget
{
  Ui::FormKeyList*     ui;

  QAbstractTableModel* mModel;
  QStandardItemModel*  mListModel;

  Q_OBJECT

public:
  explicit FormKeyList(QAbstractTableModel* _Model, QWidget* parent = 0);
  ~FormKeyList();

public:
  void SetList(const QStringList& list);
  void GetList(QStringList& list);

private:
  void AddItem(int ind);

private:
  void OnComboNewIndexChanged(int index);
  void OnListSelectionChanged();

private slots:
  void on_actionAdd_triggered();
  void on_actionRemove_triggered();
};

