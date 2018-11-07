#=============================================================================
#  Mscore
#  Linux Music Score Editor
#
#  Copyright (C) 2008 by Werner Schweer and others
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

set(CMAKE_C_FLAGS "-DNEWBUFFER -DPMNULL")

if (APPLE)
  file(GLOB_RECURSE INCS "*.h")

  add_library (
    portmidi STATIC
      ${INCS}
      pm_common/pmutil.c
      pm_common/portmidi.c
      pm_mac/finddefault.c
      pm_mac/pmmac.c
      pm_mac/pmmacosxcm.c
      pm_mac/readbinaryplist.c
      porttime/porttime.c
      porttime/ptmacosx_mach.c
      porttime/ptmacosx_cf.c
  )
else (APPLE)
  add_library (
    portmidi STATIC
      pm_common/pmutil.c
      pm_win/pmwinmm.c
      pm_win/pmwin.c
      pm_common/portmidi.c
      porttime/porttime.c
      porttime/ptwinmm.c
  )
endif (APPLE)

if (MSVC)
  set_target_properties( portmidi PROPERTIES
    COMPILE_FLAGS "/wd4028 /wd4057 /wd4100 /wd4133 /wd4189 /wd4206 /wd4244 /wd4267 /wd4311 /wd4312 /wd4456 /wd4706"
  )
else (MSVC)
  if (MINGW)
    if (BUILD_64)
      set_target_properties( portmidi PROPERTIES
        COMPILE_FLAGS "-Wno-int-to-pointer-cast -Wno-pointer-to-int-cast -Wno-incompatible-pointer-types"
      )
    endif (BUILD_64)
  endif (MINGW)
endif (MSVC)

include_directories(
      ${PROJECT_SOURCE_DIR}/thirdparty/portmidi/pm_common
      ${PROJECT_SOURCE_DIR}/thirdparty/portmidi/porttime
      )


