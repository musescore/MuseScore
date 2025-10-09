#!/usr/bin/env bash

echo "=== Clear All Muse Sounds ==="

MUSE_SOUNDS_PATH="/root/Muse Sounds"

if [ -d "$MUSE_SOUNDS_PATH" ]; then
    echo "Removing all Muse Sounds from $MUSE_SOUNDS_PATH..."
    rm -rf "$MUSE_SOUNDS_PATH"
    
    if [ $? -eq 0 ]; then
        echo "All Muse Sounds removed successfully"
    else
        echo "Error: Failed to remove Muse Sounds"
        exit 1
    fi
else
    echo "Muse Sounds directory not found at $MUSE_SOUNDS_PATH"
    echo "Nothing to clear"
fi

