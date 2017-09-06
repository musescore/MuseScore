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

//---------------------------------------------------------
//   PluginManager
//---------------------------------------------------------

PluginManager::PluginManager(QWidget* parent)
   : QDialog(parent)
      {
      setObjectName("PluginManager");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      connect(definePluginShortcut, SIGNAL(clicked()), SLOT(definePluginShortcutClicked()));
      connect(clearPluginShortcut, SIGNAL(clicked()), SLOT(clearPluginShortcutClicked()));
      connect(reloadPlugins, SIGNAL(clicked()), SLOT(reloadPluginsClicked()));
      readSettings();
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void PluginManager::init()
      {
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
      loadList(false);
      connect(pluginList, SIGNAL(itemChanged(QListWidgetItem*)), SLOT(pluginLoadToggled(QListWidgetItem*)));
      connect(pluginList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
         SLOT(pluginListItemChanged(QListWidgetItem*, QListWidgetItem*)));
      }

//---------------------------------------------------------
//   loadList - populate the listbox.
//---------------------------------------------------------

void PluginManager::loadList(bool forceRefresh)
      {
      QStringList saveLoaded; // If forcing a refresh, the load flags are lost. Keep a copy and reapply.
      int n = preferences.pluginList.size();
      if (forceRefresh && n > 0) {
            for (int i = 0; i < n; i++) {
                  PluginDescription& d = preferences.pluginList[i];
                  if (d.load) {
                        saveLoaded.append(d.path);
                        mscore->unregisterPlugin(&d);  // This will force the menu to rebuild.
                        }
                  }
            }
      preferences.updatePluginList(forceRefresh);
      n = preferences.pluginList.size();
      pluginList->clear();
      for (int i = 0; i < n; ++i) {
            PluginDescription& d = preferences.pluginList[i];
            Shortcut* s = &d.shortcut;
            localShortcuts[s->key()] = new Shortcut(*s);
            if (saveLoaded.contains(d.path)) d.load = true;
            QListWidgetItem* item = new QListWidgetItem(QFileInfo(d.path).completeBaseName(),  pluginList);
            item->setFlags(item->flags() | Qt::ItemIsEnabled);
            item->setCheckState(d.load ? Qt::Checked : Qt::Unchecked);
            item->setData(Qt::UserRole, i);
            }
      prefs = preferences;
      if (n) {
            pluginList->setCurrentRow(0);
            pluginListItemChanged(pluginList->item(0), 0);
            }
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
      int n = prefs.pluginList.size();
      for (int i = 0; i < n; ++i) {
            PluginDescription& d = prefs.pluginList[i];
            if (d.load)
                  mscore->registerPlugin(&d);
            else
                  mscore->unregisterPlugin(&d);
            }
      preferences = prefs;
      preferences.write();
      disconnect(pluginList, SIGNAL(itemChanged(QListWidgetItem*)));
      disconnect(pluginList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)));
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
      pluginName->setText(fi.completeBaseName());
      pluginPath->setText(fi.absolutePath());
      pluginVersion->setText(d.version);
      pluginShortcut->setText(d.shortcut.keysToString());
      pluginDescription->setText(d.description);
      }

//---------------------------------------------------------
//   pluginLoadToggled
//---------------------------------------------------------

void PluginManager::pluginLoadToggled(QListWidgetItem* item)
      {
      int idx = item->data(Qt::UserRole).toInt();
      PluginDescription* d = &prefs.pluginList[idx];
      d->load = (item->checkState() == Qt::Checked);
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
//   reloadPluginShortcutClicked
//---------------------------------------------------------

void PluginManager::reloadPluginsClicked()
      {
      loadList(true);
      QMessageBox::information(0,
            tr("MuseScore"),
            tr("Plugins reloaded."),
            QMessageBox::Ok, QMessageBox::NoButton);
      }

//---------------------------------------------------------
//   clearPluginShortcutClicked
//---------------------------------------------------------

void PluginManager::clearPluginShortcutClicked()
      {
      QListWidgetItem* item = pluginList->currentItem();
      if (!item)
            return;
      int idx = item->data(Qt::UserRole).toInt();
      PluginDescription* pd = &prefs.pluginList[idx];
      Shortcut* s = &pd->shortcut;
      s->clear();

      QAction* action = s->action();
      action->setShortcuts(s->keys());
//      mscore->addAction(action);

      pluginShortcut->setText(s->keysToString());
      prefs.dirty = true;
      }

//---------------------------------------------------------
//   writeSettings
//---------------------------------------------------------

void PluginManager::writeSettings()
      {
      MuseScore::saveGeometry(this);
      }

//---------------------------------------------------------
//   readSettings
//---------------------------------------------------------

void PluginManager::readSettings()
      {
      MuseScore::restoreGeometry(this);
      }

}

