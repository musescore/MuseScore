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

#ifndef __PLUGIN_MANAGER_H__
#define __PLUGIN_MANAGER_H__

#include "ui_pluginManager.h"
#include "shortcut.h"

namespace Ms {
//---------------------------------------------------------
//   PluginDescription
//---------------------------------------------------------

struct PluginDescription {
      QString path;
      QString version;
      QString description;
      bool load;
      Shortcut shortcut;
      QString menuPath;
      };

struct PluginPackageMeta {
      QString name;
      std::tuple<bool, bool, bool> compatibility;
      QStringList categories;
      QString page_url;
      };

//---------------------------------------------------------
//   PluginPackageSource
//---------------------------------------------------------
enum PluginPackageSource {
      UNKNOWN,
      GITHUB,           // from a Github branch(usually master)
      GITHUB_RELEASE,   // from a Github release
      ATTACHMENT        // from an attachment in corresponding plugin page
      };


//---------------------------------------------------------
//   PluginPackageDescription
//   Info about how the package was fetched
//---------------------------------------------------------
struct PluginPackageDescription {
      // contains update info when there's an update. Not serialized to xml for now
      PluginPackageDescription* update = nullptr; 
      QString package_name;
      PluginPackageSource source = UNKNOWN;
      QString direct_link;
      QString dir; // path of directory of this package
      std::vector<QString> qml_paths;
      QString latest_commit; // valid when source set to GITHUB
      int release_id; // valid when source set to GITHUB_RELEASE
      // TODO: add last modified field for source set to ATTACHMENT

      };

//---------------------------------------------------------
//   PluginManager
//---------------------------------------------------------

class PluginManager : public QDialog, public Ui::PluginManager {
      Q_OBJECT

      QMap<QString, Shortcut*> localShortcuts;
      bool shortcutsChanged;
      QList<PluginDescription> _pluginList;

      void readSettings();
      void loadList(bool forceRefresh);

      virtual void closeEvent(QCloseEvent*);
      virtual void accept();

   private slots:
      void definePluginShortcutClicked();
      void clearPluginShortcutClicked();
      void pluginListWidgetItemChanged(QListWidgetItem*, QListWidgetItem*);
      void pluginLoadToggled(QListWidgetItem*);
      void reloadPluginsClicked();

   signals:
      void closed(bool);

   public:
      PluginManager(QWidget* parent = 0);
      void writeSettings();
      void init();

      bool readPluginList();
      void writePluginList();
      void updatePluginList(bool forceRefresh=false);

      int pluginCount() {return _pluginList.size();}
      PluginDescription* getPluginDescription(int idx) {return &_pluginList[idx];}
      };

extern bool collectPluginMetaInformation(PluginDescription*);

} // namespace Ms
#endif

