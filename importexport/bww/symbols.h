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

#ifndef SYMBOLS_H
#define SYMBOLS_H

/**
 \file
 Definition of tokens types for bww lexer and parser
 */

namespace Bww {

  enum Symbol : unsigned char
  {
    COMMENT,
    HEADER,
    STRING,
    CLEF,
    KEY,
    TEMPO,
    TSIG,
    PART,
    BAR,
    NOTE,
    TIE,
    TRIPLET,
    DOT,
    GRACE,
    UNKNOWN,
    NONE
  };

  enum StartStop : unsigned char
  {
    ST_NONE,
    ST_START,
    ST_CONTINUE,
    ST_STOP
  };

  extern QString symbolToString(Symbol s);

} // namespace Bww

#endif // SYMBOLS_H
