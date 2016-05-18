//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "musescore.h"
#include "masterpalette.h"
#include "symboldialog.h"
#include "palette.h"
#include "keyedit.h"
#include "libmscore/score.h"
#include "libmscore/clef.h"
#include "libmscore/fingering.h"
#include "libmscore/spacer.h"
#include "libmscore/layoutbreak.h"
#include "libmscore/dynamic.h"
#include "libmscore/bracket.h"
#include "libmscore/arpeggio.h"
#include "libmscore/glissando.h"
#include "libmscore/repeat.h"
#include "libmscore/breath.h"
#include "libmscore/harmony.h"
#include "libmscore/text.h"
#include "libmscore/tempotext.h"
#include "libmscore/instrchange.h"
#include "libmscore/rehearsalmark.h"
#include "libmscore/stafftext.h"
#include "libmscore/note.h"
#include "libmscore/tremolo.h"
#include "libmscore/chordline.h"

#include "timedialog.h"

namespace Ms {

extern void populateIconPalette(Palette* p, const IconAction* a);
extern Palette* newKeySigPalette();
extern Palette* newBarLinePalette(bool);
extern Palette* newLinesPalette(bool);
extern Palette* newAccidentalsPalette();
extern QMap<QString, QStringList>* smuflRanges();

//---------------------------------------------------------
//   showMasterPalette
//---------------------------------------------------------

void MuseScore::showMasterPalette(const QString& s)
      {
      QAction* a = getAction("masterpalette");

      if (masterPalette == 0) {
            masterPalette = new MasterPalette(this);
            connect(masterPalette, SIGNAL(closed(bool)), a, SLOT(setChecked(bool)));
            mscore->stackUnder(masterPalette);
            }
      // when invoked via other actions, the main "masterpalette" action is not toggled automatically
      if (!s.isEmpty()) {
            // display if not already
            if (!a->isChecked())
                  a->setChecked(true);
            else {
                  // master palette is open; close only if command match current item
                  if (s == masterPalette->selectedItem())
                        a->setChecked(false);
                  // otherwise switch tabs
                  }
            }
      masterPalette->setVisible(a->isChecked());
      if (!s.isEmpty())
            masterPalette->selectItem(s);
      }

//---------------------------------------------------------
//   createPalette
//---------------------------------------------------------

Palette* MasterPalette::createPalette(int w, int h, bool grid, double mag)
      {
      Palette* sp = new Palette;
      PaletteScrollArea* psa = new PaletteScrollArea(sp);
      psa->setRestrictHeight(false);
      sp->setMag(mag);
      sp->setGrid(w, h);
      sp->setDrawGrid(grid);
      sp->setReadOnly(true);
      stack->addWidget(psa);
      return sp;
      }

//---------------------------------------------------------
//   selectItem
//---------------------------------------------------------

void MasterPalette::selectItem(const QString& s)
      {
      for (int idx = 0; idx < treeWidget->topLevelItemCount(); ++idx) {
            if (treeWidget->topLevelItem(idx)->text(0) == s) {
                  treeWidget->setCurrentItem(treeWidget->topLevelItem(idx));
                  break;
                  }
            }
      }

//---------------------------------------------------------
//   selectedItem
//---------------------------------------------------------

QString MasterPalette::selectedItem()
      {
      return treeWidget->currentItem()->text(0);
      }

//---------------------------------------------------------
//   addPalette
//---------------------------------------------------------

void MasterPalette::addPalette(Palette* sp)
      {
      sp->setReadOnly(true);
      PaletteScrollArea* psa = new PaletteScrollArea(sp);
      psa->setRestrictHeight(false);
      QTreeWidgetItem* item = new QTreeWidgetItem(QStringList(sp->name()));
      item->setData(0, Qt::UserRole, stack->count());
      stack->addWidget(psa);
      treeWidget->addTopLevelItem(item);
      }

//---------------------------------------------------------
//   MasterPalette
//---------------------------------------------------------

MasterPalette::MasterPalette(QWidget* parent)
   : QWidget(parent, Qt::Dialog)
      {
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

      treeWidget->clear();

      addPalette(MuseScore::newGraceNotePalette(false));
      addPalette(MuseScore::newClefsPalette(false));
      keyEditor = new KeyEditor;

      QTreeWidgetItem* item = new QTreeWidgetItem(QStringList(tr("Key Signatures")));
      item->setData(0, Qt::UserRole, stack->count());
      stack->addWidget(keyEditor);
      treeWidget->addTopLevelItem(item);

      item = new QTreeWidgetItem(QStringList(tr("Time Signatures")));
      item->setData(0, Qt::UserRole, stack->count());
      timeDialog = new TimeDialog;
      stack->addWidget(timeDialog);
      treeWidget->addTopLevelItem(item);

      addPalette(MuseScore::newBarLinePalette(false));
      addPalette(MuseScore::newLinesPalette(false));
      addPalette(MuseScore::newArpeggioPalette());
      addPalette(MuseScore::newBreathPalette());
      addPalette(MuseScore::newBracketsPalette());
      addPalette(MuseScore::newArticulationsPalette(false));

      addPalette(MuseScore::newAccidentalsPalette(false));

      addPalette(MuseScore::newDynamicsPalette(false, true));
      addPalette(MuseScore::newFingeringPalette());
      addPalette(MuseScore::newNoteHeadsPalette());
      addPalette(MuseScore::newTremoloPalette());
      addPalette(MuseScore::newRepeatsPalette());
      addPalette(MuseScore::newTempoPalette(false));
      addPalette(MuseScore::newTextPalette());
      addPalette(MuseScore::newBreaksPalette());
      addPalette(MuseScore::newBagpipeEmbellishmentPalette());
      addPalette(MuseScore::newBeamPalette(false));
      addPalette(MuseScore::newFramePalette());

      item = new QTreeWidgetItem(QStringList(tr("Symbols")));
      item->setData(0, Qt::UserRole, -1);
      treeWidget->addTopLevelItem(item);

      for (const QString& s : smuflRanges()->keys()) {
            QTreeWidgetItem* child = new QTreeWidgetItem(QStringList(s));
            child->setData(0, Qt::UserRole, stack->count());
            item->addChild(child);
            stack->addWidget(new SymbolDialog(s));
            }

      connect(treeWidget, &QTreeWidget::currentItemChanged, this, &MasterPalette::currentChanged);
      connect(treeWidget, &QTreeWidget::itemClicked, this, &MasterPalette::clicked);
      }

//---------------------------------------------------------
//   currentChanged
//---------------------------------------------------------

void MasterPalette::currentChanged(QTreeWidgetItem* item, QTreeWidgetItem*)
      {
      int idx = item->data(0, Qt::UserRole).toInt();
      if (idx != -1)
            stack->setCurrentIndex(idx);
      }

//---------------------------------------------------------
//   clicked
//---------------------------------------------------------

void MasterPalette::clicked(QTreeWidgetItem* item, int)
      {
      int idx = item->data(0, Qt::UserRole).toInt();
      if (idx == -1)
            item->setExpanded(!item->isExpanded());
      }

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void MasterPalette::closeEvent(QCloseEvent* ev)
      {
      if (timeDialog->dirty())
            timeDialog->save();
      if (keyEditor->dirty())
            keyEditor->save();
      emit closed(false);
      QWidget::closeEvent(ev);
      }

}

