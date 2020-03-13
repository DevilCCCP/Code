#include <Lib/DbUi/AdminWindow.h>


DefineClassS(TreeSchema);
DefineClassS(ToolSchema);

class MainWindowZ: public AdminWindow
{
  int          mCameraType;
  int          mArmType;
  int          mAnalCType;
  int          mAnalIoType;
  int          mAnalDoorType;
  int          mAnalRegNumType;
  int          mAnalIgnType;
  int          mArmId;

protected:
  /*override */virtual void GetTreeSchema(TreeSchema& schema) const Q_DECL_OVERRIDE;
  /*override */virtual void GetToolSchema(ToolSchema& schema) const Q_DECL_OVERRIDE;
  /*override */virtual void GetPropertiesSchema(QStringList& schema) const Q_DECL_OVERRIDE;
  /*override */virtual QWidget* GetSelectObjectWidget(const ObjectItemS& object) Q_DECL_OVERRIDE;

  /*override */virtual void OnObjectTypeUpdated(const ObjectTypeTableS& objectTypeTable) Q_DECL_OVERRIDE;
  /*override */virtual void OnObjectsUpdated(const ObjectTableS& objectTable) Q_DECL_OVERRIDE;

public:
  explicit MainWindowZ(Db& _Db, UpInfo* _UpInfo, const CtrlManagerS& _Manager, QWidget *parent = 0);
};
