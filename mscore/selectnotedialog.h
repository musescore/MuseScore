//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: select.h 3779 2010-12-19 11:39:26Z wschweer $
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

#ifndef __SELECTNOTEDIALOG_H__
#define __SELECTNOTEDIALOG_H__

#include "ui_selectnotedialog.h"

namespace Ms {

struct NotePattern;
class Note;

//---------------------------------------------------------
//   SelectNoteDialog
//---------------------------------------------------------

class SelectNoteDialog : public QDialog, Ui::SelectNoteDialog {
      Q_OBJECT
      const Note* n;

      virtual void hideEvent(QHideEvent*);
   public:
      SelectNoteDialog(const Note* n, QWidget* parent);
      void setPattern(NotePattern* p);
      bool doReplace() const       { return replace->isChecked();       }
      bool doAdd() const           { return add->isChecked();           }
      bool doSubtract() const      { return subtract->isChecked();      }
      bool doFromSelection() const { return fromSelection->isChecked(); }
      bool isInSelection() const   { return inSelection->isChecked();   }
      void setSameStringVisible(bool v) { sameString->setVisible(v); string->setVisible(v); }
      };

} // namespace Ms
#endif

