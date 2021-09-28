#pragma once

#include <QIcon>

#include <Lib/Db/DbTable.h>


DefineClassS(TreeValidator);
DefineStructS(DbTreeSchema);
DefineStructS(TableEditSchema);

struct DbTreeSchema
{
  enum EConnection {
    eRoot,
    eChild,
    eLink,
    eMultiLink
  };

  DbTableBS            Table;
  TreeValidatorS       Validator;
  TableEditSchemaS     EditSchema;
  QList<DbTreeSchemaS> TreeChilds;
  EConnection          Connection;
  int                  KeyIndex;
  DbTableBS            MultiLinkTable;
  int                  MultiLinkIndex;

  QString              Name;
  QString              IconName;
  QIcon                Icon;
};
