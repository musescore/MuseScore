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
//   setLiteral
///   sets literal/jazz and dirty flag if the passed
///   bool is different than current
//---------------------------------------------------
void RealizedHarmony::setLiteral(bool literal)
      {
      if (_literal == literal)
            return;
      _literal = literal;
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
//   notes
///   returns the list of notes
//---------------------------------------------------
const QMap<int, int> RealizedHarmony::generateNotes(int rootTpc, int bassTpc,
                                                bool literal, Voicing voicing, int transposeOffset) const
      {
      QMap<int, int> notes;
      int rootPitch = tpc2pitch(rootTpc) + transposeOffset;
      //euclidian mod, we need to treat this new pitch as a pitch between
      //0 and 11, so that voicing remains consistent across transposition
      if (rootPitch < 0)
            rootPitch += PITCH_DELTA_OCTAVE;
      else
            rootPitch %= PITCH_DELTA_OCTAVE;

      //create root note or bass note in second octave below middle C
      if (bassTpc != Tpc::TPC_INVALID && voicing != Voicing::ROOT_ONLY)
            notes.insert(tpc2pitch(bassTpc) + transposeOffset
                        + 3*PITCH_DELTA_OCTAVE, bassTpc);
      else
            notes.insert(rootPitch + 3*PITCH_DELTA_OCTAVE, rootTpc);


      switch (voicing) {
            case Voicing::ROOT_ONLY:
                  break;
            case Voicing::AUTO: //auto is close voicing for now since it is the most robust
                  // FALLTHROUGH
            case Voicing::CLOSE:
                  //Voices notes in close position in the first octave above middle C
                  {
                  notes.insert(rootPitch + 5*PITCH_DELTA_OCTAVE, rootTpc);
                  //ensure that notes fall under a specific range
                  //for now this range is between 5*12 and 6*12
                  QMap<int, int> intervals = getIntervals(rootTpc, literal);
                  QMapIterator<int, int> i(intervals);
                  while (i.hasNext()) {
                        i.next();
                        notes.insert((rootPitch + (i.key() % 128)) % PITCH_DELTA_OCTAVE +
                                      5*PITCH_DELTA_OCTAVE, i.value());
                        }
                  }
                  break;
            case Voicing::OPEN:
                  break;
            case Voicing::DROP_2:
                  {
                  //TODO - PHV: maybe make this jazz interpretation only?
                  //drop the second from the highest note down an octave, will probably need to
                  //only take the first 4 notes

                  //select 4 notes from list
                  QMap<int, int> intervals = normalizeNoteMap(getIntervals(rootTpc, literal), rootTpc, rootPitch, 4);
                  if (intervals.size() < 4)
                        intervals.insert(rootPitch, rootTpc); //double root if we only have a few notes
                  QMapIterator<int, int> i(intervals);
                  i.toBack();

                  int counter = 0; //counter to drop the second note
                  while (i.hasPrevious()) {
                        i.previous();
                        if (++counter == 2)
                              notes.insert(i.key() + 4*PITCH_DELTA_OCTAVE, i.value());
                        else
                              notes.insert(i.key() + 5*PITCH_DELTA_OCTAVE, i.value());
                        }
                  }
                  break;
            case Voicing::THREE_NOTE:
                  {
                  //Three note open voicing, maybe remove this

                  QMap<int, int> intervals = normalizeNoteMap(getIntervals(rootTpc, literal), rootTpc, rootPitch, 2);
                  QMapIterator<int, int> i(intervals);

                  i.next();
                  notes.insert(i.key() + 5*PITCH_DELTA_OCTAVE, i.value());

                  i.next();
                  notes.insert(i.key() + 5*PITCH_DELTA_OCTAVE, i.value());
                  }
                  break;
            case Voicing::FOUR_NOTE:
                  {
                  //four note open voicing
                  QMap<int, int> relIntervals = getIntervals(rootTpc, literal);
                  QMap<int, int> intervals = normalizeNoteMap(relIntervals, rootTpc, rootPitch, 3);
                  QMapIterator<int, int> i(intervals);
                  i.toBack();

                  int counter = 0; //how many notes have been added
                  while (i.hasPrevious()) {
                        i.previous();

                        if (counter % 2)
                              notes.insert(i.key() + 4*PITCH_DELTA_OCTAVE, i.value());
                        else
                              notes.insert(i.key() + 5*PITCH_DELTA_OCTAVE, i.value());
                        ++counter;
                        }
                  if (counter < 3) {
                        //TODO - PHV: probably should extract this into its own recursive function
                        intervals = normalizeNoteMap(relIntervals, rootTpc, rootPitch, 3 - counter, true);
                        QMapIterator<int, int> i(intervals);
                        i.toBack();
                        while (i.hasPrevious()) {
                              i.previous();

                              if (counter % 2)
                                    notes.insert(i.key() + 4*PITCH_DELTA_OCTAVE, i.value());
                              else
                                    notes.insert(i.key() + 5*PITCH_DELTA_OCTAVE, i.value());
                              ++counter;
                              }
                        }
                  }
                  break;
            case Voicing::SIX_NOTE:
                  //six note voicing
                  {
                  QMap<int, int> relIntervals = getIntervals(rootTpc, literal);
                  QMap<int, int> intervals = normalizeNoteMap(relIntervals, rootTpc, rootPitch, 5);
                  QMapIterator<int, int> i(intervals);
                  i.toBack();

                  int counter = 0; //how many notes have been added
                  while (i.hasPrevious()) {
                        i.previous();

                        if (counter % 2)
                              notes.insert(i.key() + 4*PITCH_DELTA_OCTAVE, i.value());
                        else
                              notes.insert(i.key() + 5*PITCH_DELTA_OCTAVE, i.value());
                        ++counter;
                        }
                  if (counter < 5) {
                        //TODO - PHV: probably should extract this into its own recursive function
                        intervals = normalizeNoteMap(relIntervals, rootTpc, rootPitch, 5 - counter, true); //include root
                        QMapIterator<int, int> i(intervals);
                        i.toBack();
                        while (i.hasPrevious()) {
                              i.previous();

                              if (counter % 2)
                                    notes.insert(i.key() + 4*PITCH_DELTA_OCTAVE, i.value());
                              else
                                    notes.insert(i.key() + 5*PITCH_DELTA_OCTAVE, i.value());
                              ++counter;
                              }
                        }
                  }
                  break;
            default:
                  break;
            }
      return notes;
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

      _notes = generateNotes(rootTpc, bassTpc, _literal, _voicing, transposeOffset);
      }

//---------------------------------------------------
//   getIntervals
///   gets a map from intervals to TPCs based on
///   a passed root tpc (this allows for us to
///   keep pitches, but transpose notes on the score)
///
///   Weighting System:
///   - Rank 0: 3rd, altered 5th, suspensions (characteristic notes)
///   - Rank 1: 7th
///   - Rank 2: 9ths
///   - Rank 3: 13ths where applicable and (in minor chords) 11ths
///   - Rank 3: Other alterations and additions
///   - Rank 4: 5th and (in major/dominant chords) 11th
//---------------------------------------------------
QMap<int, int> RealizedHarmony::getIntervals(int rootTpc, bool literal) const
      {
      //TODO - PHV: clean this up, it's really messy and doesn't always work in all cases

      //RANKING SYSTEM
      static const int RANK_MULT = 128;
      static const int RANK_3RD = 0;
      static const int RANK_7TH = 1;
      static const int RANK_9TH = 2;
      static const int RANK_ADD = 3;
      static const int RANK_OMIT = 4;

      QMap<int, int> ret;

      const ParsedChord* p = _harmony->parsedForm();
      QString quality = p->quality();
      int ext = p->extension().toInt();
      const QStringList& modList = p->modifierList();

      int omit = 0;

      //handle modifiers
      //TODO - PHV: maybe find a better way, or optimize this
      for (int i = 0; i < modList.size(); ++i) {
            QString s = modList[i];
            //find number
            for (int c = 0; c < s.length(); ++c) {
                  if (s[c].isDigit()) {
                        Q_ASSERT(c > 0); //we shouldn't have just a number

                        int alter = 0;
                        int cutoff = c;
                        int deg = s.right(s.length() - c).toInt();
                        //account for if the flat/sharp is stuck to the end of add
                        if (s[c-1] == '#') {
                              cutoff -= 1;
                              alter = +1;
                              }
                        else if (s[c-1] == 'b') {
                              cutoff -= 1;
                              alter = -1;
                              }
                        QString extType = s.left(cutoff);
                        if (extType == "" || extType == "major") { //alteration
                              if (deg == 9)
                                    ret.insert(step2pitchInterval(deg, alter) + RANK_MULT*RANK_9TH, tpcInterval(rootTpc, deg, alter));
                              else
                                    ret.insert(step2pitchInterval(deg, alter) + RANK_MULT*RANK_ADD, tpcInterval(rootTpc, deg, alter));
                              omit |= 1 << deg;
                              }
                        else if (extType == "sus") {
                              ret.insert(step2pitchInterval(deg, alter) + RANK_MULT*RANK_3RD, tpcInterval(rootTpc, deg, alter));
                              omit |= 1 << 3;
                              }
                        else if (extType == "no" || ext == 5)
                              omit |= 1 << deg;
                        else if (ext == 5)
                              omit |= 1 << 3;
                        }
                  }
            }

      //handle chord quality
      if (quality == "minor") {
            if (!(omit & (1 << 3)))
                  ret.insert(step2pitchInterval(3, -1) + RANK_MULT*RANK_3RD, tpcInterval(rootTpc, 3, -1));     //min3
            if (!(omit & (1 << 5)))
                  ret.insert(step2pitchInterval(5, 0) + RANK_MULT*RANK_OMIT, tpcInterval(rootTpc, 5, 0));       //p5
            }
      else if (quality == "augmented") {
            if (!(omit & (1 << 3)))
                  ret.insert(step2pitchInterval(3, 0) + RANK_MULT*RANK_3RD, tpcInterval(rootTpc, 3, 0));      //maj3
            if (!(omit & (1 << 5)))
                  ret.insert(step2pitchInterval(5, +1) + RANK_MULT*RANK_3RD, tpcInterval(rootTpc, 5, +1));    //p5
            }
      else if (quality == "diminished" || quality == "half-diminished") {
            if (!(omit & (1 << 3)))
                  ret.insert(step2pitchInterval(3, -1) + RANK_MULT*RANK_3RD, tpcInterval(rootTpc, 3, -1));     //min3
            if (!(omit & (1 << 5)))
                  ret.insert(step2pitchInterval(5, -1) + RANK_MULT*RANK_3RD, tpcInterval(rootTpc, 5, -1));     //dim5
            }
      else { //major or dominant
            if (!(omit & (1 << 3)))
                  ret.insert(step2pitchInterval(3, 0) + RANK_MULT*RANK_3RD, tpcInterval(rootTpc, 3, 0));      //maj3
            if (!(omit & (1 << 5)))
                  ret.insert(step2pitchInterval(5, 0) + RANK_MULT*RANK_OMIT, tpcInterval(rootTpc, 5, 0));      //p5
            }

      //handle extension
      switch (ext) {
            case 13:
                  if (!(omit & (1 << 13)))
                        ret.insert(9 + RANK_MULT*RANK_ADD, tpcInterval(rootTpc, 13, 0));     //maj13
                  // FALLTHROUGH
            case 11:
                  if (!(omit & (1 << 11))) {
                        if (quality == "minor")
                              ret.insert(5 + RANK_MULT*RANK_ADD, tpcInterval(rootTpc, 11, 0));     //maj11
                        else if (literal)
                              ret.insert(5 + RANK_MULT*RANK_OMIT, tpcInterval(rootTpc, 11, 0));     //maj11
                        }
                  // FALLTHROUGH
            case 9:
                  if (!(omit & (1 << 9)))
                        ret.insert(2 + RANK_MULT*RANK_9TH, tpcInterval(rootTpc, 9, 0));     //maj9
                  // FALLTHROUGH
            case 7:
                  if (!(omit & (1 << 7))) {
                        if (quality == "major")
                              ret.insert(11 + RANK_MULT*RANK_7TH, tpcInterval(rootTpc, 7, 0));
                        else if (quality == "diminished")
                              ret.insert(9 + RANK_MULT*RANK_7TH, tpcInterval(rootTpc, 7, -2));
                        else //dominant or augmented or minor
                              ret.insert(10 + RANK_MULT*RANK_7TH, tpcInterval(rootTpc, 7, -1));
                        }
                  break;
            case 6:
                  if (!(omit & (1 << 6)))
                        ret.insert(9 + RANK_MULT*RANK_ADD, tpcInterval(rootTpc, 6, 0));     //maj6
                  break;
            case 5:
                  //omitted third already
                  break;
            case 4:
                  if (!(omit & (1 << 4)))
                        ret.insert(5 + RANK_MULT*RANK_ADD, tpcInterval(rootTpc, 4, 0));     //p4
                  break;
            case 2:
                  if (!(omit & (1 << 2)))
                        ret.insert(2 + RANK_MULT*RANK_ADD, tpcInterval(rootTpc, 2, 0));     //maj2
                  break;
            case 69:
                  ret.insert(9 + RANK_MULT*RANK_ADD, tpcInterval(rootTpc, 6, 0));     //maj6
                  ret.insert(2 + RANK_MULT*RANK_ADD, tpcInterval(rootTpc, 9, 0));     //maj6
                  break;
            default:
                  break;
            }


      /* Old method, still keeping this for now until more thoroughly tested
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

            */
      return ret;
      }

//---------------------------------------------------
//   normalizeNoteMap
///   normalize the note map from intervals to create pitches between 0 and 12
///   and resolve any weighting system.
//---------------------------------------------------
QMap<int, int> RealizedHarmony::normalizeNoteMap(QMap<int, int> intervals, int rootTpc, int rootPitch, int max, bool includeRoot) const
      {
      QMap<int, int> ret;
      QMapIterator<int, int> itr(intervals);

      if (includeRoot) {
            --max;
            ret.insert(rootPitch, rootTpc);
            }
      for (int i = 0; i < max; ++i) {
            if (!itr.hasNext())
                  break;
            itr.next();
            ret.insert((itr.key() % 128 + rootPitch) % PITCH_DELTA_OCTAVE, itr.value());
            }
      return ret;
      }

}
