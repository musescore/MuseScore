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

class Scale {
public:
      bool loadScale(const QString&);
      double getTuning(const Note* note) const;

private:
      QString getNextLine(QTextStream& in);

      std::vector<std::string> originalNotes;
      std::map<Tpc, double> computedTunings;

      std::vector<Tpc> getNotePositions7();
      std::vector<Tpc> getNotePositions12();
      std::vector<Tpc> getNotePositions17();
      std::vector<Tpc> getNotePositions21();
      std::vector<Tpc> getNotePositions35();

      void computeTuning();

      static void fillMissing7(std::map<Tpc, double>& notes);
      static void fillMissing12(std::map<Tpc, double>& notes);
      static void fillMissing17(std::map<Tpc, double>& notes);
      static void fillMissing21(std::map<Tpc, double>& notes);

      static constexpr int NB_ONLY_NOTES = 7;
      static constexpr int NB_ALL_SEMITONES = 12;
      static constexpr int NB_BOTH_ACCIDENTALS = 17;
      static constexpr int NB_ALL_SINGLE_ACCIDENTALS = 21;
      static constexpr int NB_ALL_NOTES = 35;
};

}     // namespace Ms

#endif