#!/bin/sh
#
# Copyright 2008 Eitan Isaacson
# Copyright 2008 Christian Egli
# Copyright 2012 Michael Whapples
#
# This file is part of liblouis.
# 
# liblouis is free software: you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation, either version 2.1 of the
# License, or (at your option) any later version.
# 
# liblouis is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public
# License along with liblouis. If not, see
# <http://www.gnu.org/licenses/>.
#
# autogen.sh glue for liblouis
#
# Requires: automake 1.9, autoconf 2.57+
# Conflicts: autoconf 2.13
set -e

# Refresh GNU autotools toolchain.
echo Cleaning autotools files...
find . -type d -name autom4te.cache -print0 | xargs -0 rm -rf \;
find . -type f \( -name missing -o -name install-sh -o -name mkinstalldirs \
	-o -name depcomp -o -name ltmain.sh -o -name configure \
	-o -name config.sub -o -name config.guess -o -name config.h.in \
        -o -name mdate-sh -o -name texinfo.tex \
	-o -name Makefile.in -o -name aclocal.m4 \) -print0 | xargs -0 rm -f

echo Running autoreconf...
autoreconf --force --install

exit 0
