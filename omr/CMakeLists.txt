#=============================================================================
#  MusE
#  Linux Music Editor
#
#  Copyright (C) 2002-2006 by Werner Schweer and others
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

if (OCR)
      set (OCR_SRC ocr.cpp)
endif (OCR)

add_library (omr STATIC
      omrview.cpp pdf.cpp omrpage.cpp
      skew.cpp utils.cpp
      omr.cpp pattern.cpp importpdf.cpp
      ${OCR_SRC}
      )
      
if (BUILD_PCH)
      target_link_libraries(omr global)
      target_precompile_headers(omr REUSE_FROM global)
endif (BUILD_PCH)
