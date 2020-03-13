#define HideUserKeys
#define NoServer

[Types]
Name: "default"; Description: {cm:TypesProgram}

[Components]
Name: Claster; Description: {cm:ComponentsClaster}; ExtraDiskSpaceRequired: 209715200; Types: default
Name: Update; Description: {cm:ComponentsUpdate}; Types: default
Name: ProgramFiles; Description: {cm:ComponentsProgramFiles}; Types: default
Name: Server; Description: {cm:ComponentsServer}; Types: default

