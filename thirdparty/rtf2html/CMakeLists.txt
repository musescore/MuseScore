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

declare_module(rtf2html)

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

set(MODULE_USE_COVERAGE OFF)

setup_module()

target_no_warning(${MODULE} -Wno-conversion)
target_no_warning(${MODULE} -Wno-deprecated-declarations)
