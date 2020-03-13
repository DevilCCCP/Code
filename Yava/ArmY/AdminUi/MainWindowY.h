#include <Lib/DbUi/MainWindow.h>


DefineClassS(TreeSchema);
DefineClassS(ToolSchema);

class MainWindowY: public MainWindow
{
  ObjectTypeS mObjectType;
  int         mCameraType;
  int         mAnalType;
  int         mArmType;

protected:
  /*override */virtual void GetTreeSchema(TreeSchema& schema) const Q_DECL_OVERRIDE;
  /*override */virtual void GetToolSchema(ToolSchema& schema) const Q_DECL_OVERRIDE;
  /*override */virtual void GetPropertiesSchema(QStringList& schema) const Q_DECL_OVERRIDE;
  /*override */virtual QWidget* GetSelectObjectWidget(const ObjectItemS& object) Q_DECL_OVERRIDE;

public:
  explicit MainWindowY(const Db& _Db, const CtrlManagerS& _Manager, QWidget *parent = 0);
};
