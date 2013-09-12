//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: editstaff.h 3629 2010-10-26 10:40:47Z wschweer $
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

#ifndef __SELINSTRUMENT_H__
#define __SELINSTRUMENT_H__

#include "ui_selectinstr.h"

namespace Ms {

class Instrument;
class InstrumentTemplate;

//---------------------------------------------------------
//   SelectInstrument
//---------------------------------------------------------

class SelectInstrument : public QDialog, private Ui::SelectInstrument {
      Q_OBJECT

   private slots:
      void buildTemplateList();
      void expandOrCollapse(const QModelIndex &);
      void on_instrumentList_itemSelectionChanged();
      void on_instrumentList_itemDoubleClicked(QTreeWidgetItem* item, int);

      void on_search_textChanged(const QString &searchPhrase);
      void on_clearSearch_clicked();

      void on_instrumentGenreFilter_currentIndexChanged(int);
      void filterInstrumentsByGenre(QTreeWidget *, QString);

   public:
      SelectInstrument(const Instrument&, QWidget* parent = 0);
      const InstrumentTemplate* instrTemplate() const;
      };


} // namespace Ms
#endif

