//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2016 Werner Schweer and others
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

#ifndef __REALIZEDHARMONY_H__
#define __REALIZEDHARMONY_H__

#include "chordlist.h"

namespace Ms {

//voicing modes to use
enum class Voicing : char {
      AUTO
      };

//rhythm mode to use
enum class Rhythm : char {
      AUTO
      };

//-----------------------------------------
//    Realized Harmony
///     holds information and functions
///     to assist in the realization of chord
///     symbols. This is what is used to
///     allow for chord symbol playback
//-----------------------------------------
class RealizedHarmony {
      QMap<int, int> notes;

      Voicing voicing;
      Rhythm rhythm;

      //whether or not the current notes QMap is up to date
      bool dirty;

      ChordDescription* description;

   public:
      RealizedHarmony(ChordDescription* cd,
                      Voicing v = Voicing::AUTO, Rhythm r = Rhythm::AUTO)
                      : description(cd), voicing(v), rhythm(r), dirty(1) {}

      void setDescription(ChordDescription* cd);
      void setVoicing(Voicing v);
      void setRhythm(Rhythm r);

      const ChordDescription* description() const { return description; }
      Voicing voicing() const { return voicing; }
      Rhythm rhythm() const { return rhythm; }

      //TODO - PHV: consider what to do here, we might want to keep bass and root
      const QMap<int, int>& notes() const { return dirty ? notes : 0; }

      void update(int rootTpc, int bassTpc); //updates the notes map

      };

}


#endif // __REALIZEDHARMONY_H__
