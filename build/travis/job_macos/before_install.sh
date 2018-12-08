#!/bin/bash

export QT_SHORT_VERSION=5.9
export QT_LONG_VERSION=5.9.3
export QT_INSTALLER_ROOT=qt-opensource-mac-x64-clang-${QT_LONG_VERSION}
export QT_INSTALLER_FILENAME=${QT_INSTALLER_ROOT}.dmg

export QT_PATH=$HOME/qt
export QT_MACOS=$QT_PATH/$QT_SHORT_VERSION/clang_64
export PATH=$PATH:$QT_MACOS/bin

if [ "${TRAVIS_PULL_REQUEST}" = "false" ]; then
openssl aes-256-cbc -K $encrypted_99b076488ab1_key -iv $encrypted_99b076488ab1_iv -in build/travis/resources/osuosl_nighlies_rsa.enc -out build/travis/resources/osuosl_nighlies_rsa -d
# Make ssh dir
mkdir $HOME/.ssh/
# Copy over private key, and set permissions
cp build/travis/resources/osuosl_nighlies_rsa $HOME/.ssh/osuosl_nighlies_rsa
# set permission
chmod 600 $HOME/.ssh/osuosl_nighlies_rsa
# Create known_hosts
touch $HOME/.ssh/known_hosts
# Add osuosl key to known host
ssh-keyscan ftp-osl.osuosl.org >> $HOME/.ssh/known_hosts

expect << EOF
  spawn ssh-add $HOME/.ssh/osuosl_nighlies_rsa
  expect "Enter passphrase"
  send "${OSUOSL_NIGHTLY_PASSPHRASE}\r"
  expect eof
EOF

#set NIGHTLY_BUILD variable if MSCORE_UNSTABLE is TRUE in CMakeLists.txt
if [ "$(grep '^[[:blank:]]*set( *MSCORE_UNSTABLE \+TRUE *)' CMakeLists.txt)" ]
then
export NIGHTLY_BUILD=TRUE
fi

export MSCORE_RELEASE_CHANNEL=$(grep '^[[:blank:]]*set *( *MSCORE_RELEASE_CHANNEL' CMakeLists.txt | awk -F \" '{print $2}')

fi
