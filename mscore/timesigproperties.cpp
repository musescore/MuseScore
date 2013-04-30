//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: restproperties.cpp 1840 2009-05-20 11:57:51Z wschweer $
//
//  Copyright (C) 2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "timesigproperties.h"
#include "libmscore/timesig.h"
#include "libmscore/mcursor.h"
#include "libmscore/durationtype.h"
#include "libmscore/score.h"
#include "libmscore/chord.h"
#include "libmscore/measure.h"
#include "libmscore/part.h"
#include "exampleview.h"
#include "musescore.h"

extern void populateIconPalette(Palette* p, const IconAction* a);

//---------------------------------------------------------
//   createScore
//---------------------------------------------------------

static Score* createScore(TimeSig* ts, int n, TDuration::DurationType t, std::vector<Chord*>* chords)
      {
      Fraction sig(ts->sig());
      MCursor c;
      c.setTimeSig(ts->sig());
      c.createScore("score8");
      c.addPart("voice");
      c.move(0, 0);
      c.addKeySig(0);
      TimeSig* nts = c.addTimeSig(ts->sig());
      GroupNode node {0, 0};
      Groups ng;
      ng.push_back(node);

      nts->setGroups(ng);
      Groups g = ts->groups();
      if (g.empty())
            g = Groups::endings(ts->sig());     // initialize with default

      for (int i = 0; i < n; ++i) {
            Chord* chord = c.addChord(67, t);
            int tick = chord->rtick();
            chord->setBeamMode(g.beamMode(tick, t));
            chords->push_back(chord);
            }

      c.score()->parts().front()->setLongName("");
      c.score()->style()->set(ST_linearStretch, 1.1);
      return c.score();
      }

//---------------------------------------------------------
//    TimeSigProperties
//---------------------------------------------------------

TimeSigProperties::TimeSigProperties(TimeSig* t, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      timesig = t;
      zText->setText(timesig->numeratorString());
      nText->setText(timesig->denominatorString());
      Fraction nominal = timesig->sig() * timesig->stretch();
      nominal.reduce();
      zNominal->setValue(nominal.numerator());
      nNominal->setValue(nominal.denominator());
      zActual->setValue(timesig->sig().numerator());
      nActual->setValue(timesig->sig().denominator());
      switch (timesig->timeSigType()) {
            case TSIG_NORMAL:
                  textButton->setChecked(true);
                  break;
            case TSIG_FOUR_FOUR:
                  fourfourButton->setChecked(true);
                  break;
            case TSIG_ALLA_BREVE:
                  allaBreveButton->setChecked(true);
                  break;
            }
      //
      // TODO: xx/yy were yy >= 16
      //

      int n = nominal.numerator() * (8 / nominal.denominator());

      score8  = createScore(t, n, TDuration::V_EIGHT, &chords8);
      n <<= 1;
      score16 = createScore(t, n, TDuration::V_16TH, &chords16);
      n <<= 1;
      score32 = createScore(t, n, TDuration::V_32ND, &chords32);

      view8->setScore(score8);
      view16->setScore(score16);
      view32->setScore(score32);
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
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void TimeSigProperties::accept()
      {
      if (zText->text() != timesig->numeratorString())
            timesig->setNumeratorString(zText->text());
      if (nText->text() != timesig->denominatorString())
            timesig->setDenominatorString(nText->text());

      TimeSigType ts = TSIG_NORMAL;
      if (textButton->isChecked())
            ts = TSIG_NORMAL;
      else if (fourfourButton->isChecked())
            ts = TSIG_FOUR_FOUR;
      else if (allaBreveButton->isChecked())
            ts = TSIG_ALLA_BREVE;

      Fraction actual(zActual->value(), nActual->value());
      Fraction nominal(zNominal->value(), nNominal->value());
      timesig->setSig(actual, ts);
      timesig->setStretch(nominal / actual);

      Groups groups;
      for (Chord* chord : chords8)
            groups.addStop(chord->rtick(), chord->durationType().type(), chord->beamMode());
      for (Chord* chord : chords16)
            groups.addStop(chord->rtick(), chord->durationType().type(), chord->beamMode());
      for (Chord* chord : chords32)
            groups.addStop(chord->rtick(), chord->durationType().type(), chord->beamMode());
      timesig->setGroups(groups);
      QDialog::accept();
      }

