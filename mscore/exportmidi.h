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

#include "midi/midifile.h"

namespace Ms {

class Score;
class TempoMap;

//---------------------------------------------------------
//   ExportMidi
//---------------------------------------------------------

class ExportMidi {
      QFile f;
      Score* cs;

      //---------------------------------------------------
      //   PauseMap
      //    MIDI files cannot contain pauses so need to insert
      //    extra ticks extra ticks and tempo changes instead.
      //---------------------------------------------------

      class PauseMap : std::map<int, int> {
            int offsetAtUTick(int utick) const;

         public:
            TempoMap* tempomapWithPauses;

            void calculate(const Score* s);
            inline int addPauseTicks(int utick) const { return utick + this->offsetAtUTick(utick); }
            };

      PauseMap pauseMap;

      void writeHeader();

   public:
      MidiFile mf;

      ExportMidi(Score* s) { cs = s; }
      bool write(const QString& name, bool midiExpandRepeats, bool exportRPNs);
      };

} // namespace Ms
#endif

