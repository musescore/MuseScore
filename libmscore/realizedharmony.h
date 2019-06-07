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
      AUTO, ROOT_ONLY
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
      Harmony* _harmony;

      QMap<int, int> _notes; //map from pitch to tpc

      Voicing _voicing;
      Rhythm _rhythm;

      //whether or not the current notes QMap is up to date
      bool _dirty;

   public:
      RealizedHarmony() : _harmony(0), _notes(QMap<int, int>()), _voicing(Voicing::AUTO),
            _rhythm(Rhythm::AUTO), _dirty(1) {}
      RealizedHarmony(Harmony*);

      void setDescription(int);
      void setVoicing(Voicing);
      void setRhythm(Rhythm);

      Voicing voicing() const { return _voicing; }
      Rhythm rhythm() const { return _rhythm; }

      bool valid() const { return !_dirty && _harmony; }

      const QList<int> pitches() const { return _notes.values(); }

      //TODO - PHV: consider what to do here, we might want to keep bass and root
      //also consider if we should update or do any checks for if we call notes
      //without updating first
      const QMap<int, int>& notes() const;

      void update(int rootTpc, int bassTpc); //updates the notes map

      };

}


#endif // __REALIZEDHARMONY_H__
