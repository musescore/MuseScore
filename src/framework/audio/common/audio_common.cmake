# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-CLA-applies
#
# MuseScore
# Music Composition & Notation
#
# Copyright (C) 2025 MuseScore BVBA and others
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

set(AUDIO_COMMON_SRC

    ${CMAKE_CURRENT_LIST_DIR}/audiotypes.h
    ${CMAKE_CURRENT_LIST_DIR}/soundfonttypes.h
    ${CMAKE_CURRENT_LIST_DIR}/audioerrors.h
    ${CMAKE_CURRENT_LIST_DIR}/audiosanitizer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/audiosanitizer.h
    ${CMAKE_CURRENT_LIST_DIR}/audioutils.h

    # Rpc
    ${CMAKE_CURRENT_LIST_DIR}/rpc/irpcchannel.h
    ${CMAKE_CURRENT_LIST_DIR}/rpc/rpcpacker.h
    ${CMAKE_CURRENT_LIST_DIR}/rpc/platform/general/generalrpcchannel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rpc/platform/general/generalrpcchannel.h
)
