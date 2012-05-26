//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: tuplet.cpp -1   $
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

//---------------------------------------------------------
//   TupletDialog
//---------------------------------------------------------

TupletDialog::TupletDialog(QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      }

//---------------------------------------------------------
//   setupTuplet
//---------------------------------------------------------

void TupletDialog::setupTuplet(Tuplet* tuplet)
      {
      tuplet->setRatio(Fraction(actualNotes->value(), normalNotes->value()));
      if (number->isChecked())
            tuplet->setNumberType(Tuplet::SHOW_NUMBER);
      else if (relation->isChecked())
            tuplet->setNumberType(Tuplet::SHOW_RELATION);
      else if (noNumber->isChecked())
            tuplet->setNumberType(Tuplet::NO_TEXT);
      if (autoBracket->isChecked())
            tuplet->setBracketType(Tuplet::AUTO_BRACKET);
      else if (bracket->isChecked())
            tuplet->setBracketType(Tuplet::SHOW_BRACKET);
      else if (noBracket->isChecked())
            tuplet->setBracketType(Tuplet::SHOW_NO_BRACKET);
      }

//---------------------------------------------------------
//   tupletDialog
//    create tuplet dialog
//---------------------------------------------------------

void MuseScore::tupletDialog()
      {
      if (!cs)
            return;
      ChordRest* cr = cs->getSelectedChordRest();
      if (cr == 0)
            return;
      if (cr->durationType() < TDuration(TDuration::V_128TH)) {
            noteTooShortForTupletDialog();
            return;
            }
      TupletDialog td;
      if (!td.exec())
            return;

      Tuplet* tuplet = new Tuplet(cs);
      tuplet->setTrack(cr->track());
      tuplet->setTick(cr->tick());
      td.setupTuplet(tuplet);
      //      tuplet->setRatio(tuplet->ratio().reduced());
      Fraction f1(cr->duration());
      tuplet->setDuration(f1);
      Fraction f = f1 * tuplet->ratio();
      f.reduce();

      qDebug("len %s  ratio %s  base %s\n",
         qPrintable(f1.print()),
         qPrintable(tuplet->ratio().print()),
         qPrintable(f.print()));

      tuplet->setBaseLen(Fraction(1, f.denominator()));

      Measure* measure = cr->measure();
      tuplet->setParent(measure);

      cs->cmdCreateTuplet(cr, tuplet);
      }


