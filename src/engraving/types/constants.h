/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MU_ENGRAVING_CONSTANTS_H
#define MU_ENGRAVING_CONSTANTS_H

#include "bps.h"

namespace mu::engraving {
struct Constants
{
    static constexpr int MSC_VERSION = 450;
    static constexpr const char* MSC_VERSION_STR = "4.50";

// History:
//    1.3   added staff->_barLineSpan
//    1.4   (Version 0.9)
//    1.5   save xoff/yoff in mm instead of pixel
//    1.6   save harmony base/root as tpc value
//    1.7   invert semantic of page fill limit
//    1.8   slur id, slur anchor in in Note
//    1.9   image size stored in mm instead of pixel (Versions 0.9.2 -0.9.3)
//    1.10  TextLine properties changed (Version 0.9.4)
//    1.11  Instrument name in part saved as TextC (Version 0.9.5)
//    1.12  use durationType, remove tickLen
//    1.13  Clefs: userOffset is not (mis)used for vertical layout position
//    1.14  save user modified beam position as spatium value (Versions 0.9.6 - 1.3)

//    1.15  save timesig inline; Lyrics "endTick" replaced by "ticks"
//    1.16  spanners (hairpin, trill etc.) are now inline and have no ticks anymore
//    1.17  new <Score> toplevel structure to support linked parts (excerpts)
//    1.18  save lyrics as subtype to chord/rest to allow them associated with
//          grace notes
//    1.19  replace text style numbers by text style names; box margins are now
//          used
//    1.20  instrument names are saved as html again
//    1.21  no cleflist anymore
//    1.22  timesig changed
//    1.23  measure property for actual length
//    1.24  default image size is spatium dependent
//      -   symbol numbers in TextLine() replaced by symbol names
//          TextStyle: frameWidth, paddingWidth are now in Spatium units (instead of mm)

//    2.00  (Version 2.0)
//    2.01  save SlurSegment position relative to staff
//    2.02  save instrumentId, note slashes
//    2.03  save Box topGap, bottomGap in spatium units
//    2.04  added hideSystemBarLine flag to Staff
//    2.05  breath segment changed to use tick of following chord rather than preceding chord
//    2.06  Glissando moved from final chord to start note (Version 2.0.x)
//
//    2.07  irregular, breakMMrest, more style options, system divider, bass string for tab (3.0)

//    3.00  (Version 3.0 alpha)
//    3.01  -
//    3.02  Engraving improvements for 3.6

//    4.00 (Version 4.0)
//       - The style is stored in a separate file (inside mscz)
//       - The ChordList is stored in a separate file (inside mscz)

//    4.10 (Version 4.1)
//       - New "Expression" item
//       - A bunch of new options for dynamics
//       - Clefs carry a "header" tag in the file (istead of trying to guess it from context)
//       - New "Ornament" item with new properties and options
//       - New "Capo" item

//    4.20 (Version 4.2)
//       - By default, frames are not cloned to parts
//       - Corrections to key signature and transposition (#18998)
//       - New inside/outside style for ties

//    4.40 (Version 4.4)
//       - New property for cross-staff beam positioning
//       - Copyrights and page numbers now have styles of their own (separate from header/footer)
//
//    4.50 (Version 4.5)
//       - New property to set mergeMatchingRests at score level, with staff level changed to AutoOnOff from bool
//       - New mmRest options and offset property

    constexpr static int DIVISION = 480;
    constexpr static BeatsPerSecond DEFAULT_TEMPO = 2.0; //default tempo is equal 120 bpm
};
}

#endif // MU_ENGRAVING_CONSTANTS_H
