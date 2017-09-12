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

#ifndef __PLUGIN_MANAGER_H__
#define __PLUGIN_MANAGER_H__

#include "ui_pluginManager.h"
#include "preferences.h"

namespace Ms {

class Shortcut;

//---------------------------------------------------------
//   PluginManager
//---------------------------------------------------------

class PluginManager : public QDialog, public Ui::PluginManager {
      Q_OBJECT

      QMap<QString, Shortcut*> localShortcuts;
      bool shortcutsChanged;
      Preferences prefs;

      void readSettings();
      void loadList(bool forceRefresh);

      virtual void closeEvent(QCloseEvent*);
      virtual void accept();

   private slots:
      void definePluginShortcutClicked();
      void clearPluginShortcutClicked();
      void pluginListItemChanged(QListWidgetItem*, QListWidgetItem*);
      void pluginLoadToggled(QListWidgetItem*);
      void reloadPluginsClicked();

   signals:
      void closed(bool);

   public:
      PluginManager(QWidget* parent = 0);
      void writeSettings();
      void init();
      };


} // namespace Ms
#endif

