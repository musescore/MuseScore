//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __EXPORTMIDI_H__
#define __EXPORTMIDI_H__

#include "midifile.h"

//---------------------------------------------------------
//   ExportMidi
//---------------------------------------------------------

class ExportMidi {
      QFile f;
      Score* cs;

      void writeHeader();

   public:
      MidiFile mf;

      ExportMidi(Score* s) { cs = s; }
      bool write(const QString& name, bool midiExpandRepeats);
      };

#endif

