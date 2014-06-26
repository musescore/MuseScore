//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: boxproperties.cpp 5149 2011-12-29 08:38:43Z wschweer $
//
//  Copyright (C) 2009 Werner Schweer and others
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

#include "boxproperties.h"
#include "libmscore/box.h"
#include "libmscore/score.h"
#include "libmscore/undo.h"

namespace Ms {

//---------------------------------------------------------
//   BoxProperties
//---------------------------------------------------------

BoxProperties::BoxProperties(Box* b, QWidget* parent)
   : QDialog(parent)
      {
      _box = b;
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

      qreal _spatium = b->spatium();

      if (b->type() == Element::Type::HBOX) {     // enable width and set it to box width
            frameWidth->setEnabled(true);
            frameWidth->setValue(b->boxWidth().val());
            }
      if(b->type() == Element::Type::VBOX) {     // enable height and set it to box height
            frameHeight->setEnabled(true);
            frameHeight->setValue(b->boxHeight().val());
            }
      topGap->setValue(b->topGap() / _spatium);
      bottomGap->setValue(b->bottomGap() / _spatium);

      leftMargin->setValue(b->leftMargin());
      rightMargin->setValue(b->rightMargin());
      topMargin->setValue(b->topMargin());
      bottomMargin->setValue(b->bottomMargin());
      connect(dialogButtonBox, SIGNAL(accepted()), SLOT(ok()));
      }

//---------------------------------------------------------
//   ok
//---------------------------------------------------------

void BoxProperties::ok()
      {
      Element::Type type = _box->type();
      // scan selection and update each element of the same type of this one
      qreal _spatium = _box->spatium();
      foreach (Element* elem, _box->score()->selection().elements()) {
            if (elem->type() == type) {  // if current element matches type of this box, push new box props in undo stack
                  Box* box = static_cast<Box*>(elem);
                  box->score()->undo(new ChangeBoxProperties(box, leftMargin->value(), topMargin->value(),
                        rightMargin->value(), bottomMargin->value(),
                        Spatium(frameHeight->value()), Spatium(frameWidth->value()),
                        topGap->value() * _spatium, bottomGap->value() * _spatium));
                  }
            }
      }
}



