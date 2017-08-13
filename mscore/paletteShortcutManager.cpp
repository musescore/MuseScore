//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: palette.cpp 5576 2012-04-24 19:15:22Z wschweer $
//
//  Copyright (C) 2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "paletteShortcutManager.h"
#include "shortcutcapturedialog.h"
#include "musescore.h"
#include "palette.h"
#include "palettebox.h"
#include "libmscore/arpeggio.h"
#include "libmscore/bagpembell.h"
#include "libmscore/glissando.h"
#include "libmscore/pedal.h"
#include "libmscore/tempotext.h"

namespace Ms {

//---------------------------------------------------------
//   PaletteShortcutManager
//---------------------------------------------------------

PaletteShortcutManager::PaletteShortcutManager(QWidget *parent) : QDialog(parent)
      {
      setObjectName("PaletteShortcutManager");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      connect(definePaletteShortcut, SIGNAL(clicked()), SLOT(definePaletteShortcutClicked()));
      connect(clearPaletteShortcut, SIGNAL(clicked()), SLOT(clearPaletteShortcutClicked()));
      readSettings();
      }

//---------------------------------------------------------
//   setIds
//---------------------------------------------------------

void PaletteShortcutManager::setIds()
      {
      int i = 0;
      for (Palette* p : mscore->getPaletteBox()->palettes()) {
            for (PaletteCell* c : p->getCells()) {
                  if (c->name == "Show More") {
                        continue;
                        }
                  c->id = i;
                  PaletteCellDescription pd = preferences.paletteCellList[i];
                  preferences.paletteCellList[i].cell = c;
                  i++;
                  }
            }
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void PaletteShortcutManager::init()
      {
      localShortcuts.clear();
      for (const Shortcut* s : Shortcut::shortcuts())
            localShortcuts[s->key()] = new Shortcut(*s);
      shortcutsChanged = false;

      int n = preferences.paletteCellList.size();
      paletteCellList->clear();
      paletteCellList->setSortingEnabled(false);
      for (int i = 0; i < n; ++i) {
            PaletteCellDescription& d = preferences.paletteCellList[i];
            if (d.cell->name == "Show More") {
                  continue;
                  }
            Shortcut* s = &d.shortcut;
            if (s->keys().isEmpty())
                  localShortcuts[s->key()] = new Shortcut(*s);
            else
                  localShortcuts[s->key()] = s;

            QString itemName = d.cell->parent->name().replace("&&", "&") + ": " + d.cell->name;
            QListWidgetItem* item = new QListWidgetItem(itemName,  paletteCellList);
            item->setFlags(item->flags() | Qt::ItemIsEnabled);
            item->setData(Qt::UserRole, i);
            }

      if (n) {
            paletteCellList->setCurrentRow(0);
            paletteCellListItemChanged(paletteCellList->item(0), 0);
            }

      connect(paletteCellList, SIGNAL(itemChanged(QListWidgetItem*)), SLOT(paletteCellLoadToggled(QListWidgetItem*)));
      connect(paletteCellList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
          SLOT(paletteCellListItemChanged(QListWidgetItem*, QListWidgetItem*)));
      }

//---------------------------------------------------------
//   init1
//---------------------------------------------------------

void PaletteShortcutManager::init1()
      {
      localShortcuts.clear();
      for (const Shortcut* s : Shortcut::shortcuts())
            localShortcuts[s->key()] = new Shortcut(*s);
      shortcutsChanged = false;
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void PaletteShortcutManager::accept()
      {
      if (shortcutsChanged) {
            shortcutsChanged = false;
            for (const Shortcut* s : localShortcuts) {
                  Shortcut* os = Shortcut::getShortcut(s->key());
                  if (os) {
                        if (!os->compareKeys(*s))
                              os->setKeys(s->keys());
                        }
                  }
            Shortcut::dirty = true;
            }
      preferences.write();
      disconnect(paletteCellList, SIGNAL(itemChanged(QListWidgetItem*)));
      disconnect(paletteCellList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)));
      QDialog::accept();
      }

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void PaletteShortcutManager::closeEvent(QCloseEvent* ev)
      {
      emit closed(false);
      QWidget::closeEvent(ev);
      }

//---------------------------------------------------------
//   paletteCellListItemChanged
//---------------------------------------------------------

void PaletteShortcutManager::paletteCellListItemChanged(QListWidgetItem* item, QListWidgetItem*)
      {
      if (!item)
            return;
      int idx = item->data(Qt::UserRole).toInt();
      const PaletteCellDescription& d = preferences.paletteCellList[idx];
      paletteCellName->setText(d.cell->name);
      paletteShortcut->setText(d.shortcut.keysToString());
      paletteCellDescription->setText(d.description);
      }

//---------------------------------------------------------
//   definePaletteShortcutClicked
//---------------------------------------------------------

void PaletteShortcutManager::definePaletteShortcutClicked()
      {
      QListWidgetItem* item = paletteCellList->currentItem();
      if (!item)
            return;
      for (Shortcut* s : Shortcut::shortcuts()) {
            if (s->keys().isEmpty())
                  localShortcuts[s->key()] = new Shortcut(*s);
            else
                  localShortcuts[s->key()] = s;
            }
      int idx = item->data(Qt::UserRole).toInt();
      PaletteCellDescription* pd = &preferences.paletteCellList[idx];
      Shortcut* s = &pd->shortcut;
      s->setDescr(pd->description);
      s->setState(STATE_NORMAL | STATE_NOTE_ENTRY);
      ShortcutCaptureDialog sc(s, localShortcuts, this);
      int rv = sc.exec();
      if (rv == 0)            // abort
            return;
      if (rv == 2)            // replace
            s->clear();

      s->addShortcut(sc.getKey());
      pd->shortcut = *s;
      QAction* action = s->action();
      action->setShortcuts(s->keys());
      mscore->addAction(action);
      preferences.paletteCellList[idx] = *pd;
      mscore->registerPaletteShortcut(pd);
      bool found = false;
      for (Palette* p : mscore->getPaletteBox()->palettes()) {
            for (PaletteCell* c : p->getCells()) {
                  if (c->id == pd->cell->id) {
                        c->shortcut = pd->shortcut;
                        found = true;
                        break;
                        }
                  }
            if (found)
                  break;
            }
      paletteShortcut->setText(s->keysToString());
      preferences.dirty = true;
      }

//---------------------------------------------------------
//   setShortcut
//---------------------------------------------------------

void PaletteShortcutManager::setShortcut(PaletteCell* &cell)
      {
      if (!cell)
             return;
      setIds();
      cell->shortcut.setState(STATE_NORMAL | STATE_NOTE_ENTRY);
      ShortcutCaptureDialog sc(&cell->shortcut, localShortcuts, this);
      int rv = sc.exec();
      if (rv == 0)            // abort
            return;
      if (rv == 2)            // replace
            cell->shortcut.clear();

      cell->shortcut.addShortcut(sc.getKey());
      QAction* action = cell->shortcut.action();
      action->setShortcuts(cell->shortcut.keys());
      mscore->addAction(action);
      PaletteCellDescription d;
      d.cell = cell;
      d.description = cell->name;
      d.shortcut = cell->shortcut;
      d.shortcut.setDescr(cell->name);
      d.shortcut.setState(STATE_NORMAL | STATE_NOTE_ENTRY);
      mscore->registerPaletteShortcut(&d);

      PaletteCellDescription* pd = 0;
      if (cell->id != -1) {
            pd = &preferences.paletteCellList[cell->id];
            Q_ASSERT(pd->cell->id == cell->id);
            }
      if (pd) {
            pd->shortcut = cell->shortcut;
            pd->shortcut.setDescr(cell->name);
            preferences.paletteCellList[cell->id] = *pd;
            }
      paletteShortcut->setText(cell->shortcut.keysToString());
      preferences.dirty = true;
      }

//---------------------------------------------------------
//   clearShortcut
//---------------------------------------------------------

void PaletteShortcutManager::clearShortcut(PaletteCell* cell)
      {
      PaletteCellDescription* pd = 0;
      if (cell->id != -1) {
            pd = &preferences.paletteCellList[cell->id];
            Q_ASSERT(pd->cell->id == cell->id);
            }
      if (pd) {
            pd->shortcut.clear();
            mscore->unregisterPaletteShortcut(pd);
            preferences.paletteCellList[cell->id] = *pd;
            paletteShortcut->setText(pd->shortcut.keysToString());
            }
      else {
            PaletteCellDescription d;
            d.cell = cell;
            d.description = cell->name;
            d.shortcut = cell->shortcut;
            d.shortcut.setDescr(cell->name);
            d.shortcut.setState(STATE_NORMAL | STATE_NOTE_ENTRY);
            mscore->unregisterPaletteShortcut(&d);
            }
      }

//---------------------------------------------------------
//   clearPaletteShortcutClicked
//---------------------------------------------------------

void PaletteShortcutManager::clearPaletteShortcutClicked()
      {
      setIds();
      QListWidgetItem* item = paletteCellList->currentItem();
      if (!item)
            return;
      int idx = item->data(Qt::UserRole).toInt();
      PaletteCellDescription* pd = &preferences.paletteCellList[idx];
      Shortcut* s = &pd->shortcut;
      s->clear();
      QAction* action = s->action();
      action->setShortcuts(s->keys());

      for (Palette* p : mscore->getPaletteBox()->palettes()) {
            for (PaletteCell* c : p->getCells()) {
                  if (c->id == pd->cell->id) {
                        c->shortcut = pd->shortcut;
                        break;
                        }
                  }
            }
      paletteShortcut->setText(s->keysToString());
      preferences.dirty = true;
      }

//---------------------------------------------------------
//   writeSettings
//---------------------------------------------------------

void PaletteShortcutManager::writeSettings()
      {
      MuseScore::saveGeometry(this);
      }

//---------------------------------------------------------
//   readSettings
//---------------------------------------------------------

void PaletteShortcutManager::readSettings()
      {
      MuseScore::restoreGeometry(this);
      }

}
