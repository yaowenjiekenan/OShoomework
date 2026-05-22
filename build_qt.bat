@echo off
chcp 65001 >nul
title Unix File System - Qt Version

echo.
echo  ==========================================================
echo  Build Unix File System - Qt Version
echo  ==========================================================
echo.

REM === Load environment from .env ===
REM Save PATH first, as for /f parsing can corrupt it
set "_ORIG_PATH=%PATH%"
if exist ".env" (
    for /f "usebackq eol=# delims=" %%i in (".env") do set "%%i"
)
set "PATH=%_ORIG_PATH%"
set "_ORIG_PATH="

if not defined QT_BIN (
    echo  [ERROR] .env file not found or invalid!
    echo  Please copy .env.example to .env and configure your Qt path.
    echo.
    pause
    exit /b 1
)

REM === Derive full paths ===
set QMAKE_PATH=%QT_BIN%\qmake.exe

REM === Verify paths exist ===
if not exist "%QMAKE_PATH%" (
    echo  [ERROR] qmake not found at: %QMAKE_PATH%
    echo  Please check QT_BIN in .env
    echo.
    pause
    exit /b 1
)

if not exist "%MINGW_BIN%\mingw32-make.exe" (
    if defined MINGW_BIN (
        echo  [WARN] mingw32-make not found at: %MINGW_BIN%
        echo  If using MSVC Qt build, this warning can be ignored.
        echo  If using MinGW Qt build, please check MINGW_BIN in .env
        echo.
    )
)

REM === Set PATH ===
set "PATH=%QT_BIN%;%MINGW_BIN%;%PATH%"

echo  [INFO] Qt Bin: %QT_BIN%
echo  [INFO] MinGW Bin: %MINGW_BIN%
echo.

echo  [INFO] Checking qmake...
"%QMAKE_PATH%" --version
echo.

echo  [INFO] Cleaning previous build...
if exist build rmdir /s /q build
if exist OS.exe del /q OS.exe >nul 2>&1
mkdir build\obj 2>nul
mkdir build\moc 2>nul
mkdir build\rcc 2>nul
mkdir build\uic 2>nul

echo.
echo  [INFO] Running qmake...
"%QMAKE_PATH%" OS.pro -o build\Makefile
if errorlevel 1 (
    echo  [ERROR] qmake failed
    pause
    exit /b 1
)

echo  [INFO] Building...
echo.
cd build
"%MINGW_BIN%\mingw32-make" -j4
if errorlevel 1 (
    echo.
    echo  [ERROR] Build failed
    cd ..
    pause
    exit /b 1
)

cd ..

echo.
echo  [OK] Build successful
echo.

echo  [INFO] Deploying Qt runtime files...
"%QT_BIN%\windeployqt" --no-translations --no-compiler-runtime --no-opengl-sw build\OS.exe
echo.
echo  [OK] Qt runtime deployed to build
echo.

echo  [INFO] Copying executable to root directory...
if exist "build\OS.exe" copy /y "build\OS.exe" . >nul 2>&1
for %%f in (d3dcompiler_47.dll libatomic-1.dll libgcc_s_seh-1.dll libgfortran-5.dll libgomp-1.dll libquadmath-0.dll libstdc++-6.dll libwinpthread-1.dll opengl32sw.dll Qt6Core.dll Qt6Gui.dll Qt6Network.dll Qt6Svg.dll Qt6Widgets.dll) do (
    if exist "build\%%f" copy /y "build\%%f" . >nul 2>&1
)
for %%d in (platforms imageformats iconengines styles generic networkinformation tls) do (
    if exist "build\%%d" xcopy /y /e /i "build\%%d" "%%d\" >nul 2>&1
)
echo  [OK] Runtime files copied to root directory
echo.

echo  Running File System...
echo.
echo.

if exist "build\OS.exe" (
    set "PATH=%QT_BIN%;%MINGW_BIN%;%PATH%"
    build\OS.exe
) else (
    echo  [ERROR] Executable not found
    dir build\*.exe /s /b
)

echo.
echo  ==========================================================
echo  Program exited.
pause
