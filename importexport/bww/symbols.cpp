//=============================================================================
//  BWW to MusicXML converter
//  Part of MusE Score
//  Linux Music Score Editor
//
//  Copyright (C) 2010 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

/**
 Symbol type to text translation.
 */

#include <QtCore/QString>

#include "symbols.h"

namespace Bww {

  static constexpr const char* symTable[] =
  {
    "COMMENT",
    "HEADER",
    "STRING",
    "CLEF",
    "KEY",
    "TEMPO",
    "TSIG",
    "PART",
    "BAR",
    "NOTE",
    "TIE",
    "TRIPLET",
    "DOT",
    "GRACE",
    "UNKNOWN",
    "NONE"
  };

  QString symbolToString(Symbol s)
  {
    if (static_cast<size_t>(s) >= sizeof symTable)
      return "INVALID";

    return symTable[s];
  }

} // namespace Bww
