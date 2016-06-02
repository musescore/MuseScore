export QT_SHORT_VERSION=5.4
export QT_LONG_VERSION=5.4.2
export QT_INSTALLER_ROOT=qt-opensource-mac-x64-clang-${QT_LONG_VERSION}
export QT_INSTALLER_FILENAME=${QT_INSTALLER_ROOT}.dmg

export QT_PATH=$HOME/qt
export QT_MACOS=$QT_PATH/$QT_SHORT_VERSION/clang_64
export PATH=$PATH:$QT_MACOS/bin

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
