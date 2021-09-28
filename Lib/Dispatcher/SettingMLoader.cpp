#include <QSet>

#include "SettingMLoader.h"


bool SettingMLoader::UpdateModules()
{
  bool update = false;
  if (GetSettings()->Reload()) {
    SettingsA* settings = GetSettings();
    settings->SetSilent(true);

    QList<int> oldServicesList = mServices.keys();
#if QT_VERSION >= QT_VERSION_CHECK(5, 14,0)
    QSet<int> oldServices(oldServicesList.begin(), oldServicesList.end());
#else
    QSet<int> oldServices = QSet<int>::fromList(oldServicesList);
#endif
    for (int i = 1; settings->BeginGroup(QString("Service%1").arg(i)); i++) {
      int id = settings->GetValue("id").toInt();
      QString path = settings->GetValue("path").toString();
      QString params = settings->GetValue("params").toString();
      settings->EndGroup();

      if (id == 0 || path.isEmpty()) {
        break;
      }

      oldServices.remove(id);
      auto itr = mServices.find(id);
      if (itr == mServices.end()) {
        AddModule(id, path, params.split(QChar(' ')), "");
        mServices[id] = qMakePair(path, params);
        update = true;
      } else if (itr.value().first != path || itr.value().second != params) {
        UpdateModule(id, path, params.split(QChar(' ')), "");
        mServices[id] = qMakePair(path, params);
        update = true;
      }
    }
    for (auto itr = oldServices.begin(); itr != oldServices.end(); itr++) {
      int id = *itr;
      mServices.remove(id);
      RemoveModule(id);
      update = true;
    }
  }

  return update;
}


SettingMLoader::SettingMLoader(SettingsAS& _Settings)
  : ModuleLoaderB(_Settings)
{
}

SettingMLoader::~SettingMLoader()
{
}
