taskkill /f /im comp.exe

cl /EHsc /std:c++17 /DWEBVIEW2_NO_IDL /DUNICODE /D_UNICODE ^
  /I"..\packages\Microsoft.Web.WebView2.1.0.3240.44\build\native\include" ^
  /I"..\packages\Microsoft.Windows.ImplementationLibrary.1.0.250325.1\include" ^
  comp.cpp ^
  /link /LIBPATH:"..\packages\Microsoft.Web.WebView2.1.0.3240.44\build\native\x64" ^
  WebView2LoaderStatic.lib ole32.lib uuid.lib user32.lib shell32.lib comctl32.lib shlwapi.lib advapi32.lib Shcore.lib dwmapi.lib dcomp.lib d3d11.lib windowscodecs.lib oleaut32.lib runtimeobject.lib ^
  /SUBSYSTEM:WINDOWS /OUT:comp.exe

comp.exe