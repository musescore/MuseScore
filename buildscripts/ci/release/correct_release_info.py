#!/usr/bin/env python3
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
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

import sys
def eprint(*args, **kwargs):
    print(*args, **kwargs, file=sys.stderr)

if __name__ == '__main__' and sys.prefix == sys.base_prefix:
    # Not running inside a virtual environment. Let's try to load one.
    import os
    old_dir = os.path.realpath(__file__)
    new_dir, script_name = os.path.split(old_dir)
    rel_pyi = r'.venv\Scripts\python.exe' if sys.platform == 'win32' else '.venv/bin/python'
    while new_dir != old_dir:
        abs_pyi = os.path.join(new_dir, rel_pyi)
        if os.access(abs_pyi, os.X_OK):
            eprint(f'{script_name}: Loading virtual environment:\n  {abs_pyi}')
            if sys.platform == 'win32':
                import subprocess
                raise SystemExit(subprocess.run([abs_pyi, *sys.argv]).returncode)
            os.execl(abs_pyi, abs_pyi, *sys.argv)
        old_dir = new_dir
        new_dir = os.path.dirname(new_dir)
    eprint(f'{script_name}: Not running inside a virtual environment.')
    del old_dir, new_dir, rel_pyi, abs_pyi, script_name

import sys
import json
import markdown

RELEASE_INFO_FILE = sys.argv[1]

eprint("=== Load json ===")

json_file = open(RELEASE_INFO_FILE, "r+")
release_info_json = json.load(json_file)
json_file.close()

eprint("=== Make html version of body ===")

release_body_markdown = release_info_json["body"]

release_body_html = markdown.markdown(release_body_markdown)

# Correct result of Markdown parser
# Escape single quotes
release_body_html = release_body_html.replace("'", "`")

# Correct new lines next to <ul> and </ul>
release_body_html = release_body_html.replace("\n<ul>\n", "<ul>")
release_body_html = release_body_html.replace("\n</ul>\n", "</ul>")

release_info_json["body"] = "'" + release_body_html + "'"
release_info_json["bodyMarkdown"] = release_body_markdown

eprint("=== Split release assets ===")

# For backward compatibility, we must adhere to the rule:
#   for each platform, there should be one file with the corresponding extension.
# Let's place the new files into a new field with separate handling in MuseScore.

release_assets = release_info_json["assets"]
release_new_assets = []

i = 0
while i < len(release_assets):
    asset = release_assets[i]
    name = asset.get("name")
    if ".AppImage" in name and ("aarch64" in name or "armv7l" in name):
        release_new_assets.append(asset)
        del release_assets[i]
    else:
        i += 1

release_info_json["assets"] = release_assets
release_info_json["assetsNew"] = release_new_assets

release_info_json_updated = json.dumps(release_info_json)

eprint("=== Write json ===")

json_file = open(RELEASE_INFO_FILE, "w")
json_file.write(release_info_json_updated)
json_file.close()
