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

/**
 \file
 Implementation of class Selection plus other selection related functions.
*/

#include "libmscore/select.h"
#include "selectdialog.h"
#include "libmscore/element.h"
#include "libmscore/system.h"
#include "libmscore/score.h"
#include "libmscore/slur.h"
#include "libmscore/articulation.h"
#include "musescore.h"
#include "libmscore/rest.h"

namespace Ms {

//---------------------------------------------------------
//   SelectDialog
//---------------------------------------------------------

SelectDialog::SelectDialog(const Element* _e, QWidget* parent)
   : QDialog(parent)
      {
      setObjectName("SelectDialog");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      e = _e;
      type->setText(qApp->translate("elementName", e->userName().toUtf8()));

      switch (e->type()) {
            case ElementType::ACCIDENTAL:
                  subtype->setText(qApp->translate("accidental", e->subtypeName().toUtf8()));
                  break;
            case ElementType::SLUR_SEGMENT:
                  subtype->setText(qApp->translate("elementName", e->subtypeName().toUtf8()));
                  break;
            case ElementType::FINGERING:
            case ElementType::STAFF_TEXT:
                  subtype->setText(qApp->translate("TextStyle", e->subtypeName().toUtf8()));
                  break;
            case ElementType::ARTICULATION: // comes translated, but from a different method
                  subtype->setText(toArticulation(e)->userName());
                  break;
            case ElementType::CLEF:
                  subtype->setText(qApp->translate("clefTable", e->subtypeName().toUtf8()));
                  break;
            // others come translated or don't need any or are too difficult to implement
            default: subtype->setText(e->subtypeName());
            }
      sameSubtype->setEnabled(e->subtype() != -1);
      subtype->setEnabled(e->subtype() != -1);
      inSelection->setEnabled(e->score()->selection().isRange());
      sameDuration->setEnabled(e->isRest());

      MuseScore::restoreGeometry(this);
      }

//---------------------------------------------------------
//   setPattern
//---------------------------------------------------------

void SelectDialog::setPattern(ElementPattern* p)
      {
      p->type    = int(e->type());
      p->subtype = e->subtype();
      if (e->isSlurSegment())
            p->subtype = int(toSlurSegment(e)->spanner()->type());

      if (sameStaff->isChecked()) {
            p->staffStart = e->staffIdx();
            p->staffEnd = e->staffIdx() + 1;
            }
      else if (inSelection->isChecked()) {
            p->staffStart = e->score()->selection().staffStart();
            p->staffEnd = e->score()->selection().staffEnd();
            }
      else {
            p->staffStart = -1;
            p->staffEnd = -1;
            }

      if (sameDuration->isChecked() && e->isRest()) {
            const Rest* r = toRest(e);
            p->durationTicks = r->actualTicks();
            }
      else
            p->durationTicks = Fraction(-1,1);

      if (sameBeat->isChecked())
            p->beat = e->beat();
      else
            p->beat = Fraction(0,0);

      if (sameMeasure->isChecked()) {
            auto m = e->findMeasure();
            if (!m && e->isSpannerSegment()) {
                  if (auto ss  = toSpannerSegment(e)) {
                  if (auto s   = ss->spanner())       {
                  if (auto se  = s->startElement())   {
                  if (auto mse = se->findMeasure())   {
                        m = mse;
                        }}}}
                  }
            p->measure = m;
            }
      else
            p->measure = nullptr;

      p->voice   = sameVoice->isChecked() ? e->voice() : -1;
      p->subtypeValid = sameSubtype->isChecked();
      if (sameSystem->isChecked()) {
            do {
                  if (e->type() == ElementType::SYSTEM) {
                        p->system = static_cast<const System*>(e);
                        break;
                        }
                  e = e->parent();
                  } while (e);
            }
      }

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void SelectDialog::hideEvent(QHideEvent* event)
      {
      MuseScore::saveGeometry(this);
      QWidget::hideEvent(event);
      }

}

