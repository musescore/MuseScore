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

#include "realizedharmony.h"
#include "pitchspelling.h"
#include "chordlist.h"
#include "harmony.h"

namespace Ms {

//---------------------------------------------------
//   RealizedHarmony
///   creates empty realized harmony
//---------------------------------------------------
RealizedHarmony::RealizedHarmony(Harmony* h) : _harmony(h), _rootTpc(h->rootTpc()),
      _notes(QMap<int, int>()), _voicing(Voicing::AUTO), _rhythm(Rhythm::AUTO), _dirty(1)
      {
      //TODO - PHV
      }

//---------------------------------------------------
//   setVoicing
///   sets the voicing and dirty flag if the passed
///   voicing is different than current
//---------------------------------------------------
void RealizedHarmony::setVoicing(Voicing v)
      {
      if (_voicing == v)
            return;
      _voicing = v;
      _dirty = 1;
      }

//---------------------------------------------------
//   setRhythm
///   sets the rhythm and dirty flag if the passed
///   rhythm is different than current
//---------------------------------------------------
void RealizedHarmony::setRhythm(Rhythm r)
      {
      if (_rhythm == r)
            return;
      _rhythm = r;
      _dirty = 1;
      }

//---------------------------------------------------
//   setRoot
///   sets the root and dirty flag if the passed
///   root is different than current
//---------------------------------------------------
void RealizedHarmony::setRoot(int r)
      {
      if (_rootTpc == r)
            return;
      _rootTpc = r;
      _dirty = 1;
      }

//---------------------------------------------------
//   notes
///   returns the list of notes
//---------------------------------------------------
const QMap<int, int>& RealizedHarmony::notes() const
      {
      //TODO - PHV: do something when dirty?
      return _notes;
      }

//---------------------------------------------------
//   update
///   updates the current note map, this is where all
///   of the rhythm and voicing choices matter since
///   the voicing algorithms depend on this.
//---------------------------------------------------
void RealizedHarmony::update(int rootTpc, int bassTpc)
      {
      if (!_dirty)
            return;

      _notes.clear();
      //fix magic values
      int rootPitch = tpc2pitch(rootTpc);
      _notes.insert(rootPitch + 48, rootTpc);
      switch (_voicing) {
            case Voicing::ROOT_ONLY:
                  break;
            case Voicing::AUTO:
                  {
                  QMap<int, int> intervals = getIntervals();
                  QMapIterator<int, int> i(intervals);
                  while (i.hasNext()) {
                        i.next();
                        _notes.insert(rootPitch + i.key() + 5*12, i.value());
                        }
                  }
                  break;
            default:
                  break;
            }
      }

QMap<int, int> RealizedHarmony::getIntervals() const
      {
      QMap<int, int> ret;
      const HChord chord = _harmony->getDescription()->chord;

      static const HChord dimChord(0,3,6,9);

      //make sure diminished chord has a diminished 7th
      if (chord == dimChord) {
            ret.insert(3, tpcInterval(_rootTpc, 3, -1));
            ret.insert(6, tpcInterval(_rootTpc, 5, -1));
            ret.insert(9, tpcInterval(_rootTpc, 7, -2));
            return ret;
            }

      if (chord.contains(3)) {
            if (!chord.contains(4))
                  //minor 3rd
                  ret.insert(3, tpcInterval(_rootTpc, 3, -1));
            else
                  //sharp 9
                  ret.insert(3, tpcInterval(_rootTpc, 2, 1));
            }
      if (chord.contains(4))
            ret.insert(4, tpcInterval(_rootTpc, 3, 0));
      //above is bad, fix soon

      // 7
      if (chord.contains(11)) {
            //maj7
            ret.insert(11, tpcInterval(_rootTpc, 7, 0));
            }
      else if (chord.contains(10)) {
            //7
            ret.insert(10, tpcInterval(_rootTpc, 7, -1));
            }

      // 4 or 11
      if (chord.contains(5)) {
            ret.insert(5, tpcInterval(_rootTpc, 4, 0));
            }

      // 5
      if (chord.contains(7)) {
            //natural 5
            ret.insert(7, tpcInterval(_rootTpc, 5, 0));
            if (chord.contains(6))
                  //#11
                  ret.insert(6, tpcInterval(_rootTpc, 4, 1));
            if (chord.contains(8))
                  //b13
                  ret.insert(8, tpcInterval(_rootTpc, 6, -1));
            }
      else {
            if (chord.contains(6))
                  //b5
                  ret.insert(6, tpcInterval(_rootTpc, 5, -1));
            if (chord.contains(8))
                  //#5
                  ret.insert(8, tpcInterval(_rootTpc, 5, 1));
            }

      // 6
      if (chord.contains(9)) {
            ret.insert(9, tpcInterval(_rootTpc, 6, 0));
            }

      // b9
      if (chord.contains(1))
            ret.insert(1, tpcInterval(_rootTpc, 2, -1));

      // 9
      if (chord.contains(2))
            ret.insert(2, tpcInterval(_rootTpc, 2, 0));

      return ret;
      }

}
