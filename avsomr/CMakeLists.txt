#=============================================================================
#  MuseScore
#  Music Composition & Notation
#
#  Copyright (C) 2020 MuseScore BVBA and others
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

add_library (avsomr STATIC
      avsimport.cpp
      avslog.h
      ret.cpp
      ret.h
      avsomr.cpp
      avsomr.h
      avsomrreader.cpp
      avsomrreader.h
      msmrfile.cpp
      msmrfile.h
      msmrwriter.cpp
      msmrwriter.h
      avsomrdrawer.cpp
      avsomrdrawer.h
      iavsomrrecognizer.h
      avsomrnetrecognizer.cpp
      avsomrnetrecognizer.h
      avsomrlocal.cpp
      avsomrlocal.h
      avsomrlocalrecognizer.cpp
      avsomrlocalrecognizer.h
      avsomrlocalinstaller.cpp
      avsomrlocalinstaller.h
      avsomrsetup.cpp
      avsomrsetup.h
      ui/recognitionproccessdialog.cpp
      ui/recognitionproccessdialog.h
      ui/taskbarprogress.cpp
      ui/taskbarprogress.h
      ui/infopopup.cpp
      ui/infopopup.h
      ui/setupavsomrview.cpp
      ui/setupavsomrview.h
      )

target_include_directories(avsomr
   PRIVATE
      ${PROJECT_SOURCE_DIR}
   )

if (BUILD_PCH)
   target_link_libraries(avsomr global)
   target_precompile_headers(avsomr REUSE_FROM global)
endif (BUILD_PCH)
