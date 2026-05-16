@echo off
REM Збірка в MSYS2 UCRT64 (потрібна встановлена MSYS2: https://www.msys2.org/)

cd /d "%~dp0"
set "MSYS2=C:\msys64"

if not exist "%MSYS2%\msys2_shell.cmd" (
    echo Не знайдено MSYS2 у %MSYS2%
    echo Встановіть MSYS2 або змініть шлях у build.bat
    pause
    exit /b 1
)

"%MSYS2%\msys2_shell.cmd" -ucrt64 -defterm -here -no-start -c "pacman -S --needed --noconfirm mingw-w64-ucrt-x86_64-toolchain mingw-w64-ucrt-x86_64-gtk3 make && make"
if errorlevel 1 pause
