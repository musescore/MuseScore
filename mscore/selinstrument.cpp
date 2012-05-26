//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: editstaff.cpp 3629 2010-10-26 10:40:47Z wschweer $
//
//  Copyright (C) 2010 Werner Schweer and others
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

#include "selinstrument.h"
#include "instrdialog.h"

#include "libmscore/instrument.h"
#include "libmscore/instrtemplate.h"

//---------------------------------------------------------
//   SelectInstrument
//---------------------------------------------------------

SelectInstrument::SelectInstrument(const Instrument& instrument, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      currentInstrument->setText(instrument.trackName());
      buildTemplateList();
      buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
      connect(showMore, SIGNAL(clicked()), SLOT(buildTemplateList()));
      }

//---------------------------------------------------------
//   buildTemplateList
//---------------------------------------------------------

void SelectInstrument::buildTemplateList()
      {
      populateInstrumentList(instrumentList, showMore->isChecked());
      }

//---------------------------------------------------------
//   on_instrumentList_itemSelectionChanged
//---------------------------------------------------------

void SelectInstrument::on_instrumentList_itemSelectionChanged()
      {
      QList<QTreeWidgetItem*> wi = instrumentList->selectedItems();
      bool flag = !wi.isEmpty();
      buttonBox->button(QDialogButtonBox::Ok)->setEnabled(flag);
      }

//---------------------------------------------------------
//   on_instrumentList
//---------------------------------------------------------

void SelectInstrument::on_instrumentList_itemDoubleClicked(QTreeWidgetItem*, int)
      {
      QList<QTreeWidgetItem*> wi = instrumentList->selectedItems();
      if(!wi.isEmpty())
          done(true);
      }

//---------------------------------------------------------
//   instrTemplate
//---------------------------------------------------------

const InstrumentTemplate* SelectInstrument::instrTemplate() const
      {
      QList<QTreeWidgetItem*> wi = instrumentList->selectedItems();
      if (wi.isEmpty())
            return 0;
      InstrumentTemplateListItem* item = (InstrumentTemplateListItem*)wi.front();
      return item->instrumentTemplate();
      }

