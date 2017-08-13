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

#ifndef __PALETTE_SHORTCUT_MANAGER_H__
#define __PALETTE_SHORTCUT_MANAGER_H__

#include "ui_paletteShortcutManager.h"
#include "preferences.h"
#include "palette.h"

namespace Ms {

class Shortcut;

//---------------------------------------------------------
//   PaletteShortcutManager
//---------------------------------------------------------

class PaletteShortcutManager : public QDialog, public Ui::PaletteShortcutManager {
      Q_OBJECT

      QMap<QString, Shortcut*> localShortcuts;
      bool shortcutsChanged;

      void readSettings();
      virtual void closeEvent(QCloseEvent*);
      virtual void accept();

   private slots:
      void definePaletteShortcutClicked();
      void clearPaletteShortcutClicked();
      void paletteCellListItemChanged(QListWidgetItem*, QListWidgetItem*);

   signals:
      void closed(bool);

   public:
      PaletteShortcutManager(QWidget* parent = 0);
      void writeSettings();
      void init();
      void init1();
      void setShortcut(PaletteCell* &cell);
      void clearShortcut(PaletteCell* c);
      void setIds();
      };


} // namespace Ms
#endif
