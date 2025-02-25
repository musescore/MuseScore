#!/bin/bash

# LONG_NAME is used for dmg naming, LONGER_NAME is used when renaming the .app later. LONGER_NAME can't be updated
# to "MuseScore Studio" yet, as it will prevent users from seeing the prompt asking them to replace an old version
# of MuseScore (potentially resulting in 2 copies of version 4). This will be updated on the next major release.
APPNAME=mscore
LONG_NAME="MuseScore-Studio"
LONGER_NAME="MuseScore 4"
VERSION=0

while [[ "$#" -gt 0 ]]; do
    case $1 in
        --long_name) LONG_NAME="$2"; shift ;;
        --longer_name) LONGER_NAME="$2"; shift ;;
        --version) VERSION=$2; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

if [ "$VERSION" = 0 ]; then
    SCRIPT_DIR=$(cd "$(dirname ${BASH_SOURCE[0]})" && pwd)
    VERSION=$(cmake -P "$SCRIPT_DIR/../config.cmake" | sed -n -e "s/^.*VERSION  *//p")
fi

echo "LONG_NAME: $LONG_NAME"
echo "LONGER_NAME: $LONGER_NAME"
echo "VERSION: $VERSION"

WORKING_DIRECTORY=applebuild
BACKGROUND=buildscripts/packaging/macOS/musescore-dmg-background.tiff
APP_PATH=applebuild/${APPNAME}.app

VOLNAME=${LONG_NAME}-${VERSION}
DMGNAME=${VOLNAME}Uncompressed.dmg
COMPRESSEDDMGNAME=${VOLNAME}.dmg

rm ${WORKING_DIRECTORY}/${COMPRESSEDDMGNAME}

# Tip: increase the size if error on copy or macdeployqt
hdiutil create -size 750m -fs HFS+ -volname ${VOLNAME} ${WORKING_DIRECTORY}/${DMGNAME}

# Mount the disk image
hdiutil attach ${WORKING_DIRECTORY}/${DMGNAME}

# Obtain device information
DEVS=$(hdiutil attach ${WORKING_DIRECTORY}/${DMGNAME} | cut -f 1)
DEV=$(echo $DEVS | cut -f 1 -d ' ')
VOLUME=$(mount | grep ${DEV} | cut -f 3 -d ' ')

# copy in the application bundle
cp -Rp ${APP_PATH} ${VOLUME}/${APPNAME}.app

# Deploy
echo "otool -L pre-macdeployqt"
otool -L ${VOLUME}/${APPNAME}.app/Contents/MacOS/mscore

macdeployqt ${VOLUME}/${APPNAME}.app -verbose=2 -qmldir=.

echo "otool -L post-macdeployqt"
otool -L ${VOLUME}/${APPNAME}.app/Contents/MacOS/mscore

# Remove dSYM files
echo "Remove dSYM files"
find ${VOLUME}/${APPNAME}.app/Contents -type d -name "*.dSYM" -exec rm -r {} +

# Rename Resources/qml to Resources/qml_mu. This way, VST3 plugins that also use QML
# won't find these QML files, to prevent crashes because of conflicts.
# https://github.com/musescore/MuseScore/issues/21372
# https://github.com/musescore/MuseScore/issues/24331
echo "Rename Resources/qml to Resources/qml_mu"
mv ${VOLUME}/${APPNAME}.app/Contents/Resources/qml ${VOLUME}/${APPNAME}.app/Contents/Resources/qml_mu
sed -i '' 's:Resources/qml:Resources/qml_mu:g' ${VOLUME}/${APPNAME}.app/Contents/Resources/qt.conf

echo "Rename ${APPNAME}.app to ${VOLUME}/${LONGER_NAME}.app"
mv ${VOLUME}/${APPNAME}.app "${VOLUME}/${LONGER_NAME}.app"

# Copy in background image
echo "Copy in background image"
mkdir -p ${VOLUME}/Pictures
cp ${BACKGROUND} ${VOLUME}/Pictures/background.tiff

# Add symlink to Applications folder
echo "Add symlink to Applications folder"
ln -s /Applications/ ${VOLUME}/Applications

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
        set position of item "${LONGER_NAME}.app" to {150, 200}
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

mv ${VOLUME}/Pictures ${VOLUME}/.Pictures

# Codesign
echo "Codesign"
# `codesign --deep` doesn't seem to search for code in Contents/Resources directory so sign libraries in it manually
find "${VOLUME}/${LONGER_NAME}.app/Contents/Resources" -name '*.dylib' -exec codesign --force --options runtime --deep -s "Developer ID Application: MuseScore" '{}' ';'
# Sign code in other (more conventional) locations
codesign --force --options runtime --entitlements "${WORKING_DIRECTORY}/../buildscripts/packaging/macOS/entitlements.plist" --deep -s "Developer ID Application: MuseScore" "${VOLUME}/${LONGER_NAME}.app"
echo "spctl"
spctl --assess --type execute "${VOLUME}/${LONGER_NAME}.app"
echo "Codesign verify"
codesign --verify --deep --strict --verbose=2 "${VOLUME}/${LONGER_NAME}.app"

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
hdiutil convert ${WORKING_DIRECTORY}/${DMGNAME} -format UDBZ -o ${WORKING_DIRECTORY}/${COMPRESSEDDMGNAME}

shasum -a 256 ${WORKING_DIRECTORY}/${COMPRESSEDDMGNAME}

rm ${WORKING_DIRECTORY}/${DMGNAME}
