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

namespace Ms {

class Harmony;

//voicing modes to use
enum class Voicing : char {
      AUTO = 0,
      ROOT_ONLY,
      CLOSE,
      OPEN,
      DROP_2,
      SIX_NOTE,
      FOUR_NOTE,
      THREE_NOTE
      };

//rhythm mode to use
enum class Rhythm : char {
      AUTO = 0
      };

//-----------------------------------------
//    Realized Harmony
///     holds information and functions
///     to assist in the realization of chord
///     symbols. This is what is used to
///     allow for chord symbol playback
//-----------------------------------------
class RealizedHarmony {
      Harmony* _harmony;

      QMap<int, int> _notes; //map from pitch to tpc

      Voicing _voicing;
      Rhythm _rhythm;

      //whether or not the current notes QMap is up to date
      bool _dirty;

      bool literal; //use all notes when possible and do not add any notes

   public:
      RealizedHarmony() : _harmony(0), _notes(QMap<int, int>()), _voicing(Voicing::AUTO),
            _rhythm(Rhythm::AUTO), _dirty(1) {}
      RealizedHarmony(Harmony*);

      void setVoicing(Voicing);
      void setRhythm(Rhythm);

      Voicing voicing() const { return _voicing; }
      Rhythm rhythm() const { return _rhythm; }

      bool valid() const { return !_dirty && _harmony; }

      const QList<int> pitches() const { return _notes.keys(); }
      const QList<int> tpcs() const { return _notes.values(); }

      const QMap<int, int>& notes() const;

      void update(int rootTpc, int bassTpc, int transposeOffset = 0); //updates the notes map

   private:
      //PHV: temp?
      QMap<int, int> getIntervals(int rootTpc) const;
      };
}


#endif // __REALIZEDHARMONY_H__
