@echo off
chcp 65001 >nul
title Unix File System - Qt Version

echo.
echo  ==========================================================
echo  Build Unix File System - Qt Version
echo  ==========================================================
echo.

REM Qt Configuration
set QT_DIR=F:\Qt
set QT_VERSION=6.11.1
set QT_BIN=%QT_DIR%\%QT_VERSION%\mingw_64\bin
set QT_LIB=%QT_DIR%\%QT_VERSION%\mingw_64\lib
set QT_INC=%QT_DIR%\%QT_VERSION%\mingw_64\include

REM MinGW Configuration
set MINGW_DIR=%QT_DIR%\Tools\mingw1310_64
set MINGW_BIN=%MINGW_DIR%\bin
set PATH=%QT_BIN%;%MINGW_BIN%;%PATH%

echo  [INFO] Qt Directory: %QT_DIR%\%QT_VERSION%\mingw_64
echo.

REM Check for qmake (check PATH first, then try Qt directory)
where qmake >nul 2>&1
if errorlevel 1 (
    echo  [INFO] qmake not in PATH, trying Qt directory...
    set QMAKE_PATH=%QT_BIN%\qmake.exe
    if exist "%QMAKE_PATH%" (
        echo  [OK] Found qmake at %QMAKE_PATH%
    ) else (
        echo  [ERROR] qmake not found!
        echo  Please check your Qt installation at: %QT_DIR%
        echo.
        pause
        exit /b 1
    )
) else (
    echo  [OK] qmake found in PATH
)
echo.
if exist "%QMAKE_PATH%" (
    "%QMAKE_PATH%" --version
) else (
    qmake --version
)

REM Clean previous build
echo  [INFO] Cleaning previous build...
if exist build rmdir /s /q build
if exist OS.exe del /q OS.exe >nul 2>&1
mkdir build\obj 2>nul
mkdir build\moc 2>nul
mkdir build\rcc 2>nul
mkdir build\uic 2>nul

echo.
echo  [INFO] Running qmake...
REM Run qmake
if exist "%QMAKE_PATH%" (
    "%QMAKE_PATH%" -spec win32-g++ OS.pro -o build\Makefile
) else (
    qmake -spec win32-g++ OS.pro -o build\Makefile
)
if errorlevel 1 (
    echo  [ERROR] qmake failed!
    pause
    exit /b 1
)

echo  [INFO] Building...
echo.

cd build
mingw32-make -j4
if errorlevel 1 (
    echo.
    echo  [ERROR] Build failed!
    cd ..
    pause
    exit /b 1
)

cd ..

echo.
echo  [OK] Build successful!
echo.

REM Deploy Qt runtime with windeployqt
echo  [INFO] Deploying Qt runtime files...
if exist "%QMAKE_PATH%" (
    "%QT_BIN%\windeployqt" --no-translations --no-compiler-runtime --no-opengl-sw build\OS.exe
) else (
    windeployqt --no-translations --no-compiler-runtime --no-opengl-sw build\OS.exe
)
echo.
echo  [OK] Qt runtime deployed to build\!
echo.

REM Copy runtime files to root directory for standalone execution
echo  [INFO] Copying runtime files to root directory...
copy /y build\OS.exe . >nul 2>&1
for %%f in (d3dcompiler_47.dll libatomic-1.dll libgcc_s_seh-1.dll libgfortran-5.dll libgomp-1.dll libquadmath-0.dll libstdc++-6.dll libwinpthread-1.dll opengl32sw.dll Qt6Core.dll Qt6Gui.dll Qt6Network.dll Qt6Svg.dll Qt6Widgets.dll) do (
    if exist "build\%%f" copy /y "build\%%f" . >nul 2>&1
)
REM Copy Qt plugin directories
for %%d in (platforms imageformats iconengines styles generic networkinformation tls) do (
    if exist "build\%%d" xcopy /y /e /i "build\%%d" "%%d\" >nul 2>&1
)
echo  [OK] Runtime files copied to root directory!
echo.

echo  Running File System...
echo.
echo.

if exist build\OS.exe (
    set PATH=%QT_BIN%;%MINGW_BIN%;%PATH%
    build\OS.exe
) else (
    echo  [ERROR] Executable not found!
    dir build\*.exe /s /b
)

echo.
echo  ==========================================================
echo  Program exited.
pause
