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

namespace Ms {

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
      QFile f(fn);
      if (!f.open(QIODevice::ReadOnly)) {
            MScore::lastError = strerror(errno);
            return false;
            }

      QTextStream in(&f);

      // Read description of the scala file
      QString description = getNextLine(in);

      int nbNotes = getNextLine(in).toInt();
      if (nbNotes != 7) {
            MScore::lastError = QObject::tr("MuseScore supports only Scala files with 8 notes");
            return false;
            }

      notes.push_back(0);
      for (int i = 0; i < nbNotes; ++i) {
            QString pitchLine = getNextLine(in);
            if (pitchLine.contains('/')) {
                  QStringList values = pitchLine.split('/');
                  if (values.size() != 2 || values[1].toDouble() == 0) {
                        MScore::lastError = QObject::tr("Unsupported Scala file format.");
                        return false;
                        }

                  notes.push_back(1200 * (values[0].toDouble() / values[1].toDouble()) - 1200);
            } else {
                  notes.push_back(pitchLine.toDouble());
                  }

            if (fabs(notes[i] - standardNotes[i]) > 200) {
                  MScore::lastError = QObject::tr("MuseScore supports only Scala files with notes varying at most 200 cents from standard scale");
                  return false;
                  }
            }
      return true;
      }

//---------------------------------------------------------
//   tuneNote
//---------------------------------------------------------

void Scale::tuneNote(Note* note) const
      {
      //                       c  d  e  f  g  a  b   c
      static int table[8]  = { 0, 2, 4, 5, 7, 9, 11, 12 };

      if (notes.size() != 8) {
            return;
            }

      int pitch = note->ppitch() % 12;
      for (unsigned int i = 0; i < notes.size(); ++i) {
            if (pitch < table[i+1]) {
                  if (pitch == table[i]) {
                        note->setTuning(standardNotes[i] - notes[i]);
                  } else {
                        // The pitch is in the middle of two notes.
                        note->setTuning((standardNotes[i] - notes[i]) / 2);
                        }
                  return;
                  }
            }
      }
}
