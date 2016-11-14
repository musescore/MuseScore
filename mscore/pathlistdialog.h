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

#ifndef __PATHLISTDIALOG_H__
#define __PATHLISTDIALOG_H__

#include "ui_pathlistdialog.h"

namespace Ms {

//---------------------------------------------------------
//   PathListDialog
//---------------------------------------------------------

class PathListDialog : public QDialog, public Ui::PathListDialog {
      Q_OBJECT

      virtual void hideEvent(QHideEvent*);

   private slots:
      void addClicked();
      void removeClicked();

   public:
      PathListDialog(QWidget*);
      QString path();
      void setPath(QString s);
   };


} // namespace Ms
#endif

