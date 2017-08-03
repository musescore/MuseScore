//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: masterpalette.cpp
//
//  Copyright (C) 2002-2016 Werner Schweer and others
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
#include "palettebox.h"
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
      item->setText(0, qApp->translate("Palette", sp->name().toUtf8().data()).replace("&&","&"));
      stack->addWidget(psa);
      treeWidget->addTopLevelItem(item);
      }

//---------------------------------------------------------
//   MasterPalette
//---------------------------------------------------------

MasterPalette::MasterPalette(QWidget* parent)
   : QWidget(parent, Qt::Dialog)
      {
      setObjectName("MasterPalette");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

      treeWidget->clear();

      addPalette(MuseScore::newClefsPalette(PaletteType::MASTER));
      keyEditor = new KeyEditor;

      keyItem = new QTreeWidgetItem();
      keyItem->setData(0, Qt::UserRole, stack->count());
      stack->addWidget(keyEditor);
      treeWidget->addTopLevelItem(keyItem);

      timeItem = new QTreeWidgetItem();
      timeItem->setData(0, Qt::UserRole, stack->count());
      timeDialog = new TimeDialog;
      stack->addWidget(timeDialog);
      treeWidget->addTopLevelItem(timeItem);

      addPalette(MuseScore::newBracketsPalette());
      addPalette(MuseScore::newAccidentalsPalette(PaletteType::MASTER));
      addPalette(MuseScore::newArticulationsPalette(PaletteType::MASTER));
      addPalette(MuseScore::newOrnamentsPalette());
      addPalette(MuseScore::newBreathPalette());
      addPalette(MuseScore::newGraceNotePalette(PaletteType::MASTER));
      addPalette(MuseScore::newNoteHeadsPalette());
      addPalette(MuseScore::newLinesPalette(PaletteType::MASTER));
      addPalette(MuseScore::newBarLinePalette(PaletteType::MASTER));
      addPalette(MuseScore::newArpeggioPalette());
      addPalette(MuseScore::newTremoloPalette());
      addPalette(MuseScore::newTextPalette());
      addPalette(MuseScore::newTempoPalette(PaletteType::MASTER));
      addPalette(MuseScore::newDynamicsPalette(PaletteType::MASTER));
      addPalette(MuseScore::newFingeringPalette());
      addPalette(MuseScore::newRepeatsPalette());
      addPalette(MuseScore::newFretboardDiagramPalette());
      addPalette(MuseScore::newBagpipeEmbellishmentPalette());
      addPalette(MuseScore::newBreaksPalette());
      addPalette(MuseScore::newFramePalette());
      addPalette(MuseScore::newBeamPalette(PaletteType::MASTER));

      symbolItem = new QTreeWidgetItem();
      symbolItem->setData(0, Qt::UserRole, -1);
      symbolItem->setText(0, QT_TRANSLATE_NOOP("MasterPalette", "Symbols"));
      treeWidget->addTopLevelItem(symbolItem);

      for (const QString& s : smuflRanges()->keys()) {
            QTreeWidgetItem* child = new QTreeWidgetItem(QStringList(s));
            child->setData(0, Qt::UserRole, stack->count());
            symbolItem->addChild(child);
            stack->addWidget(new SymbolDialog(s));
            }

      connect(treeWidget, &QTreeWidget::currentItemChanged, this, &MasterPalette::currentChanged);
      connect(treeWidget, &QTreeWidget::itemClicked, this, &MasterPalette::clicked);
      retranslate(true);

      MuseScore::restoreGeometry(this);
      }

//---------------------------------------------------------
//   retranslate
//---------------------------------------------------------

void MasterPalette::retranslate(bool firstTime)
      {
      keyItem->setText(0, qApp->translate("Palette", "Key Signatures"));
      timeItem->setText(0, qApp->translate("Palette", "Time Signatures"));
      symbolItem->setText(0, qApp->translate("MasterPalette", "Symbols"));
      if (!firstTime)
            retranslateUi(this);
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
      MuseScore::saveGeometry(this);
      if (timeDialog->dirty())
            timeDialog->save();
      if (keyEditor->dirty())
            keyEditor->save();
      PaletteBox* pb = mscore->getPaletteBox();
      pb->setKeyboardNavigation(false);
      emit closed(false);
      QWidget::closeEvent(ev);
      }

//---------------------------------------------------------
//   changeEvent
//---------------------------------------------------------

void MasterPalette::changeEvent(QEvent *event)
      {
      QWidget::changeEvent(event);
      if (event->type() == QEvent::LanguageChange)
            retranslate();
      }

//---------------------------------------------------------
//   keyPressEvent
//---------------------------------------------------------

void MasterPalette::keyPressEvent(QKeyEvent *event)
      {
      QTreeWidgetItem* currentItem = treeWidget->currentItem();
      int idx = treeWidget->indexOfTopLevelItem(currentItem);
      QString s = currentItem->text(0);
      if (event->key() == Qt::Key_Down) {
            if (idx >= 0 && idx < treeWidget->topLevelItemCount() - 1) {
                  selectItem(treeWidget->topLevelItem(idx+1)->text(0));
                  } 
            else if (idx == treeWidget->topLevelItemCount() - 1) {
                  if (!currentItem->isExpanded())
                        selectItem(treeWidget->topLevelItem(0)->text(0));
                  else
                        treeWidget->setCurrentItem(symbolItem->child(0));
                  }
            else {
                  int i;
                  for (i = 0; i < symbolItem->childCount(); i++) {
                        if (symbolItem->child(i) == currentItem)
                              break;
                        }
                  if (i < symbolItem->childCount()-1) {
                        treeWidget->setCurrentItem(symbolItem->child(i+1));
                        }
                  else {
                        selectItem(treeWidget->topLevelItem(0)->text(0));
                        }
                  }
            }
      else if (event->key() == Qt::Key_Up) {
            if (idx > 0)
                  selectItem(treeWidget->topLevelItem(idx-1)->text(0));
            else if (idx == 0)
                  selectItem(treeWidget->topLevelItem(treeWidget->topLevelItemCount() - 1)->text(0));
            else {
                  int i;
                  for (i = 0; i < symbolItem->childCount(); i++) {
                        if (symbolItem->child(i) == currentItem)
                              break;
                        }
                  if (i > 0) {
                        treeWidget->setCurrentItem(symbolItem->child(i-1));
                        }
                  else {
                        selectItem(treeWidget->topLevelItem(treeWidget->topLevelItemCount() - 1)->text(0));
                        }
                }
            }
      else if (event->key() == Qt::Key_Right) {
            QWidget* w = stack->currentWidget();
            QWidget* ch = w->childAt(1, 1);
            if (ch) {
                  Palette* chp = static_cast<Palette*>(ch);
                  chp->setSelected(0);
                  chp->update();
                  chp->setFocus();
                  }
            else {
                  if (stack->currentIndex() == 1) {
                        KeyEditor* k = static_cast<KeyEditor*>(w);
                        Palette* p = k->getPalette();
                        p->setSelected(0);
                        p->update();
                        p->setFocus();
                        }
                  else if (stack->currentIndex() == 2) {
                        TimeDialog* t = static_cast<TimeDialog*>(w);
                        Palette* p = t->getPalette();
                        p->setSelected(0);
                        p->update();
                        p->setFocus();
                        }
                  else if (stack->currentIndex() >= 24) {
                        SymbolDialog* s = static_cast<SymbolDialog*>(w);
                        Palette* p = s->getPalette();
                        p->setSelected(0);
                        p->update();
                        p->setFocus();
                        }
                  }
            return;
            }
      else if (event->key() == Qt::Key_Left) {
            QWidget* w = stack->currentWidget();
            QWidget* ch = w->childAt(1, 1);         
            if (ch) {
                  Palette* chp = static_cast<Palette*>(ch);
                  chp->setSelected(chp->size() - 1);
                  chp->update();
                  chp->setFocus();
                  }
            else {
                  if (stack->currentIndex() == 1) {
                        KeyEditor* k = static_cast<KeyEditor*>(w);
                        Palette* p = k->getPalette();
                        p->setSelected(p->size() - 1);
                        p->update();
                        p->setFocus();
                        }
                  else if (stack->currentIndex() == 2) {
                        TimeDialog* t = static_cast<TimeDialog*>(w);
                        Palette* p = t->getPalette();
                        p->setSelected(p->size() - 1);
                        p->update();
                        p->setFocus();
                        }
                  else if (stack->currentIndex() >= 24) {
                        SymbolDialog* s = static_cast<SymbolDialog*>(w);
                        Palette* p = s->getPalette();
                        p->setSelected(p->size() - 1);
                        p->update();
                        p->setFocus();
                        }
                  }
            return;
            }
      else if (event->key() == Qt::Key_Enter ||
               event->key() == Qt::Key_Return) {
          if (idx == 24)
                clicked(treeWidget->topLevelItem(idx), 0);
          }
      QWidget::keyPressEvent(event);
      }
}

