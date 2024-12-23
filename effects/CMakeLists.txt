#=============================================================================
#  MuseScore
#  Music Composition & Notation
#
#  Copyright (C) 2013 Werner Schweer
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2
#  as published by the Free Software Foundation and appearing in
#  the file LICENSE.GPL
#=============================================================================

if (APPLE)
      file(GLOB_RECURSE INCS "*.h")
else (APPLE)
      set(INCS "")
endif (APPLE)

QT5_ADD_RESOURCES (qrc_effects_files
     zita1/zita.qrc
     )

QT5_WRAP_UI (ui_headers
      compressor/compressor_gui.ui
      )

add_library (effects STATIC
      ${ui_headers}
      ${qrc_effects_files}
      effect.cpp
      effectgui.cpp
      noeffect/noeffect.cpp
      noeffect/noeffectgui.h
      zita1/zita.cpp
      zita1/zitagui.cpp
      compressor/compressor.cpp
      compressor/compressorgui.cpp
      ${INCS}
      )

if (BUILD_PCH)
    target_use_pch(effects)
endif (BUILD_PCH)
