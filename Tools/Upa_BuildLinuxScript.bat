@echo off
setlocal

SET script_dir=%~dp0
SET tools_dir=%script_dir:~0,-1%
for %%i in ("%tools_dir%\..") do set "project_dir=%%~fi"

set library_name=UPA
set image_tag=upa-builder:qt6.11.0-u22

echo ========================================================
echo Docker image check
echo ========================================================
docker image inspect %image_tag% >nul 2>&1
if errorlevel 1 (
    echo Building Docker image ^(one-time, ~5 minutes^)...
    docker build -t %image_tag% -f "%tools_dir%\Dockerfile" "%tools_dir%"
    if errorlevel 1 (
        echo.
        echo ERROR: Docker image build failed
        pause
        exit /b 1
    )
) else (
    echo Image %image_tag% already exists, skipping build
)

echo ========================================================
echo Build Linux AppImage
echo ========================================================
echo PROJECT_DIR: %project_dir%

docker run --rm -v "%project_dir%:/work" %image_tag%
if errorlevel 1 (
    echo.
    echo ERROR: Build failed
    pause
    exit /b 1
)

echo ========================================================
echo Done
echo ========================================================
dir /b "%project_dir%\Binary\*.AppImage"
echo.
pause

endlocal