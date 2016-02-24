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

//---------------------------------------------------------
//   Scale c'tor
//---------------------------------------------------------
Scale::Scale()
      {
      memset(computedTunings, 0, sizeof(computedTunings));
      minTpc = maxTpc = 0;
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
//   loadScalaFile
//---------------------------------------------------------

bool Scale::loadScalaFile(const QString& fn)
      {
      static constexpr int NB_ONLY_NOTES = 7;
      static constexpr int NB_ALL_SEMITONES = 12;
      static constexpr int NB_BOTH_ACCIDENTALS = 17;
      static constexpr int NB_ALL_SINGLE_ACCIDENTALS = 21;
      static constexpr int NB_ALL_NOTES = 35;

      // supported Scala types and also map to tpcs array
      static map<int, int> supportedNbs = {
            {NB_ONLY_NOTES,             0},
            {NB_ALL_SEMITONES,          1},
            {NB_BOTH_ACCIDENTALS,       2},
            {NB_ALL_SINGLE_ACCIDENTALS, 3},
            {NB_ALL_NOTES,              4}};
      static int tpcs[][TPC_NUM_OF]     =
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

      memset(computedTunings, 0, sizeof(computedTunings));
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
      int   tpcItem;
      minTpc      = TPC_MAX;
      maxTpc      = TPC_MIN;
      if (supportedNbs.find(nbNotes) == supportedNbs.end()) {
            MScore::lastError = QObject::tr("MuseScore supports only Scala files with 7, 12, 17, 21 or 35 notes");
            return false;
            }
      tpcItem = supportedNbs.find(nbNotes)->second;

      // read each line with a value for line
      for (int i = 0; i < nbNotes; ++i) {
            QString pitchLine = getNextScalaLine(in);
            // TODO once the needs of th editor are known: store strings in originalNotes,
            // convert fraction format to numeric value
            int value;
            if (pitchLine.contains('/')) {
                  QStringList values = pitchLine.split('/');
                  if (values.size() != 2 || values[1].toDouble() == 0) {
                        MScore::lastError = QObject::tr("Unsupported Scala file format.");
                        return false;
                        }

                  value = round(1200 * log(values[0].toDouble() / values[1].toDouble())) / log(2);
            } else {
                  value = round(pitchLine.toDouble());
                  }

            int tpc = tpcs[tpcItem][i];
            // convert from offset within an octave to a delta from 12EDO
            value -= ((TPC_FIRST_STEP + (tpc - TPC_MIN) * PITCH_DELTA_FIFTH) * CENTS_PER_SEMITONE) % CENTS_PER_OCTAVE;
            value %= CENTS_PER_OCTAVE;                // Scala has C at 1200 rather than at 0
            computedTunings[tpc - TPC_MIN] = value;
            if (tpc < minTpc)      minTpc = tpc;      // keep note of tpc range
            if (tpc > maxTpc)      maxTpc = tpc;
            }

      // fill missing black keys, by interpolating between adjacent white keys
      if (minTpc > TPC_E_B) {                         // if some of the 12-tone black key is missing at the bottom
            for (int i = minTpc-1; i >= TPC_E_B; i--)
                  computedTunings[i-TPC_MIN] =
                        (computedTunings[i + TPC_FLAT_TO_WHITE_BELOW - TPC_MIN] +
                         computedTunings[i + TPC_FLAT_TO_WHITE_ABOVE - TPC_MIN]) / 2;
            minTpc = TPC_E_B;
            }
      if (maxTpc < TPC_G_S) {                         // if some of the 12-tone black key is missing at the top
            for (int i = maxTpc+1; i <= TPC_G_S; i++)
                  computedTunings[i-TPC_MIN] =
                        (computedTunings[i - TPC_FLAT_TO_WHITE_BELOW - TPC_MIN] +
                         computedTunings[i - TPC_FLAT_TO_WHITE_ABOVE - TPC_MIN]) / 2;
            maxTpc = TPC_G_S;
            }

      // now, we certainly have at least 12 notes and at least one representative for each possible enharmony
      // fill the remaining notes at the bottom, from lowest known to bottom
      for (int i=minTpc-1; i>= TPC_MIN; i--)
            computedTunings[i-TPC_MIN] = computedTunings[i+TPC_DELTA_ENHARMONIC-TPC_MIN];
      // fill the remaining notes at the top, from highest knwon to top
      for (int i=maxTpc+1; i<= TPC_MAX; i++)
            computedTunings[i-TPC_MIN] = computedTunings[i-TPC_DELTA_ENHARMONIC-TPC_MIN];

      // shift everything to have a correct A
      int delta = computedTunings[TPC_A - TPC_MIN];
      if (delta != 0)
            for (int i = 0; i < TPC_NUM_OF; i++)
                  computedTunings[i] -= delta;
            
      return true;
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
