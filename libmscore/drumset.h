//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: drumset.h 5384 2012-02-27 12:21:49Z wschweer $
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
class Xml;

//---------------------------------------------------------
//   DrumInstrument
//---------------------------------------------------------

struct DrumInstrument {
      QString name;
      Note::NoteHeadGroup notehead; ///< notehead symbol set
      int line;               ///< place notehead onto this line
      MScore::Direction stemDirection;
      int voice;
      char shortcut;          ///< accelerator key (CDEFGAB)

      DrumInstrument() {}
      DrumInstrument(const char* s, Note::NoteHeadGroup nh, int l, MScore::Direction d,
         int v = 0, char sc = 0)
         : name(s), notehead(nh), line(l), stemDirection(d), voice(v), shortcut(sc) {}
      };

static const int DRUM_INSTRUMENTS = 128;

//---------------------------------------------------------
//   Drumset
//    defines note heads and line position for all
//    possible midi notes in a drumset
//---------------------------------------------------------

class Drumset {
      DrumInstrument _drum[DRUM_INSTRUMENTS];

   public:
      bool isValid(int pitch) const            { return _drum[pitch].notehead != -1; }
      Note::NoteHeadGroup noteHead(int pitch) const  { return _drum[pitch].notehead;       }
      int line(int pitch) const                { return _drum[pitch].line;           }
      int voice(int pitch) const               { return _drum[pitch].voice;          }
      MScore::Direction stemDirection(int pitch) const { return _drum[pitch].stemDirection;  }
      const QString& name(int pitch) const     { return _drum[pitch].name;           }
      int shortcut(int pitch) const            { return _drum[pitch].shortcut;       }

      void save(Xml&);
      void load(const QDomElement&);
      void clear();
      int nextPitch(int);
      int prevPitch(int);
      DrumInstrument& drum(int i) { return _drum[i]; }
      };

extern Drumset* smDrumset;
extern void initDrumset();

#endif

