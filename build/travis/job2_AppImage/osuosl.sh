#!/bin/bash

# Do not upload artifacts generated as part of a pull request
if [ $(env | grep TRAVIS_PULL_REQUEST) == "TRAVIS_PULL_REQUEST" ] ; then
  if [ "$TRAVIS_PULL_REQUEST" != "false" ] ; then
    echo "Not uploading AppImage since this is a pull request."
    exit 0
  fi
fi

FILE="$1"
[ -f "$FILE" ] || { echo "$0: Please provide a valid path to a file" >&2 ; exit 1 ;}

# install ssh key
openssl aes-256-cbc -K $encrypted_99b076488ab1_key -iv $encrypted_99b076488ab1_iv -in build/travis/resources/osuosl_nighlies_rsa.enc -out build/travis/resources/osuosl_nighlies_rsa -d

# Copy over private key, and set permissions
cp build/travis/resources/osuosl_nighlies_rsa $HOME/.ssh/osuosl_nighlies_rsa
# set permission
chmod 600 $HOME/.ssh/osuosl_nighlies_rsa
# Create known_hosts
touch $HOME/.ssh/known_hosts
# Add osuosl key to known host
ssh-keyscan ftp-osl.osuosl.org >> $HOME/.ssh/known_hosts

eval "$(ssh-agent -s)"
expect << EOF
  spawn ssh-add $HOME/.ssh/osuosl_nighlies_rsa
  expect "Enter passphrase"
  send "${OSUOSL_NIGHTLY_PASSPHRASE}\r"
  expect eof
EOF

SSH_INDENTITY=$HOME/.ssh/osuosl_nighlies_rsa

FILE_UPLOAD_PATH="$(basename "${FILE}")"

# Read architecture from file name (characters between last dash and .AppImage)
ARCH="$(basename "$FILE" | sed -r 's|^.*-([^-]*)\.AppImage$|\1|')"
case "${ARCH}" in
  x86_64|amd64 )
    ARCH_NAME="x86_64"
    ;;
  i686|i386|i[345678]86 )
    ARCH_NAME="i686"
    ;;
  armel )
    ARCH_NAME="armel"
    ;;
  armhf )
    ARCH_NAME="armhf"
    ;;
  aarch64 )
    ARCH_NAME="aarch64"
    ;;
  * )
    echo "Error: unrecognised architecture '${ARCH}'" >&2
    exit 1
    ;;
esac

# transfer file
scp -C -i $SSH_INDENTITY $FILE musescore-nightlies@ftp-osl.osuosl.org:ftp/linux/$ARCH_NAME/$FILE_UPLOAD_PATH

# delete old files
ssh -i $SSH_INDENTITY musescore-nightlies@ftp-osl.osuosl.org "cd ~/ftp/linux/$ARCH_NAME; ls MuseScoreNightly* -t | tail -n +41 | xargs rm"
