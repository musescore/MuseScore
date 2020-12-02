#=============================================================================
#  MuseScore
#  Music Composition & Notation
#
#  Copyright (C) 2002-2020 MuseScore BVBA and others
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#=============================================================================

set(MSCORE_UNSTABLE TRUE)
set(MSCORE_RELEASE_CHANNEL "testing")

if (NOT MUSESCORE_VERSION_LABEL)
    SET(MUSESCORE_VERSION_LABEL "Testing")
endif (NOT MUSESCORE_VERSION_LABEL)

SET(MUSESCORE_NAME_VERSION "${MUSESCORE_NAME} ${MUSESCORE_VERSION_MAJOR}.${MUSESCORE_VERSION_MINOR} ${MUSESCORE_VERSION_LABEL}")
