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

subdirs(
      album barline beam breath chordsymbol clef clef_courtesy compat concertpitch copypaste
      copypastesymbollist cursor durationtype dynamic earlymusic element exchangevoices hairpin instrumentchange join keysig layout links parts measure midi
      note plugins repeat rhythmicGrouping selectionfilter selectionrangedelete spanners split splitstaff timesig tools transpose tuplet text
      )

install(FILES
      ../../share/styles/chords_std.xml
      ../../share/styles/chords_jazz.xml
      ../../share/styles/chords.xml
      ../../share/styles/stdchords.xml
      ../../share/styles/jazzchords.xml
      ../../share/styles/cchords_muse.xml
      ../../share/styles/cchords_nrb.xml
      ../../share/styles/cchords_rb.xml
      ../../share/styles/cchords_sym.xml
      DESTINATION ${PROJECT_BINARY_DIR}/mtest/libmscore/styles
      )


