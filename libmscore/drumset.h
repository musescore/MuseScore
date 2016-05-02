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

#ifndef __DRUMSET_H__
#define __DRUMSET_H__

#include "mscore.h"
#include "note.h"

namespace Ms {

class Xml;

//---------------------------------------------------------
//   DrumInstrument
//---------------------------------------------------------

struct DrumInstrument {
      QString name;
      NoteHead::Group notehead; ///< notehead symbol set
      int line;                 ///< place notehead onto this line
      Direction stemDirection;
      int voice;
      char shortcut;            ///< accelerator key (CDEFGAB)

      DrumInstrument() {}
      DrumInstrument(const char* s, NoteHead::Group nh, int l, Direction d,
         int v = 0, char sc = 0)
         : name(s), notehead(nh), line(l), stemDirection(d), voice(v), shortcut(sc) {}
      };

static const int DRUM_INSTRUMENTS = 128;

//---------------------------------------------------------
//   Drumset
//    defines noteheads and line position for all
//    possible midi notes in a drumset
//---------------------------------------------------------

class Drumset {
      DrumInstrument _drum[DRUM_INSTRUMENTS];

   public:
      bool isValid(int pitch) const             { return _drum[pitch].notehead != NoteHead::Group::HEAD_INVALID; }
      NoteHead::Group noteHead(int pitch) const { return _drum[pitch].notehead;       }
      int line(int pitch) const                 { return _drum[pitch].line;           }
      int voice(int pitch) const                { return _drum[pitch].voice;          }
      Direction stemDirection(int pitch) const  { return _drum[pitch].stemDirection;  }
      const QString& name(int pitch) const      { return _drum[pitch].name;           }
      int shortcut(int pitch) const             { return _drum[pitch].shortcut;       }

      void save(Xml&) const;
      void load(XmlReader&);
      void clear();
      int nextPitch(int) const;
      int prevPitch(int) const;
      DrumInstrument& drum(int i) { return _drum[i]; }
      const DrumInstrument& drum(int i) const { return _drum[i]; }
      };

extern Drumset* smDrumset;
extern Drumset* gpDrumset;
extern void initDrumset();


}     // namespace Ms
#endif

