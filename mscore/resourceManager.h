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
#include "plugin/pluginManager.h"

namespace Ms {

class ResourceManager : public QDialog, public Ui::Resource
   {
      Q_OBJECT

    virtual void closeEvent(QCloseEvent *e) override;
    virtual void hideEvent(QHideEvent*);
    QByteArray txt;
    void displayLanguages();
    void displayExtensions();
    void displayPluginRepo();
    bool verifyFile(QString path, QString hash);
    bool verifyLanguageFile(QString filename, QString hash);
    void refreshPluginButton(int row, bool updated = true);
    /*  Extracts the package(assumed zip format for now), and installs necessary files
        to plugin directory.
        Returns true on success.
     */
    bool installPluginPackage(QString& download_pkg, PluginPackageDescription& desc);

public:
    explicit ResourceManager(QWidget *parent = 0);
    void selectLanguagesTab();
    void selectExtensionsTab();

    static inline QString baseAddr() { return "http://extensions.musescore.org/3.5/"; }
    static inline QString pluginRepoAddr() { return "https://musescore.org/en/plugins"; }
    static inline QString pluginPageAddr(QString& name) { return "https://musescore.org/project/" + name; }

private:
    QMap <QPushButton *, QString> languageButtonMap; 	// QPushButton -> filename
    QMap <QPushButton *, QString> languageButtonHashMap;// QPushButton -> hash of the file


signals:
    void pluginRepoAvailable(QByteArray repo_html);
    void languageMetaAvailable(QByteArray json);
    void extensionMetaAvailable(QByteArray json);

private slots:
    void filterPluginList();
    void parsePluginRepo(QByteArray repo_page);
    void parseLanguages(QByteArray json);
    void parseExtensions(QByteArray json);
    void downloadLanguage();
    void scanPluginUpdate();
    void downloadExtension();
    void downloadInstallPlugin();
    void updatePlugin();
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
