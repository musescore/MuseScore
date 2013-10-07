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

#include "pluginManager.h"
#include "shortcutcapturedialog.h"
#include "musescore.h"

namespace Ms {

extern bool useFactorySettings;

//---------------------------------------------------------
//   PluginManager
//---------------------------------------------------------

PluginManager::PluginManager(QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      connect(definePluginShortcut, SIGNAL(clicked()), SLOT(definePluginShortcutClicked()));
      connect(pluginList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
         SLOT(pluginListItemChanged(QListWidgetItem*, QListWidgetItem*)));
      connect(pluginLoad, SIGNAL(toggled(bool)), SLOT(pluginLoadToggled(bool)));

      prefs = preferences;
      //
      // initialize local shortcut table
      //    we need a deep copy to be able to rewind all
      //    changes on "Abort"
      //
      qDeleteAll(localShortcuts);
      localShortcuts.clear();
      foreach(const Shortcut* s, Shortcut::shortcuts())
            localShortcuts[s->key()] = new Shortcut(*s);
      shortcutsChanged = false;

      prefs.updatePluginList();
      int n = prefs.pluginList.size();
      for (int i = 0; i < n; ++i) {
            const PluginDescription& d = prefs.pluginList[i];
            QListWidgetItem* item = new QListWidgetItem(QFileInfo(d.path).baseName(),  pluginList);
            item->setData(Qt::UserRole, i);
            }
      if (n) {
            pluginList->setCurrentRow(0);
            pluginListItemChanged(pluginList->item(0), 0);
            }
      readSettings();
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void PluginManager::accept()
      {
      if (shortcutsChanged) {
            shortcutsChanged = false;
            foreach(const Shortcut* s, localShortcuts) {
                  Shortcut* os = Shortcut::getShortcut(s->key());
                  if (os) {
                        if (!os->compareKeys(*s))
                              os->setKeys(s->keys());
                        }
                  }
            Shortcut::dirty = true;
            }
      preferences = prefs;
      preferences.write();
      QDialog::accept();
      }

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void PluginManager::closeEvent(QCloseEvent* ev)
      {
      emit closed(false);
      QWidget::closeEvent(ev);
      }

//---------------------------------------------------------
//   pluginListItemChanged
//---------------------------------------------------------

void PluginManager::pluginListItemChanged(QListWidgetItem* item, QListWidgetItem*)
      {
      if (!item)
            return;
      int idx = item->data(Qt::UserRole).toInt();
      const PluginDescription& d = prefs.pluginList[idx];
      QFileInfo fi(d.path);
      pluginName->setText(fi.baseName());
      pluginPath->setText(fi.absolutePath());
      pluginLoad->setChecked(d.load);
      pluginVersion->setText(d.version);
      pluginShortcut->setText(d.shortcut.keysToString());
      pluginDescription->setText(d.description);
      }

//---------------------------------------------------------
//   pluginLoadToggled
//---------------------------------------------------------

void PluginManager::pluginLoadToggled(bool val)
      {
      QListWidgetItem* item = pluginList->currentItem();
      if (!item)
            return;
      int idx = item->data(Qt::UserRole).toInt();
      PluginDescription* d = &prefs.pluginList[idx];
      d->load = val;
      prefs.dirty = true;
      }

//---------------------------------------------------------
//   definePluginShortcutClicked
//---------------------------------------------------------

void PluginManager::definePluginShortcutClicked()
      {
      QListWidgetItem* item = pluginList->currentItem();
      if (!item)
            return;
      int idx = item->data(Qt::UserRole).toInt();
      PluginDescription* pd = &prefs.pluginList[idx];
      Shortcut* s = &pd->shortcut;
      ShortcutCaptureDialog sc(s, localShortcuts, this);
      int rv = sc.exec();
      if (rv == 0)            // abort
            return;
      if (rv == 2)            // replace
            s->clear();

      s->addShortcut(sc.getKey());
      QAction* action = s->action();
      action->setShortcuts(s->keys());
      mscore->addAction(action);

      pluginShortcut->setText(s->keysToString());
      prefs.dirty = true;
      }

//---------------------------------------------------------
//   writeSettings
//---------------------------------------------------------

void PluginManager::writeSettings()
      {
      QSettings settings;
      settings.beginGroup("PluginManager");
      settings.setValue("geometry", saveGeometry());
      settings.endGroup();
      }

//---------------------------------------------------------
//   readSettings
//---------------------------------------------------------

void PluginManager::readSettings()
      {
      if (!useFactorySettings) {
            QSettings settings;
            settings.beginGroup("PluginManager");
            restoreGeometry(settings.value("geometry").toByteArray());
            settings.endGroup();
            }
      }

}

