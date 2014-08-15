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
#include "selectdialog.h"
#include "libmscore/element.h"
#include "libmscore/system.h"
#include "libmscore/score.h"

namespace Ms {

//---------------------------------------------------------
//   SelectDialog
//---------------------------------------------------------

SelectDialog::SelectDialog(const Element* _e, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      e = _e;
      type->setText(e->name());

      subtype->setText(e->subtypeName());
      sameSubtype->setEnabled(e->subtype() != -1);
      subtype->setEnabled(e->subtype() != -1);
      inSelection->setEnabled(e->score()->selection().isRange());    
      }

//---------------------------------------------------------
//   setPattern
//---------------------------------------------------------

void SelectDialog::setPattern(ElementPattern* p)
      {
      p->type    = int(e->type());
      p->subtype = int(e->subtype());

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

      p->voice   = sameVoice->isChecked() ? e->voice() : -1;
      p->subtypeValid = sameSubtype->isChecked();
      p->system  = 0;
      if (sameSystem->isChecked()) {
            do {
                  if (e->type() == Element::Type::SYSTEM) {
                        p->system = static_cast<const System*>(e);
                        break;
                        }
                  e = e->parent();
                  } while (e);
            }
      }
}

