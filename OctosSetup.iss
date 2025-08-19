[Setup]
AppName=Octos
AppVersion={#AppVersion}
DefaultGroupName=Octos
OutputBaseFilename=OctosSetup
Compression=lzma
SolidCompression=yes
UninstallDisplayIcon={app}\Octos.exe
ArchitecturesAllowed=x86compatible x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
DefaultDirName={commonpf}\Octos
ChangesEnvironment=true
DisableWelcomePage=no
WizardImageFile=./img/wizard-screen.bmp
WizardSmallImageFile=./img/small-wizard.bmp
DisableDirPage=false
OutputDir=build
; LicenseFile=LICENSE

[Files]
; Source: "build\Release\Octos.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\build\x64\Release\*"; DestDir: "{app}"; Excludes: ".\build\Release\img\*.png"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: Is64BitInstallMode
Source: ".\build\Win32\Release\*"; DestDir: "{app}"; Excludes: ".\build\Release\img\*.png"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: not Is64BitInstallMode
; Source: ".\vcpkg_installed\x64-windows\bin\*.dll"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\Octos"; Filename: "{app}\Octos.exe"; Tasks: startmenuicon
Name: "{group}\Uninstall Octos"; Filename: "{uninstallexe}"; Tasks: startmenuicon
Name: "{commondesktop}\Octos"; Filename: "{app}\Octos.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\Octos.exe"; Description: "Launch Octos"; Flags: nowait postinstall skipifsilent

[Tasks]
Name: desktopicon; Description: "Create a Desktop shortcut"; GroupDescription: "Additional shortcuts:"
Name: startmenuicon; Description: "Create a Start Menu shortcut"; GroupDescription: "Additional shortcuts:"
Name: envPath; Description: "Add Octos to PATH"; GroupDescription: "Add the 'octos' toolset to PATH?"

[UninstallDelete]
Type: filesandordirs; Name: "{localappdata}\Octos"

[Code]
const EnvironmentKey = 'SYSTEM\CurrentControlSet\Control\Session Manager\Environment';

procedure EnvAddPath(Path: string);
var
    Paths: string;
begin
    { Retrieve current path (use empty string if entry not exists) }
    if not RegQueryStringValue(HKEY_LOCAL_MACHINE, EnvironmentKey, 'Path', Paths) then
        Paths := '';

    { Skip if string already found in path }
    if Pos(';' + Uppercase(Path) + ';', ';' + Uppercase(Paths) + ';') > 0 then
        exit;

    { Append properly with semicolon }
    if Paths <> '' then
        Paths := Paths + ';';
    Paths := Paths + Path;

    { Overwrite (or create if missing) path environment variable }
    if RegWriteStringValue(HKEY_LOCAL_MACHINE, EnvironmentKey, 'Path', Paths) then
        Log(Format('The [%s] added to PATH: [%s]', [Path, Paths]))
    else
        Log(Format('Error while adding the [%s] to PATH: [%s]', [Path, Paths]));
end;

procedure EnvRemovePath(Path: string);
var
    Paths: string;
    P: Integer;
begin
    { Skip if registry entry not exists }
    if not RegQueryStringValue(HKEY_LOCAL_MACHINE, EnvironmentKey, 'Path', Paths) then
        exit;

    { Find string in path }
    P := Pos(';' + Uppercase(Path) + ';', ';' + Uppercase(Paths) + ';');
    if P = 0 then exit;

    { Remove it safely }
    Delete(Paths, P, Length(Path) + 1);
    if (P <= Length(Paths)) and (Copy(Paths, P, 1) = ';') then
        Delete(Paths, P, 1);

    { Overwrite path environment variable }
    if RegWriteStringValue(HKEY_LOCAL_MACHINE, EnvironmentKey, 'Path', Paths) then
        Log(Format('The [%s] removed from PATH: [%s]', [Path, Paths]))
    else
        Log(Format('Error while removing the [%s] from PATH: [%s]', [Path, Paths]));
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
    if (CurStep = ssPostInstall) and WizardIsTaskSelected('envPath') then
        EnvAddPath(ExpandConstant('{app}'));
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
    if CurUninstallStep = usPostUninstall then
        EnvRemovePath(ExpandConstant('{app}'));
end;