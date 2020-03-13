[Files]
; Misc
Source: "{#HeadPath}/Local/Logo.ico"; DestDir: "{app}";

; ProgramFiles
#ifndef NoYavaArm
Source: "{#BuildPath}/{#ProductAbbr}_armd.exe"; DestDir: "{app}"; Components: ProgramFiles; BeforeInstall: PreArm;
Source: "{#BuildPath}/{#ProductAbbr}_ctrl.exe"; DestDir: "{app}"; Components: ProgramFiles;
Source: "{#BuildPath}/{#ProductAbbr}_plr.exe"; DestDir: "{app}"; Components: ProgramFiles;
#endif
Source: "{#BuildPath}/{#ProductAbbr}_mon.exe"; DestDir: "{app}"; Components: ProgramFiles;
Source: "{#BuildPath}/{#ProductAbbr}_admin.exe"; DestDir: "{app}"; Components: ProgramFiles;

; Server
Source: "{#BuildPath}/{#ProductAbbr}_srvd.exe"; DestDir: "{app}"; Components: Server; BeforeInstall: PreServer;
Source: "{#BuildPath}/{#ProductAbbr}_lilo.exe"; DestDir: "{app}"; Components: Server;
Source: "{#BuildPath}/{#ProductAbbr}_repc.exe"; DestDir: "{app}"; Components: Server;
#ifndef NoYavaAnal
Source: "{#BuildPath}/{#ProductAbbr}_va.exe"; DestDir: "{app}"; Components: Server;
#else
Source: "{#BuildPath}/{#ProductAbbr}_video.exe"; DestDir: "{app}"; Components: Server;
#endif
Source: "{#HeadPath}\Misc\Db\StorageV\*.sql"; DestDir: "{app}\Scripts"; Components: Server;

#ifndef NoYavaArm
; SDL 2        
Source: "{#BuildPath}/SDL2.dll"; DestDir: "{app}"; Components: ProgramFiles Server;
#endif

; ffmpeg
Source: "{#BuildPath}/avcodec-57.dll"; DestDir: "{app}"; Components: ProgramFiles Server;
Source: "{#BuildPath}/avdevice-57.dll"; DestDir: "{app}"; Components: ProgramFiles Server;
Source: "{#BuildPath}/avfilter-6.dll"; DestDir: "{app}"; Components: ProgramFiles Server;
Source: "{#BuildPath}/avformat-57.dll"; DestDir: "{app}"; Components: ProgramFiles Server;
Source: "{#BuildPath}/avutil-55.dll"; DestDir: "{app}"; Components: ProgramFiles Server;
Source: "{#BuildPath}/postproc-54.dll"; DestDir: "{app}"; Components: ProgramFiles Server;
Source: "{#BuildPath}/swresample-2.dll"; DestDir: "{app}"; Components: ProgramFiles Server;
Source: "{#BuildPath}/swscale-4.dll"; DestDir: "{app}"; Components: ProgramFiles Server;

; mfx
Source: "{#BuildPath}/libmfxsw32.dll"; DestDir: "{app}"; Components: ProgramFiles Server;

[Icons]
Name: "{group}\{cm:msgShortcutAdminka}"; Filename: "{app}\{#ProductAbbr}_admin.exe"; Components: ProgramFiles;
Name: "{commondesktop}\{cm:msgShortcutAdminka}"; Filename: "{app}\{#ProductAbbr}_admin.exe"; Components: ProgramFiles;
Name: "{group}\{cm:msgShortcutMonitor}"; Filename: "{app}\{#ProductAbbr}_mon.exe"; Components: ProgramFiles;
Name: "{commondesktop}\{cm:msgShortcutMonitor}"; Filename: "{app}\{#ProductAbbr}_mon.exe"; Components: ProgramFiles;
#ifndef NoYavaArm
Name: "{group}\{cm:msgShortcutArm}"; Filename: "{app}\{#ProductAbbr}_armd.exe"; Components: ProgramFiles;
Name: "{commondesktop}\{cm:msgShortcutArm}"; Filename: "{app}\{#ProductAbbr}_armd.exe"; Components: ProgramFiles;
#endif
Name: "{group}\uninstall"; Filename: "{app}\unins000.exe"

[UninstallDelete]
Type: files; Name: {app}\.va_*; Components: ProgramFiles;
