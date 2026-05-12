#!/bin/sh
set -e

APP_NAME="UniversalPpsAnalyzer"
WORK=/work

echo "=== Building ${APP_NAME} ==="
BUILD_DIR="/tmp/UPA_Build"
rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

qmake "${WORK}/Library/UniversalPpsAnalyzer/UniversalPpsAnalyzer.pro"
make -j$(nproc)

BINARY_PATH="$(find "${BUILD_DIR}" -name "${APP_NAME}" -type f -executable | head -n1)"
[ -z "${BINARY_PATH}" ] && { echo "ERROR: binary not found"; exit 1; }

echo "=== Packaging AppImage ==="
APPIMG_DIR="/tmp/AppImageBuild"
rm -rf "${APPIMG_DIR}"
mkdir -p "${APPIMG_DIR}"
cd "${APPIMG_DIR}"

cp "${BINARY_PATH}" ./

# Icon — use provided one or generate a placeholder
if [ -f "${WORK}/Resources/icon.png" ]; then
    cp "${WORK}/Resources/icon.png" "${APP_NAME}.png"
else
    convert -size 256x256 xc:steelblue "${APP_NAME}.png"
fi

# Desktop file
cat > "${APP_NAME}.desktop" <<EOF
[Desktop Entry]
Type=Application
Name=Universal PPS Analyzer
Exec=${APP_NAME}
Icon=${APP_NAME}
Categories=Utility;
Terminal=false
EOF

# Build AppImage
mkdir -p AppDir
/opt/linuxdeploy-x86_64.AppImage --appdir AppDir \
    -e "${APP_NAME}" \
    -d "${APP_NAME}.desktop" \
    -i "${APP_NAME}.png" \
    --plugin qt --output appimage

# Copy result to Binary/
mkdir -p "${WORK}/Binary"
cp *.AppImage "${WORK}/Binary/UniversalPpsAnalyzer-x86_64.AppImage"

echo "=== Done: ${WORK}/Binary/$(ls "${WORK}/Binary/" | grep AppImage) ==="