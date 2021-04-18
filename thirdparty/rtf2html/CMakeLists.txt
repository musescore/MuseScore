#=============================================================================
#  Mscore
#  Linux Music Score Editor
#
#  Copyright (C) 2009 by Werner Schweer and others
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
