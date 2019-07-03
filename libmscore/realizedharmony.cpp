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
RealizedHarmony::RealizedHarmony(Harmony* h) : _harmony(h),
      _notes(QMap<int, int>()), _voicing(Voicing::AUTO), _rhythm(Rhythm::AUTO), _dirty(1)
      {
      //TODO - PHV: should we be doing something on creation of a realized harmony?
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
///
///   transposeOffset -- is the necessary adjustment
///   that is added to the root and bass
///   to get the correct sounding pitch
//---------------------------------------------------
void RealizedHarmony::update(int rootTpc, int bassTpc, int transposeOffset /*= 0*/)
      {
      //if (!_dirty)
      //      return;
      //FIXME - PHV: temp removal to test offset

      int rootPitch = tpc2pitch(rootTpc) + transposeOffset;
      //euclidian mod, we need to treat this new pitch as a pitch between
      //0 and 11, so that voicing remains consistent across transposition
      if (rootPitch < 0)
            rootPitch += PITCH_DELTA_OCTAVE;
      else
            rootPitch %= PITCH_DELTA_OCTAVE;

      _notes.clear();
      //fix magic values
      if (_voicing != Voicing::ROOT_ONLY) {
            if (bassTpc != Tpc::TPC_INVALID)
                  _notes.insert(tpc2pitch(bassTpc) + transposeOffset
                                + 4*PITCH_DELTA_OCTAVE, bassTpc);
            else
                  _notes.insert(rootPitch + 4*PITCH_DELTA_OCTAVE, rootTpc);
            }

      switch (_voicing) {
            case Voicing::ROOT_ONLY:
                  break;
            case Voicing::AUTO:
                  {
                  _notes.insert(rootPitch + 5*PITCH_DELTA_OCTAVE, rootTpc);
                  //ensure that notes fall under a specific range
                  //for now this range is between 5*12 and 6*12
                  QMap<int, int> intervals = getIntervals(rootTpc);
                  QMapIterator<int, int> i(intervals);
                  while (i.hasNext()) {
                        i.next();
                        _notes.insert((rootPitch + i.key()) % PITCH_DELTA_OCTAVE +
                                      5*PITCH_DELTA_OCTAVE, i.value());
                        }
                  }
                  break;
            default:
                  break;
            }
      }

//---------------------------------------------------
//   getIntervals
///   gets a map from intervals to TPCs based on
///   a passed root tpc (this allows for us to
///   keep pitches, but transpose notes on the score)
//---------------------------------------------------
QMap<int, int> RealizedHarmony::getIntervals(int rootTpc) const
      {
      //TODO - PHV: use ParsedChord rather than HChord
      QMap<int, int> ret;
      const HChord chord = _harmony->getDescription()->chord;

      static const HChord dimChord(0,3,6,9);

      //make sure diminished chord has a diminished 7th
      if (chord == dimChord) {
            ret.insert(3, tpcInterval(rootTpc, 3, -1));
            ret.insert(6, tpcInterval(rootTpc, 5, -1));
            ret.insert(9, tpcInterval(rootTpc, 7, -2));
            return ret;
            }

      if (chord.contains(3)) {
            if (!chord.contains(4))
                  ret.insert(3, tpcInterval(rootTpc, 3, -1)); //minor 3rd
            else
                  ret.insert(3, tpcInterval(rootTpc, 2, 1)); //#9
            }
      if (chord.contains(4))
            ret.insert(4, tpcInterval(rootTpc, 3, 0));

      // 7
      if (chord.contains(11)) {
            ret.insert(11, tpcInterval(rootTpc, 7, 0)); //maj7
            }
      else if (chord.contains(10)) {
            ret.insert(10, tpcInterval(rootTpc, 7, -1)); //dom7
            }

      // 4 or 11
      if (chord.contains(5)) {
            ret.insert(5, tpcInterval(rootTpc, 4, 0));
            }

      // 5
      if (chord.contains(7)) {
            //natural 5
            ret.insert(7, tpcInterval(rootTpc, 5, 0));
            if (chord.contains(6))
                  ret.insert(6, tpcInterval(rootTpc, 4, 1)); //#11
            if (chord.contains(8))
                  ret.insert(8, tpcInterval(rootTpc, 6, -1)); //b13
            }
      else {
            if (chord.contains(6))
                  ret.insert(6, tpcInterval(rootTpc, 5, -1)); //b5
            if (chord.contains(8))
                  ret.insert(8, tpcInterval(rootTpc, 5, 1)); //#5
            }

      // 6
      if (chord.contains(9)) {
            ret.insert(9, tpcInterval(rootTpc, 6, 0));
            }

      // b9
      if (chord.contains(1))
            ret.insert(1, tpcInterval(rootTpc, 2, -1));

      // 9
      if (chord.contains(2))
            ret.insert(2, tpcInterval(rootTpc, 2, 0));

      return ret;
      }

}
