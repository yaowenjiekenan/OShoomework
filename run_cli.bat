@echo off
chcp 65001 >nul

set QT_BIN=D:\Qt6.11\6.8.3\mingw_64\bin
set MINGW_BIN=D:\Qt6.11\Tools\mingw1310_64\bin
set PATH=%QT_BIN%;%MINGW_BIN%;%PATH%

echo Deploying Qt runtime...
"%QT_BIN%\windeployqt" --no-translations --no-compiler-runtime --no-opengl-sw OS.exe >nul 2>&1

echo.
echo ==========================================================
echo   Unix File System - CLI Mode
echo ==========================================================
echo.

OS.exe --cli
echo.
echo ----------------------------------------------------------
echo Program exited (code: %ERRORLEVEL%). Press any key.
pause >nul
