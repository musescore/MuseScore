#!/bin/sh

cd "$(dirname "$0")"

case $1 in
(portable|p)
	portable=''
	;;
(*)
	portable='.\\"'
	;;
esac

sed \
    -e 's!@MAN_MSCORE_UPPER@!MSCORE!g' \
    -e 's!@Variables_substituted_by_CMAKE_on_installation@!!g' \
    -e 's!@MSCORE_INSTALL_SUFFIX@!!g' \
    -e 's!@MUSESCORE_NAME_VERSION@!MuseScore 3!g' \
    -e 's!@MAN_PORTABLE@!'"$portable"'!g' \
    -e 's!@PORTABLE_INSTALL_NAME@!MuseScorePortable!g' \
    -e 's!@CMAKE_INSTALL_PREFIX@!/usr!g' \
    -e 's!@Mscore_SHARE_NAME@!share/!g' \
    -e 's!@Mscore_INSTALL_NAME@!mscore-3.0/!g' \
    <mscore.1.in | man -l -
