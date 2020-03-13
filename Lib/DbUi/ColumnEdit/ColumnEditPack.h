#include "ColumnEditA.h"


DefineClassS(FilesPackage);
class FormEditPack;

class ColumnEditPack: public QObject, public ColumnEditA
{
  FormEditPack*       mCtrl;
  FilesPackageS       mFilesPackage;

  Q_OBJECT

public:
  /*override */virtual QWidget* CreateControl(QWidget* parent) Q_DECL_OVERRIDE;
  /*override */virtual bool LoadValue(const QVariant& value) Q_DECL_OVERRIDE;
  /*override */virtual bool SaveValue(QVariant& value) Q_DECL_OVERRIDE;

public:
  ColumnEditPack();
};

