@echo off
REM Запуск меню з Провідника (подвійний клік).
REM Потрібно: MSYS2 + збірка (make) у терміналі UCRT64.

cd /d "%~dp0"
set "LAUNCHER_HOME=%CD%"

if not exist "bin\launcher.exe" (
    echo Спочатку зберіть проєкт у MSYS2 UCRT64:
    echo   cd /c/шлях/до/app
    echo   pacman -S mingw-w64-ucrt-x86_64-toolchain mingw-w64-ucrt-x86_64-gtk3 make
    echo   make
    pause
    exit /b 1
)

REM Шлях до GTK DLL (змініть, якщо MSYS2 не в C:\msys64)
if exist "C:\msys64\ucrt64\bin" set "PATH=C:\msys64\ucrt64\bin;%PATH%"

start "" "bin\launcher.exe"
