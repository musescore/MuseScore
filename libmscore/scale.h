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
      static constexpr int CENTS_PER_SEMITONE = 100;
      static constexpr int CENTS_PER_OCTAVE = 1200;

      Scale();
      bool    loadScalaFile(const QString&);
      int     getTuning(const Note* note) const;
      int*    getComputedTunings() { return computedTunings; }

private:
      QString originalNotes[TPC_NUM_OF];   // the strings originaly in the source
      int     computedTunings[TPC_NUM_OF]; // the tuning of each TPC as delta from 12EDO note
      QString name;                        // human readable name of the scale
      // all TPC_NUM_OF items of computedNotes are always filled up
      // but only the minTpc-maxTpc range has new infomration; the other, if any, only duplicate
      // data in this range through enharmony. Having these two fields is mostly useful for the editor.
      int     minTpc;                      // the min tpc for which there are meaningful data
      int     maxTpc;                      // the max tpc for which there are meaningful data

      QString getNextScalaLine(QTextStream& in);
};

}     // namespace Ms

#endif