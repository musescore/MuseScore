#=============================================================================
#  MuseScore
#  Music Composition & Notation
#
#  Copyright (C) 2011 Werner Schweer
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2
#  as published by the Free Software Foundation and appearing in
#  the file LICENSE.GPL
#=============================================================================

include (${PROJECT_SOURCE_DIR}/build/functions.cmake)

set(MANUAL_DEST "${PROJECT_BINARY_DIR}/share/manual/plugins/plugins3.html")

fn__copy_during_build(
      "${CMAKE_CURRENT_SOURCE_DIR}/plugins3.html"
      "${MANUAL_DEST}"
      )

add_custom_target(pluginDocumentation
      ALL
      DEPENDS "${MANUAL_DEST}"
      )
