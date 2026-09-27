#!/usr/bin/env bash

echo "Package"
trap 'echo Package failed; exit 1' ERR

APP_NAME="MuseScore Studio"
VOL_NAME="MuseScore-Studio"
DO_SIGN=false
# Matched against the common names of the identities in the keychain.
SIGN_IDENTITY="Developer ID Application: MuseScore"
APPLE_TEAM_ID=""
APPLE_USERNAME=""
APPLE_PASSWORD=""

while [[ "$#" -gt 0 ]]; do
    case $1 in
        --app-name) APP_NAME="$2"; shift ;;
        --vol-name) VOL_NAME="$2"; shift ;;
        --sign) DO_SIGN=true ;;
        --team-id) APPLE_TEAM_ID="$2"; shift ;;
        --user) APPLE_USERNAME="$2"; shift ;;
        --password) APPLE_PASSWORD="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

DO_NOTARIZE=false
if $DO_SIGN && [ -n "$APPLE_USERNAME" ] && [ -n "$APPLE_PASSWORD" ] && [ -n "$APPLE_TEAM_ID" ]; then
    DO_NOTARIZE=true
fi

# Sign one binary or bundle for distribution. --timestamp and --options runtime
# are both required for notarization; the entitlements file is optional, so pass
# an empty first argument to sign without one.
sign_code() {
    local entitlements="$1"; shift
    local entitlements_args=()
    if [ -n "$entitlements" ]; then
        entitlements_args=(--entitlements "$entitlements")
    fi
    codesign --force \
        --timestamp \
        --options runtime \
        -s "$SIGN_IDENTITY" \
        "${entitlements_args[@]}" \
        "$@"
}

# Submit one file to Apple's notary service and wait for the verdict. Always
# prints the notarization log, because notarytool's own summary rarely says what
# went wrong.
notarize_file() {
    local path="$1"
    local attempt status submit_output submission_id

    for attempt in 1 2 3; do
        status=0
        submit_output="$(xcrun notarytool submit "$path" \
            --apple-id "$APPLE_USERNAME" \
            --team-id "$APPLE_TEAM_ID" \
            --password "$APPLE_PASSWORD" \
            --wait 2>&1)" || status=$?
        echo "$submit_output"

        submission_id="$(awk '/id: / { print $2; exit }' <<<"$submit_output")"
        if [ -n "$submission_id" ]; then
            xcrun notarytool log "$submission_id" \
                --apple-id "$APPLE_USERNAME" \
                --team-id "$APPLE_TEAM_ID" \
                --password "$APPLE_PASSWORD" \
                || echo "Failed to fetch the notarization log"
        fi

        if [ $status -eq 0 ]; then
            return 0
        fi

        # Invalid and Rejected are the notary service's considered verdicts on
        # this exact file; submitting it again would only waste a round trip.
        if grep -qE 'status: (Invalid|Rejected)' <<<"$submit_output"; then
            echo "The notary service rejected ${path}; not retrying."
            return $status
        fi

        if [ $attempt -eq 3 ]; then
            echo "notarytool failed; giving up after 3 attempts."
            return $status
        fi

        # The notary service is occasionally unavailable, and a whole macOS
        # build is a lot to throw away over that.
        echo "notarytool failed; retrying in 30s"
        sleep 30
    done
}

################################################################
# Deploy
################################################################

APP_PATH=applebuild/mscore.app

echo "otool -L pre-macdeployqt"
otool -L ${APP_PATH}/Contents/MacOS/mscore

echo "macdeployqt"
# An array, so that an identity given by name rather than by hash does not
# word-split into four bogus arguments.
sign_args=()
if $DO_SIGN; then
    sign_args=("-sign-for-notarization=$SIGN_IDENTITY")
else
    # macdeployqt rewrites the load commands of everything it deploys, which
    # invalidates the signature that Qt shipped its libraries and plugins with.
    # Ad-hoc signatures are cheap and need no certificate, so use those; the
    # alternative is leaving behind binaries that macOS refuses to load.
    sign_args=("-codesign=-")
fi
macdeployqt ${APP_PATH} \
    -verbose=2 \
    -qmldir=. \
    -executable="${APP_PATH}/Contents/PlugIns/MuseScoreQuickLookPreviewExtension.appex/Contents/MacOS/MuseScoreQuickLookPreviewExtension" \
    "${sign_args[@]}"

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
    sign_code "src/macos_integration/entitlements-distribution.plist" \
        "${APP_PATH}/Contents/PlugIns/MuseScoreQuickLookPreviewExtension.appex"

    # Sign the bundled dylibs that are loaded at runtime and therefore
    # invisible to macdeployqt's dependency walk (e.g. libsndfile, vorbis)
    echo "Sign bundled dylibs"
    dylibs=()
    while IFS= read -r dylib; do
        dylibs+=("$dylib")
    done < <(find "${APP_PATH}/Contents/Frameworks" -maxdepth 1 -type f -name "*.dylib")
    if [ ${#dylibs[@]} -gt 0 ]; then
        sign_code "" "${dylibs[@]}"
    fi

    # Re-sign main app after removing dSYM files and renaming qml folder
    echo "Re-sign main app"
    sign_code "buildscripts/packaging/macOS/entitlements.plist" "${APP_PATH}"

    echo "Codesign verify"
    codesign --verify --deep --strict --verbose=2 "${APP_PATH}"

    # Notarize and staple the .app before sealing the DMG, so that it is
    # accepted by Gatekeeper even when it is dragged out of the DMG by hand.
    if $DO_NOTARIZE; then
        echo "Notarize the app"
        APP_ZIP="applebuild/app-notarization.zip"
        rm -f "$APP_ZIP"
        ditto -c -k --keepParent "${APP_PATH}" "$APP_ZIP"
        notarize_file "$APP_ZIP"
        rm -f "$APP_ZIP"

        echo "Staple the app"
        xcrun stapler staple "${APP_PATH}"
        xcrun stapler validate "${APP_PATH}"

        # Only meaningful now that the ticket is stapled: Gatekeeper rejects a
        # signed but unnotarized app.
        echo "spctl"
        spctl --assess --type execute -vvv "${APP_PATH}"
    else
        echo "Skipping notarization of the app"

        # Informational: without a ticket this can only say "Unnotarized
        # Developer ID", which is expected here and must not fail the build.
        echo "spctl (unnotarized build)"
        spctl --assess --type execute -vvv "${APP_PATH}" || true
    fi
else
    # Removing the dSYM bundles and renaming Resources/qml invalidated the
    # signatures that macdeployqt just made, so sign again, from the inside out.
    echo "Ad-hoc code sign"

    # As above: signed here rather than by macdeployqt, which cannot see them.
    find "${APP_PATH}/Contents/Frameworks" -maxdepth 1 -type f -name "*.dylib" \
        -exec codesign --force -s - {} +

    # The same flags CMake signs the extension with at build time.
    codesign --force \
        --options runtime \
        --entitlements "src/macos_integration/entitlements.plist" \
        -s - \
        "${APP_PATH}/Contents/PlugIns/MuseScoreQuickLookPreviewExtension.appex"

    codesign --force -s - "${APP_PATH}"

    echo "Codesign verify"
    codesign --verify --deep --strict --verbose=2 "${APP_PATH}"
fi

################################################################
# Create DMG
################################################################

DMG_NAME="${VOL_NAME}-uncompressed.dmg"
COMPRESSED_DMG_NAME="${VOL_NAME}.dmg"

rm -f "applebuild/${COMPRESSED_DMG_NAME}"

# Tip: increase the size if error on copy
hdiutil create -size 800m -fs APFS -volname "${VOL_NAME}" "applebuild/${DMG_NAME}"

# Mount the disk image
VOLUME="/Volumes/${VOL_NAME}"
ATTACH_OUTPUT=$(hdiutil attach "applebuild/${DMG_NAME}" -mountpoint "${VOLUME}")
echo "${ATTACH_OUTPUT}"
DEV=$(echo "${ATTACH_OUTPUT}" | head -n1 | awk '{print $1}')

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
    # Detach by device: a failed eject can leave the image attached with the
    # mountpoint already gone
    if hdiutil detach "${DEV}"; then
        break
    fi
    if ! hdiutil info | grep -qE "^${DEV}(s[0-9]+)?[[:space:]]"; then
        echo "Disk image is already detached"
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
hdiutil convert "applebuild/${DMG_NAME}" -format ULFO -o "applebuild/${COMPRESSED_DMG_NAME}"

if $DO_SIGN; then
    echo "Codesign DMG"
    codesign --timestamp \
        -s "$SIGN_IDENTITY" \
        "applebuild/${COMPRESSED_DMG_NAME}"

    codesign --verify --verbose=2 "applebuild/${COMPRESSED_DMG_NAME}"

    # A second submission, on top of the one for the .app: the ticket stapled to
    # the app covers the app alone, and it is the DMG that Gatekeeper checks
    # when someone opens the download.
    if $DO_NOTARIZE; then
        echo "Notarize the DMG"
        notarize_file "applebuild/${COMPRESSED_DMG_NAME}"

        echo "Staple the DMG"
        xcrun stapler staple "applebuild/${COMPRESSED_DMG_NAME}"
        xcrun stapler validate "applebuild/${COMPRESSED_DMG_NAME}"

        # The last word on whether what we are about to ship would open on
        # someone else's Mac, asked of the .app as it is actually distributed:
        # inside the sealed DMG.
        echo "Check the Gatekeeper policy"
        MOUNT_POINT="$(mktemp -d)"
        hdiutil attach "applebuild/${COMPRESSED_DMG_NAME}" \
            -mountpoint "$MOUNT_POINT" -nobrowse -readonly
        MOUNTED_APP="$(ls -d "$MOUNT_POINT"/*.app)"
        syspolicy_status=0
        xcrun syspolicy_check distribution "$MOUNTED_APP" || syspolicy_status=$?
        hdiutil detach "$MOUNT_POINT"
        rmdir "$MOUNT_POINT" || true
        if [ $syspolicy_status -ne 0 ]; then
            echo "syspolicy_check failed"
            exit 1
        fi
        echo "syspolicy_check passed"
    else
        echo "Skipping notarization of the DMG"
    fi
fi

shasum -a 256 "applebuild/${COMPRESSED_DMG_NAME}"

rm "applebuild/${DMG_NAME}"
