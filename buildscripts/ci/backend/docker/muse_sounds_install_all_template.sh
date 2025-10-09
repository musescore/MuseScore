#!/usr/bin/env bash

echo "=== Install All Muse Sounds ==="

# All available Muse Sounds UUIDs
# Muse Strings = 0013e17f-55dd-42fb-bba3-110337a64849
# Muse Woodwinds = f2fbf84c-06da-45c9-be57-f2722d9fd363
# Muse Brass = fbd15858-9185-41ed-918b-8e360a698dbe
# Muse Percussion = e4a60bd8-4a44-4c5a-ad3a-3bccf48a5f64
# Muse Choir = 6a0d49c3-df4b-4f86-ad8f-61c131b7428c
# Muse Harp = 3a82bdc5-b4aa-4c02-aca1-1b3d4914154a
# Muse Keys = d6353666-c534-4c48-833e-c86f0926656f
# Muse Guitars Vol. 1 = 90c55796-5d20-47de-ac4c-62a641b4dbed
# Muse Drumline = d2e779d2-3ec9-4337-82c4-f689f7dd9581

ALL_MUSE_SOUNDS_UUIDS="0013e17f-55dd-42fb-bba3-110337a64849,f2fbf84c-06da-45c9-be57-f2722d9fd363,fbd15858-9185-41ed-918b-8e360a698dbe,e4a60bd8-4a44-4c5a-ad3a-3bccf48a5f64,6a0d49c3-df4b-4f86-ad8f-61c131b7428c,3a82bdc5-b4aa-4c02-aca1-1b3d4914154a,d6353666-c534-4c48-833e-c86f0926656f,90c55796-5d20-47de-ac4c-62a641b4dbed,d2e779d2-3ec9-4337-82c4-f689f7dd9581"

echo "Installing all Muse Sounds..."
echo "UUIDs: $ALL_MUSE_SOUNDS_UUIDS"

muse-sounds-manager --headless --install-musesounds=$ALL_MUSE_SOUNDS_UUIDS

if [ $? -eq 0 ]; then
    echo "All Muse Sounds installed successfully"
else
    echo "Error: Failed to install Muse Sounds"
    exit 1
fi

