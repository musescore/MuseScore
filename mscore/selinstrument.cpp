//=============================================================================
//  MuseScore
//  Linux Music Score Editor
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
#include "musescore.h"

#include "libmscore/instrument.h"
#include "libmscore/instrtemplate.h"

namespace Ms {

extern void filterInstruments(QTreeWidget *instrumentList, const QString &searchPhrase = QString(""));

//---------------------------------------------------------
//   SelectInstrument
//---------------------------------------------------------

SelectInstrument::SelectInstrument(const Instrument* instrument, QWidget* parent)
   : QDialog(parent)
      {
      setObjectName("SelectInstrument");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      currentInstrument->setText(instrument->trackName());
      buildTemplateList();
      buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
      instrumentSearch->setFilterableView(instrumentList);

      // get last saved, user-selected instrument genre and set filter to it
      QSettings settings;
      settings.beginGroup("selectInstrument");
      if (!settings.value("selectedGenre").isNull()){
            QString selectedGenre = settings.value("selectedGenre").value<QString>();
            instrumentGenreFilter->setCurrentText(selectedGenre);
            }
      settings.endGroup();

      MuseScore::restoreGeometry(this);
      }

//---------------------------------------------------------
//   buildTemplateList
//---------------------------------------------------------

void SelectInstrument::buildTemplateList()
      {
      // clear search if instrument list is updated
      instrumentSearch->clear();

      populateInstrumentList(instrumentList);
      populateGenreCombo(instrumentGenreFilter);
      }

//---------------------------------------------------------
//   expandOrCollapse
//---------------------------------------------------------

void SelectInstrument::expandOrCollapse(const QModelIndex &model)
      {
      if(instrumentList->isExpanded(model))
            instrumentList->collapse(model);
      else
            instrumentList->expand(model);
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

//---------------------------------------------------------
//   on_search_textChanged
//---------------------------------------------------------

void SelectInstrument::on_search_textChanged(const QString&)
      {
      // searching is done in Ms::SearchBox so here we just reset the
      // genre dropdown to ensure that the search includes all genres
      const int idxAllGenres = 0;
      if (instrumentGenreFilter->currentIndex() != idxAllGenres) {
            instrumentGenreFilter->blockSignals(true);
            instrumentGenreFilter->setCurrentIndex(idxAllGenres);
            instrumentGenreFilter->blockSignals(false);
            }
      }


//---------------------------------------------------------
//   on_instrumentGenreFilter_currentTextChanged
//---------------------------------------------------------

void SelectInstrument::on_instrumentGenreFilter_currentIndexChanged(int index)
      {
      QSettings settings;
      settings.beginGroup("selectInstrument");  // hard coded, since this is also used in instrwidget
      settings.setValue("selectedGenre", instrumentGenreFilter->currentText());
      settings.endGroup();

      QString id = instrumentGenreFilter->itemData(index).toString();

      // Redisplay tree, only showing items from the selected genre
      filterInstrumentsByGenre(instrumentList, id);
      }


//---------------------------------------------------------
//   filterInstrumentsByGenre
//---------------------------------------------------------

void SelectInstrument::filterInstrumentsByGenre(QTreeWidget *instrList, QString genre)
      {
      QTreeWidgetItemIterator iList(instrList);
      while (*iList) {
            (*iList)->setHidden(true);
            InstrumentTemplateListItem* itli = static_cast<InstrumentTemplateListItem*>(*iList);
            InstrumentTemplate *it=itli->instrumentTemplate();

            if(it) {
                  if (genre == "all" || it->genreMember(genre)) {
                        (*iList)->setHidden(false);

                        QTreeWidgetItem *iParent = (*iList)->parent();
                        while(iParent) {
                              if(!iParent->isHidden())
                                    break;

                              iParent->setHidden(false);
                              iParent = iParent->parent();
                              }
                        }
                  }
            ++iList;
            }
      }

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void SelectInstrument::hideEvent(QHideEvent* event)
      {
      MuseScore::saveGeometry(this);
      QWidget::hideEvent(event);
      }

}

