#!/bin/bash
# Run this script from MuseScore's root directory

set -e # exit on error
set -x # echo commands


# update translation on transifex
if [ "$TRAVIS" = true ] && [ "$TRAVIS_PULL_REQUEST" == false ] ; then
pip install transifex-client

cat > ~/.transifexrc <<EOL
[https://www.transifex.com]
hostname = https://www.transifex.com
password = $TRANSIFEX_PASSWORD
token =
username = $TRANSIFEX_USER
EOL

tx push -s

fi

# build MuseScore
make revision
make debug CPUS=2 PREFIX="$HOME/software"
make installdebug CPUS=2 PREFIX="$HOME/software"
