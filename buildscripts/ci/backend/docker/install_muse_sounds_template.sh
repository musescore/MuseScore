#!/usr/bin/env bash

echo "=== Install Muse Sounds Manager ==="

MUSE_SOUNDS_S3_URL=https://s3.amazonaws.com/extensions.musescore.org/4.0/musesounds

# Download DEB package
DEB_PACKAGE=Muse_Sounds_Manager_x64_0.0.0.890.deb
echo "Downloading Muse Sounds Manager from $MUSE_SOUNDS_S3_URL/$DEB_PACKAGE"

wget --show-progress -O /tmp/$DEB_PACKAGE "$MUSE_SOUNDS_S3_URL/$DEB_PACKAGE"

# Install DEB package
echo "Installing Muse Sounds Manager..."
dpkg -i /tmp/$DEB_PACKAGE || apt-get install -f -y

# Check installation with -v flag
echo "Verifying installation..."
muse-sounds-manager -v

# Clean up
rm /tmp/$DEB_PACKAGE

echo "Muse Sounds Manager installed successfully"
