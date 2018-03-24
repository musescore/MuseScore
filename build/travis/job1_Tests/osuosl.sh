#!/bin/bash

# Do not upload artefacts generated as part of a pull request
if [ $(env | grep TRAVIS_PULL_REQUEST) == "TRAVIS_PULL_REQUEST" ] ; then
  if [ "$TRAVIS_PULL_REQUEST" != "false" ] ; then
    echo "Not uploading since this is a pull request."
    exit 0
  fi
fi

FILE="$1"
[ -f "$FILE" ] || { echo "$0: Please provide a valid path to a file" >&2 ; exit 1 ;}

# instal ssh key
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

# transfer file
scp -C -i $SSH_INDENTITY $FILE musescore-nightlies@ftp-osl.osuosl.org:ftp/linux/src/$FILE_UPLOAD_PATH
