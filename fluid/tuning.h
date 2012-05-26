/*
  ZynAddSubFX - a software synthesizer

  Microtonal.h - Tuning settings and microtonal capabilities
  Copyright (C) 2002-2005 Nasca Octavian Paul
  Author: Nasca Octavian Paul

  This program is free software; you can redistribute it and/or modify
  it under the terms of version 2 of the GNU General Public License
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License (version 2 or later) for more details.

  You should have received a copy of the GNU General Public License (version 2)
  along with this program; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

*/

#ifndef __TUNING_H__
#define __TUNING_H__

#define MAX_OCTAVE_SIZE         128

//---------------------------------------------------------
//   Tuning
//---------------------------------------------------------

class Tuning {
      bool linetotunings(int nline, const char *line);
      QByteArray loadLine(QFile* file);
      int octavesize;
      struct {
            unsigned char type;     //1 for cents or 2 for division

            // the real tuning (eg. +1.05946 for one halftone)
            // or 2.0 for one octave
            float tuning;

            //the real tuning is x1/x2
            unsigned int x1, x2;
            } octave[MAX_OCTAVE_SIZE], tmpoctave[MAX_OCTAVE_SIZE];

   public:
      Tuning();

      float getnotefreq(int note, int keyshift);

      bool invertupdown;       // if the keys are inversed (the pitch is lower to keys from the right direction)
      int invertupdowncenter;  // the central key of the inversion
      bool enabled;            // false for 12 key temperate scale, true for microtonal
      unsigned char aNote;     // the note of "A" key
      float aFreq;             // the frequency of the "A" note
      int scaleshift;          // if the scale is "tuned" to a note, you can tune to other note

      unsigned char firstkey, lastkey;    //first and last key (to retune)
      unsigned char middlenote;           //The middle note where scale degree 0 is mapped to

      unsigned char mapsize;  // Map size
      bool mappingenabled;    // Mapping ON/OFF
      short int mapping[128]; // Mapping (keys)

      unsigned char globalfinedetune;

      int getoctavesize();                      // Return the current octave size
      bool loadscl(const QString& filename);    //load the tunnings from a .scl file
      bool loadkbm(const QString& filename);    //load the mapping from .kbm file

      QString name;
      QString comment;
      };

#endif

