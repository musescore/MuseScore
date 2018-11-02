//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2011 Werner Schweer and others
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

#include "tupletdialog.h"
#include "libmscore/tuplet.h"
#include "libmscore/score.h"
#include "libmscore/chord.h"
#include "libmscore/note.h"
#include "libmscore/xml.h"
#include "preferences.h"
#include "libmscore/style.h"
#include "libmscore/text.h"
#include "libmscore/element.h"
#include "libmscore/utils.h"
#include "libmscore/measure.h"
#include "libmscore/undo.h"
#include "libmscore/stem.h"
#include "musescore.h"
#include "scoreview.h"

namespace Ms {

//---------------------------------------------------------
//   TupletDialog
//---------------------------------------------------------

TupletDialog::TupletDialog(QWidget* parent)
   : QDialog(parent)
      {
      setObjectName("TupletDialog");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      MuseScore::restoreGeometry(this);
      }

//---------------------------------------------------------
//   setupTuplet
//---------------------------------------------------------

void TupletDialog::setupTuplet(Tuplet* tuplet)
      {
      tuplet->setRatio(Fraction(actualNotes->value(), normalNotes->value()));
      if (number->isChecked())
            tuplet->setNumberType(TupletNumberType::SHOW_NUMBER);
      else if (relation->isChecked())
            tuplet->setNumberType(TupletNumberType::SHOW_RELATION);
      else if (noNumber->isChecked())
            tuplet->setNumberType(TupletNumberType::NO_TEXT);
      if (autoBracket->isChecked())
            tuplet->setBracketType(TupletBracketType::AUTO_BRACKET);
      else if (bracket->isChecked())
            tuplet->setBracketType(TupletBracketType::SHOW_BRACKET);
      else if (noBracket->isChecked())
            tuplet->setBracketType(TupletBracketType::SHOW_NO_BRACKET);
      }

//---------------------------------------------------------
//   tupletDialog
//    create tuplet dialog
//---------------------------------------------------------

Tuplet* MuseScore::tupletDialog()
      {
      if (!cs)
            return 0;
      ChordRest* cr = 0;
      if (cv->noteEntryMode()) {
            cs->expandVoice();
            cs->changeCRlen(cs->inputState().cr(), cs->inputState().duration());
            cr = cs->inputState().cr();
            }
      else
            cr = cs->getSelectedChordRest();
      if (cr == 0)
            return 0;
      if (cr->durationType() < TDuration(TDuration::DurationType::V_128TH) && cr->durationType() != TDuration(TDuration::DurationType::V_MEASURE)) {
            noteTooShortForTupletDialog();
            return 0;
            }
      Measure* measure = cr->measure();
      if (measure && measure->isMMRest())
            return 0;

      TupletDialog td;
      if (!td.exec())
            return 0;

      Tuplet* tuplet = new Tuplet(cs);
      tuplet->setTrack(cr->track());
      tuplet->setTick(cr->tick());
      td.setupTuplet(tuplet);
      Fraction f1(cr->duration());
      tuplet->setDuration(f1);
      Fraction f = f1 * Fraction(1, tuplet->ratio().denominator());
      f.reduce();

      qDebug("len %s  ratio %s  base %s",
         qPrintable(f1.print()),
         qPrintable(tuplet->ratio().print()),
         qPrintable(f.print()));

      Fraction fbl(1, f.denominator());
      if (TDuration::isValid(fbl))
            tuplet->setBaseLen(fbl);
      else
            tuplet->setBaseLen(TDuration::DurationType::V_INVALID);

      if (tuplet->baseLen() == TDuration::DurationType::V_INVALID) {
            QMessageBox::warning(0,
               tr("Tuplet Error"),
               tr("Cannot create tuplet with ratio %1 for duration %2").arg(tuplet->ratio().print()).arg(f1.print()));
            delete tuplet;
            return 0;
            }

      tuplet->setParent(measure);

      return tuplet;
      }

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void TupletDialog::hideEvent(QHideEvent* event)
      {
      MuseScore::saveGeometry(this);
      QWidget::hideEvent(event);
      }
}

