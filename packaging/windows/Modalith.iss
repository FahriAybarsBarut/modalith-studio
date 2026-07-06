#ifndef AppVersion
  #define AppVersion "0.2.0"
#endif
#ifndef SourceDir
  #define SourceDir "..\..\dist\Modalith-Studio-0.2.0-Windows-x64"
#endif
#ifndef OutputDir
  #define OutputDir "..\..\dist"
#endif

#define MyAppName "Modalith Studio"
#define MyAppPublisher "Modalith Engineering"
#define MyAppExeName "modalith_studio.exe"

[Setup]
AppId={{FC58D8D7-A4D5-46CB-9B2B-63650A13CE2D}
AppName={#MyAppName}
AppVersion={#AppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\Modalith Studio
DefaultGroupName=Modalith Studio
DisableProgramGroupPage=yes
LicenseFile=..\..\LICENSE
OutputDir={#OutputDir}
OutputBaseFilename=Modalith-Studio-Setup-{#AppVersion}
Compression=lzma2/max
SolidCompression=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
WizardStyle=modern
UninstallDisplayIcon={app}\{#MyAppExeName}
VersionInfoVersion={#AppVersion}.0
VersionInfoCompany={#MyAppPublisher}
VersionInfoDescription=Modalith Studio Windows Installer
VersionInfoProductName={#MyAppName}
VersionInfoProductVersion={#AppVersion}

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "{#SourceDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{autoprograms}\Modalith Studio"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\Modalith Studio"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut"; GroupDescription: "Additional shortcuts:"

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "Launch Modalith Studio"; Flags: nowait postinstall skipifsilent
