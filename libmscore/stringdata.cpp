//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "stringdata.h"
#include "chord.h"
#include "note.h"
#include "part.h"
#include "score.h"
#include "staff.h"
#include "undo.h"
#include "segment.h"

namespace Ms {

//---------------------------------------------------------
//   StringData
//---------------------------------------------------------

bool StringData::bFretting = false;

StringData::StringData(int numFrets, int numStrings, int strings[])
      {
      instrString strg = { 0, false};
      _frets = numFrets;

      for (int i = 0; i < numStrings; i++) {
            strg.pitch = strings[i];
            stringTable.append(strg);
            }
      }

StringData::StringData(int numFrets, QList<instrString>& strings)
      {
      _frets = numFrets;

      stringTable.clear();
      foreach(instrString i, strings)
            stringTable.append(i);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void StringData::read(XmlReader& e)
      {
      stringTable.clear();
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "frets")
                  _frets = e.readInt();
            else if (tag == "string") {
                  instrString strg;
                  strg.open  = e.intAttribute("open", 0);
                  strg.pitch = e.readInt();
                  stringTable.append(strg);
                  }
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void StringData::write(XmlWriter& xml) const
      {
      xml.stag("StringData");
      xml.tag("frets", _frets);
      foreach(instrString strg, stringTable) {
            if (strg.open)
                  xml.tag("string open=\"1\"", strg.pitch);
            else
                  xml.tag("string", strg.pitch);
            }
      xml.etag();
      }

//---------------------------------------------------------
//   convertPitch
//   Finds string and fret for a note.
//
//   Fills *string and *fret with suitable values for pitch at given tick of given staff,
//   using the highest possible string.
//   If note cannot be fretted, uses fret 0 on nearest string and returns false
//
//    Note: Strings are stored internally from lowest (0) to highest (strings()-1),
//          but the returned *string value references strings in reversed, 'visual', order:
//          from highest (0) to lowest (strings()-1)
//---------------------------------------------------------

bool StringData::convertPitch(int pitch, Staff* staff, const Fraction& tick, int* string, int* fret) const
      {
      return convertPitch(pitch, pitchOffsetAt(staff, tick), string, fret);
      }

//---------------------------------------------------------
//   getPitch
//    Returns the pitch corresponding to the string / fret combination
//    at given tick of given staff.
//    Returns INVALID_PITCH if not possible
//    Note: frets above max fret are accepted.
//---------------------------------------------------------

int StringData::getPitch(int string, int fret, Staff* staff, const Fraction& tick) const
      {
      return getPitch(string, fret, pitchOffsetAt(staff, tick));
      }

//---------------------------------------------------------
//   fret
//    Returns the fret corresponding to the pitch / string combination
//    at given tick of given staff.
//    Returns FRET_NONE if not possible
//---------------------------------------------------------

int StringData::fret(int pitch, int string, Staff* staff, const Fraction& tick) const
      {
      return fret(pitch, string, pitchOffsetAt(staff, tick));
      }

//---------------------------------------------------------
//   fretChords
//    Assigns fretting to all the notes of each chord in the same segment of chord
//    re-using existing fretting wherever possible
//
//    Minimizes fret conflicts (multiple notes on the same string)
//    but marks as fretConflict notes which cannot be fretted
//    (outside tablature range) or which cannot be assigned
//    a separate string
//---------------------------------------------------------

void StringData::fretChords(Chord * chord) const
      {
      int   nFret, minFret, maxFret, nNewFret, nTempFret;
      int   nString, nNewString, nTempString;

      if(bFretting)
            return;
      bFretting = true;

      // we need to keep track of string allocation
#if (!defined (_MSCVER) && !defined (_MSC_VER))
      int bUsed[strings()];                    // initially all strings are available
#else
      // MSVC does not support VLA. Replace with std::vector. If profiling determines that the
      //    heap allocation is slow, an optimization might be used.
      std::vector<int> bUsed(strings());
#endif
      for(nString=0; nString<strings(); nString++)
            bUsed[nString] = 0;
      // we also need the notes sorted in order of string (from highest to lowest) and then pitch
      QMap<int, Note *> sortedNotes;
      int   count = 0;
      // store staff pitch offset at this tick, to speed up actual note pitch calculations
      // (ottavas not implemented yet)
      int transp = chord->staff() ? chord->part()->instrument()->transpose().chromatic : 0;     // TODO: tick?
      int pitchOffset = /*chord->staff()->pitchOffset(chord->segment()->tick())*/ - transp;
      // if chord parent is not a segment, the chord is special (usually a grace chord):
      // fret it by itself, ignoring the segment
      if (chord->parent()->type() != ElementType::SEGMENT)
            sortChordNotes(sortedNotes, chord, pitchOffset, &count);
      else {
            // scan each chord of seg from same staff as 'chord', inserting each of its notes in sortedNotes
            Segment* seg = chord->segment();
            int trk;
            int trkFrom = (chord->track() / VOICES) * VOICES;
            int trkTo   = trkFrom + VOICES;
            for(trk = trkFrom; trk < trkTo; ++trk) {
                  Element* ch = seg->elist().at(trk);
                  if (ch && ch->type() == ElementType::CHORD)
                        sortChordNotes(sortedNotes, toChord(ch), pitchOffset, &count);
                  }
            }
      // determine used range of frets
      minFret = INT32_MAX;
      maxFret = INT32_MIN;
      foreach(Note* note, sortedNotes) {
            if (note->string() != STRING_NONE)
                  bUsed[note->string()]++;
            if (note->fret() != FRET_NONE && note->fret() < minFret)
                  minFret = note->fret();
            if (note->fret() != FRET_NONE && note->fret() > maxFret)
                  maxFret = note->fret();
      }

      // scan chord notes from highest, matching with strings from the highest
      foreach(Note * note, sortedNotes) {
            nString     = nNewString    = note->string();
            nFret       = nNewFret      = note->fret();
            note->setFretConflict(false);       // assume no conflicts on this note
            // if no fretting (any invalid fretting has been erased by sortChordNotes() )
            if (nString == STRING_NONE /*|| nFret == FRET_NONE || getPitch(nString, nFret) != note->pitch()*/) {
                  // get a new fretting
                  if (!convertPitch(note->pitch(), pitchOffset, &nNewString, &nNewFret) ) {
                        // no way to fit this note in this tab:
                        // mark as fretting conflict
                        note->setFretConflict(true);
                        // store fretting change without affecting chord context
                        if (nFret != nNewFret)
                              note->undoChangeProperty(Pid::FRET, nNewFret);
                        if (nString != nNewString)
                              note->undoChangeProperty(Pid::STRING, nNewString);
                        continue;
                        }
                  // note can be fretted: use string
                  else {
                        bUsed[nNewString]++;
                        }
                  }

            // if the note string (either original or newly assigned) is also used by another note
            if (bUsed[nNewString] > 1) {
                  // attempt to find a suitable string, from topmost
                  for (nTempString=0; nTempString < strings(); nTempString++) {
                        if (bUsed[nTempString] < 1
                                    && (nTempFret=fret(note->pitch(), nTempString, pitchOffset)) != FRET_NONE) {
                              bUsed[nNewString]--;    // free previous string
                              bUsed[nTempString]++;   // and occupy new string
                              nNewFret   = nTempFret;
                              nNewString = nTempString;
                              break;
                              }
                        }
                  }

            // TODO : try to optimize used fret range, avoiding eccessively open positions

            // if fretting did change, store as a fret change
            if (nFret != nNewFret)
                  note->undoChangeProperty(Pid::FRET, nNewFret);
            if (nString != nNewString)
                  note->undoChangeProperty(Pid::STRING, nNewString);
            }

      // check for any remaining fret conflict
      foreach(Note * note, sortedNotes)
            if (bUsed[note->string()] > 1)
                  note->setFretConflict(true);

      bFretting = false;
      }


//---------------------------------------------------------
//   frettedStrings
//    Returns the number of fretted strings.
//---------------------------------------------------------

int StringData::frettedStrings() const
      {
      int num = 0;
      for (auto s : stringTable)
            if (!s.open)
                  num++;
      return num;
      }


//********************
// STATIC METHODS
//********************

//---------------------------------------------------------
//   pitchOffsetAt
//   Computes the pitch offset relevant for string data calculation at the given point.
//
//   For string data calculations, pitch offset may depend on transposition, capos and, possibly, ottavas.
//---------------------------------------------------------

int StringData::pitchOffsetAt(Staff* staff, const Fraction& /*tick*/)
      {
      int transp = staff ? staff->part()->instrument()->transpose().chromatic : 0;  // TODO: tick?
      return (/*staff->pitchOffset(tick)*/ - transp);
      }

//********************
// PRIVATE METHODS
//********************

//---------------------------------------------------------
//   convertPitch
//   Finds string and fret for a note.
//
//   Fills *string and *fret with suitable values for pitch / pitchOffset,
//   using the highest possible string.
//   If note cannot be fretted, uses fret 0 on nearest string and returns false
//
//    Note: Strings are stored internally from lowest (0) to highest (strings()-1),
//          but the returned *string value references strings in reversed, 'visual', order:
//          from highest (0) to lowest (strings()-1)
//---------------------------------------------------------

bool StringData::convertPitch(int pitch, int pitchOffset, int* string, int* fret) const
      {
      int strings = stringTable.size();
      if (strings < 1)
            return false;

      pitch += pitchOffset;

      // if above max fret on highest string, fret on first string, but return failure
      if(pitch > stringTable.at(strings-1).pitch + _frets) {
            *string = 0;
            *fret   = 0;
            return false;
            }

      // look for a suitable string, starting from the highest
      // NOTE: this assumes there are always enough frets to fill
      // the interval between any fretted string and the next
      for (int i = strings-1; i >=0; i--) {
            instrString strg = stringTable.at(i);
            if(pitch >= strg.pitch) {
                  if (pitch == strg.pitch || !strg.open)
                  *string = strings - i - 1;
                  *fret   = pitch - strg.pitch;
                  return true;
                  }
            }

      // if no string found, pitch is below lowest string:
      // fret on last string, but return failure
      *string = strings-1;
      *fret   = 0;
      return false;
      }

//---------------------------------------------------------
//   getPitch
//    Returns the pitch corresponding to the string / fret / pitchOffset combination.
//    Returns INVALID_PITCH if not possible
//    Note: frets above max fret are accepted.
//---------------------------------------------------------

int StringData::getPitch(int string, int fret, int pitchOffset) const
      {
      int strings = stringTable.size();
      if (string < 0 || string >= strings)
            return INVALID_PITCH;
      instrString strg = stringTable.at(strings - string - 1);
      return strg.pitch - pitchOffset + (strg.open ? 0 : fret);
      }

//---------------------------------------------------------
//   fret
//    Returns the fret corresponding to the pitch / string / pitchOffset combination.
//    returns FRET_NONE if not possible
//---------------------------------------------------------

int StringData::fret(int pitch, int string, int pitchOffset) const
      {
      int strings = stringTable.size();
      if (strings < 1)                          // no strings at all!
            return FRET_NONE;

      if (string < 0 || string >= strings)      // no such a string
            return FRET_NONE;

      pitch += pitchOffset;

      int fret = pitch - stringTable[strings - string - 1].pitch;
      // fret number is invalid or string cannot be fretted
      if (fret < 0 || fret >= _frets || (fret > 0 && stringTable[strings - string - 1].open))
            return FRET_NONE;
      return fret;
      }

//---------------------------------------------------------
//   sortChordNotes
//    Adds to sortedNotes the notes of Chord in string/pitch order
//    Note: notes are sorted first by string (top string being 0),
//          then by negated pitch (higher pitches resulting in lower key),
//          then by order of submission to disambiguate notes with the same pitch.
//          Everything else being equal, this makes notes in higher-numbered voices
//          to be sorted after notes in lower-numbered voices (voice 2 after voice 1 and so on)
//    Notes without a string assigned yet, are sorted according to the lowest string which can accommodate them.
//---------------------------------------------------------

void StringData::sortChordNotes(QMap<int, Note *>& sortedNotes, const Chord *chord, int pitchOffset, int* count) const
{
      int   key, string, fret;

      foreach(Note * note, chord->notes()) {
            string      = note->string();
            fret        = note->fret();
            // if note not fretted yet or current fretting no longer valid,
            // use most convenient string as key
            if (string <= STRING_NONE || fret <= FRET_NONE
                        || getPitch(string, fret, pitchOffset) != note->pitch()) {
                  note->setString(STRING_NONE);
                  note->setFret(FRET_NONE);
                  convertPitch(note->pitch(), pitchOffset, &string, &fret);
                  }
            key = string * 100000;
            key += -(note->pitch()+pitchOffset) * 100 + *count;     // disambiguate notes of equal pitch
            sortedNotes.insert(key, note);
            (*count)++;
            }
}

#if 0
//---------------------------------------------------------
//   MusicXMLStepAltOct2Pitch
//---------------------------------------------------------

/**
 Convert MusicXML \a step / \a alter / \a octave to midi pitch.
 Note: similar to (part of) xmlSetPitch in mscore/importxml.cpp.
 TODO: combine ?
 */

static int MusicXMLStepAltOct2Pitch(char step, int alter, int octave)
      {
      int istep = step - 'A';
      //                       a  b   c  d  e  f  g
      static int table[7]  = { 9, 11, 0, 2, 4, 5, 7 };
      if (istep < 0 || istep > 6) {
            qDebug("MusicXMLStepAltOct2Pitch: illegal step %d, <%c>", istep, step);
            return -1;
            }
      int pitch = table[istep] + alter + (octave+1) * 12;

      if (pitch < 0)
            pitch = -1;
      if (pitch > 127)
            pitch = -1;

      return pitch;
      }

//---------------------------------------------------------
//   Read MusicXML
//---------------------------------------------------------

void StringData::readMusicXML(XmlReader& e)
      {
      _frets = 25;

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "staff-lines") {
                  int val = e.readInt();
                  if (val > 0) {
                        // resize the string table and init with zeroes
                        stringTable = QVector<int>(val).toList();
                        }
                  else
                        qDebug("StringData::readMusicXML: illegal staff-lines %d", val);
                  }
            else if (tag == "staff-tuning") {
                  int     line   = e.intAttribute("line");
                  QString step;
                  int     alter  = 0;
                  int     octave = 0;
                  while (e.readNextStartElement()) {
                        const QStringRef& tag(e.name());
                        if (tag == "tuning-alter")
                              alter = e.readInt();
                        else if (tag == "tuning-octave")
                              octave = e.readInt();
                        else if (tag == "tuning-step")
                              step = e.readElementText();
                        else
                              e.unknown();
                        }
                  if (0 < line && line <= stringTable.size()) {
                        int pitch = MusicXMLStepAltOct2Pitch(step[0].toLatin1(), alter, octave);
                        if (pitch >= 0)
                              stringTable[line - 1] = pitch;
                        else
                              qDebug("StringData::readMusicXML invalid string %d tuning step/alter/oct %s/%d/%d",
                                     line, qPrintable(step), alter, octave);
                        }
                  }
            else if (tag == "capo") {
                  ; // not supported: silently ignored
                  }
            else {
                  ; // others silently ignored
                  }
            }
      }
#endif

//---------------------------------------------------------
//   Write MusicXML
//---------------------------------------------------------

void StringData::writeMusicXML(XmlWriter& /*xml*/) const
      {
      }

}

