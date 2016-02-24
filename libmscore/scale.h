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

#include <vector>
#include "note.h"

namespace Ms {

class Scale {
public:
      bool loadScale(const QString&);
      void tuneNote(Note* note) const;

private:
      QString getNextLine(QTextStream& in);

      std::vector<double> notes;
      std::vector<double> standardNotes = { 0, 200, 400, 500, 700, 900, 1100, 1200 };
};

}     // namespace Ms

#endif