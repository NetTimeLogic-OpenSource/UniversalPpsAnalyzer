@echo off

SET script_dir=%~dp0
SET tools_dir=%script_dir%

set library_name=UPA

echo ========================================================
echo Build
echo ========================================================
set build_dir=%tools_dir%..\Build
echo BUILD_DIR: %build_dir%
rmdir /q /s %build_dir%
mkdir %build_dir%

REM NOTE: change for different QT installation paths and versions
set PATH=C:\Qt\Tools\mingw530_32\bin;%PATH%
set PATH=C:\Qt\Static\5.11\bin;%PATH%

cd %build_dir%
qmake %tools_dir%..\Library\UniversalPpsAnalyzer\UniversalPpsAnalyzer_static.pro
mingw32-make -f %build_dir%\Makefile.Release -j32

echo ========================================================
echo Binary
echo ========================================================
set binary_dir=%tools_dir%..\Binary
echo BINARY_DIR: %binary_dir%

xcopy /y %build_dir%\release\UniversalPpsAnalyzer.exe                                  %binary_dir%\
