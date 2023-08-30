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

#include <QMap>
#include <QList>

namespace Ms {

class Harmony;
class Fraction;

//voicing modes to use
enum class Voicing : signed char {
      INVALID = -1,
      AUTO = 0,
      ROOT_ONLY,
      CLOSE,
      DROP_2,
      SIX_NOTE,
      FOUR_NOTE,
      THREE_NOTE
      };

//duration to realize notes for
enum class HDuration : signed char {
      INVALID = -1,
      UNTIL_NEXT_CHORD_SYMBOL = 0,  //lasts until the next chord symbol or end of the schore
      STOP_AT_MEASURE_END,          //lasts until next chord symbol or measure end
      SEGMENT_DURATION              //lasts for the duration of the segment
      };


//-----------------------------------------
//    Realized Harmony
///     holds information and functions
///     to assist in the realization of chord
///     symbols. This is what is used to
///     allow for chord symbol playback
//-----------------------------------------
class RealizedHarmony {

   public:
      using PitchMap = QMultiMap<int, int>; //map from pitch to tpc
      using PitchMapIterator = QMapIterator<int, int>;

   private:
      Harmony* _harmony;

      PitchMap _notes;

      Voicing _voicing = Voicing::INVALID;
      HDuration _duration = HDuration::INVALID;

      //whether or not the current notes QMap is up to date
      bool _dirty;

      bool _literal = false; //use all notes when possible and do not add any notes

   public:
      RealizedHarmony() : _harmony(0), _notes(PitchMap()), _dirty(1) {}
      RealizedHarmony(Harmony* h) : _harmony(h), _notes(PitchMap()), _dirty(1) {}

      void setVoicing(Voicing);
      void setDuration(HDuration);
      void setLiteral(bool);
      void setDirty(bool dirty) { cascadeDirty(dirty); } //set dirty flag and cascade
      void setHarmony(Harmony* h) { _harmony = h; }

      Voicing voicing() const { return _voicing; }
      HDuration duration() const { return _duration; }
      bool literal() const { return _literal; }
      Harmony* harmony() { return _harmony; }

      bool valid() const { return !_dirty && _harmony; }

      const QList<int> pitches() const { return notes().keys(); }
      const QList<int> tpcs() const { return notes().values(); }

      const PitchMap& notes() const;
      const PitchMap generateNotes(int rootTpc, int bassTpc, bool literal,
                                                  Voicing voicing, int transposeOffset) const;

      void update(int rootTpc, int bassTpc, int transposeOffset = 0); //updates the notes map

      Fraction getActualDuration(int utick, HDuration durationType = HDuration::INVALID) const;

   private:
      PitchMap getIntervals(int rootTpc, bool literal = true) const;
      PitchMap normalizeNoteMap(const PitchMap& intervals, int rootTpc, int rootPitch, int max = 128, bool enforceMaxAsGoal = false) const;
      void cascadeDirty(bool dirty);
      };
}


#endif // __REALIZEDHARMONY_H__
