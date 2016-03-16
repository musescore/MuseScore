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

add_custom_target(referenceDocumentation
      ALL
      DEPENDS en/doc.html de/doc.html en/doc.qhp de/doc.qhp
      )

add_custom_command(TARGET referenceDocumentation
      POST_BUILD
      COMMAND Qt5::qcollectiongenerator ${PROJECT_SOURCE_DIR}/rdoc/en/doc.qhcp -o doc_en.qhc
      COMMAND Qt5::qcollectiongenerator ${PROJECT_SOURCE_DIR}/rdoc/de/doc.qhcp -o doc_de.qhc
      )

install(FILES
      ${PROJECT_BINARY_DIR}/rdoc/doc_en.qhc
      ${PROJECT_BINARY_DIR}/rdoc/doc_de.qhc
  DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}manual)

