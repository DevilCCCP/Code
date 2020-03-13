#pragma once

#include <QWidget>
#include <QVBoxLayout>

#include <Lib/Db/Db.h>

#include "ToolSchema.h"
#include "ActionIcons.h"
#include "ToolAction.h"


namespace Ui {
class ToolForm;
}

DefineClassS(AdminWindow);

class ToolForm: public QWidget
{
  Ui::ToolForm* ui;

  AdminWindow*           mAdminWindow;
  const Db&              mDb;
  ObjectTableS           mObjectTable;
  ObjectItemS            mSelectedItem;
  bool                   mIsSelectedTemplate;

  ToolSchemaS            mToolSchema;
  QWidget*               mToolWidget;
  QVBoxLayout*           mToolLayout;
  ActionIcons            mActionIcons;
  ToolActionList         mActions;
  ToolActionListList     mDialogActions;
  QList<QAction*>        mCustomActions;
  ToolActionMap          mToolActionMap;
  QMap<int, ObjectItemS> mTypeObjects;
  ObjectItemS            mTypeObjectDefault;

  QString                mLastNameTemplate;
  QString                mLastNameTemplateBase;
  int                    mLastNameAdding;
  bool                   mFreeze;

  Q_OBJECT

public:
  ToolSchemaS GetToolSchema() { return mToolSchema; }

public:
  void UpdateSchema(const ObjectTypeTableS& objectTypeTable);
  void Clear();
  bool SetObject(const ObjectTableS& objectTable, const ObjectItemS& item);
  void AddCustomAction(QAction* action);
  void Freeze(bool freeze);

private:
  void AddTools();
  void AddParentLink(const ToolLinkItem* link, int parentTypeId, bool parentExists);
  void AddChildLink(const ToolLinkItem* link, int childTypeId);
  void AddLinkTo(int parentTypeId, bool uniq, bool create);
  void AddLinkToSameMaster(int parentTypeId);
  void AddLinkFrom(int childTypeId, bool uniq, bool create);
  void AddLinkFromSameMaster(int childTypeId);
  bool IsChildExist(int parentId, int childTypeId);
  bool IsSameMaster(int idSibling, bool from, bool& connected);

  void AddOneAction(const ToolAction& action);
  void AddMultyActions(const QList<ToolAction>& actions);
  void AddFormActions(const QList<ToolAction>& actions);
  void AddFormActions(QMenu* menu, const QList<ToolAction>& actions);

  void OnAction();
  void OnFormAction();
  void OnCustomAction();
  void OnMultyAction();
  void OnDialogAction();
  void OpenActionsForm(const ToolActionList& actions);
  void DoAction(const ToolAction& action);
  void CreateSingle(const ObjectItemS& baseItem);
  void CreateSlave(int masterId, const ObjectItemS& baseItem);
  void RemoveObject(const ObjectItemS& item);
  void CreateLink(int masterId, int slaveId);
  void RemoveLink(int masterId, int slaveId);
  void SetObjectState(const ObjectItemS& item, int state);
  void RebootTree(const ObjectItemS& item);
  QString GenerateName(const QString& baseName);

  const ObjectItemS& GetTypeObject(const ObjectItemS& item);

public:
  explicit ToolForm(AdminWindow* _AdminWindow, const Db& _Db, const ToolSchemaS& _ToolSchema, QWidget *parent = 0);
  ~ToolForm();
};

