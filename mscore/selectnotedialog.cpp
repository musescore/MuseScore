//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: select.cpp -1   $
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
#include "selectnotedialog.h"
#include "libmscore/element.h"
#include "libmscore/system.h"
#include "libmscore/score.h"
#include "libmscore/chord.h"
#include "libmscore/sym.h"
#include "libmscore/segment.h"
#include "musescore.h"

namespace Ms {

//---------------------------------------------------------
//   SelectDialog
//---------------------------------------------------------

SelectNoteDialog::SelectNoteDialog(const Note* _n, QWidget* parent)
   : QDialog(parent)
      {
      setObjectName("SelectNoteDialog");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      n = _n;
      notehead->setText(NoteHead::group2userName(n->headGroup()));
      pitch->setText(n->tpcUserName());
      string->setText(QString::number(n->string()+1));
      type->setText(n->noteTypeUserName());
      duration->setText(n->chord()->durationUserName());
      name->setText(tpc2name(n->tpc(), NoteSpellingType::STANDARD, NoteCaseType::AUTO, false));
      inSelection->setEnabled(n->score()->selection().isRange());
      MuseScore::restoreGeometry(this);
      }

//---------------------------------------------------------
//   setPattern
//---------------------------------------------------------

void SelectNoteDialog::setPattern(NotePattern* p)
      {
      if (sameNotehead->isChecked())
            p->notehead = n->headGroup();
      if (samePitch->isChecked())
            p->pitch = n->pitch();
      if (sameString->isChecked())
            p->string = n->string();
      if (sameName->isChecked())
            p->tpc = n->tpc();
      if (sameType->isChecked())
            p->type = n->noteType();
      if (sameDuration->isChecked())
            p->duration = n->chord()->actualDurationType();

      if (sameStaff->isChecked()) {
            p->staffStart = n->staffIdx();
            p->staffEnd = n->staffIdx() + 1;
            }
      else if (inSelection->isChecked()) {
            p->staffStart = n->score()->selection().staffStart();
            p->staffEnd = n->score()->selection().staffEnd();
            }
      else {
            p->staffStart = -1;
            p->staffEnd = -1;
            }

      p->voice   = sameVoice->isChecked() ? n->voice() : -1;
      p->system  = 0;
      if (sameSystem->isChecked())
            p->system = n->chord()->segment()->system();
      }

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void SelectNoteDialog::hideEvent(QHideEvent* event)
      {
      MuseScore::saveGeometry(this);
      QWidget::hideEvent(event);
      }

}

