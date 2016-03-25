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
      // set validators for numerator and denominator strings
      // which only accept '+', '(', ')', digits and some time symb conventional representations
      QRegExp rx("[0-9+CO()\\x00A2\\x00D8]*");
      QValidator *validator = new QRegExpValidator(rx, this);
      zText->setValidator(validator);
      nText->setValidator(validator);

      Fraction nominal = timesig->sig() / timesig->stretch();
      nominal.reduce();
      zNominal->setValue(nominal.numerator());
      nNominal->setValue(nominal.denominator());
      Fraction sig(timesig->sig());
      zActual->setValue(sig.numerator());
      nActual->setValue(sig.denominator());
      zNominal->setEnabled(false);
      nNominal->setEnabled(false);

       // TODO: fix http://musescore.org/en/node/42341
      // for now, editing of actual (local) time sig is disabled in dialog
      // but more importantly, the dialog should make it clear that this is "local" change only
      // and not normally the right way to add 7/4 to a score
      zActual->setEnabled(false);
      nActual->setEnabled(false);
      switch (timesig->timeSigType()) {
            case TimeSigType::NORMAL:
                  textButton->setChecked(true);
                  break;
            case TimeSigType::FOUR_FOUR:
                  fourfourButton->setChecked(true);
                  break;
            case TimeSigType::ALLA_BREVE:
                  allaBreveButton->setChecked(true);
                  break;
            }

      // set ID's of other symbols
      static const SymId prolatioSymbols[] = {
            SymId::mensuralProlation1,          // tempus perfectum, prol. perfecta
            SymId::mensuralProlation2,          // tempus perfectum, prol. imperfecta
            SymId::mensuralProlation3,          // tempus perfectum, prol. imperfecta, dimin.
            SymId::mensuralProlation4,          // tempus perfectum, prol. perfecta, dimin.
            SymId::mensuralProlation5,          // tempus imperf. prol. perfecta
//            SymId::mensuralProlation6,              // same shape as common time
            SymId::mensuralProlation7,          // tempus imperf., prol. imperfecta, reversed
            SymId::mensuralProlation8,          // tempus imperf., prol. perfecta, dimin.
//            SymId::mensuralProlation9,              // same shape as alla breve
            SymId::mensuralProlation10,         // tempus imperf., prol imperfecta, dimin., reversed
            SymId::mensuralProlation11,         // tempus inperf., prol. perfecta, reversed
            };

      ScoreFont* scoreFont = gscore->scoreFont();
      int idx = 0;
      for (SymId symId : prolatioSymbols) {
            const QString& str = scoreFont->toString(symId);
            if (str.size() > 0) {
                  otherCombo->setItemData(idx, (int)symId);
                  // if time sig matches this symbol string, set as selected
                  if (timesig->timeSigType() == TimeSigType::NORMAL && timesig->denominatorString().isEmpty()
                  && timesig->numeratorString() == str) {
                        textButton->setChecked(false);
                        otherButton->setChecked(true);
                        otherCombo->setCurrentIndex(idx);
                        }
                  }
            idx++;
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

      TimeSigType ts = TimeSigType::NORMAL;
      if (textButton->isChecked())
            ts = TimeSigType::NORMAL;
      else if (fourfourButton->isChecked())
            ts = TimeSigType::FOUR_FOUR;
      else if (allaBreveButton->isChecked())
            ts = TimeSigType::ALLA_BREVE;
      else if (otherButton->isChecked()) {
            // if other symbol, set as normal text...
            ts = TimeSigType::NORMAL;
            ScoreFont* scoreFont = timesig->score()->scoreFont();
            SymId symId = (SymId)( otherCombo->itemData(otherCombo->currentIndex()).toInt() );
            // ...and set numerator to font string for symbol and denominator to empty string
            timesig->setNumeratorString(scoreFont->toString(symId));
            timesig->setDenominatorString(QString());
            }

      Fraction actual(zActual->value(), nActual->value());
      Fraction nominal(zNominal->value(), nNominal->value());
      timesig->setSig(actual, ts);
      timesig->setStretch(nominal / actual);

      Groups g = groups->groups();
      timesig->setGroups(g);
      QDialog::accept();
      }
}

