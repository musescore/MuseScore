//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011-2012 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "scale.h"
#include "mscore.h"
#include "note.h"
#include <set>

using namespace std;

namespace Ms {

const int Scale::CENTS_PER_SEMITONE;
const int Scale::CENTS_PER_OCTAVE;
const int Scale::STANDARD_NOTES[TPC_NUM_OF] =
      { 300, 1000, 500, 0, 700, 200, 900, 400, 1100, 600, 100, 800,
        300, 1000, 500, 0, 700, 200, 900, 400, 1100, 600, 100, 800,
        300, 1000, 500, 0, 700, 200, 900, 400, 1100, 600, 100
      };
const int Scale::NB_SCALES;
const int Scale::TPCS[NB_SCALES][TPC_NUM_OF]     =
      { {TPC_D,   TPC_E,   TPC_F,   TPC_G,   TPC_A,   TPC_B,   TPC_C },
            {TPC_C_S, TPC_D,   TPC_E_B, TPC_E,   TPC_F,   TPC_F_S, TPC_G,   TPC_G_S, TPC_A,   TPC_B_B, TPC_B,   TPC_C },
            {TPC_C_S, TPC_D_B, TPC_D,   TPC_D_S, TPC_E_B, TPC_E,   TPC_F,   TPC_F_S, TPC_G_B, TPC_G,   TPC_G_S, TPC_A_B,
                  TPC_A,   TPC_A_S, TPC_B_B, TPC_B, TPC_C },
            {TPC_C_S, TPC_D_B, TPC_D,   TPC_D_S, TPC_E_B, TPC_E,   TPC_F_B, TPC_E_S, TPC_F,   TPC_F_S, TPC_G_B, TPC_G,
                  TPC_G_S, TPC_A_B, TPC_A,   TPC_A_S, TPC_B_B, TPC_B,   TPC_C_B, TPC_B_S, TPC_C },
            //            TODO: 35-note list TO BE CHECKED!!!
            {TPC_D_BB,TPC_C_S, TPC_D_B, TPC_C_SS,TPC_D,   TPC_E_BB,TPC_D_S, TPC_E_B, TPC_D_SS,TPC_E,   TPC_F_BB,TPC_E_S,
                  TPC_F_B, TPC_E_SS,TPC_F,   TPC_G_BB,TPC_F_S, TPC_G_B, TPC_F_SS,TPC_G,   TPC_A_BB, TPC_G_S, TPC_A_B,
                  TPC_G_SS,TPC_A,   TPC_B_BB,TPC_A_S, TPC_B_B, TPC_A_SS,TPC_B,   TPC_C_BB,TPC_B_S,  TPC_C_B, TPC_B_SS, TPC_C }
      };

const int Scale::ABSOLUTE_CENTS;
const int Scale::DELTA_CENTS;
const int Scale::ABSOLUTE_FREQUENCY;

const int Scale::A_REFERENCE;
const int Scale::C_REFERENCE;

const int Scale::NB_ONLY_NOTES;
const int Scale::NB_ALL_SEMITONES;
const int Scale::NB_BOTH_ACCIDENTALS;
const int Scale::NB_ALL_SINGLE_ACCIDENTALS;
const int Scale::NB_ALL_NOTES;

// supported Scala types and also map to tpcs array
const map<int, int> Scale::SUPPORTED_NBS = {
      {NB_ONLY_NOTES,             0},
      {NB_ALL_SEMITONES,          1},
      {NB_BOTH_ACCIDENTALS,       2},
      {NB_ALL_SINGLE_ACCIDENTALS, 3},
      {NB_ALL_NOTES,              4}};

//---------------------------------------------------------
//   Scale default c'tor
//---------------------------------------------------------

Scale::Scale()
      {
      name = "Equal Temperament";
      memset(computedTunings, 0, sizeof(computedTunings));
      minTpc = -1;
      maxTpc = 33;
      for (int tpc = minTpc; tpc <= maxTpc; ++tpc)
            originalNotes[tpc - TPC_MIN] = QString::number(STANDARD_NOTES[tpc - TPC_MIN]);
      updatePitches = false;
      }

//---------------------------------------------------------
//   Scale c'tor
//---------------------------------------------------------

Scale::Scale(const ScaleParams& params)
      {
      name = "Custom Temperament";
      auto minMax = computeMinMaxTpc(params.nbNotes);
      minTpc = minMax.first;
      maxTpc = minMax.second;
      for (int tpc = minTpc; tpc <= maxTpc; ++tpc)
            originalNotes[tpc - TPC_MIN] = params.notes[tpc - TPC_MIN];
      computeTunings(params.storingMode, params.storeFifths);
      updatePitches = false;
      }

//---------------------------------------------------------
//   getNextLine
//---------------------------------------------------------

QString Scale::getNextScalaLine(QTextStream& in)
      {
      while (!in.atEnd()) {
            QString line = in.readLine();
            if (line[0] == '!')
                  continue;
            return line;
            }
      return QString();
      }

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void Scale::clear()
      {
      memset(computedTunings, 0, sizeof(computedTunings));
      minTpc = TPC_MAX;
      maxTpc = TPC_MIN;
      for (int tpc = TPC_MIN; tpc <= TPC_MAX; ++tpc)
            originalNotes[tpc - TPC_MIN] = "";
      }

//---------------------------------------------------------
//   loadScalaFile
//---------------------------------------------------------

bool Scale::loadScalaFile(const QString& fn)
      {
      clear();
      QFile f(fn);
      if (!f.open(QIODevice::ReadOnly)) {
            qDebug() << "Could not open file " << fn;
            MScore::lastError = strerror(errno);
            return false;
            }

      QTextStream in(&f);

      // Read description of the scala file
      name = getNextScalaLine(in);

      int nbNotes = getNextScalaLine(in).toInt();
      minTpc      = TPC_MAX;
      maxTpc      = TPC_MIN;
      if (SUPPORTED_NBS.find(nbNotes) == SUPPORTED_NBS.end()) {
            MScore::lastError = "MuseScore supports only Scala files with 7, 12, 17, 21 or 35 notes";
            return false;
            }
      int tpcItem = SUPPORTED_NBS.find(nbNotes)->second;

      // read each line with a value for line
      for (int i = 0; i < nbNotes; ++i) {
            int tpc = TPCS[tpcItem][i];
            originalNotes[tpc - TPC_MIN] = getNextScalaLine(in);
            if (originalNotes[tpc - TPC_MIN].contains('/')) {
                  QStringList values = originalNotes[tpc - TPC_MIN].split('/');
                  if (values.size() != 2 || values[1].toDouble() == 0) {
                        MScore::lastError = "Unsupported Scala file format.";
                        return false;
                        }
                  }
            if (tpc < minTpc)      minTpc = tpc;      // keep note of tpc range
            if (tpc > maxTpc)      maxTpc = tpc;
            }
      computeTunings();

      return true;
      }

//---------------------------------------------------------
//   convertValue
//---------------------------------------------------------

float Scale::convertValue(QString value, int mode)
      {
      float floatValue = 0;
      if (value.contains('/')) {
            if (mode == ABSOLUTE_CENTS || mode == ABSOLUTE_FREQUENCY) {
                  QStringList values = value.split('/');
                  if (values[1].toDouble() == 0)
                        values[1] = "1";
                  floatValue = 1200 * log(values[0].toDouble() / values[1].toDouble()) / log(2);
                  }
            }
      else if (mode == ABSOLUTE_CENTS)
            floatValue = value.toDouble();
      else if (mode == DELTA_CENTS)
            floatValue = value.toDouble();
      else if (mode == ABSOLUTE_FREQUENCY)
            floatValue = 1200 * log(value.toDouble()) / log(2);
      else
            return 0;

      return floatValue;
      }

//---------------------------------------------------------
//   ConvertNoteValue
//---------------------------------------------------------

float Scale::convertNoteValue(QString noteValue, int tpc, int mode, bool storeFifths, float prevValue)
      {
      float value = convertValue(noteValue, mode);

      if (storeFifths)
            value += prevValue;

      if (mode == ABSOLUTE_CENTS || mode == ABSOLUTE_FREQUENCY) {
            // convert from offset within an octave to a delta from 12EDO
            if (!storeFifths)
                  value -= getStandardNoteValue(tpc);
            else
                  value -= getDifference(getStandardNoteValue(tpc),
                        getStandardNoteValue(prevNote(tpc)));
            }

      value = (value >= CENTS_PER_OCTAVE) ? value - CENTS_PER_OCTAVE : value;
      return value;
      }

//---------------------------------------------------------
//   getStandardNoteValue
//---------------------------------------------------------

int Scale::getStandardNoteValue(int tpc)
      {
      return ((TPC_FIRST_STEP + (tpc - TPC_MIN) * PITCH_DELTA_FIFTH) * CENTS_PER_SEMITONE) % CENTS_PER_OCTAVE;
      }

//---------------------------------------------------------
//   computeTunings
//---------------------------------------------------------

void Scale::computeTunings(int mode, bool storeFifths)
      {
      memset(computedTunings, 0, sizeof(computedTunings));
      // read each line with a value for line
      for (int tpc = minTpc; tpc <= maxTpc; ++tpc) {
            if (storeFifths && tpc == minTpc) {
                  computedTunings[0] = 0;
                  continue;
                  }

            QString pitchLine = originalNotes[tpc - TPC_MIN];
            float prevValue = (tpc - TPC_MIN == 0) ? 0 : computedTunings[prevNote(tpc) - TPC_MIN];
            float value = convertNoteValue(pitchLine, tpc, mode, storeFifths, prevValue);
            computedTunings[tpc - TPC_MIN] = value;
            }

      // fill missing black keys, by interpolating between adjacent white keys
      if (minTpc > TPC_E_B) {                         // if some of the 12-tone black key is missing at the bottom
            for (int i = minTpc-1; i >= TPC_E_B; i--)
                  computedTunings[i-TPC_MIN] =
                        (computedTunings[i + TPC_FLAT_TO_WHITE_BELOW - TPC_MIN] +
                         computedTunings[i + TPC_FLAT_TO_WHITE_ABOVE - TPC_MIN]) / 2;
            }
      if (maxTpc < TPC_G_S) {                         // if some of the 12-tone black key is missing at the top
            for (int i = maxTpc+1; i <= TPC_G_S; i++)
                  computedTunings[i-TPC_MIN] =
                        (computedTunings[i - TPC_FLAT_TO_WHITE_BELOW - TPC_MIN] +
                         computedTunings[i - TPC_FLAT_TO_WHITE_ABOVE - TPC_MIN]) / 2;
            }

      // now, we certainly have at least 12 notes and at least one representative for each possible enharmony
      // fill the remaining notes at the bottom, from lowest known to bottom
      for (int i=TPC_E_B - 1; i>= TPC_MIN; i--)
            computedTunings[i-TPC_MIN] = computedTunings[i+TPC_DELTA_ENHARMONIC-TPC_MIN];
      // fill the remaining notes at the top, from highest knwon to top
      for (int i=TPC_G_S + 1; i<= TPC_MAX; i++)
            computedTunings[i-TPC_MIN] = computedTunings[i-TPC_DELTA_ENHARMONIC-TPC_MIN];

      // shift everything to have a correct A
      float delta = computedTunings[TPC_A - TPC_MIN];
      if (delta != 0) {
            for (int i = 0; i < TPC_NUM_OF; i++)
                  computedTunings[i] -= delta;
            }
      }

//---------------------------------------------------------
//   prevNote
//---------------------------------------------------------

int Scale::prevNote(int tpc)
      {
      if (tpc == minTpc)
            return maxTpc;
      return tpc - 1;
      }

//---------------------------------------------------------
//   prevNote
//---------------------------------------------------------

int Scale::prevNote(int tpc, int minTpc, int maxTpc)
      {
      if (tpc == minTpc)
            return maxTpc;
      return tpc - 1;
      }

//---------------------------------------------------------
//    getDifference
//    Keep this function around in case we need special
//    logic for notes that wrap around.
//---------------------------------------------------------

float Scale::getDifference(float noteCents1, float noteCents2)
      {
      return noteCents1 - noteCents2;
      }

pair<int, int> Scale::computeMinMaxTpc(int nbNotes)
      {
      int tpcItem = SUPPORTED_NBS.find(nbNotes)->second;
      int minTpc = TPC_MAX;
      int maxTpc = TPC_MIN;
      for (int i = 0; i < nbNotes; ++i) {
            minTpc = minTpc > TPCS[tpcItem][i] ? TPCS[tpcItem][i] : minTpc;
            maxTpc = maxTpc < TPCS[tpcItem][i] ? TPCS[tpcItem][i] : maxTpc;
            }
      return pair<int, int>(minTpc, maxTpc);
      }

//---------------------------------------------------------
//   recomputeNotes
//---------------------------------------------------------

void Scale::recomputeNotes(const ScaleParams& from, ScaleParams& to)
      {
      bool storeFractions = false;
      if (from.storeFifths == false && to.storeFifths == false &&
          from.reference == to.reference &&
          from.storingMode != Scale::DELTA_CENTS &&
          to.storingMode != Scale::DELTA_CENTS) {
            storeFractions = true;
            }

      Scale sFrom(from);
      float* computedTunings = sFrom.getComputedTunings();

      float delta = 0;
      if (to.reference == C_REFERENCE)
            delta = computedTunings[TPC_C - TPC_MIN];
      else if (to.reference == A_REFERENCE)
            delta = computedTunings[TPC_A - TPC_MIN];

      if (delta != 0) {
            for (int i = 0; i < TPC_NUM_OF; i++)
                  computedTunings[i] = computedTunings[i] - delta;
            }

      // Add back the standard note values
      for (int tpc = TPC_MIN; tpc <= TPC_MAX; ++tpc)
            computedTunings[tpc - TPC_MIN] += getStandardNoteValue(tpc);

      auto minMax = computeMinMaxTpc(to.nbNotes);
      for (int i = 0; i < TPC_NUM_OF; ++i)
            to.notes[i] = "";
      int minTpc = minMax.first;
      int maxTpc = minMax.second;

      if (!to.storeFifths) {
            for (int tpc = minTpc; tpc <= maxTpc; ++tpc) {
                  if (storeFractions && from.notes[tpc - TPC_MIN].contains('/')) {
                        to.notes[tpc - TPC_MIN] = from.notes[tpc - TPC_MIN];
                        continue;
                        }

                  float value = computedTunings[tpc - TPC_MIN];
                  if (to.storingMode == ABSOLUTE_CENTS)
                        to.notes[tpc - TPC_MIN] = QString::number(value);
                  else if (to.storingMode == DELTA_CENTS)
                        to.notes[tpc - TPC_MIN] =
                              QString::number(value - getStandardNoteValue(tpc));
                  else if (to.storingMode == ABSOLUTE_FREQUENCY)
                        to.notes[tpc - TPC_MIN] = QString::number(exp(((double)value / 1200) * log(2)));
                  }
            }
      else {
            for (int tpc = minTpc; tpc <= maxTpc; ++tpc) {
                  int prevTpc = prevNote(tpc, minTpc, maxTpc);

                  float value = getDifference(computedTunings[tpc - TPC_MIN],
                        computedTunings[prevTpc - TPC_MIN]);

                  if (to.storingMode == ABSOLUTE_CENTS)
                        to.notes[tpc - TPC_MIN] = QString::number(value);
                  else if (to.storingMode == DELTA_CENTS)
                        to.notes[tpc - TPC_MIN] = QString::number(value -
                              getDifference(getStandardNoteValue(tpc), getStandardNoteValue(prevTpc)));
                  else if (to.storingMode == ABSOLUTE_FREQUENCY)
                        to.notes[tpc - TPC_MIN] = QString::number(exp(((double)value / 1200) * log(2)));
                  }
            }
      }

//---------------------------------------------------------
//   getTuning
//---------------------------------------------------------

int Scale::getTuning(const Note* note) const
      {
      int tpc = note->tpc1();
      Q_ASSERT(tpc >= TPC_MIN || tpc <= TPC_MAX);
      return computedTunings[tpc - TPC_MIN];
      }
}
