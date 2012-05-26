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

//---------------------------------------------------------
//    TimeSigProperties
//---------------------------------------------------------

TimeSigProperties::TimeSigProperties(TimeSig* t, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      timesig = t;
      zText->setText(timesig->zText());
      nText->setText(timesig->nText());
      zNominal->setValue(timesig->sig().numerator());
      nNominal->setValue(timesig->sig().denominator());
      zActual->setValue(timesig->actualSig().numerator());
      nActual->setValue(timesig->actualSig().denominator());
      switch(timesig->subtype()) {
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
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void TimeSigProperties::accept()
      {
      if (zText->text() != timesig->zText() || nText->text() != timesig->nText())
            timesig->setText(zText->text(), nText->text());
//      timesig->setSig(Fraction(zNominal->value(), nNominal->value()));
//      timesig->setActualSig(Fraction(zActual->value(), nActual->value()));
      if (textButton->isChecked())
            timesig->setSubtype(TSIG_NORMAL);
      else if (fourfourButton->isChecked()) {
            timesig->setSubtype(TSIG_FOUR_FOUR);
            qDebug("fourfour\n");
            }
      else if (allaBreveButton->isChecked()) {
            timesig->setSubtype(TSIG_ALLA_BREVE);
            qDebug("alla breve\n");
            }
      // setSig() and setActualSig must be AFTER setSubType()
      // as setSubType() also reset sig
      timesig->setSig(Fraction(zNominal->value(), nNominal->value()));
      timesig->setActualSig(Fraction(zActual->value(), nActual->value()));
      QDialog::accept();
      }

