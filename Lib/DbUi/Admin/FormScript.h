#pragma once

#include <QWidget>
#include <QFileDialog>
#include <QStandardItemModel>

#include <Lib/Db/Db.h>


namespace Ui {
class FormScript;
}

DefineClassS(FormScript);
DefineClassS(ObjectModel);
DefineClassS(ToolForm);
DefineClassS(PropertyForm);
DefineClassS(CsvWriter);
DefineClassS(CsvReader);
DefineStructS(DefaultObject);

struct DefaultObject {
  ObjectItemS            TemplItem;
  QMap<QString, QString> Properties;
  QMap<QString, QString> PropKeyNameMap;
  QMap<QString, QString> PropNameKeyMap;

  DefaultObject(const ObjectItemS& _TemplItem): TemplItem(_TemplItem) { }
};

class FormScript: public QWidget
{
  Ui::FormScript*           ui;

  ObjectModel*              mObjectModel;
  ToolForm*                 mToolForm;
  PropertyForm*             mPropertyForm;
  QStandardItemModel*       mScriptModel;
  bool                      mIsUpdateScript;

  QMultiMap<int, DefaultObjectS> mTemplateObjects;
  QMap<QString, DefaultObjectS>  mTemplateNamesMap;
  QMap<int, QString>             mTypeNames;
  QStringList                    mCustomHeaders;

  QFileDialog               mFileDialog;
  QStringList               mUseLog;
  QMap<int, int>            mScriptIdMap;
  QVector<int>              mScriptSkipLines;
  int                       mCurrentLine;
  int                       mObjectOk;
  int                       mObjectOkOne;
  int                       mObjectPart;
  int                       mObjectPartOneOk;
  int                       mObjectPartOneFail;
  int                       mObjectFail;

  Q_OBJECT

public:
  explicit FormScript(QWidget* parent = 0);
  ~FormScript();

public:
  void Init(ObjectModel* _ObjectModel, ToolForm* _ToolForm, PropertyForm* _PropertyForm);
  void Activated();

private:
  bool GenerateScript(bool update);
  void GenerateObjectOne(int id, const ObjectItemS& item, bool isTemplate = false);
  void GenerateObjectIdent(int id, const ObjectItemS& item);
  bool GenerateProperties(int id, int typeId, bool isTemplate, DefaultObject*& templateObj, QMap<QString, QString>& diffProperties);
  bool LoadTemplates();
  bool LoadTypeNames();

  void Create(bool update);
  bool SaveFile(CsvWriter* writer);
  bool LoadFile(CsvReader* reader);

  bool UseScript();
  bool UseScriptLine(int j);
  void AddLineLogOk(const QString& text);
  void AddLineLogWarning(const QString& text);
  void AddLineLogError(const QString& text);
  void AddLineLogFatal(const QString& text);
  void AddLineLogInfo(const QString& text);

signals:
  void OnScriptDone();

private slots:
  void on_actionScriptCreate_triggered();
  void on_actionScriptSave_triggered();
  void on_actionScriptOpen_triggered();
  void on_actionUseScript_triggered();
  void on_actionScriptCreateUp_triggered();
};
