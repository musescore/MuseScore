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

#include "tupletproperties.h"
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

//---------------------------------------------------------
//   TupletProperties
//---------------------------------------------------------

TupletProperties::TupletProperties(Tuplet* t, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      tuplet = t;
      switch(tuplet->numberType()) {
            case Tuplet::SHOW_NUMBER:
                  number->setChecked(true);
                  break;
            case Tuplet::SHOW_RELATION:
                  relation->setChecked(true);
                  break;
            case Tuplet::NO_TEXT:
                  noNumber->setChecked(true);
                  break;
            }
      switch(tuplet->bracketType()) {
            case Tuplet::AUTO_BRACKET:
                  autoBracket->setChecked(true);
                  break;
            case Tuplet::SHOW_BRACKET:
                  bracket->setChecked(true);
                  break;
            case Tuplet::SHOW_NO_BRACKET:
                  noBracket->setChecked(true);
                  break;
            }
      }

//---------------------------------------------------------
//   numberType
//---------------------------------------------------------

int TupletProperties::numberType() const
      {
      if (number->isChecked())
            return Tuplet::SHOW_NUMBER;
      else if (relation->isChecked())
            return Tuplet::SHOW_RELATION;
      else /* if (noNumber->isChecked()) */
            return Tuplet::NO_TEXT;
      }

//---------------------------------------------------------
//   bracketType
//---------------------------------------------------------

int TupletProperties::bracketType() const
      {
      if (autoBracket->isChecked())
            return Tuplet::AUTO_BRACKET;
      else if (bracket->isChecked())
            return Tuplet::SHOW_BRACKET;
      else /* if (noBracket->isChecked()) */
            return Tuplet::SHOW_NO_BRACKET;
      }

