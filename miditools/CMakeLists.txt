#=============================================================================
#  MuseScore
#  Music Composition & Notation
#
#  Copyright (C) 2013 Werner Schweer
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2
#  as published by the Free Software Foundation and appearing in
#  the file LICENCE.GPL
#=============================================================================

add_executable (smf2xml smf2xml.cpp xmlwriter.cpp midifile.cpp)

set_target_properties(smf2xml
      PROPERTIES COMPILE_FLAGS "-g -Wall -Wextra"
      )
target_link_libraries(smf2xml
      ${QT_LIBRARIES}
      )

add_executable (xml2smf xml2smf.cpp xmlreader.cpp midifile.cpp)

set_target_properties (
      xml2smf
      PROPERTIES COMPILE_FLAGS "-g -Wall -Wextra"
      )
target_link_libraries(xml2smf
      ${QT_LIBRARIES}
      )

