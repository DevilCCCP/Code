[Types]
Name: "full"; Description: {cm:TypesFull}
Name: "client"; Description: {cm:TypesClient}
Name: "server"; Description: {cm:TypesServer}
Name: "custom"; Description: {cm:TypesCustom}; Flags: iscustom

[Components]
Name: Claster; Description: {cm:ComponentsClaster}; ExtraDiskSpaceRequired: 209715200; Types: full custom;
Name: Update; Description: {cm:ComponentsUpdate}; Types: full server client custom
Name: ProgramFiles; Description: {cm:ComponentsProgramFiles}; Types: full client custom;
Name: Server; Description: {cm:ComponentsServer}; Types: full server custom;

[UninstallRun]
Filename: {app}\{#ProductAbbr}_srvd.exe; Parameters: "stop"; Flags: shellexec waituntilterminated runhidden; \
  StatusMsg: {cm:msgStopServer}; Components: Server;
Filename: {app}\{#ProductAbbr}_srvd.exe; Parameters: "uninstall"; Flags: shellexec waituntilterminated runhidden; \
  StatusMsg: {cm:msgStopServer}; Components: Server;
