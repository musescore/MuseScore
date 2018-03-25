//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: metaedit.h 5290 2012-02-07 16:27:27Z wschweer $
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

#ifndef __METAEDIT_H__
#define __METAEDIT_H__

#include "ui_metaedit.h"

namespace Ms {

class Score;

//---------------------------------------------------------
//   MetaEditDialog
//---------------------------------------------------------

class MetaEditDialog : public QDialog, public Ui::MetaEditDialog {
      Q_OBJECT

      Score* m_score;

      bool m_dirty;

      virtual void hideEvent(QHideEvent*) override;

      bool isSystemTag(QString tag) const;
//      bool isTransformedSystemTag(QString tag) const;

      bool save();

private slots:
      void newClicked();
      void setDirty(bool dirty = true);
      void saveClicked() { save(); } // slot for save function(which has a return value).

   public slots:
      virtual void accept();

   public:
      MetaEditDialog(Score*, QWidget *parent = nullptr);
      };


} // namespace Ms
#endif

