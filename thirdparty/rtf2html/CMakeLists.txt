#[[
// SPDX-License-Identifier: GPL-3.0-only
// MuseScore-CLA-applies
//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 3 as
//  published by the Free Software Foundation.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.
//=============================================================================]]

set(MODULE rtf2html)

set(MODULE_SRC
    common.h
    config.h
    dbg_iter.h
    fmt_opts.cpp
    fmt_opts.h
    rtf2html.cpp
    rtf_keyword.cpp
    rtf_keyword.h
    rtf_table.cpp
    rtf_table.h
    rtf_tools.h
    )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)

target_no_warning(${MODULE} -Wno-conversion)
