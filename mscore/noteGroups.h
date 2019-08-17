//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2013 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#ifndef __NOTE_GROUPS_H__
#define __NOTE_GROUPS_H__

#include "ui_note_groups.h"
#include "libmscore/fraction.h"
#include "libmscore/groups.h"

namespace Ms {

class Chord;
class Score;

//---------------------------------------------------------
//   NoteGroups
//---------------------------------------------------------

class NoteGroups : public QGroupBox, Ui::NoteGroups {
      Q_OBJECT

      std::vector<Chord*> chords8;
      std::vector<Chord*> chords16;
      std::vector<Chord*> chords32;
      Groups _groups;
      Fraction _sig;
      QString _z, _n;

      Score* createScore(int n, TDuration::DurationType t, std::vector<Chord*>* chords);
      void updateBeams(Chord*, Beam::Mode);

   private slots:
      void resetClicked();
      void noteClicked(Note*);
      void beamPropertyDropped(Chord*, Icon*);

   public:
      NoteGroups(QWidget* parent);
      void setSig(Fraction sig, const Groups&, const QString& zText, const QString& nText);
      Groups groups();
      };


} // namespace Ms
#endif

