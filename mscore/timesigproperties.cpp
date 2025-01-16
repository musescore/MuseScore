//=============================================================================
//  MuseScore
//  Linux Music Score Editor
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

#include "exampleview.h"
#include "icons.h"
#include "musescore.h"
#include "timesigproperties.h"

#include "libmscore/timesig.h"
#include "libmscore/mcursor.h"
#include "libmscore/score.h"
#include "libmscore/measure.h"
#include "libmscore/part.h"

namespace Ms {

//---------------------------------------------------------
//    TimeSigProperties
//---------------------------------------------------------

TimeSigProperties::TimeSigProperties(TimeSig* t, QWidget* parent)
   : QDialog(parent)
      {
      setObjectName("TimeSigProperties");
      setupUi(this);
      fourfourButton->setIcon(*icons[int(Icons::timesig_common_ICON)]);
      allaBreveButton->setIcon(*icons[int(Icons::timesig_allabreve_ICON)]);

      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      timesig = t;

      zText->setText(timesig->numeratorString());
      nText->setText(timesig->denominatorString());
      // set validators for numerator and denominator strings
      // which only accepts '(', ')', '*' (and 'x', 'X', '×'), '+', '-', '/' (and '÷'), digits, '=', and some time symb conventional representations
      QRegExp rx("[()\\*+\\-/0-9=COXcox\\x00A2\\x00BD\\x00BC\\x00D7\\x00D8\\x00F7\\xE910-\\xE91A]*");
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

      // TODO: fix https://musescore.org/en/node/42341
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
            case TimeSigType::CUT_BACH:
//                  cutBachButton->setChecked(true);
//                  break;
            case TimeSigType::CUT_TRIPLE:
//                  cutTripleButton->setChecked(true);
                  break;
            }

      // set ID's of other symbols
      struct ProlatioTable {
            SymId id;
            Icons icon;
            };
      static const std::vector<ProlatioTable> prolatioList = {
            { SymId::mensuralProlation1,  Icons::timesig_prolatio01_ICON },  // Tempus perfectum cum prolatione perfecta (9/8)
            { SymId::mensuralProlation2,  Icons::timesig_prolatio02_ICON },  // Tempus perfectum cum prolatione imperfecta (3/4)
            { SymId::mensuralProlation3,  Icons::timesig_prolatio03_ICON },  // Tempus perfectum cum prolatione imperfecta diminution 1 (3/8)
            { SymId::mensuralProlation4,  Icons::timesig_prolatio04_ICON },  // Tempus perfectum cum prolatione perfecta diminution 2 (9/16)
            { SymId::mensuralProlation5,  Icons::timesig_prolatio05_ICON },  // Tempus imperfectum cum prolatione perfecta (6/8)
            //{ SymId::mensuralProlation6,  Icons::timesig_prolatio06_ICON },  // Tempus imperfectum cum prolatione imperfecta (2/4)
            { SymId::mensuralProlation7,  Icons::timesig_prolatio07_ICON },  // Tempus imperfectum cum prolatione imperfecta diminution 1 (2/2)
            { SymId::mensuralProlation8,  Icons::timesig_prolatio08_ICON },  // Tempus imperfectum cum prolatione imperfecta diminution 2 (6/16)
            //{ SymId::mensuralProlation9,  Icons::timesig_prolatio09_ICON },  // Tempus imperfectum cum prolatione imperfecta diminution 3 (2/2)
            { SymId::mensuralProlation10, Icons::timesig_prolatio10_ICON },  // Tempus imperfectum cum prolatione imperfecta diminution 4
            { SymId::mensuralProlation11, Icons::timesig_prolatio11_ICON },  // Tempus imperfectum cum prolatione imperfecta diminution 5
            };

      ScoreFont* scoreFont = gscore->scoreFont();
      int idx = 0;
      otherCombo->clear();
      for (ProlatioTable pt : prolatioList) {
            const QString& str = scoreFont->toString(pt.id);
            if (str.size() > 0) {
                  otherCombo->addItem(*icons[int(pt.icon)],"", int(pt.id));
                  // if time sig matches this symbol string, set as selected
                  if (timesig->timeSigType() == TimeSigType::NORMAL && timesig->denominatorString().isEmpty()
                     && timesig->numeratorString() == str) {
                        textButton->setChecked(false);
                        otherButton->setChecked(true);
                        otherCombo->setCurrentIndex(idx);

                        // set the custom text fields to empty
                        zText->setText(QString());
                        nText->setText(QString());
                        }
                  }
            idx++;
            }

      Groups g = t->groups();
      if (g.empty())
            g = Groups::endings(timesig->sig());     // initialize with default
      groups->setSig(timesig->sig(), g, timesig->numeratorString(), timesig->denominatorString());

      MuseScore::restoreGeometry(this);
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void TimeSigProperties::accept()
      {
      TimeSigType ts = TimeSigType::NORMAL;
      if (textButton->isChecked() || otherButton->isChecked())
            ts = TimeSigType::NORMAL;
      else if (fourfourButton->isChecked())
            ts = TimeSigType::FOUR_FOUR;
      else if (allaBreveButton->isChecked())
            ts = TimeSigType::ALLA_BREVE;

      Fraction actual(zActual->value(), nActual->value());
      Fraction nominal(zNominal->value(), nNominal->value());
      timesig->setSig(actual, ts);
      timesig->setStretch(nominal / actual);

      if (zText->text() != timesig->numeratorString())
            timesig->setNumeratorString(zText->text());
      if (nText->text() != timesig->denominatorString())
            timesig->setDenominatorString(nText->text());

      if (otherButton->isChecked()) {
            ScoreFont* scoreFont = timesig->score()->scoreFont();
            SymId symId = (SymId)( otherCombo->itemData(otherCombo->currentIndex()).toInt() );
            // ...and set numerator to font string for symbol and denominator to empty string
            timesig->setNumeratorString(scoreFont->toString(symId));
            timesig->setDenominatorString(QString());
            }

      Groups g = groups->groups();
      timesig->setGroups(g);
      QDialog::accept();
      }

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void TimeSigProperties::hideEvent(QHideEvent* event)
      {
      MuseScore::saveGeometry(this);
      QWidget::hideEvent(event);
      }
}

