[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl"

[CustomMessages]
TypesClient=Client
TypesServer=Server
TypesFull=Full (new claster)
TypesCustom=Custom
TypesDefault=Predefined
TypesProgram=Program

ComponentsClaster=New claster's DB
ComponentsUpdate=Update service
ComponentsServer=Server of claster
ComponentsProgramFiles=Client programs

strArmSuffix= Workstation

msgNeedNet35=.Net 3.5 required.
msgNeedRedist10=Microsoft Visual C++ 2010 (x86) redistributable package required.
msgNeedRedist12=Microsoft Visual C++ 2013 (x86) redistributable package required.

msgInstallPostgres=PostgreSQL 9 installing
msgRunPostgres92=PostgreSQL 9 starting
msgStopPostgres92=PostgreSQL 9 stopping
msgPostgres92InstallerMissed=Installer of PostgreSQL 9.2 required on the media for 'Claster' installation (postgresql-9.2.6-3-windows.exe).
msgPostgres92DirectoryExists=Installation of PostgreSQL impossible, directory not empty. Path: 

msgInstallServer=Server installation
msgStartServer=Starting server
msgStopServer=Stopping server

msgClasterOptionsCaption=Claster settings
msgClasterOptionsDescription=Claster DB settings
msgClasterOptionsSubCaption=Input DNS name or IP address of server, which will be availabe for other units in %nclaster.
msgClasterOptionsSubCaption2=Input port number (default for PostgreSQL: 5432).
msgClasterOptionsSubCaption3=This port must be included into exceptions of your Firewall.
msgClasterOptionsServer=IP/DNS name
msgClasterOptionsPort=Port
msgClasterOptionsPortState=States:
msgClasterDataCaption=Claster data base
msgClasterDataDescription=Location of PostgreSQL data directory
msgClasterDataInfo=Path:
msgClasterServerInvalid=Input IP address is invalid.
msgClasterPortInvalid=Port is unavailable, choose another.

msgServerOptionsCaption=Server settings
msgServerOptionsDescription=Server mandatory settings
msgServerOptionsSubCaption=Input IP server address, which will be availabe for other units of claster.
msgServerOptionsSubCaption2=Input server name.
msgServerOptionsSubCaption3=Input server UID.
msgServerOptionsIp=Server IP
msgServerOptionsName=Server name
msgServerOptionsGuid=Server UID

msgGuidGenerateFail=UID generate failed. Setup UID manually.

msgShowUserKeys=Claster creation done.%nTo add units to claster your'll be needed claster connection file 'connect2.ini', which was generated at: %n
msgCopyConnect=Copy claster connection file
msgServerInitError=Failed to create .ini file needed for Server component
msgArmInitError=Failed to create .ini file needed for Program files component
msgCopyConnectCaption=Select claster connection file
msgCopyConnectManual=Connection file copy cancelled. To work properly file must be coppied manually.
msgCopyError=Copying error.
msgCopyLicCaption=License file selection
msgLicFilter=Settings files (*.ini)|*.ini|All files|*.*
msgCopyLicManual=License file copy cancelled. To work properly file must be coppied manually.
