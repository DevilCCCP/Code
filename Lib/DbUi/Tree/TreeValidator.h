#pragma once

#include <QString>

#include <Lib/Db/DbTableA.h>


DefineClassS(TreeValidator);

class TreeValidator {
public:
  /*new */virtual bool IsValid(const DbItemBS& item, const QList<DbItemBS>& parentItemList, QString& errorText) = 0;

public:
  TreeValidator();
  virtual ~TreeValidator() = 0;
};
