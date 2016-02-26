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

const int Scale::NB_ONLY_NOTES;
const int Scale::NB_ALL_SEMITONES;
const int Scale::NB_BOTH_ACCIDENTALS;
const int Scale::NB_ALL_SINGLE_ACCIDENTALS;
const int Scale::NB_ALL_NOTES;

//---------------------------------------------------------
//   getNextLine
//---------------------------------------------------------

QString Scale::getNextLine(QTextStream& in)
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
//   loadScale
//---------------------------------------------------------

bool Scale::loadScale(const QString& fn)
      {
      static map<int, vector<Tpc>> notePositionsMap = {
            {NB_ONLY_NOTES, getNotePositions7()},
            {NB_ALL_SEMITONES, getNotePositions12()},
            {NB_BOTH_ACCIDENTALS, getNotePositions17()},
            {NB_ALL_SINGLE_ACCIDENTALS, getNotePositions21()},
            {NB_ALL_NOTES, getNotePositions35()}};
      computedTunings.clear();

      QFile f(fn);
      if (!f.open(QIODevice::ReadOnly)) {
            MScore::lastError = strerror(errno);
            return false;
            }

      QTextStream in(&f);

      // Read description of the scala file
      QString description = getNextLine(in);

      int nbNotes = getNextLine(in).toInt();
      if (notePositionsMap.find(nbNotes) == notePositionsMap.end()) {
            MScore::lastError = QObject::tr("MuseScore supports only Scala files with 7, 12, 17, 21 or 35 notes");
            return false;
            }

      vector<double> readNotes;
      for (int i = 0; i < nbNotes; ++i) {
            QString pitchLine = getNextLine(in);
            originalNotes.push_back(pitchLine.trimmed().toStdString());
            if (pitchLine.contains('/')) {
                  QStringList values = pitchLine.split('/');
                  if (values.size() != 2 || values[1].toDouble() == 0) {
                        MScore::lastError = QObject::tr("Unsupported Scala file format.");
                        return false;
                        }

                  readNotes.push_back(1200 * log(values[0].toDouble() / values[1].toDouble()) / log(2));
            } else {
                  readNotes.push_back(pitchLine.toDouble());
                  }
            }

      auto notePositionsIter = notePositionsMap.find(readNotes.size());
      if (notePositionsIter == notePositionsMap.end()) {
            MScore::lastError = QObject::tr("Unsupported Scala file format.");
                  return false;
                  }
            auto notePositions = notePositionsIter->second;

            for (size_t i = 0; i < originalNotes.size(); ++i) {
                  computedTunings[notePositions[i]] = readNotes[i];
                  }
      if (notePositionsMap.find(originalNotes.size()) == notePositionsMap.end()) {
            MScore::lastError = QObject::tr("MuseScore supports only Scala files with 7, 12, 17, 21 or 35 notes");
            return false;
            }

      // Call all fillMissing functions.
      // They will fill only if the notes are missing.
      fillMissing7(computedTunings);

      computeTuning();
      return true;
      }

//---------------------------------------------------------
//   getNotePositions7
//---------------------------------------------------------
vector<Tpc> Scale::getNotePositions7() {
      static vector<Tpc> notesToFill = {TPC_D, TPC_E, TPC_F, TPC_G, TPC_A, TPC_B, TPC_C};
      return notesToFill;
      }

//---------------------------------------------------------
//   getNotePositions12
//---------------------------------------------------------
vector<Tpc> Scale::getNotePositions12() {
      static vector<Tpc> notesToFill = {
            TPC_C_S, TPC_D,
            TPC_E_B, TPC_E,
                     TPC_F,
            TPC_F_S, TPC_G,
            TPC_G_S, TPC_A,
            TPC_B_B, TPC_B,
                     TPC_C};
      return notesToFill;
      }

//---------------------------------------------------------
//   getNotePositions17
//---------------------------------------------------------
vector<Tpc> Scale::getNotePositions17() {
      static vector<Tpc> notesToFill = {
            TPC_D_S, TPC_C_B, TPC_D,
            TPC_E_S, TPC_D_B, TPC_E,
                              TPC_F,
            TPC_G_S, TPC_F_B, TPC_G,
            TPC_A_S, TPC_G_B, TPC_A,
            TPC_B_S, TPC_A_B, TPC_B,
                              TPC_C};
      return notesToFill;
      }

//---------------------------------------------------------
//   getNotePositions21
//---------------------------------------------------------
vector<Tpc> Scale::getNotePositions21() {
      static vector<Tpc> notesToFill = {
            TPC_D_S, TPC_C_B, TPC_D,
            TPC_E_S, TPC_D_B, TPC_E,
            TPC_F_B, TPC_E_S, TPC_F,
            TPC_G_S, TPC_F_B, TPC_G,
            TPC_A_S, TPC_G_B, TPC_A,
            TPC_B_S, TPC_A_B, TPC_B,
            TPC_C_B, TPC_B_S, TPC_C};
      return notesToFill;
      }

//---------------------------------------------------------
//   getNotePositions35
//---------------------------------------------------------
vector<Tpc> Scale::getNotePositions35() {
      static vector<Tpc> notesToFill = {
            TPC_D_BB, TPC_D_S, TPC_C_B, TPC_C_SS, TPC_D,
            TPC_E_BB, TPC_E_S, TPC_D_B, TPC_D_SS, TPC_E,
            TPC_F_BB, TPC_F_B, TPC_E_S, TPC_E_SS, TPC_F,
            TPC_G_BB, TPC_G_S, TPC_F_B, TPC_F_SS, TPC_G,
            TPC_A_BB, TPC_A_S, TPC_G_B, TPC_G_SS, TPC_A,
            TPC_B_BB, TPC_B_S, TPC_A_B, TPC_A_SS, TPC_B,
            TPC_C_BB, TPC_C_B, TPC_B_S, TPC_B_SS, TPC_C};
      return notesToFill;
      }

//---------------------------------------------------------
//   fillMissing7
//---------------------------------------------------------
void Scale::fillMissing7(std::map<Tpc, double>& notes) {
      // check if the notes were filled from the file
      if (notes.find(TPC_C_S) == notes.end()) {
            notes[TPC_C_S] = notes[TPC_D] / 2;
            notes[TPC_E_B] = (notes[TPC_D] + notes[TPC_E]) / 2;
            notes[TPC_F_S] = (notes[TPC_F] + notes[TPC_G]) / 2;
            notes[TPC_G_S] = (notes[TPC_G] + notes[TPC_A]) / 2;
            notes[TPC_B_B] = (notes[TPC_A] + notes[TPC_B]) / 2;
            }

      fillMissing12(notes);
      }

//---------------------------------------------------------
//   fillMissing12
//---------------------------------------------------------
void Scale::fillMissing12(std::map<Tpc, double>& notes) {
      // check if the notes were filled from the file
      if (notes.find(TPC_D_B) == notes.end()) {
            notes[TPC_D_B] = notes[TPC_C_S];
            notes[TPC_D_S] = notes[TPC_E_B];
            notes[TPC_G_B] = notes[TPC_F_S];
            notes[TPC_A_B] = notes[TPC_G_S];
            notes[TPC_A_S] = notes[TPC_B_B];
            }

      fillMissing17(notes);
      }

//---------------------------------------------------------
//   fillMissing17
//---------------------------------------------------------
void Scale::fillMissing17(std::map<Tpc, double>& notes) {
      // check if the notes were filled from the file
      if (notes.find(TPC_E_S) == notes.end()) {
            notes[TPC_E_S] = notes[TPC_F_B] = (notes[TPC_E] + notes[TPC_F]) / 2;
            notes[TPC_B_S] = notes[TPC_C_B] = (notes[TPC_B] + notes[TPC_C]) / 2;
            }

      fillMissing21(notes);
      }

//---------------------------------------------------------
//   fillMissing21
//---------------------------------------------------------
void Scale::fillMissing21(std::map<Tpc, double>& notes) {
      // check if the notes were filled from the file
      if (notes.find(TPC_B_SS) != notes.end()) {
            return;
            }

      notes[TPC_B_SS] = notes[TPC_D_BB] = notes[TPC_C];
      notes[TPC_C_SS] = notes[TPC_E_BB] = notes[TPC_D];
      notes[TPC_D_SS] = notes[TPC_F_BB] = notes[TPC_E];
      notes[TPC_E_SS] = notes[TPC_G_BB] = notes[TPC_F];
      notes[TPC_F_SS] = notes[TPC_A_BB] = notes[TPC_G];
      notes[TPC_G_SS] = notes[TPC_B_BB] = notes[TPC_A];
      notes[TPC_A_SS] = notes[TPC_C_BB] = notes[TPC_B];
      }

//---------------------------------------------------------
//   computeTuning
//---------------------------------------------------------

void Scale::computeTuning() {
      static map<Tpc, double> standardNotes = [] {
            map<Tpc, double> standardNotes = {
                  {TPC_D, 200},
                  {TPC_E, 400},
                  {TPC_F, 500},
                  {TPC_G, 700},
                  {TPC_A, 900},
                  {TPC_B, 1100},
                  {TPC_C, 1200}};
            fillMissing7(standardNotes);

            // Between TPC_E -> TPC_F and TPC_B -> TPC_C there is only 100
            // so we need to fix the bemol and sharp notes
            standardNotes[TPC_E_S] = 500;
            standardNotes[TPC_F_B] = 400;
            standardNotes[TPC_B_S] = 1200;
            standardNotes[TPC_C_B] = 1100;

            return standardNotes;
            }();

      for (auto& noteIter : computedTunings)
            noteIter.second = noteIter.second - standardNotes[noteIter.first];
      }

//---------------------------------------------------------
//   getTuning
//---------------------------------------------------------

double Scale::getTuning(const Note* note) const
      {
      auto computedNote = computedTunings.find((Tpc)note->tpc1());

      if (computedNote == computedTunings.end()) {
            return 0;
            }

      return computedNote->second;
      }
}
