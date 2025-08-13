[Setup]
AppName=Octos
AppVersion=1.0
DefaultDirName={pf}\Octos
DefaultGroupName=Octos
OutputBaseFilename=OctosSetup
Compression=lzma
SolidCompression=yes
UninstallDisplayIcon={app}\Octos.exe

[Files]
Source: "build\Release\Octos.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\assets\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: ".\vcpkg_installed\x64-windows\bin\*.dll"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\Octos"; Filename: "{app}\Octos.exe"
Name: "{commondesktop}\Octos"; Filename: "{app}\Octos.exe"
Name: "{group}\Uninstall Octos"; Filename: "{uninstallexe}"

[Run]
Filename: "{app}\Octos.exe"; Description: "Launch Octos"; Flags: nowait postinstall skipifsilent

; [Tasks]
; Name: modifypath; Description: "Add Octos directory to PATH"; Flags: unchecked

; [Code]
; const
;   EnvironmentKey = 'SYSTEM\CurrentControlSet\Control\Session Manager\Environment';

; procedure AddToPath;
; var
;   PathValue: string;
; begin
;   if not RegQueryStringValue(HKEY_LOCAL_MACHINE, EnvironmentKey, 'Path', PathValue) then
;     PathValue := '';
;   if Pos(LowerCase(ExpandConstant('{app}')), LowerCase(PathValue)) = 0 then
;   begin
;     if PathValue <> '' then
;       PathValue := PathValue + ';';
;     PathValue := PathValue + ExpandConstant('{app}');
;     RegWriteStringValue(HKEY_LOCAL_MACHINE, EnvironmentKey, 'Path', PathValue);
;     SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0,
;       LPARAM(PChar('Environment')), SMTO_ABORTIFHUNG, 5000, Longint(nil^));
;   end;
; end;

; procedure RemoveFromPath;
; var
;   PathValue, NewPath: string;
;   Parts: TArrayOfString;
;   I: Integer;
; begin
;   if not RegQueryStringValue(HKEY_LOCAL_MACHINE, EnvironmentKey, 'Path', PathValue) then
;     Exit;
;   StringChangeEx(PathValue, ExpandConstant('{app}'), '', True);
;   Parts := SplitString(PathValue, ';');
;   NewPath := '';
;   for I := 0 to GetArrayLength(Parts) - 1 do
;     if Trim(Parts[I]) <> '' then
;     begin
;       if NewPath <> '' then
;         NewPath := NewPath + ';';
;       NewPath := NewPath + Parts[I];
;     end;
;   RegWriteStringValue(HKEY_LOCAL_MACHINE, EnvironmentKey, 'Path', NewPath);
;   SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0,
;     LPARAM(PChar('Environment')), SMTO_ABORTIFHUNG, 5000, Longint(nil^));
; end;

; procedure CurStepChanged(CurStep: TSetupStep);
; begin
;   if (CurStep = ssPostInstall) and IsTaskSelected('modifypath') then
;     AddToPath;
; end;

; procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
; begin
;   if CurUninstallStep = usPostUninstall then
;     RemoveFromPath;
; end;