#=============================================================================
#  MuseScore
#  Music Composition & Notation
#  $Id:$
#
#  Copyright (C) 2011 Werner Schweer
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2
#  as published by the Free Software Foundation and appearing in
#  the file LICENSE.GPL
#=============================================================================

add_custom_target(referenceDocumentation
      ALL
      DEPENDS doc.html doc.qhp
      )

add_custom_command(TARGET referenceDocumentation
      POST_BUILD
      COMMAND qcollectiongenerator ${PROJECT_SOURCE_DIR}/rdoc/doc.qhcp -o doc.qhc
      )

install(FILES ${PROJECT_BINARY_DIR}/rdoc/doc.qhc DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}manual)

