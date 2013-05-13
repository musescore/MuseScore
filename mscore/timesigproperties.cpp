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

namespace Ms {

extern void populateIconPalette(Palette* p, const IconAction* a);

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

      Groups g = t->groups();
      if (g.empty())
            g = Groups::endings(timesig->sig());     // initialize with default
      groups->setSig(timesig->sig(), g);
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

      Groups g = groups->groups();
      timesig->setGroups(g);
      QDialog::accept();
      }
}

