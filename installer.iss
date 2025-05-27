[Setup]
AppId={{E2F2C7AB-4977-4F49-AD70-1DC44B7680D1}}
AppName=My WebView2 App
AppVersion=1.0
AppPublisher=Your Name
AppPublisherURL=https://your-website.com
DefaultDirName={autopf}\My WebView2 App
DefaultGroupName=My WebView2 App
OutputDir=.
OutputBaseFilename=MyWebView2Installer
Compression=lzma
SolidCompression=yes

[Files]
Source: "build\app.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "assets\*"; DestDir: "{app}\assets"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\My WebView2 App"; Filename: "{app}\app.exe"
Name: "{group}\Uninstall My WebView2 App"; Filename: "{uninstallexe}"

[Run]
Filename: "{app}\app.exe"; Description: "Launch My WebView2 App"; Flags: nowait postinstall skipifsilent