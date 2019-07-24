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

#include "plugin/pluginUpdater.h"
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

//---------------------------------------------------------
//   PluginManager
//---------------------------------------------------------

class PluginManager : public QObject {
      Q_OBJECT

      QLineEdit* pluginName;
      QLineEdit* pluginPath;
      QLineEdit* pluginVersion;
      QLineEdit* pluginShortcut;
      QLabel* label_shortcut;
      QLabel* label_version;
      QTextBrowser* pluginDescription;
      QTreeWidget* pluginTreeWidget;
      QMap<QString, Shortcut*> localShortcuts;
      bool shortcutsChanged;
      QList<PluginDescription> _pluginList;
      QMap <QString, PluginPackageDescription> _pluginPackageList; // plugin page url -> description of installed plugin
      PluginPackageDescription* getPluginPackage(PluginDescription* desc);
      void loadList(bool forceRefresh);
      void refreshList();
      bool uninstallPlugin(PluginDescription* p);
      bool uiAttached = false;

   private slots:
      void definePluginShortcutClicked();
      void clearPluginShortcutClicked();
      void pluginTreeWidgetItemChanged(QTreeWidgetItem*, QTreeWidgetItem*);
      void pluginLoadToggled(QTreeWidgetItem*, int);
      void reloadPluginsClicked();
      void updatePluginPackage(const QString url, PluginPackageDescription* desc);
      void commitPlugin(const QString url, PluginPackageDescription* desc); // called from plugin worker

   signals:
      void closed(bool);
      

   public:
      PluginManager(QWidget* parent = 0);
      virtual void accept();
      void init();
      void setupUI(QLineEdit* pluginName, QLineEdit* pluginPath, QLineEdit* pluginVersion,
            QLineEdit* pluginShortcut, QTextBrowser* pluginDescription, QTreeWidget* pluginTreeWidget,
            QLabel* label_shortcut, QLabel* label_version);
      void disAttachUI();
      bool readPluginList();
      bool readPluginPackageList();
      void writePluginList();
      void writePluginPackageList();
      void updatePluginList(bool forceRefresh = false);

      const PluginPackageDescription getPackageDescription(QString page_url) const { return _pluginPackageList.value(page_url); }
      bool isPackageInstalled(QString page_url) const { return _pluginPackageList.contains(page_url); }

      int pluginCount() { return _pluginList.size(); }
      PluginDescription* getPluginDescription(int idx) { return &_pluginList[idx]; }
      bool uninstallPluginPackage(const QString& page_url);
      };

extern bool collectPluginMetaInformation(PluginDescription*);

} // namespace Ms
#endif
