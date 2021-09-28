#include "ColumnEditA.h"


DefineClassS(FilesPackage);
class FormEditPack;

class ColumnEditPack: public QObject, public ColumnEditA
{
  FormEditPack*       mCtrl;
  FilesPackageS       mFilesPackage;

  Q_OBJECT

public:
  /*override */virtual QWidget* CreateControl(QWidget* parent) override;
  /*override */virtual bool LoadValue(const QVariant& value) override;
  /*override */virtual bool SaveValue(QVariant& value) override;

public:
  ColumnEditPack();
};

