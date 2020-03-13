#pragma once

#include <QObject>
#include <QPoint>

#include <Lib/Include/Common.h>


DefineClassS(Editing);

class Editing: public QObject
{
public:
  enum EMode {
    eModeNormal = 0,
    eModeProp1,
    eModeProp2,
    eModeProp3,
    eModeErase,
  };

private:
  static Editing*           mSelf;

  PROPERTY_GET(EMode,        Mode)
  PROPERTY_GET(bool,         HasUndo)
  PROPERTY_GET(bool,         HasRedo)

  PROPERTY_GET_SET(int,      CurrentPropLevel)

  Q_OBJECT

public:
  static Editing* Instance() { return mSelf; }

public slots:
  void ModeChange(EMode value);
  void UndoChanged(bool hasUndo, bool hasRedo);

signals:
  void ModeChanged();
  void HasUndoChanged();
  void HasRedoChanged();

public:
  Editing();
};

#define qEditing (Editing::Instance())
