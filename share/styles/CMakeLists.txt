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

install(FILES
      MuseJazz.mss
      chords_legacy.xml
      chords_std.xml
      chords_jazz.xml
      chords.xml
      stdchords.xml
      jazzchords.xml
      cchords_muse.xml
      cchords_nrb.xml
      cchords_rb.xml
      cchords_sym.xml
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}styles
      )

add_subdirectory(MSN)
