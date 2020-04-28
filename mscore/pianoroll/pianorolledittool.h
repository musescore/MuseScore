//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2009-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __PIANOROLLEDITTOOL_H__
#define __PIANOROLLEDITTOOL_H__

namespace Ms {

enum PianoRollEditTool
{
    SELECT,
    INSERT_NOTE,
    APPEND_NOTE,
    CUT_CHORD,
    CHANGE_LENGTH,
    ERASE,
    TIE,

    LAST  //Marker for end of list - not a tool
};

} // namespace Ms

#endif

