# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2026 MuseScore Limited
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
# along with this program. If not, see <https://www.gnu.org/licenses/>.

muse_create_module(muse_audio_common)

target_include_directories(muse_audio_common PRIVATE ..)

target_sources(muse_audio_common PRIVATE
    audiotypes.h
    soundfonttypes.h
    audioerrors.h
    audioutils.h
    audiosanitizer.cpp
    audiosanitizer.h
    iaudiothreadsecurer.h
    audiothreadsecurer.cpp
    audiothreadsecurer.h
    workmode.cpp
    workmode.h
    alignmentbuffer.h

    # Rpc
    rpc/irpcchannel.h
    rpc/rpcpacker.h
)

if(OS_IS_WASM)
    target_sources(muse_audio_common PRIVATE
        rpc/platform/web/webrpcchannel.cpp
        rpc/platform/web/webrpcchannel.h
    )
else()
    target_sources(muse_audio_common PRIVATE
        rpc/platform/general/generalrpcchannel.cpp
        rpc/platform/general/generalrpcchannel.h
    )
endif()
