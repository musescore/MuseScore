//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __MIDIINSTRUMENT_H__
#define __MIDIINSTRUMENT_H__

//---------------------------------------------------------
//   MidiInstrument
//---------------------------------------------------------

struct MidiInstrument {
      int type;
      int hbank, lbank, patch;
      int split;
      const char* name;

      static QString instrName(int type, int hbank, int lbank, int program);
      };

extern MidiInstrument minstr[];

#endif

