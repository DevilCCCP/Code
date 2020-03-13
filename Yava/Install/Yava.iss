#define ProductName "Система Наблюдения и Анализа"
#define ProductPublisher "Devil"
#define ProductPublisherUrl ""

#define ProductAbbr "yava"
#define ProjectName "Yava"
#define ProjectGuid "{{3d64e7ff-e8d6-45c7-af47-1f726ce789aa}"
#define License

#define HeadPath SourcePath + "/../"
#define RootPath SourcePath + "/../../"
#define BuildPath HeadPath + "bin/Release/"

#include RootPath + "Install/MessagesRu.iss"
#include RootPath + "Install/Full.iss"

[UninstallRun]
Filename: {app}\{#ProductAbbr}_srvd.exe; Parameters: "stop"; Flags: shellexec waituntilterminated runhidden; \
  StatusMsg: {cm:msgStopServer}; Components: Server;
Filename: {app}\{#ProductAbbr}_srvd.exe; Parameters: "uninstall"; Flags: shellexec waituntilterminated runhidden; \
  StatusMsg: {cm:msgStopServer}; Components: Server;

#include RootPath + "Install/Common.iss"

[CustomMessages]
msgShortcutAdminka=АРМ администратора СНА
msgShortcutMonitor=АРМ мониторинга СНА
msgShortcutArm=АРМ оператора СНА

#include RootPath + "Install/CommonV.iss"
