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

#include "noteGroups.h"
#include "libmscore/chord.h"
#include "libmscore/mcursor.h"
#include "libmscore/timesig.h"
#include "libmscore/score.h"
#include "libmscore/part.h"
#include "musescore.h"

namespace Ms {

extern void populateIconPalette(Palette* p, const IconAction* a);

//---------------------------------------------------------
//   createScore
//---------------------------------------------------------

Score* NoteGroups::createScore(int n, TDuration::DurationType t, std::vector<Chord*>* chords)
      {
      MCursor c;
      c.setTimeSig(_sig);
      c.createScore("score8");
      c.addPart("voice");
      c.move(0, 0);
      c.addKeySig(0);
      TimeSig* nts = c.addTimeSig(_sig);
      GroupNode node {0, 0};
      Groups ng;
      ng.push_back(node);
      nts->setGroups(ng);

      for (int i = 0; i < n; ++i) {
            Chord* chord = c.addChord(67, t);
            int tick = chord->rtick();
            chord->setBeamMode(_groups.beamMode(tick, t));
            chords->push_back(chord);
            }

      c.score()->parts().front()->setLongName("");
      c.score()->style()->set(ST_linearStretch, 1.1);
      return c.score();
      }

//---------------------------------------------------------
//   NoteGroups
//---------------------------------------------------------

NoteGroups::NoteGroups(QWidget* parent)
   : QGroupBox(parent)
      {
      setupUi(this);
      static const IconAction bpa[] = {
            { ICON_SBEAM,    "beam-start" },
            { ICON_MBEAM,    "beam-mid" },
            { ICON_BEAM32,   "beam32" },
            { ICON_BEAM64,   "beam64" },
            { -1, ""}
            };

      iconPalette->setName(QT_TRANSLATE_NOOP("Palette", "Beam Properties"));
      iconPalette->setGrid(27, 40);
      iconPalette->setMag(.5);
      iconPalette->setDrawGrid(true);
      populateIconPalette(iconPalette, bpa);
      iconPalette->setReadOnly(true);

      connect(resetGroups, SIGNAL(clicked()), SLOT(resetClicked()));
      connect(view8,  SIGNAL(noteClicked(Note*)), SLOT(noteClicked(Note*)));
      connect(view16, SIGNAL(noteClicked(Note*)), SLOT(noteClicked(Note*)));
      connect(view32, SIGNAL(noteClicked(Note*)), SLOT(noteClicked(Note*)));
      }

//---------------------------------------------------------
//   setSig
//---------------------------------------------------------

void NoteGroups::setSig(Fraction sig, const Groups& g)
      {
      _sig    = sig;
      _groups = g;
      chords8.clear();
      chords16.clear();
      chords32.clear();
      int n   = _sig.numerator() * (8 / _sig.denominator());
      view8->setScore(createScore(n, TDuration::V_EIGHT, &chords8));
      n <<= 1;
      view16->setScore(createScore(n, TDuration::V_16TH, &chords16));
      n <<= 1;
      view32->setScore(createScore(n, TDuration::V_32ND, &chords32));
      }

//---------------------------------------------------------
//   groups
//---------------------------------------------------------

Groups NoteGroups::groups()
      {
      Groups g;
      for (Chord* chord : chords8)
            g.addStop(chord->rtick(), chord->durationType().type(), chord->beamMode());
      for (Chord* chord : chords16)
            g.addStop(chord->rtick(), chord->durationType().type(), chord->beamMode());
      for (Chord* chord : chords32)
            g.addStop(chord->rtick(), chord->durationType().type(), chord->beamMode());
      return g;
      }

//---------------------------------------------------------
//   resetClicked
//---------------------------------------------------------

void NoteGroups::resetClicked()
      {
      setSig(_sig, _groups);
      }

//---------------------------------------------------------
//   note8Clicked
//---------------------------------------------------------

void NoteGroups::noteClicked(Note* note)
      {
      Chord* chord = note->chord();
      if (chord->beamMode() == BeamMode::AUTO)
            chord->setBeamMode(BeamMode::BEGIN);
      else if (chord->beamMode() == BeamMode::BEGIN)
            chord->setBeamMode(BeamMode::AUTO);
      chord->score()->doLayout();
      view8->update();
      view16->update();
      view32->update();
      }
}

