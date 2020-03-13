//#define ProductName "Product"
//#define ProductPublisher "Company"
//#define ProductPublisherUrl "www.company.org"

//#define ProductAbbr "yaproj1"
//#define ProjectName "Proj1"
//#define ProjectGuid "{{11223344-6666-1313-1313-666166626663}"

#define AppVer ReadIni(HeadPath + "/Local/Version.ini", "General", "Ver", "unknown")

#define DbCode = Exec(SourcePath + "!db_prepare.bat", ProjectName, SourcePath, 1, SW_HIDE)
#if DbCode
 #error "Exec db fail"
#endif
#define BuildCode = Exec(HeadPath + "Misc/InnoSetup/!build_project.bat", ProjectName, SourcePath, 1, SW_HIDE)
#if BuildCode
 #error "Exec build fail"
#endif

#define Qt5 GetEnv('Qt5')
#define psql "postgresql-9.2.6-3-windows.exe"
#ifdef PostgresBig
#define pg_conf "postgresqlb.conf"
#else
#define pg_conf "postgresql.conf"
#endif

[Setup]
AppId={#ProjectGuid}
AppName={#ProductName}
AppPublisher={#ProductPublisher}
AppPublisherURL={#ProductPublisherUrl}
AppVersion={#AppVer}
DefaultDirName={pf}\{#ProjectName}
DefaultGroupName={#ProductName}
UninstallDisplayIcon={app}\Logo.ico
Compression=lzma2/fast
SolidCompression=yes
OutputDir={#SourcePath}
OutputBaseFilename={#ProjectName}Install_{#AppVer}
PrivilegesRequired=admin

[Dirs]
Name: "{app}\Log"; Permissions: users-modify; Components: ProgramFiles Server;
Name: "{app}\Var"; Permissions: users-modify; Components: ProgramFiles Server;
#ifndef NoUpdate
Name: "{app}\Scripts"; Permissions: users-modify; Components: ProgramFiles Server;
Name: "{app}\Updates"; Permissions: users-modify; Components: ProgramFiles Server Update;
#endif

[Files]
; All
#ifndef NoUpdate
Source: "{#HeadPath}\Install\Db\update.sql"; DestDir: "{app}\Updates"; Components: ProgramFiles Server Update;
#endif

; Qt5
Source: "{#Qt5}\bin\Qt5Sql.dll"; DestDir: "{app}"; Components: ProgramFiles Server Update;
Source: "{#Qt5}\bin\Qt5Network.dll"; DestDir: "{app}"; Components: ProgramFiles Server Update;
Source: "{#Qt5}\bin\Qt5Core.dll"; DestDir: "{app}"; Components: ProgramFiles Server Update;
Source: "{#Qt5}\bin\Qt5Gui.dll"; DestDir: "{app}"; Components: ProgramFiles;
Source: "{#Qt5}\bin\Qt5Widgets.dll"; DestDir: "{app}"; Components: ProgramFiles;
Source: "{#Qt5}\bin\Qt5WinExtras.dll"; DestDir: "{app}"; Components: ProgramFiles;

Source: "{#Qt5}\plugins\sqldrivers\qsqlpsql.dll"; DestDir: "{app}\plugins\sqldrivers"; Components: ProgramFiles Server Update;
Source: "{#Qt5}\plugins\imageformats\qjpeg.dll"; DestDir: "{app}\plugins\imageformats"; Components: ProgramFiles Server Update;
Source: "{#Qt5}\plugins\platforms\qwindows.dll"; DestDir: "{app}\platforms"; Components: ProgramFiles Server Update;
Source: "{#ExternPath}\Install\libeay32.dll"; DestDir: "{app}"; Components: ProgramFiles Server Update;
Source: "{#ExternPath}\Install\libintl.dll"; DestDir: "{app}"; Components: ProgramFiles Server Update;
Source: "{#ExternPath}\Install\libpq.dll"; DestDir: "{app}"; Components: ProgramFiles Server Update;
Source: "{#ExternPath}\Install\ssleay32.dll"; DestDir: "{app}"; Components: ProgramFiles Server Update;

; OpenSSL
Source: "{#BuildPath}\libeay32MD.dll"; DestDir: "{app}"; Components: Claster ProgramFiles Server Update;
Source: "{#BuildPath}\ssleay32MD.dll"; DestDir: "{app}"; Components: Claster ProgramFiles Server Update;

; Claster
Source: "{#Qt5}\bin\Qt5Core.dll"; DestDir: "{app}\inst"; Flags: deleteafterinstall; Components: Claster;
Source: "{#ExternPath}\Install\KeyGenerator.exe"; DestDir: "{app}\inst"; Flags: deleteafterinstall; Components: Claster;
Source: "{#BuildPath}\libeay32MD.dll"; DestDir: "{app}\inst"; Flags: deleteafterinstall; Components: Claster;
Source: "{#BuildPath}\ssleay32MD.dll"; DestDir: "{app}\inst"; Flags: deleteafterinstall; Components: Claster;
Source: "{#Qt5}\bin\Qt5Core.dll"; Flags: dontcopy; Components: Claster;
Source: "{#Qt5}\bin\Qt5Network.dll"; Flags: dontcopy; Components: Claster;
Source: "{#ExternPath}\Install\SetupEx.dll"; Flags: dontcopy ; Components: Claster;
Source: "{src}\{#psql}"; DestDir: "{tmp}"; Flags: deleteafterinstall external; Components: Claster; BeforeInstall: CheckPostgresDir;
Source: "{#HeadPath}\Misc\Postgres\Moscow"; DestDir: "{app}\inst"; Flags: deleteafterinstall; Components: Claster;
Source: "{#HeadPath}\Misc\Postgres\pg_hba4.conf"; DestDir: "{app}\inst"; Flags: deleteafterinstall; Components: Claster;
Source: "{#HeadPath}\Misc\Postgres\pg_hba6.conf"; DestDir: "{app}\inst"; Flags: deleteafterinstall; Components: Claster;
Source: "{#HeadPath}\Misc\Postgres\{#pg_conf}"; DestDir: "{app}\inst"; Flags: deleteafterinstall; Components: Claster;
Source: "{#HeadPath}\Misc\Postgres\pgsql_inst.bat"; DestDir: "{app}\inst"; Flags: deleteafterinstall; Components: Claster;
Source: "{#HeadPath}\Misc\Postgres\pgsql_uninst.bat"; DestDir: "{app}\inst"; Components: Claster;
Source: "{#HeadPath}\Install\Db\create_groups.sql"; DestDir: "{app}\inst"; Flags: deleteafterinstall; Components: Claster;
Source: "{#HeadPath}\Install\Db\create_db.sql"; DestDir: "{app}\inst"; Flags: deleteafterinstall; Components: Claster;
Source: "{#HeadPath}\Install\Db\install.sql"; DestDir: "{app}\inst"; Flags: deleteafterinstall; Components: Claster;

; Common

; Misc
Source: "{#HeadPath}\Local\Version.ini"; DestDir: "{app}";

#ifndef NoUpdate
; Update
Source: "{#BuildPath}\{#ProductAbbr}_upd.exe"; DestDir: "{app}"; Components: Update; BeforeInstall: PreUpdate;
Source: "{#BuildPath}\{#ProductAbbr}_upl.exe"; DestDir: "{app}"; Components: Update;
Source: "{#BuildPath}\{#ProductAbbr}_inst.exe"; DestDir: "{app}"; Components: Update;
#endif

[Run]
; Claster
Filename: {tmp}\{#psql}; StatusMsg: {cm:msgInstallPostgres}; \
  Parameters: "--mode unattended --unattendedmodeui minimal --extract-only 1 --prefix ""{app}\PostgreSQL"""; \
  Flags: shellexec waituntilterminated; Components: Claster;
Filename: {app}\inst\KeyGenerator.exe; WorkingDir: {app}\inst; Parameters: "{code:GetClasterKeyParams}"; Flags: shellexec waituntilterminated runhidden; \
  StatusMsg: {cm:msgRunPostgres92}; Components: Claster;
Filename: {app}\inst\pgsql_inst.bat; WorkingDir: {app}\inst; Parameters: """{app}\PostgreSQL"" {code:GetClasterInstParams}"; Flags: shellexec waituntilterminated runhidden; \
  StatusMsg: {cm:msgRunPostgres92}; AfterInstall: ShowUserKeys('{app}\inst'); Components: Claster;

; ProgramFiles
Filename: {sys}\icacls.exe; WorkingDir: {app}; Parameters: """{app}"" /grant *S-1-1-0:F"; Flags: shellexec waituntilterminated runhidden; \
  StatusMsg: {cm:msgCopyConnect}; Components: Server ProgramFiles Update;

; Server
#ifndef NoServer
Filename: {app}\{#ProductAbbr}_srvd.exe; WorkingDir: {app}; Parameters: "install --skip-license"; Flags: shellexec waituntilterminated runhidden; \
  StatusMsg: {cm:msgInstallServer}; Components: Server;
Filename: {app}\{#ProductAbbr}_srvd.exe; WorkingDir: {app}; Parameters: "start"; Flags: shellexec waituntilterminated runhidden; \
  StatusMsg: {cm:msgStartServer}; Components: Server;
#endif
#ifndef NoUpdate
; Update
Filename: {app}\{#ProductAbbr}_upd.exe; WorkingDir: {app}; Parameters: "install --skip-license"; Flags: shellexec waituntilterminated runhidden; \
  StatusMsg: {cm:msgInstallServer}; Components: Update;
Filename: {app}\{#ProductAbbr}_upd.exe; WorkingDir: {app}; Parameters: "start"; Flags: shellexec waituntilterminated runhidden; \
  StatusMsg: {cm:msgStartServer}; Components: Update;
#endif

[UninstallRun]
; Update
Filename: {app}\{#ProductAbbr}_upd.exe; Parameters: "stop"; Flags: shellexec waituntilterminated runhidden; \
  StatusMsg: {cm:msgStopServer}; Components: Update;
Filename: {app}\{#ProductAbbr}_upd.exe; Parameters: "uninstall"; Flags: shellexec waituntilterminated runhidden; \
  StatusMsg: {cm:msgStopServer}; Components: Update;
; Server
Filename: {app}\{#ProductAbbr}_srvd.exe; Parameters: "stop"; Flags: shellexec waituntilterminated runhidden; \
  StatusMsg: {cm:msgStopServer}; Components: Server;
Filename: {app}\{#ProductAbbr}_srvd.exe; Parameters: "uninstall"; Flags: shellexec waituntilterminated runhidden; \
  StatusMsg: {cm:msgStopServer}; Components: Server;
; Claster
Filename: {app}\inst\pgsql_uninst.bat; Parameters: """{app}\PostgreSQL"" {#ProductAbbr}"; Flags: shellexec waituntilterminated runhidden; \
  StatusMsg: {cm:msgStopPostgres92}; Components: Claster;

[UninstallDelete]
Type: filesandordirs; Name: {app}\PostgreSQL; Components: Claster;
Type: filesandordirs; Name: {app}\inst; Components: Claster;
Type: filesandordirs; Name: {app}\Log; Components: Server ProgramFiles Update;
Type: filesandordirs; Name: {app}\Var; Components: Server ProgramFiles Update;
#ifndef NoUpdate
Type: filesandordirs; Name: {app}\Scripts; Components: Server ProgramFiles Update;
Type: filesandordirs; Name: {app}\Updates; Components: Server ProgramFiles Update;
#endif
Type: dirifempty; Name: {app};

[Code]
type
   TNewStaticTextArray = array[0..3] of TNewStaticText;
var
  gCancelWithoutPrompt: boolean;

  gClasterPage: TWizardPage;
  gClasterPage_data: TNewEdit;
  gClasterPage_comboBox: TNewComboBox;
  gClasterPage_edit: TNewEdit;
  gClasterPage_portInfoText: TNewStaticText;
  gClasterPage_portInfo: TNewStaticTextArray;
  gClasterKeyParams: String;
  gClasterInstParams: String;
  gClasterPort: String;
  gClasterDataPath: String;

  gClasterDataPage: TInputDirWizardPage;

  gServerPage: TWizardPage;
  gServerPage_edit: TNewEdit;
  gServerPage_edit2: TNewEdit;
  gServerPage_comboBox: TNewComboBox;
  gServerName: String;
  gServerGuid: String;
  gServerIp: String;

  gKeysPath: String;
  gLicDone: Boolean;
  gConnectionDone: Boolean;

// *** import section ***
procedure Qt5Network_Fake();
external 'Fake@files:Qt5Network.dll stdcall delayload';

procedure Qt5Core_Fake();
external 'Fake@files:Qt5Core.dll stdcall delayload';

function SetupEx_CreateInfo(flag:Integer): Integer;
external 'CreateInfo@files:SetupEx.dll stdcall delayload loadwithalteredsearchpath';

procedure SetupEx_NextInfo(ind: Integer; buffer: AnsiString; size: Integer);
external 'NextInfo@files:SetupEx.dll stdcall delayload loadwithalteredsearchpath';

procedure SetupEx_ReleaseInfo();
external 'ReleaseInfo@files:SetupEx.dll stdcall delayload loadwithalteredsearchpath';

function SetupEx_TestPort(port:Integer): Integer;
external 'TestPort@files:SetupEx.dll stdcall delayload loadwithalteredsearchpath';

function GetFileAttributes(lpFileName: String): DWORD;
 external 'GetFileAttributesW@kernel32.dll stdcall';

function SetFileAttributes(lpFileName: String; dwFileAttributes: DWORD): BOOL; 
external 'SetFileAttributesW@kernel32.dll stdcall';

function CoCreateGuid(var Guid:TGuid):integer;
 external 'CoCreateGuid@ole32.dll stdcall';
// ^^^ import section ^^^

function IsRedist10Installed(): Boolean;
var
  IsInstalled: Cardinal;
begin
  Result := false;
;  if (IsWin64) then begin
;    if RegQueryDWordValue(HKEY_LOCAL_MACHINE_64, 'SOFTWARE\Microsoft\VisualStudio\10.0\VC\VCRedist\x86', 'Installed', IsInstalled) then begin
;      Result := true;
;    end;
;  end;
  if not Result then begin
    if RegQueryDWordValue(HKEY_LOCAL_MACHINE, 'SOFTWARE\Microsoft\VisualStudio\10.0\VC\Runtimes\x86', 'Installed', IsInstalled) then begin
      Result := true;
    end;
  end;
  if not Result then begin
    if RegQueryDWordValue(HKEY_LOCAL_MACHINE, 'SOFTWARE\Microsoft\VisualStudio\10.0\VC\VCRedist\x86', 'Installed', IsInstalled) then begin
      Result := true;
    end;
  end;
end;

function IsRedist12Installed(): Boolean;
var
  IsInstalled: Cardinal;
begin
  Result := false;
;  if (IsWin64) then begin
;    if RegQueryDWordValue(HKEY_LOCAL_MACHINE_64, 'SOFTWARE\Microsoft\VisualStudio\12.0\VC\VCRedist\x86', 'Installed', IsInstalled) then begin
;      Result := true;
;    end;
;  end;
  if not Result then begin
    if RegQueryDWordValue(HKEY_LOCAL_MACHINE, 'SOFTWARE\Microsoft\VisualStudio\12.0\VC\Runtimes\x86', 'Installed', IsInstalled) then begin
      Result := true;
    end;
  end;
  if not Result then begin
    if RegQueryDWordValue(HKEY_LOCAL_MACHINE, 'SOFTWARE\Microsoft\VisualStudio\12.0\VC\VCRedist\x86', 'Installed', IsInstalled) then begin
      Result := true;
    end;
  end;
end;

function IsDotnet35Installed() : Boolean;
var
  DotIsInstalled: Cardinal;
begin
  Result := false;
  if RegQueryDWordValue(HKEY_LOCAL_MACHINE, 'SOFTWARE\Microsoft\NET Framework Setup\NDP\v3.5', 'Install', DotIsInstalled) then
    if DotIsInstalled = 1 then
      Result := true;
end;

function FormatGuid(Guid:TGuid):string;
begin
  result:=Format('%.8x-%.4x-%.4x-%.2x-%.2x-%.2x-%.2x-%.2x-%.2x-%.2x-%.2x', [Guid.D1, Guid.D2, Guid.D3, Guid.D4[0], Guid.D4[1], Guid.D4[2], Guid.D4[3], Guid.D4[4], Guid.D4[5], Guid.D4[6], Guid.D4[7]]);
end;

procedure SetReadOnly(filename: String);
var
 attr : DWord;
begin
  attr := GetFileAttributes(filename);
  if (attr <> $FFFFFFFF) then begin
    if ((attr and 1) = 0)  then begin
      attr := attr xor 1;
      SetFileAttributes(filename, attr);
    end;
  end;
end;

procedure RemoveReadOnly(filename: String);
var
 attr : DWord;
begin
  attr := GetFileAttributes(filename);
  if (attr <> $FFFFFFFF) then begin
    if ((attr and 1) = 1)  then begin
      attr := attr xor 1;
      SetFileAttributes(filename, attr);
    end;
  end;
end;

procedure CreateClasterPage;
var
  staticTextInfo, staticTextInfo2, staticTextInfoLast, staticTextServer, staticTextPort: TNewStaticText;
  i: Integer;
begin

  gClasterPage := CreateCustomPage(wpSelectComponents
    ,ExpandConstant('{cm:msgClasterOptionsCaption}'),ExpandConstant('{cm:msgClasterOptionsDescription}'));

  staticTextInfo := TNewStaticText.Create(gClasterPage);
  staticTextInfo.Width := gClasterPage.SurfaceWidth;
  staticTextInfo.Height := ScaleY(20);
  staticTextInfo.Top := ScaleY(20);
  staticTextInfo.Caption := ExpandConstant('{cm:msgClasterOptionsSubCaption}');
  staticTextInfo.AutoSize := True;
  staticTextInfo.Parent := gClasterPage.Surface;

  staticTextServer := TNewStaticText.Create(gClasterPage);
  staticTextServer.Width := ScaleX(80);
  staticTextServer.Height := ScaleY(20);
  staticTextServer.Top := staticTextInfo.Top + staticTextInfo.Height + ScaleY(8);
  staticTextServer.Caption := ExpandConstant('{cm:msgClasterOptionsServer}');
  staticTextServer.AutoSize := True;
  staticTextServer.Parent := gClasterPage.Surface;

  gClasterPage_comboBox := TNewComboBox.Create(gClasterPage);
  gClasterPage_comboBox.Top := staticTextInfo.Top + staticTextInfo.Height + ScaleY(8);
  gClasterPage_comboBox.Left := ScaleX(90);
  gClasterPage_comboBox.Width := gClasterPage.SurfaceWidth - ScaleX(140);
  gClasterPage_comboBox.Parent := gClasterPage.Surface;
  gClasterPage_comboBox.Style := csDropDownList;

  staticTextInfo2 := TNewStaticText.Create(gClasterPage);
  staticTextInfo2.Width := gClasterPage.SurfaceWidth;
  staticTextInfo2.Height := ScaleY(20);
  staticTextInfo2.Top := gClasterPage_comboBox.Top + gClasterPage_comboBox.Height + ScaleY(8);
  staticTextInfo2.Caption := ExpandConstant('{cm:msgClasterOptionsSubCaption2}');
  staticTextInfo2.AutoSize := True;
  staticTextInfo2.Parent := gClasterPage.Surface;

  staticTextPort := TNewStaticText.Create(gClasterPage);
  staticTextPort.Width := ScaleX(80);
  staticTextPort.Height := ScaleY(20);
  staticTextPort.Top := staticTextInfo2.Top + staticTextInfo2.Height + ScaleY(8);
  staticTextPort.Caption := ExpandConstant('{cm:msgClasterOptionsPort}');
  staticTextPort.AutoSize := True;
  staticTextPort.Parent := gClasterPage.Surface;

  gClasterPage_edit := TNewEdit.Create(gClasterPage);
  gClasterPage_edit.Top := staticTextInfo2.Top + staticTextInfo2.Height + ScaleY(8);
  gClasterPage_edit.Left := ScaleX(90);
  gClasterPage_edit.Width := ScaleX(80);
  gClasterPage_edit.Text := '5432';
  gClasterPage_edit.Parent := gClasterPage.Surface;

  gClasterPage_portInfoText := TNewStaticText.Create(gClasterPage);
  gClasterPage_portInfoText.Width := ScaleX(70);
  gClasterPage_portInfoText.Height := ScaleY(20);
  gClasterPage_portInfoText.Top := staticTextInfo2.Top + staticTextInfo2.Height + ScaleY(8);
  gClasterPage_portInfoText.Left := ScaleX(190);
  gClasterPage_portInfoText.Caption := ExpandConstant('{cm:msgClasterOptionsPortState}');
  gClasterPage_portInfoText.AutoSize := True;
  gClasterPage_portInfoText.Parent := gClasterPage.Surface;
  
  for i := 0 to 3 do begin
    gClasterPage_portInfo[i] := TNewStaticText.Create(gClasterPage);
    gClasterPage_portInfo[i].Width := ScaleX(30);
    gClasterPage_portInfo[i].Height := ScaleY(20);
    gClasterPage_portInfo[i].Top := staticTextInfo2.Top + staticTextInfo2.Height + ScaleY(8);
    gClasterPage_portInfo[i].Left := ScaleX(260) + i * ScaleX(35);
    gClasterPage_portInfo[i].Caption := IntToStr(5432 + i);
    gClasterPage_portInfo[i].AutoSize := True;
    gClasterPage_portInfo[i].Parent := gClasterPage.Surface;
  end;
  
  staticTextInfoLast := TNewStaticText.Create(gClasterPage);
  staticTextInfoLast.Width := gClasterPage.SurfaceWidth;
  staticTextInfoLast.Height := ScaleY(20);
  staticTextInfoLast.Top := gClasterPage_edit.Top + gClasterPage_edit.Height + ScaleY(16);
  staticTextInfoLast.Caption := ExpandConstant('* {cm:msgClasterOptionsSubCaption3}');
  staticTextInfoLast.AutoSize := True;
  staticTextInfoLast.Parent := gClasterPage.Surface;
  staticTextInfoLast.Font.Color := TColor($4444aa);
end;

procedure CreateClasterDataPage;
var
  staticTextInfo, staticTextInfo2, staticTextInfo3, staticTextServer, staticTextName, staticTextGuid: TNewStaticText;
begin

  gClasterDataPage := CreateInputDirPage(gClasterPage.ID
    ,ExpandConstant('{cm:msgClasterDataCaption}'), ExpandConstant('{cm:msgClasterDataDescription}'), '', False, 'data');

  gClasterDataPage.Add(ExpandConstant('{cm:msgClasterDataInfo}'));
end;

procedure CreateServerPage;
var
  staticTextInfo, staticTextInfo2, staticTextInfo3, staticTextServer, staticTextName, staticTextGuid: TNewStaticText;
begin

  gServerPage := CreateCustomPage(gClasterDataPage.ID
    ,ExpandConstant('{cm:msgServerOptionsCaption}'),ExpandConstant('{cm:msgServerOptionsDescription}'));

  staticTextInfo := TNewStaticText.Create(gServerPage);
  staticTextInfo.Width := gServerPage.SurfaceWidth;
  staticTextInfo.Height := ScaleY(20);
  staticTextInfo.Top := ScaleY(20);
  staticTextInfo.Caption := ExpandConstant('{cm:msgServerOptionsSubCaption}');
  staticTextInfo.AutoSize := True;
  staticTextInfo.Parent := gServerPage.Surface;

  staticTextServer := TNewStaticText.Create(gServerPage);
  staticTextServer.Width := ScaleX(80);
  staticTextServer.Height := ScaleY(20);
  staticTextServer.Top := staticTextInfo.Top + staticTextInfo.Height + ScaleY(8);
  staticTextServer.Caption := ExpandConstant('{cm:msgServerOptionsIp}');
  staticTextServer.AutoSize := True;
  staticTextServer.Parent := gServerPage.Surface;

  gServerPage_comboBox := TNewComboBox.Create(gServerPage);
  gServerPage_comboBox.Top := staticTextInfo.Top + staticTextInfo.Height + ScaleY(8);
  gServerPage_comboBox.Left := ScaleX(90);
  gServerPage_comboBox.Width := gServerPage.SurfaceWidth - ScaleX(140);
  gServerPage_comboBox.Parent := gServerPage.Surface;
  gServerPage_comboBox.Style := csDropDownList;

  staticTextInfo2 := TNewStaticText.Create(gServerPage);
  staticTextInfo2.Width := gServerPage.SurfaceWidth;
  staticTextInfo2.Height := ScaleY(20);
  staticTextInfo2.Top := gServerPage_comboBox.Top + gServerPage_comboBox.Height + ScaleY(8);
  staticTextInfo2.Caption := ExpandConstant('{cm:msgServerOptionsSubCaption2}');
  staticTextInfo2.AutoSize := True;
  staticTextInfo2.Parent := gServerPage.Surface;

  staticTextName := TNewStaticText.Create(gServerPage);
  staticTextName.Width := ScaleX(80);
  staticTextName.Height := ScaleY(20);
  staticTextName.Top := staticTextInfo2.Top + staticTextInfo2.Height + ScaleY(8);
  staticTextName.Caption := ExpandConstant('{cm:msgServerOptionsName}');
  staticTextName.AutoSize := True;
  staticTextName.Parent := gServerPage.Surface;

  gServerPage_edit := TNewEdit.Create(gServerPage);
  gServerPage_edit.Top := staticTextInfo2.Top + staticTextInfo2.Height + ScaleY(8);
  gServerPage_edit.Left := ScaleX(90);
  gServerPage_edit.Width := gServerPage.SurfaceWidth - ScaleX(140);
  gServerPage_edit.Text := 'Video server';
  gServerPage_edit.Parent := gServerPage.Surface;

  staticTextInfo3 := TNewStaticText.Create(gServerPage);
  staticTextInfo3.Width := gServerPage.SurfaceWidth;
  staticTextInfo3.Height := ScaleY(20);
  staticTextInfo3.Top := gServerPage_edit.Top + gServerPage_edit.Height + ScaleY(8);
  staticTextInfo3.Caption := ExpandConstant('{cm:msgServerOptionsSubCaption3}');
  staticTextInfo3.AutoSize := True;
  staticTextInfo3.Parent := gServerPage.Surface;

  staticTextGuid := TNewStaticText.Create(gServerPage);
  staticTextGuid.Width := ScaleX(80);
  staticTextGuid.Height := ScaleY(20);
  staticTextGuid.Top := staticTextInfo3.Top + staticTextInfo3.Height + ScaleY(8);
  staticTextGuid.Caption := ExpandConstant('{cm:msgServerOptionsGuid}');
  staticTextGuid.AutoSize := True;
  staticTextGuid.Parent := gServerPage.Surface;

  gServerPage_edit2 := TNewEdit.Create(gServerPage);
  gServerPage_edit2.Top := staticTextInfo3.Top + staticTextInfo3.Height + ScaleY(8);
  gServerPage_edit2.Left := ScaleX(90);
  gServerPage_edit2.Width := gServerPage.SurfaceWidth - ScaleX(140);
  gServerPage_edit2.Text := '11223344-5566-7788-99aa-bbccddeeff00';
  gServerPage_edit2.Parent := gServerPage.Surface;
end;

procedure InitializeWizard;
begin
  CreateClasterPage;
  CreateClasterDataPage;
  CreateServerPage;

  gKeysPath := ExpandConstant('{src}');
  gLicDone := false;
  gConnectionDone := false;
end;

procedure CheckPostgresDir();
begin
  while (Not FileExists(ExpandConstant('{src}\{#psql}'))) do
  begin
    if (MsgBox(ExpandConstant('{cm:msgPostgres92InstallerMissed} {src}\{#psql}'),mbCriticalError,MB_RETRYCANCEL) = IDCANCEL) then begin
      gCancelWithoutPrompt := true;
      WizardForm.Close;
      exit;
    end;
  end;

  if DirExists(ExpandConstant('{app}\PostgreSQL')) then begin
    MsgBox(ExpandConstant('{cm:msgPostgres92DirectoryExists} {app}\PostgreSQL'),mbCriticalError,MB_OK);
    gCancelWithoutPrompt := true;
    WizardForm.Close;
  end;
end;

function ShouldSkipPage(PageID: Integer): Boolean;
var
  i, size, state, bestPort, length: Integer;
  buffer: AnsiString;
  guid: TGuid;
begin
  Result := false;
  if (PageID = gClasterPage.ID) then begin
    if (IsComponentSelected('Claster')) then begin
      gClasterPage_comboBox.Items.Clear();
      size := SetupEx_CreateInfo(7);
      for i := 0 to size - 1 do begin
        buffer := StringOfChar(#0, 256);
        SetupEx_NextInfo(i, buffer, 256);
        SetLength(buffer, Pos(#0, buffer) - 1);
        gClasterPage_comboBox.Items.Add(buffer);
      end;
      SetupEx_ReleaseInfo();
      gClasterPage_comboBox.ItemIndex := 0;

      bestPort := 5666;
      for i := 0 to 3 do begin
        state := SetupEx_TestPort(5432 + i);
        if (state > 0) then begin
          if (bestPort > 5432 + i) then
            bestPort := 5432 + i;
          gClasterPage_portInfo[i].Font.Color := TColor($22cc22);
        end else begin
          gClasterPage_portInfo[i].Font.Color := TColor($2222cc);
        end;
      end;

      while (bestPort = 5666) do begin
        state := SetupEx_TestPort(5432 + i);
        if (state > 0) then begin
          bestPort := 5432 + i;
        end;
        i := i + 1;
      end;

      gClasterPage_edit.Text := IntToStr(bestPort);
    end else begin
      Result := true;
    end;
  end else if (PageID = gClasterDataPage.ID) then begin
    if (IsComponentSelected('Claster')) then begin
      ;
    end else begin
      Result := true;
    end;
  end else if (PageID = gServerPage.ID) then begin
#ifndef NoServer
    if (IsComponentSelected('Server')) then begin
      gServerPage_comboBox.Items.Clear();
      size := SetupEx_CreateInfo(3);
      for i := 0 to size - 1 do begin
        buffer := StringOfChar(#0, 256);
        SetupEx_NextInfo(i, buffer, 256);
        length := Pos(#0, buffer) - 1;
        SetLength(buffer, length);
        if (i = 0) then begin
          gServerPage_edit.Text := buffer;
        end else if (length < 16) then begin
          gServerPage_comboBox.Items.Add(buffer);
        end;        
      end;
      SetupEx_ReleaseInfo();
      gServerPage_comboBox.ItemIndex := 0;

      if (CoCreateGuid(guid) = 0) then begin
        gServerPage_edit2.Text := FormatGuid(guid);
      end;
    end else begin
      Result := true;
    end;
#else
    Result := true;
#endif
  end;
end;

function InitializeSetup(): Boolean;
begin
  if Not IsRedist10Installed() then begin
    SuppressibleMsgBox(ExpandConstant('{cm:msgNeedRedist10}'), mbCriticalError, MB_OK, MB_OK);
    Result := False;
    Exit;
  end;

#ifdef VC12
  if Not IsRedist12Installed() then begin
    SuppressibleMsgBox(ExpandConstant('{cm:msgNeedRedist12}'), mbCriticalError, MB_OK, MB_OK);
    Result := False;
    Exit;
  end;
#endif

  gCancelWithoutPrompt := false;
  result := true;
end;

function CheckClasterServer(): Boolean;
begin
  result := true;
end;

function CheckClasterPort(): Boolean;
  var 
    port: Integer;
begin
  gClasterPort := gClasterPage_edit.Text;
  port := StrToInt(gClasterPort);
  if (SetupEx_TestPort(port) > 0) then begin
    result := true;
  end else begin
    result := false;
  end;
end;

function NextButtonClick(CurPageID: Integer): Boolean;
begin
  result := true;
  if (CurPageID = gClasterPage.ID) then begin
    if (not CheckClasterServer()) then begin
      SuppressibleMsgBox(ExpandConstant('{cm:ClasterServerInvalid}'), mbInformation, MB_OK, MB_OK);
      result := false;
    end else if (not CheckClasterPort()) then begin
      SuppressibleMsgBox(ExpandConstant('{cm:ClasterPortInvalid}'), mbInformation, MB_OK, MB_OK);
      result := false;
    end else begin
      gClasterKeyParams := '{#ProductAbbr} ' + gClasterPage_comboBox.Text + ' ' + gClasterPort;
    end;

    gClasterDataPage.Values[0] := ExpandConstant('{app}\PostgreSQL\data');
  end else if (CurPageID = gClasterDataPage.ID) then begin
    gClasterDataPath := gClasterDataPage.Values[0];
    gClasterInstParams := '{#ProductAbbr} "' + gClasterDataPath + '" ' + gClasterPort;    
  end else if (CurPageID = gServerPage.ID) then begin
    gServerName := gServerPage_edit.Text;
    gServerGuid := gServerPage_edit2.Text;
    gServerIp := gServerPage_comboBox.Text;
  end;
end;

procedure CancelButtonClick(CurPageID: Integer; var Cancel, Confirm: Boolean);
begin
  Confirm := not gCancelWithoutPrompt;
end;

procedure CurUninstallStepChanged(curUninstallStep: TUninstallStep);
begin
  if curUninstallStep = usUninstall then begin
#ifdef RemovePgData
    if MsgBox('Do you want to delete data directory "' + gClasterDataPath + '" ?', mbConfirmation, MB_YESNO) = idYes then begin
      DelTree(gClasterDataPath, True, True, True);
    end;
#endif
  end;
end;

function GetClasterKeyParams(value: String): String;
begin
  result := gClasterKeyParams;
end;

function GetClasterInstParams(value: String): String;
begin
  result := gClasterInstParams;
end;

procedure ShowUserKeys(path: String);
begin
#ifndef HideUserKeys
  SuppressibleMsgBox(ExpandConstant('{cm:msgShowUserKeys}' + path), mbInformation, MB_OK, MB_OK);
  //SuppressibleMsgBox(ExpandConstant('{cm:msgShowServerConnect}' + path), mbInformation, MB_OK, MB_OK);
#endif
  gKeysPath := ExpandConstant('{app}' + '\inst');
end;

procedure SelectConnect();
var
  filename: String;
  destfile: String;
begin
  filename := 'connect2.ini';
  if (GetOpenFileName(ExpandConstant('{cm:msgCopyConnectCaption}'), filename, gKeysPath, '', '')) then begin
    destfile := ExpandConstant('{app}') + '/Var/connection.ini';
    RemoveReadOnly(destfile);
    if (FileCopy(filename, destfile, false)) then begin
      SetReadOnly(destfile);
      gConnectionDone := true;
    end else begin
      SuppressibleMsgBox(ExpandConstant('{cm:msgCopyError}'), mbError, MB_OK, MB_OK);
    end;
  end else begin
    SuppressibleMsgBox(ExpandConstant('{cm:msgCopyConnectManual}'), mbError, MB_OK, MB_OK);
  end;
end;

procedure SelectLic();
var
  filename: String;
  destfile: String;
begin
  filename := 'key.ini';
  if (GetOpenFileName(ExpandConstant('{cm:msgCopyLicCaption}'), filename, ExpandConstant('{src}'), ExpandConstant('{cm:msgLicFilter}'), 'txt')) then begin
    destfile := ExpandConstant('{app}') + '/Var/key.ini';
    RemoveReadOnly(destfile);
    if (FileCopy(filename, destfile, false)) then begin
      SetReadOnly(destfile);
      gLicDone := true;
    end else begin
      SuppressibleMsgBox(ExpandConstant('{cm:msgCopyError}'), mbError, MB_OK, MB_OK);
    end;
  end else begin
    SuppressibleMsgBox(ExpandConstant('{cm:msgCopyLicManual}'), mbError, MB_OK, MB_OK);
  end;
end;

function CreateComponentSettings(component: String; name: String; guid: String; ip: String): Boolean;
var
  settings: TArrayOfString;
begin
  SetArrayLength(settings, 4);
  settings[0] := '[General]';
  settings[1] := 'Name=' + name;
  settings[2] := 'Guid=' + guid;
  settings[3] := 'IP=' + ip;

  result := SaveStringsToUTF8File(ExpandConstant('{app}') + '/Var/' + component + '.ini', settings, false);
end;

#ifndef NoUpdate
procedure PreUpdate();
begin
#ifdef License
  if (not gLicDone) then begin
    SelectLic();
  end;
#endif
  if (not gConnectionDone) then begin
    if (not IsComponentSelected('Claster')) then begin
      SelectConnect();
    end;
  end;
end;
#endif

procedure PreServer();
begin
#ifdef License
  if (not gLicDone) then begin
    SelectLic();
  end;
#endif
  if (not gConnectionDone) then begin
    if (not IsComponentSelected('Claster')) then begin
      SelectConnect();
    end;
  end;

  if (not CreateComponentSettings('{#ProductAbbr}_srvd', gServerName, gServerGuid, gServerIp)) then begin
    SuppressibleMsgBox(ExpandConstant('{cm:msgServerInitError}'), mbError, MB_OK, MB_OK);
  end;
end;

procedure PreArm();
var
  i, size, length: Integer;
  buffer: AnsiString;
  guid: TGuid;
  armName, armGuid: String;
begin
#ifdef License
  if (not gLicDone) then begin
    SelectLic();
  end;
#endif
  if (not gConnectionDone) then begin
    if (not IsComponentSelected('Claster')) then begin
      SelectConnect();
    end;
  end;

  size := SetupEx_CreateInfo(1);
  buffer := StringOfChar(#0, 256);
  SetupEx_NextInfo(0, buffer, 256);
  length := Pos(#0, buffer) - 1;
  SetLength(buffer, length);
  armName := buffer + ExpandConstant('{cm:strArmSuffix}');

  if (CoCreateGuid(guid) = 0) then begin
    armGuid := FormatGuid(guid);
  end else begin
    armGuid := armName;
  end;

  if (not CreateComponentSettings('{#ProductAbbr}_armd', armName, armGuid, '')) then begin
    SuppressibleMsgBox(ExpandConstant('{cm:msgArmInitError}'), mbError, MB_OK, MB_OK);
  end;
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if (CurStep = ssPostInstall) then begin
  end;
end;

