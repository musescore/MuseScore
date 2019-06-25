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
      QString package_name;
      PluginPackageSource source = UNKNOWN;
      QString direct_link;
      QString dir; // path of directory of this package
      std::vector<QString> qml_paths;
      QString latest_commit; // valid when source set to GITHUB
      int release_id; // valid when source set to GITHUB_RELEASE
      QDateTime last_modified; // valid when source set to ATTACHMENT
      };

//---------------------------------------------------------
//   PluginWorker
//   A class that manages blocking procedures including downloading, installing
//   and checking for update. Each PluginWorker object lives in a separate thread.
//---------------------------------------------------------
class PluginWorker : public QObject {
      Q_OBJECT
            PluginPackageDescription& desc;
public:
      PluginWorker(PluginPackageDescription& desc) : desc(desc) {}
public slots:
      // bool checkUpdate(); // may be discarded
      void update(); // called in ResourceManager::updatePlugin()
      bool analyzePluginPage(QString page_url); // replace ResourceManager::analyzePluginPage
      void download(); // replace Resource::downloadPluginPackage

      };



} // namespace Ms

#endif