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
#include "score.h"
#include "undo.h"

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

void StringData::write(Xml& xml) const
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
//   Fills *string and *fret with suitable values for pitch
//   using the highest possible string.
//   If note cannot be fretted, uses fret 0 on nearest string and returns false
//
//    Note: Strings are stored internally from lowest (0) to highest (strings()-1),
//          but the returned *string value references strings in reversed, 'visual', order:
//          from highest (0) to lowest (strings()-1)
//---------------------------------------------------------

bool StringData::convertPitch(int pitch, int* string, int* fret) const
      {
      int strings = stringTable.size();
      if (strings < 1)
            return false;

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
//   Returns the pitch corresponding to the string / fret combination
//---------------------------------------------------------

int StringData::getPitch(int string, int fret) const
      {
      int strings = stringTable.size();
      if (strings < 1)
            return INVALID_PITCH;
      instrString strg = stringTable.at(strings - string - 1);
      return strg.pitch + (strg.open ? 0 : fret);
      }

//---------------------------------------------------------
//   fret
//    Returns the fret corresponding to the pitch / string combination
//    returns -1 if not possible
//---------------------------------------------------------

int StringData::fret(int pitch, int string) const
      {
      int strings = stringTable.size();
      if (strings < 1)                          // no strings at all!
            return FRET_NONE;

      if (string < 0 || string >= strings)      // no such a string
            return FRET_NONE;
      int fret = pitch - stringTable[strings - string - 1].pitch;
      // fret number is invalid or string cannot be fretted
      if (fret < 0 || fret >= _frets || (fret > 0 && stringTable[strings - string - 1].open))
            return FRET_NONE;
      return fret;
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
      int   nFret, nNewFret, nTempFret;
      int   nString, nNewString, nTempString;

      if(bFretting)
            return;
      bFretting = true;

      // we need to keep track of each string we allocate ourselves within this algorithm
      bool bUsed[strings()];                    // initially all strings are available
      for(nString=0; nString<strings(); nString++)
            bUsed[nString] = false;
      // we also need the notes sorted in order of string (from highest to lowest) and then pitch
      QMap<int, Note *> sortedNotes;
      int   count = 0;
      // if chord parent is not a segment, the chord is special (usually a grace chord):
      // fret it by itself, ignoring the segment
      if (chord->parent()->type() != Element::Type::SEGMENT)
            sortChordNotes(sortedNotes, chord, &count);
      else {
            // scan each chord of seg from same staff as 'chord', inserting each of its notes in sortedNotes
            Segment* seg = chord->segment();
            int trk;
            int trkFrom = (chord->track() / VOICES) * VOICES;
            int trkTo   = trkFrom + VOICES;
            for(trk = trkFrom; trk < trkTo; ++trk) {
                  Element* ch = seg->elist().at(trk);
                  if (ch && ch->type() == Element::Type::CHORD)
                        sortChordNotes(sortedNotes, static_cast<Chord*>(ch), &count);
                  }
            }

      // scan chord notes from highest, matching with strings from the highest
      foreach(Note * note, sortedNotes) {
            nString     = nNewString    = note->string();
            nFret       = nNewFret      = note->fret();
            note->setFretConflict(false);       // assume no conflicts on this note
            // if no fretting yet or current fretting is no longer valid
            if (nString == STRING_NONE || nFret == FRET_NONE || getPitch(nString, nFret) != note->pitch()) {
                  // get a new fretting
                  if(!convertPitch(note->pitch(), &nNewString, &nNewFret) ) {
                        // no way to fit this note in this tab:
                        // mark as fretting conflict
                        note->setFretConflict(true);
                        // store fretting change without affecting chord context
                        if (nFret != nNewFret)
                              note->score()->undoChangeProperty(note, P_ID::FRET, nNewFret);
                        if (nString != nNewString)
                              note->score()->undoChangeProperty(note, P_ID::STRING, nNewString);
                        continue;
                        }

                  // check this note is not using the same string of another note of this chord
                  foreach(Note * note2, sortedNotes) {
                        // if same string...
                        if(note2 != note && note2->string() == nNewString) {
                              // ...attempt to fret this note on its old string
                              if( (nTempFret=fret(note->pitch(), nString)) != FRET_NONE) {
                                    nNewFret   = nTempFret;
                                    nNewString = nString;
                                    }
                              break;
                              }
                        }
                  }

            // check we are not reusing a string we already used
            if(bUsed[nNewString]) {
                  // ...try with each other string, from the highest
                  for(nTempString=0; nTempString < strings(); nTempString++) {
                        if(bUsed[nTempString])
                              continue;
                        if( (nTempFret=fret(note->pitch(), nTempString)) != FRET_NONE) {
                              // suitable string found
                              nNewFret    = nTempFret;
                              nNewString  = nTempString;
                              break;
                              }
                        }
                  // if we run out of strings
                  if(nTempString >= strings()) {
                        // no way to fit this chord in this tab:
                        // mark this note as fretting conflict
                        note->setFretConflict(true);
//                        continue;
                        }
                  }

            // if fretting did change, store as a fret change
            if (nFret != nNewFret)
                  note->score()->undoChangeProperty(note, P_ID::FRET, nNewFret);
            if (nString != nNewString)
                  note->score()->undoChangeProperty(note, P_ID::STRING, nNewString);

            bUsed[nNewString] = true;           // string is used
            }
      bFretting = false;
      }

//---------------------------------------------------------
//   sortChordNotes
//   Adds to sortedNotes the notes of Chord in string/pitch order
//---------------------------------------------------------

void StringData::sortChordNotes(QMap<int, Note *>& sortedNotes, const Chord *chord, int* count) const
{
      int   key;

      foreach(Note * note, chord->notes()) {
            key = note->string()*100000;
            if(key < 0)                               // in case string is -1
                  key = -key;
            key -= note->pitch() * 100 + *count;      // disambiguate notes of equal pitch
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

void StringData::writeMusicXML(Xml& /*xml*/) const
      {
      }

}

