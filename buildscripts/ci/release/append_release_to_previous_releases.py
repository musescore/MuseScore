#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2024 MuseScore Limited
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
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

import sys
import json

CURRENT_RELEASE_INFO_FILE = sys.argv[1]
PREVIOUS_RELEASE_INFO_FILE = sys.argv[2]

print("=== Load jsons ===")

json_file = open(CURRENT_RELEASE_INFO_FILE, "r+")
current_release_info_json = json.load(json_file)
json_file.close()

json_file = open(PREVIOUS_RELEASE_INFO_FILE, "r+")
previous_release_info_json = json.load(json_file)
json_file.close()

print("=== Append current release notes to previous releases notes ===")

tag_name = current_release_info_json["tag_name"]
version = tag_name[1:]
new_release = {"version": version, "notes": current_release_info_json["bodyMarkdown"]}

if "releases" not in previous_release_info_json:
    previous_release_info_json["releases"] = []

is_release_already_in_previous_releases = False
for release in previous_release_info_json["releases"]:
    if release["version"] in version:
        release["notes"] = new_release["notes"]
        is_release_already_in_previous_releases = True

if not is_release_already_in_previous_releases:
    previous_release_info_json["releases"].append(new_release)

previous_release_info_json_updated = json.dumps(previous_release_info_json)

print("=== Write json ===")

json_file = open(PREVIOUS_RELEASE_INFO_FILE, "w")
json_file.write(previous_release_info_json_updated)
json_file.close()
