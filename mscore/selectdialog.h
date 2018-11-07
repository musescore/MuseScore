//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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

#ifndef __SELECTDIALOG_H__
#define __SELECTDIALOG_H__

#include "ui_selectdialog.h"

namespace Ms {

class Element;
struct ElementPattern;

//---------------------------------------------------------
//   SelectDialog
//---------------------------------------------------------

class SelectDialog : public QDialog, Ui::SelectDialog {
      Q_OBJECT
      const Element* e;

      virtual void hideEvent(QHideEvent*);
   public:
      SelectDialog(const Element* e, QWidget* parent);
      void setPattern(ElementPattern* p);
      bool doReplace() const       { return replace->isChecked();       }
      bool doAdd() const           { return add->isChecked();           }
      bool doSubtract() const      { return subtract->isChecked();      }
      bool doFromSelection() const { return fromSelection->isChecked(); }
      bool isInSelection() const   { return inSelection->isChecked();   }
      };


} // namespace Ms
#endif

