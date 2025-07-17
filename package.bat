mkdir AppPackage
rmdir /s /q build
mkdir build
cmake -S . -B build
cmake --build build --config Release --preset=default
copy build\Release\octos.exe AppPackage\
xcopy assets\* AppPackage\ /s /e /i
mkdir AppPackage\Assets
@REM xcopy icons\* AppPackage\Assets\ /s /e /i
@REM copy AppxManifest.xml AppPackage\
MakeAppx pack /d AppPackage /p octos-installer.msix