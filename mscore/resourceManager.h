//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef RESOURCE_H
#define RESOURCE_H

#include "ui_resourceManager.h"
#include "downloadUtils.h"
#include "plugin/pluginManager.h"
#include "plugin/pluginUpdater.h"

namespace Ms {

class ResourceManager : public QDialog, public Ui::Resource
   {
    Q_OBJECT

    virtual void hideEvent(QHideEvent*);
    QByteArray txt;
    void displayLanguages();
    void displayExtensions();
    void displayPluginRepo();
    void displayPlugins();
    bool verifyFile(QString path, QString hash);
    bool verifyLanguageFile(QString filename, QString hash);
    void refreshPluginButton(int row, bool updated = true);
    /*  Analyzes the plugin page at `url` and writes download-related info to `desc`.
     *  Returns true if an update is needed.
     */
    /*  Extracts the package(assumed zip format for now), and installs necessary files
        to plugin directory.
        Returns true on success.
     */
    bool installPluginPackage(QString& download_pkg, PluginPackageDescription& desc);
    /*  Check update for one single plugin.
    
     */
    void writePluginPackages();
    /*  Reads plugin descriptions from xml. `pluginDescriptionMap` would be flushed.
        Should only be called at the beginning
     */
    bool readPluginPackages();
    bool isPluginLocal(PluginDescription& desc);

public:
    explicit ResourceManager(QWidget *parent = 0);
    void selectLanguagesTab();
    void selectExtensionsTab();
    void commitPlugin(const QString& url, PluginPackageDescription& desc);

    static inline QString baseAddr() { return "http://extensions.musescore.org/3.5/"; }
    static inline QString pluginAddr() { return "https://musescore.org/en/plugins"; }
    static inline QString pluginPageAddr(QString& name) { return "https://musescore.org/project/" + name; }

private:
    QMap <QPushButton *, QString> languageButtonMap; 	// QPushButton -> filename
    QMap <QPushButton *, QString> languageButtonHashMap;// QPushButton -> hash of the file

    QMap <QString, PluginPackageDescription> pluginDescriptionMap; // plugin page url -> description of installed plugin

    QThreadPool workerThreadPool;

private slots:
    void filterPluginList();
    void downloadLanguage();
    void scanPluginUpdate();
    void downloadExtension();
    void downloadInstallPlugin();
    void uninstallPluginPackage();
    void uninstallExtension();
    void refreshPluginButton(int row, PluginStatus status);
   };

class ExtensionFileSize : public QTableWidgetItem
      {
      int _size;

   public:
      ExtensionFileSize(const int i);
      int getSize() const { return _size; }
      bool operator<(const QTableWidgetItem& nextItem) const;

      };

class LanguageFileSize : public QTableWidgetItem
      {
      double _size;

   public:
      LanguageFileSize(const double d);
      double getSize() const { return _size; }
      bool operator<(const QTableWidgetItem& nextItem) const;

      };

}
#endif // RESOURCE_H
