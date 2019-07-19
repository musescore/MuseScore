//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#ifndef __PLUGIN_UPDATER_H__
#define __PLUGIN_UPDATER_H__

#include <map>

namespace Ms {
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

extern std::map<PluginPackageSource, QString> PluginPackageSourceVerboseStr;

//---------------------------------------------------------
//   PluginPackageLink
//---------------------------------------------------------
struct PluginPackageLink {
      PluginPackageSource source;
      QString url; // valid for GITHUB_RELEASE and ATTACHMENT
      QString user; // valid for GITHUB
      QString repo; // valid for GITHUB
      QString branch; // valid for GITHUB
      int newline_index;
      QString curr_line;
      int score = 0;
      QString hint_above;
      };

//---------------------------------------------------------
//   PluginPackageDescription
//   Info about how the package was fetched
//---------------------------------------------------------
struct PluginPackageDescription {
      // contains update info when there's an update. Not serialized to xml for now
      PluginPackageDescription* update = nullptr;
      QString desc_text;
      bool load = false;
      QString package_name;
      PluginPackageSource source = UNKNOWN;
      QString direct_link;
      QString dir; // path of directory of this package
      std::vector<QString> qml_paths;
      unsigned int valid_count = 0;
      QString latest_commit; // valid when source set to GITHUB
      int release_id; // valid when source set to GITHUB_RELEASE
      QDateTime last_modified; // valid when source set to ATTACHMENT
      };

enum PluginStatus {
      NOT_INSTALLED, INSTALL_FAILED, INSTALL_FAILED_INVALID,
      ANALYZING, ANALYZE_FAILED,
      DOWNLOADING, DOWNLOAD_FAILED,
      UPDATED, UPDATE_AVAILABLE
      };

struct PluginButtonStatus {
      //PluginStatus s;
      QString display_text;
      bool install_enable;
      bool uninstall_enable;
      };


class ResourceManager;

//---------------------------------------------------------
//   PluginWorker
//   A class that manages blocking procedures including downloading, installing
//   and checking for update. Each PluginWorker object lives in a separate thread.
//---------------------------------------------------------
class PluginWorker : public QObject {
      Q_OBJECT
public:
      PluginWorker(ResourceManager* r);
      PluginWorker(const PluginPackageDescription& desc, ResourceManager* r);
public slots:
      bool analyzePluginPage(QString page_url); // replace ResourceManager::analyzePluginPage
      QString download(const QString& page_url, bool update = false); // replace Resource::downloadPluginPackage
      bool install(QString& download_pkg, PluginStatus* result = nullptr);
      void downloadInstall(QPushButton* install);
      void updateInstall(QPushButton* install); // called in ResourceManager::updatePlugin()
      void checkUpdate(QPushButton* install);
      void detached();

signals:
      void pluginStatusChanged(int idx, PluginStatus status);
      void finished();
      void pluginInstalled(const QString url, PluginPackageDescription* desc);
      void updateAvailable(const QString url, PluginPackageDescription* desc);
private:
      PluginPackageDescription desc;
      PluginStatus status;
      ResourceManager* r;
      };



} // namespace Ms


#endif