#!/usr/bin/env bash

MUSE_CDN_URL=https://muse-cdn.com
DEB_PACKAGE=Muse_Sounds_Manager_x64.deb

echo "=== Install Muse Sounds Manager ==="
echo "Downloading Muse Sounds Manager from $MUSE_CDN_URL/$DEB_PACKAGE"

# Download DEB package
wget --show-progress -O /tmp/$DEB_PACKAGE "$MUSE_CDN_URL/$DEB_PACKAGE"

# Install DEB package
echo "Installing Muse Sounds Manager..."
dpkg -i /tmp/$DEB_PACKAGE || apt-get install -f -y

# Check installation with -v flag
echo "Verifying installation..."
muse-sounds-manager -v

# Install MuseSounds
# Muse Strings
muse-sounds-manager --headless --install-musesounds=0013e17f-55dd-42fb-bba3-110337a64849

# Clean up
rm /tmp/$DEB_PACKAGE

echo "Muse Sounds Manager installed successfully"
