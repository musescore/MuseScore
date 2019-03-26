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
// As a proof of concept, and for convenience, I just include pluginManager.h to use some plugin related definitions for now.
// Maybe all contents in pluginManager.h will be integrated into resource manager later.
#include "plugin/pluginManager.h"

namespace Ms {

class ResourceManager : public QDialog, public Ui::Resource
   {
    Q_OBJECT

    virtual void hideEvent(QHideEvent*);
    QByteArray txt;
    void displayLanguages();
    void displayExtensions();
    void displayPlugins();
    bool verifyFile(QString path, QString hash);
    bool verifyLanguageFile(QString filename, QString hash);
    /*  Analyzes the plugin page at `url` and writes download-related info to `desc`.
     *  Returns true if an update is needed.
     */
    bool analyzePluginPage(QString url, PluginPackageDescription& desc);
    /*  Extracts the package(assumed zip format for now), and installs necessary files
        to plugin directory.
        Returns true on success.
     */
    bool installPluginPackage(QString& download_pkg, PluginPackageDescription& desc);
    void refreshPluginButton(QWidget* button_group);
    void writePluginPackages();
    bool readPluginPackages();

public:
    explicit ResourceManager(QWidget *parent = 0);
    void selectLanguagesTab();
    void selectExtensionsTab();

    static inline QString baseAddr() { return "http://extensions.musescore.org/3.5/"; }
    static inline QString pluginAddr() { return "https://musescore.org/en/plugins"; }
    static inline QString pluginPageAddr(QString& name) { return "https://musescore.org/project/" + name; }

private:
    QMap <QPushButton *, QString> languageButtonMap; 	// QPushButton -> filename
    QMap <QPushButton *, QString> languageButtonHashMap;// QPushButton -> hash of the file
    QMap <QPushButton *, PluginPackageMeta> pluginButtonURLMap;// QPushButton -> plugin page url
    QMap <QString, PluginPackageDescription> pluginDescriptionMap; // plugin page url -> description of installed plugin

private slots:
    void downloadLanguage();
    void downloadExtension();
    void downloadPluginPackage();
    void uninstallPluginPackage();
    void uninstallExtension();
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
