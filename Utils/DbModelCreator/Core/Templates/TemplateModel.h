#pragma once

#include <Lib/Db/Db.h>
#include <Lib/DbUi/DbTableModel.h>
#include <LibPREFIX/DbPREFIX/ClassT.h>


DefineClassS(ClassTModel);

class ClassTModel: public DbTableModel<ClassT>
{
protected:
  /*override */virtual QString Text(int row, int column) const Q_DECL_OVERRIDE;

public:
  ClassTModel(QObject* parent = 0);
};

