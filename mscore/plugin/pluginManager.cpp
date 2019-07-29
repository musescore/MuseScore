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
   : QObject(parent)
      {
      setObjectName("PluginManager");
      }

//---------------------------------------------------------
//   setupUI
//   Called by resourcemanager to give the resource manager's UI elements
//---------------------------------------------------------

void PluginManager::setupUI(QLineEdit* pluginName_, QLineEdit* pluginPath_, QLineEdit* pluginVersion_,
      QLineEdit* pluginShortcut_, QTextBrowser* pluginDescription_, QTreeWidget* pluginTreeWidget_,
      QLabel* label_shortcut_, QLabel* label_version_, QPushButton* defineShortcut_, QPushButton* clearShortcut_)
      {
      pluginName = pluginName_;
      pluginPath = pluginPath_;
      pluginVersion = pluginVersion_;
      pluginShortcut = pluginShortcut_;
      pluginDescription = pluginDescription_;
      pluginTreeWidget = pluginTreeWidget_;
      label_shortcut = label_shortcut_;
      label_version = label_version_;
      definePluginShortcut = defineShortcut_;
      clearPluginShortcut = clearShortcut_;
      uiAttached = true;
      }

//---------------------------------------------------------
//   disAttachUI
//---------------------------------------------------------

void PluginManager::disAttachUI()
      {
      uiAttached = false;
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
      // fills _pluginPackageList from xml
      readPluginPackageList();
      loadList(false);
      connect(pluginTreeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), SLOT(pluginLoadToggled(QTreeWidgetItem*, int)));
      connect(pluginTreeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem*)),
            SLOT(pluginTreeWidgetItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)));
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

bool PluginManager::readPluginPackageList()
      {
      QFile f(dataPath + "/pluginpackages.xml");
      if (!f.exists())
            return false;
      if (!f.open(QIODevice::ReadOnly)) {
            qDebug("Cannot open plugin package file <%s>", qPrintable(f.fileName()));
            return false;
            }
      XmlReader e(&f);
      while (e.readNextStartElement()) {
            if (e.name() == "museScore") {
                  while (e.readNextStartElement()) {
                        const QStringRef& tag(e.name());
                        if (tag == "PluginPackage") {
                              QString page_url;
                              PluginPackageDescription desc;
                              while (e.readNextStartElement()) {
                                    const QStringRef t(e.name());
                                    if (t == "description")
                                          desc.desc_text = e.readElementText();
                                    else if (t == "pageURL")
                                          page_url = e.readElementText();
                                    else if (t == "pkgName")
                                          desc.package_name = e.readElementText();
                                    else if (t == "source")
                                          desc.source = (PluginPackageSource)e.readInt();
                                    else if (t == "directLink")
                                          desc.direct_link = e.readElementText();
                                    else if (t == "path")
                                          desc.dir = e.readElementText();
                                    else if (t == "qmlPath") {
                                          while (e.readNextStartElement()) {
                                                const QStringRef t_(e.name());
                                                if (t_ == "qml")
                                                      desc.qml_paths.push_back(e.readElementText());
                                                else
                                                      e.unknown();
                                                }
                                          }
                                    else if (t == "GitHubReleaseID")
                                          desc.release_id = e.readInt();
                                    else if (t == "GitHubLatestSha")
                                          desc.latest_commit = e.readElementText();
                                    else if (t == "LastModified")
                                          desc.last_modified = QDateTime::fromString(e.readElementText());
                                    else
                                          e.unknown();
                                    }
                              if (!page_url.isEmpty())
                                    _pluginPackageList.insert(page_url, desc);
                              else
                                    qDebug("Missing plugin page url.");
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

void PluginManager::writePluginPackageList()
      {
      QDir dir;
      dir.mkpath(dataPath);
      QFile f(dataPath + "/pluginpackages.xml");
      if (!f.open(QIODevice::WriteOnly)) {
            qDebug("cannot create plugin package file <%s>", qPrintable(f.fileName()));
            return;
            }
      XmlWriter xml(0, &f);
      xml.header();
      xml.stag("museScore version=\"" MSC_VERSION "\"");
      for (const QString &pkg : _pluginPackageList.keys()) {
            auto& v = _pluginPackageList.value(pkg);
            xml.stag("PluginPackage");
            xml.tag("description", v.desc_text);
            xml.tag("pageURL", pkg);
            xml.tag("pkgName", v.package_name);
            xml.tag("source", v.source);
            xml.tag("directLink", v.direct_link);
            xml.tag("path", v.dir);
            xml.stag("qmlPath");
            for (const QString& path : v.qml_paths)
                  xml.tag("qml", path);
            xml.etag();
            if (v.source == GITHUB_RELEASE)
                  xml.tag("GitHubReleaseID", v.release_id);
            if (v.source == GITHUB)
                  xml.tag("GitHubLatestSha", v.latest_commit);
            if (v.source == ATTACHMENT)
                  xml.tag("LastModified", v.last_modified.toString());
            xml.etag();
            }
      xml.etag();
      }

void PluginManager::updatePluginList(bool forceRefresh)
      {
#ifdef SCRIPT_INTERFACE
      QList<QString> pluginPathList;
      pluginPathList.append(dataPath + "/plugins");
      pluginPathList.append(mscoreGlobalShare + "plugins");
      pluginPathList.append(preferences.getString(PREF_APP_PATHS_MYPLUGINS));
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

void PluginManager::updatePluginPackage(const QString url, PluginPackageDescription* desc)
      {
      _pluginPackageList[url] = *desc;
      delete desc;
      }

void PluginManager::commitPlugin(const QString url, PluginPackageDescription* desc)
      {
      _pluginPackageList[url] = *desc;
      delete desc;
      // maybe there's a more efficient way than `loadList`
      loadList(false);
      }

bool PluginManager::uninstallPluginPackage(const QString& page_url)
      {
      if (!_pluginPackageList.contains(page_url))
            return true;
      PluginPackageDescription& desc = _pluginPackageList[page_url];
      // unregister
      for (QString& qml_path : desc.qml_paths) {
            for (auto it = _pluginList.begin(); it < _pluginList.end(); it++) {
                  if (it->path == qml_path) {
                        mscore->unregisterPlugin(&*it);
                        it = _pluginList.erase(it);
                        }
                  }
            }
      // remove the folder
      // In Qt 5.11 and earlier, qml cache files(.qmlc) may stay in the plugin folder, which casues
      // failure to remove the folder
      QDir d(desc.dir);
      if (!d.removeRecursively()) {
            qDebug("Plugin uninstalled incompletely.");
      }
      _pluginPackageList.remove(page_url);
      loadList(false);
      return true;
      }


PluginPackageDescription * PluginManager::getPluginPackage(PluginDescription * desc)
      {
      for (auto& item : _pluginPackageList)
            for (auto& p : item.qml_paths)
                  if (p == desc->path)
                        return &item;
      return nullptr;
      }

static constexpr int TypeRole = Qt::UserRole + 1; // another user role used in QTreeWidgetItem's data

//---------------------------------------------------------
//   loadList - populate the listbox.
//---------------------------------------------------------

void PluginManager::loadList(bool forceRefresh)
      {
      if (!uiAttached) return;
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
      pluginTreeWidget->clear();
      // firstly, add plugin packages
      std::map<PluginPackageDescription*, QTreeWidgetItem*> tree_map;
      QTreeWidgetItem* first_item = nullptr;
      for (auto &page_url : _pluginPackageList.keys()) {
            PluginPackageDescription& desc = _pluginPackageList[page_url];
            auto* package_item = new QTreeWidgetItem(pluginTreeWidget);
            if (!first_item)
                  first_item = package_item;
            tree_map[&desc] = package_item;
            package_item->setData(0, Qt::DisplayRole, desc.package_name);
            package_item->setData(0, Qt::UserRole, page_url);
            package_item->setData(0, TypeRole, true);
            package_item->setFlags(package_item->flags() | Qt::ItemIsEnabled);
            package_item->setCheckState(0, Qt::Unchecked);
            }
      // add packages' qml files and local stand-alone qmls
      for (int i = 0; i < n; ++i) {
            PluginDescription& d = _pluginList[i];
            Shortcut* s = &d.shortcut;
            localShortcuts[s->key()] = new Shortcut(*s);
            if (saveLoaded.contains(d.path)) d.load = true;
            PluginPackageDescription* package = getPluginPackage(&d);
            QTreeWidgetItem* item;
            if (package) {
                  // not stand-alone plugin
                  QTreeWidgetItem* parent_widget = tree_map[package];
                  auto plugin_check = d.load ? Qt::Checked : Qt::Unchecked;
                  if (parent_widget->childCount() == 0)
                        parent_widget->setCheckState(0, plugin_check);
                  else if (parent_widget->checkState(0) != Qt::PartiallyChecked)
                        parent_widget->setCheckState(0, (plugin_check != parent_widget->checkState(0)) ? Qt::PartiallyChecked : plugin_check);
                  item = new QTreeWidgetItem(parent_widget);
                  }
            else
                  item = new QTreeWidgetItem(pluginTreeWidget);
            QFileInfo(d.path).completeBaseName();
            item->setFlags(item->flags() | Qt::ItemIsEnabled);
            item->setCheckState(0, d.load ? Qt::Checked : Qt::Unchecked);
            item->setData(0, Qt::UserRole, i);
            item->setData(0, TypeRole, false);
            item->setData(0, Qt::DisplayRole, QFileInfo(d.path).completeBaseName());
            }
      if (n) {
            pluginTreeWidget->setCurrentItem(first_item);
            pluginTreeWidgetItemChanged(first_item, 0);
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
      writePluginPackageList();
      if (Shortcut::dirty)
            Shortcut::save();
      Shortcut::dirty = false;

      disconnect(pluginTreeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)));
      disconnect(pluginTreeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)));
      }


//---------------------------------------------------------
//   pluginListWidgetItemChanged
//---------------------------------------------------------

void PluginManager::pluginTreeWidgetItemChanged(QTreeWidgetItem* item, QTreeWidgetItem*)
      {
      if (!uiAttached)
            return;
      if (!item)
            return;
      if (!item->parent() && item->data(0,TypeRole).toBool()) {
            // root, i.e., a package
            pluginShortcut->setHidden(true);
            label_shortcut->setHidden(true);
            const QString& page_url = item->data(0, Qt::UserRole).toString();
            const PluginPackageDescription& desc = _pluginPackageList.value(page_url);
            pluginName->setText(desc.package_name);
            pluginPath->setText(desc.dir);
            // show different info
            label_version->setText(tr("Source:"));
            //pluginVersion->setText(PluginPackageSourceVerboseStr[desc.source]);
            pluginVersion->setText(desc.direct_link);
            pluginDescription->setHtml(desc.desc_text);
            definePluginShortcut->setEnabled(false);
            clearPluginShortcut->setEnabled(false);
            }
      else {
            // leaf, i.e., a qml file
            if (!item)
                  return;
            int idx = item->data(0, Qt::UserRole).toInt();
            const PluginDescription& d = _pluginList[idx];
            QFileInfo fi(d.path);
            pluginName->setText(fi.completeBaseName());
            pluginPath->setText(fi.absolutePath());
            label_version->setText(tr("Version:"));
            pluginVersion->setText(d.version);
            pluginShortcut->setHidden(false);
            label_shortcut->setHidden(false);
            pluginShortcut->setText(d.shortcut.keysToString());
            pluginDescription->setText(d.description);
            definePluginShortcut->setEnabled(true);
            clearPluginShortcut->setEnabled(true);
            }
      }

//---------------------------------------------------------
//   pluginLoadToggled
//---------------------------------------------------------

void PluginManager::pluginLoadToggled(QTreeWidgetItem* item, int col)
      {
      pluginTreeWidget->blockSignals(true);
      if (!item->parent()) {
            // root, i.e., a package
            if (item->checkState(col) == Qt::PartiallyChecked)
                  return;
            for (int i = 0; i < item->childCount(); i++) {
                  QTreeWidgetItem* qml = item->child(i);
                  int idx = qml->data(col, Qt::UserRole).toInt();
                  PluginDescription* d = &_pluginList[idx];
                  d->load = (item->checkState(col) == Qt::Checked);
                  qml->setCheckState(col, item->checkState(col));
                  }
            }
      else {
            int idx = item->data(col, Qt::UserRole).toInt();
            PluginDescription* d = &_pluginList[idx];
            d->load = (item->checkState(0) == Qt::Checked);
            QTreeWidgetItem* parent_widget = item->parent();
            int selected_count = 0;
            for (int i = 0; i < parent_widget->childCount(); i++) {
                  if (parent_widget->child(i)->checkState(0) == Qt::Checked)
                        selected_count++;
                  }
            parent_widget->setCheckState(0, selected_count > 0 ? (selected_count < parent_widget->childCount() ? Qt::PartiallyChecked : Qt::Checked) : Qt::Unchecked);
            }
      pluginTreeWidget->blockSignals(false);
      }

//---------------------------------------------------------
//   definePluginShortcutClicked
//---------------------------------------------------------

void PluginManager::definePluginShortcutClicked()
      {
      QTreeWidgetItem* item = pluginTreeWidget->currentItem();
      if (!item)
            return;
      if (!item->parent()) {
            qDebug("Calling on a package node.");
            return;
            }
      int idx = item->data(0, Qt::UserRole).toInt();
      PluginDescription* pd = &_pluginList[idx];
      Shortcut* s = &pd->shortcut;
      ShortcutCaptureDialog sc(s, localShortcuts, (QWidget*)parent());
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
      QTreeWidgetItem* item = pluginTreeWidget->currentItem();
      if (!item)
            return;
      if (!item->parent())
            return;
      int idx = item->data(0, Qt::UserRole).toInt();
      PluginDescription* pd = &_pluginList[idx];
      Shortcut* s = &pd->shortcut;
      s->clear();

      QAction* action = s->action();
      action->setShortcuts(s->keys());
//      mscore->addAction(action);

      pluginShortcut->setText(s->keysToString());
      }



}

