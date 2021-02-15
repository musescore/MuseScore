#!/usr/bin/env bash

BASE_DIR=$1

echo "Install ccache"
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    sudo apt install ccache
elif [[ "$OSTYPE" == "darwin"* ]]; then
    brew install ccache
else
    echo "Unsopported OS: $OSTYPE"; exit 1;
fi

echo "Setup config"

mkdir -p ~/.ccache
echo "base_dir = ${BASE_DIR}" > ~/.ccache/ccache.conf
echo "compression = true" >> ~/.ccache/ccache.conf
echo "compression_level = 6" >> ~/.ccache/ccache.conf
echo "max_size = 2G" >> ~/.ccache/ccache.conf
echo "sloppiness=pch_defines,time_macros" >> ~/.ccache/ccache.conf
cat ~/.ccache/ccache.conf
ccache -s
ccache -z      