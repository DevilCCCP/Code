#pragma once

#include <QMainWindow>
#include <QTimer>
#include <QVBoxLayout>

#include <Lib/Db/Db.h>
#include <Lib/Ui/MainWindow2.h>


DefineClassS(AdminWidget);
DefineClassS(CtrlManager);
DefineClassS(ObjectModel);
DefineClassS(PropertyForm);
DefineClassS(ToolForm);
DefineClassS(TreeSchema);
DefineClassS(ToolSchema);
DefineClassS(UpWaiter);
DefineClassS(UpInfo);
DefineClassS(QStandardItemModel);

namespace Ui {
class AdminWindow;
}

class AdminWindow: public MainWindow2
{
  Ui::AdminWindow*     ui;

  Db&                 mDb;
  CtrlManagerS        mManager;
  UpWaiter*           mUpWaiter;

  PROPERTY_SGET(ObjectTypeTable)
  PROPERTY_SGET(ObjectTable)

  ObjectModel*        mObjectModel;
  PropertyForm*       mPropertyForm;
  ToolForm*           mToolForm;
  QStandardItemModel* mConnectionsModel;
  bool                mInit;
  QList<AdminWidget*> mExtraWidgets;

  ObjectItemS         mSelectedItem;
  int                 mSelectedId;
  int                 mSelectedParentId;
  int                 mSelectedNextId;

  QWidget*            mCurrentWidget;
  QWidget*            mCurrentObjWidget;

  Q_OBJECT

public:
  explicit AdminWindow(Db& _Db, UpInfo* _UpInfo, const CtrlManagerS& _Manager, QWidget* parent = 0);
  ~AdminWindow();

public:
  CtrlManagerS Manager() { return mManager; }
  const Db& GetDb() { return mDb; }

protected:
  QVBoxLayout*  SystemObjectLayout() const;
  PropertyForm* GetPropertyForm()    const { return mPropertyForm; }
  ToolForm*     GetToolForm()        const { return mToolForm; }

protected:
  /*override */virtual void showEvent(QShowEvent* event) Q_DECL_OVERRIDE;

protected:
  /*new */virtual void GetTreeSchema(TreeSchema& schema) const = 0;
  /*new */virtual void GetToolSchema(ToolSchema& schema) const = 0;
  /*new */virtual void GetPropertiesSchema(QStringList& schema) const;
  /*new */virtual QWidget* GetSelectObjectWidget(const ObjectItemS& object);
  /*new */virtual QWidget* GetAboutWidget();
  /*new */virtual void InitExtras();

  /*new */virtual void OnObjectTypeUpdated(const ObjectTypeTableS& objectTypeTable);
  /*new */virtual void OnObjectsUpdated(const ObjectTableS& objectTable);

protected:
  void AddTabWidget(int index, AdminWidget* widget, const QIcon& icon, const QString& label);
  bool GetObjectTypeId(const char* abbr, int& id);

signals:
  void ReloadTree();

private slots:
  void ScriptDone();
  void CreateTree();

private:
  void Init();
  void InitTree();
  void InitScript();
  void InitAbout();
  void InitConnections();

  void InitTools();
  void InitProperties();

  void OnSysTreeCurrentChanged(const QModelIndex& selected, const QModelIndex& deselected);
  void OnPropertyNameEdited();

private slots:
  void on_tabWidgetMain_currentChanged(int index);
  void on_listViewConnections_activated(const QModelIndex& index);
  void on_labelConnectionChange_linkActivated(const QString&);
};

