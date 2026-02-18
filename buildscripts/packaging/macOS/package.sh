#!/usr/bin/env bash

echo "Package"
trap 'echo Package failed; exit 1' ERR

APP_NAME="MuseScore Studio"
VOL_NAME="MuseScore-Studio"
DO_SIGN=false

while [[ "$#" -gt 0 ]]; do
    case $1 in
        --app-name) APP_NAME="$2"; shift ;;
        --vol-name) VOL_NAME="$2"; shift ;;
        --sign) DO_SIGN=true ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

################################################################
# Deploy
################################################################

APP_PATH=applebuild/mscore.app

echo "otool -L pre-macdeployqt"
otool -L ${APP_PATH}/Contents/MacOS/mscore

echo "macdeployqt"
if $DO_SIGN; then
    sign_args="-sign-for-notarization=Developer ID Application: MuseScore"
else
    sign_args=""
fi
macdeployqt ${APP_PATH} \
    -verbose=2 \
    -qmldir=. \
    $sign_args

echo "otool -L post-macdeployqt"
otool -L ${APP_PATH}/Contents/MacOS/mscore

# Remove dSYM files
echo "Remove dSYM files"
find ${APP_PATH}/Contents -type d -name "*.dSYM" -exec rm -r {} +

# Rename Resources/qml to Resources/qml_mu. This way, VST3 plugins that also use QML
# won't find these QML files, to prevent crashes because of conflicts.
# https://github.com/musescore/MuseScore/issues/21372
# https://github.com/musescore/MuseScore/issues/24331
echo "Rename Resources/qml to Resources/qml_mu"
mv ${APP_PATH}/Contents/Resources/qml ${APP_PATH}/Contents/Resources/qml_mu
sed -i '' 's:Resources/qml:Resources/qml_mu:g' ${APP_PATH}/Contents/Resources/qt.conf

if $DO_SIGN; then
    # Re-sign appex to ensure proper entitlements
    echo "Re-sign appex"
    codesign --force \
        --options runtime \
        --entitlements "src/macos_integration/entitlements.plist" \
        -s "Developer ID Application: MuseScore" \
        "${APP_PATH}/Contents/PlugIns/MuseScoreQuickLookPreviewExtension.appex"

    # Re-sign main app after removing dSYM files and renaming qml folder
    echo "Re-sign main app"
    codesign --force \
        --options runtime \
        --entitlements "buildscripts/packaging/macOS/entitlements.plist" \
        -s "Developer ID Application: MuseScore" \
        "${APP_PATH}"

    echo "Codesign verify"
    codesign --verify --deep --strict --verbose=2 "${APP_PATH}"

    echo "spctl"
    spctl --assess --type execute -vvv "${APP_PATH}"
else
    echo "Skipping code signing"
fi

################################################################
# Create DMG
################################################################

DMG_NAME="${VOL_NAME}-uncompressed.dmg"
COMPRESSED_DMG_NAME="${VOL_NAME}.dmg"

rm -f "applebuild/${COMPRESSED_DMG_NAME}"

# Tip: increase the size if error on copy
hdiutil create -size 650m -fs HFS+ -volname "${VOL_NAME}" "applebuild/${DMG_NAME}"

# Mount the disk image
hdiutil attach "applebuild/${DMG_NAME}"

# Obtain device information
DEVS=$(hdiutil attach "applebuild/${DMG_NAME}" | cut -f 1)
DEV=$(echo $DEVS | cut -f 1 -d ' ')
VOLUME=$(mount | grep ${DEV} | cut -f 3 -d ' ')

# copy in the application bundle
cp -Rp ${APP_PATH} "${VOLUME}/${APP_NAME}.app"

# Copy in background image
echo "Copy in background image"
BACKGROUND=buildscripts/packaging/macOS/musescore-dmg-background.tiff
mkdir -p "${VOLUME}/Pictures"
cp ${BACKGROUND} "${VOLUME}/Pictures/background.tiff"

# Add symlink to Applications folder
echo "Add symlink to Applications folder"
ln -s /Applications/ "${VOLUME}/Applications"

# Decorate disk image
echo "Decorate disk image"
osascript <<-EOF
tell application "Finder"
    set f to POSIX file ("${VOLUME}" as string) as alias
    tell folder f
        open
        tell container window
            set toolbar visible to false
            set statusbar visible to false
            set current view to icon view
            delay 1 -- sync
            set the bounds to {0, 0, 589, 435}
        end tell
        delay 1 -- sync
        set icon size of the icon view options of container window to 120
        set arrangement of the icon view options of container window to not arranged
        set position of item "${APP_NAME}.app" to {150, 200}
        close
        set position of item "Applications" to {439, 200}
        open
        set background picture of the icon view options of container window to file "background.tiff" of folder "Pictures"
        set the bounds of the container window to {0, 0, 589, 435}
        update without registering applications
        delay 5 -- sync
        close
    end tell
    delay 5 -- sync
end tell
EOF

mv "${VOLUME}/Pictures" "${VOLUME}/.Pictures"

echo "Unmount"
for i in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20; do
    # Unmount the disk image
    hdiutil detach $DEV
    if [ $? -eq 0 ]; then
        break
    fi
    if [ $i -eq 20 ]; then
        echo "Failed to unmount the disk image; exiting after 20 retries."
        exit 1
    fi
    echo "Failed to unmount the disk image; retrying in 30s"
    sleep 30
done

# Convert the disk image to read-only
hdiutil convert "applebuild/${DMG_NAME}" -format UDBZ -o "applebuild/${COMPRESSED_DMG_NAME}"

shasum -a 256 "applebuild/${COMPRESSED_DMG_NAME}"

rm "applebuild/${DMG_NAME}"
