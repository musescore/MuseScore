//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "pluginManager.h"
#include "qmlpluginengine.h"
#include "shortcutcapturedialog.h"
#include "musescore.h"
#include "libmscore/xml.h"
#include "preferences.h"


namespace Ms {

static const QByteArray pluginShortcutActionName = "plugin-run";

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
}

//---------------------------------------------------------
//   readPluginList
//---------------------------------------------------------

bool PluginManager::readPluginList()
      {
      QFile f(dataPath + "/plugins.xml");
      if (!f.exists())
            return false;
      if (!f.open(QIODevice::ReadOnly)) {
            qDebug("Cannot open plugins file <%s>", qPrintable(f.fileName()));
            return false;
            }
      XmlReader e(&f);
      while (e.readNextStartElement()) {
            if (e.name() == "museScore") {
                  while (e.readNextStartElement()) {
                        const QStringRef& tag(e.name());
                        if (tag == "Plugin") {
                              PluginDescription d;
                              while (e.readNextStartElement()) {
                                    const QStringRef& t(e.name());
                                    if (t == "path")
                                          d.path = e.readElementText();
                                    else if (t == "load")
                                          d.load = e.readInt();
                                    else if (t == "SC")
                                          d.shortcut.read(e);
                                    else if (t == "version")
                                          d.version = e.readElementText();
                                    else if (t == "description")
                                          d.description = e.readElementText();
                                    else
                                          e.unknown();
                                    }
                              d.shortcut.setState(STATE_NORMAL | STATE_NOTE_ENTRY | STATE_EDIT |
                                          STATE_ALLTEXTUAL_EDIT | STATE_PLAY | STATE_FOTO | STATE_LOCK );
                              d.shortcut.setKey(pluginShortcutActionName);
                              if (d.path.endsWith(".qml"))
                                    _pluginList.append(d);
                              }
                        else
                              e.unknown();
                        }
                  }
            else
                  e.unknown();
            }
      return true;
}

//---------------------------------------------------------
//   writePluginList
//---------------------------------------------------------

void PluginManager::writePluginList()
      {
      QDir dir;
      dir.mkpath(dataPath);
      QFile f(dataPath + "/plugins.xml");
      if (!f.open(QIODevice::WriteOnly)) {
            qDebug("cannot create plugin file <%s>", qPrintable(f.fileName()));
            return;
            }
      XmlWriter xml(0, &f);
      xml.header();
      xml.stag("museScore version=\"" MSC_VERSION "\"");
      foreach(const PluginDescription& d, _pluginList) {
            xml.stag("Plugin");
            xml.tag("path", d.path);
            xml.tag("load", d.load);
            xml.tag("version", d.version);
            xml.tag("description", d.description);
            if (!d.shortcut.keys().isEmpty())
                  d.shortcut.write(xml);
            xml.etag();
            }
      xml.etag();
      f.close();
}

//---------------------------------------------------------
//   updatePluginList
//    scan plugin folders for new plugins and update
//    pluginList
//---------------------------------------------------------

#ifdef SCRIPT_INTERFACE
static void updatePluginList(QList<QString>& pluginPathList, const QString& pluginPath,
   QList<PluginDescription>& pluginList)
      {
      QDirIterator it(pluginPath, QDir::NoDot|QDir::NoDotDot|QDir::Dirs|QDir::Files,
         QDirIterator::Subdirectories);
      while (it.hasNext()) {
            it.next();
            QFileInfo fi = it.fileInfo();
            QString path(fi.absoluteFilePath());
            if (fi.isFile()) {
                  if (path.endsWith(".qml")) {
                        bool alreadyInList = false;
                        foreach (const PluginDescription& p, pluginList) {
                              if (p.path == path) {
                                    alreadyInList = true;
                                    break;
                                    }
                              }
                        if (!alreadyInList) {
                              PluginDescription p;
                              p.path = path;
                              p.load = false;
                              p.shortcut.setKey(pluginShortcutActionName);
                              if (collectPluginMetaInformation(&p))
                                    pluginList.append(p);
                              }
                        }
                  }
            else
                  updatePluginList(pluginPathList, path, pluginList);
            }
      }
#endif

void PluginManager::updatePluginList(bool forceRefresh)
      {
#ifdef SCRIPT_INTERFACE
      QList<QString> pluginPathList;
      pluginPathList.append(dataPath + "/plugins");
      pluginPathList.append(mscoreGlobalShare + "plugins");
      QString p = preferences.getString(PREF_APP_PATHS_MYPLUGINS);
      if (!p.isEmpty())
            pluginPathList.append(p);
      if (forceRefresh) {
            _pluginList.clear();
            QmlPluginEngine* engine = mscore->getPluginEngine();
            engine->clearComponentCache(); //TODO: Check this doesn't have unwanted side effects.
            }

      for (QString _pluginPath : pluginPathList) {
            Ms::updatePluginList(pluginPathList, _pluginPath, _pluginList);
            }
      //remove non existing files
      auto i = _pluginList.begin();
      while (i != _pluginList.end()) {
            PluginDescription d = *i;
            QFileInfo fi(d.path);
            if (!fi.exists())
                  i = _pluginList.erase(i);
            else
                  ++i;
            }
#endif
      }

//---------------------------------------------------------
//   loadList - populate the listbox.
//---------------------------------------------------------

void PluginManager::loadList(bool forceRefresh)
      {
      QStringList saveLoaded; // If forcing a refresh, the load flags are lost. Keep a copy and reapply.
      int n = _pluginList.size();
      if (forceRefresh && n > 0) {
            for (int i = 0; i < n; i++) {
                  PluginDescription& d = _pluginList[i];
                  if (d.load) {
                        saveLoaded.append(d.path);
                        mscore->unregisterPlugin(&d);  // This will force the menu to rebuild.
                        }
                  }
            }
      updatePluginList(forceRefresh);
      n = _pluginList.size();
      pluginListWidget->clear();
      disconnect(pluginListWidget, nullptr, nullptr, nullptr);
      for (int i = 0; i < n; ++i) {
            PluginDescription& d = _pluginList[i];
            Shortcut* s = &d.shortcut;
            localShortcuts[s->key()] = new Shortcut(*s);
            if (saveLoaded.contains(d.path)) d.load = true;
            QListWidgetItem* item = new QListWidgetItem(QFileInfo(d.path).completeBaseName(),  pluginListWidget);
            item->setFlags(item->flags() | Qt::ItemIsEnabled);
            item->setCheckState(d.load ? Qt::Checked : Qt::Unchecked);
            item->setData(Qt::UserRole, i);
            }
      connect(pluginListWidget, SIGNAL(itemChanged(QListWidgetItem*)), SLOT(pluginLoadToggled(QListWidgetItem*)));
      connect(pluginListWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
              SLOT(pluginListWidgetItemChanged(QListWidgetItem*,QListWidgetItem*)));
      if (n) {
            pluginListWidget->setCurrentRow(0);
            pluginListWidgetItemChanged(pluginListWidget->item(0), 0);
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
      int n = _pluginList.size();
      for (int i = 0; i < n; ++i) {
            PluginDescription& d = _pluginList[i];
            if (d.load)
                  mscore->registerPlugin(&d);
            else
                  mscore->unregisterPlugin(&d);
            }

      writePluginList();
      if (Shortcut::dirty)
            Shortcut::save();
      Shortcut::dirty = false;

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
//   pluginListWidgetItemChanged
//---------------------------------------------------------

void PluginManager::pluginListWidgetItemChanged(QListWidgetItem* item, QListWidgetItem*)
      {
      if (!item)
            return;
      int idx = item->data(Qt::UserRole).toInt();
      const PluginDescription& d = _pluginList[idx];
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
      PluginDescription* d = &_pluginList[idx];
      d->load = (item->checkState() == Qt::Checked);
      }

//---------------------------------------------------------
//   definePluginShortcutClicked
//---------------------------------------------------------

void PluginManager::definePluginShortcutClicked()
      {
      QListWidgetItem* item = pluginListWidget->currentItem();
      if (!item)
            return;
      int idx = item->data(Qt::UserRole).toInt();
      PluginDescription* pd = &_pluginList[idx];
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
      QListWidgetItem* item = pluginListWidget->currentItem();
      if (!item)
            return;
      int idx = item->data(Qt::UserRole).toInt();
      PluginDescription* pd = &_pluginList[idx];
      Shortcut* s = &pd->shortcut;
      s->clear();

      QAction* action = s->action();
      action->setShortcuts(s->keys());
//      mscore->addAction(action);

      pluginShortcut->setText(s->keysToString());
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

