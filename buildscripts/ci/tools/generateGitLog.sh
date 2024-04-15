#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore Limited
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.set -e

options="--all -10"
messages_command="git log --pretty=tformat:'%s' ${options[@]}"
lines_command="git log --pretty=tformat:'<b>%h</b> -  __XX123XX__ </li>' ${options[@]}"

# damn bash arrays
OLDIFS=$IFS
IFS=$'\n'

messages=($(bash -c "$messages_command"))
lines=($(bash -c "$lines_command"))

IFS=$OLDIFS

mlen=${#messages[@]}
llen=${#lines[@]}

OUTPUT=${OUTPUT}"<ul>"
for (( i=0; i<${mlen}; i++ )); do
  message=$(perl -MHTML::Entities -e '$msg=encode_entities($ARGV[1]);$l=$ARGV[0];$l =~ s/__XX123XX__/$msg/;print $l;' "${lines[$i]}" "${messages[$i]}")
  OUTPUT=${OUTPUT}"<li>$message"
done
OUTPUT=${OUTPUT}"</ul>"

echo $OUTPUT

