//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __SCALE_H__
#define __SCALE_H__

/**
 \file
 Definition of Scale class.
 */

#include "note.h"

namespace Ms {

struct ScaleParams {
      QString notes[TPC_NUM_OF];
      QString aTuning;
      int     nbNotes;
      int     storingMode;
      int     reference;
      bool    storeFifths;
};

class Scale {
public:
      static constexpr int            CENTS_PER_SEMITONE = 100;
      static constexpr int            CENTS_PER_OCTAVE = 1200;
      static const int                STANDARD_NOTES[TPC_NUM_OF];
      static constexpr int            NB_SCALES = 5;
      static const int                TPCS[NB_SCALES][TPC_NUM_OF];
      static const std::map<int, int> SUPPORTED_NBS;

      static constexpr int            ABSOLUTE_CENTS = 0;
      static constexpr int            DELTA_CENTS = 1;
      static constexpr int            ABSOLUTE_FREQUENCY = 2;

      static constexpr int            A_REFRENCE = 0;
      static constexpr int            C_REFERENCE = 1;
      static constexpr int            NO_REFERENCE = 2;

      static constexpr int            NB_ONLY_NOTES = 7;
      static constexpr int            NB_ALL_SEMITONES = 12;
      static constexpr int            NB_BOTH_ACCIDENTALS = 17;
      static constexpr int            NB_ALL_SINGLE_ACCIDENTALS = 21;
      static constexpr int            NB_ALL_NOTES = 35;

      static void  recomputeNotes(const ScaleParams& from, ScaleParams& to);
      static int   getStandardNoteValue(int tpc);
      static float getDifference(float noteCents1, float noteCents2);
      static int   prevNote(int tpc, int minTpc, int maxTpc);

      Scale();
      Scale(const ScaleParams& params);
      bool    loadScalaFile(const QString&);
      float*  getComputedTunings() { return computedTunings; }

      void     computeTunings(int storingMode = ABSOLUTE_CENTS, bool storeFifths = false,
                              bool skipAtuning = false);
      int      getTuning(const Note* note) const;

      QString  getName() { return name; }
      void     setName(QString name) { this->name = name; }

      QString  getAtuning() { return aTuning; }
      void     setAtuning(QString aTuning) { this->aTuning = aTuning; }

      int      getNbNotes() { return maxTpc - minTpc + 1; }
      QString* getOriginalNotes() { return originalNotes; }
      int      getMinTpc() { return minTpc; }
      int      getMaxTpc() { return maxTpc; }

      float    convertValue(QString value, int mode);
      float    convertNoteValue(QString noteValue, int tpc, int mode,
                                bool storeFifths, float prevValue = 0);
      bool     getStoreFifths() { return storeFifths; }
      void     setStoreFifths(bool storeFifths) { this->storeFifths = storeFifths; }
      int      prevNote(int tpc);

private:
      QString  originalNotes[TPC_NUM_OF];   // the strings originaly in the source
      QString  aTuning;
      float    computedTunings[TPC_NUM_OF]; // the tuning of each TPC as delta from 12EDO note
      QString  name;                        // human readable name of the scale
      // all TPC_NUM_OF items of computedNotes are always filled up
      // but only the minTpc-maxTpc range has new infomration; the other, if any, only duplicate
      // data in this range through enharmony. Having these two fields is mostly useful for the editor.
      int      minTpc;                      // the min tpc for which there are meaningful data
      int      maxTpc;                      // the max tpc for which there are meaningful data
      bool     storeFifths;

      QString  getNextScalaLine(QTextStream& in);
      void     clear();
      static std::pair<int, int> computeMinMaxTpc(int nbNotes);
};

}     // namespace Ms

#endif
