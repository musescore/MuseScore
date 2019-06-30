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

#include "musescore.h"
#include "resourceManager.h"
#include "downloadUtils.h"
#include "plugin/pluginUpdater.h"
#include "thirdparty/qzip/qzipreader_p.h"
#include "ui_resourceManager.h"

namespace Ms {

static int PluginStatus_id = qRegisterMetaType<PluginStatus>("PluginStatus");

static inline std::tuple<bool, bool, bool> compatFromString(const QString& raw) {
      return std::make_tuple(raw.contains("1.x"), raw.contains("2.x"), raw.contains("3.x"));
}

//---------------------------------------------------------
//   GitHub utility functions
//---------------------------------------------------------
static inline QString githubReleaseAPI(const QString &user, const QString &repo) { return "https://api.github.com/repos/" + user + "/" + repo + "/releases"; }
static inline QString githubCommitAPI(const QString &user, const QString &repo, const QString &ref = "")
      {
      return "https://api.github.com/repos/" + user + "/" + repo + "/commits" + (ref.isEmpty() ? "" : ('/' + ref));
      }
static inline QString githubLatestArchiveURL(const QString &user, const QString &repo, const QString &branch = "master")
      {
      //return "https://api.github.com/repos/" + user + "/" + repo + "/zip/" + branch;
      return "https://github.com/" + user + '/' + repo + "/archive/" + branch + ".zip";
      }
static QString getLatestCommitSha(const QString& user, const QString &repo, const QString &branch = "master")
      {
      QString api_url = githubCommitAPI(user, repo, branch);
      DownloadUtils json;
      json.setTarget(api_url);
      json.download();
      QJsonDocument json_doc = QJsonDocument::fromJson(json.returnData());
      QJsonObject latest_commit;
      if (json_doc.isArray())
            // there are  multiple commits, fetch the latest
            latest_commit = json_doc.array().first().toObject();
      else
            latest_commit = json_doc.object();
      if (latest_commit.contains("sha"))
            return latest_commit["sha"].toString();
      else return {};
      }
static inline bool isCompatibleRelease(const QString& branch)
      {
      if (branch == "master" || branch == "3.x") return true;
      if (branch.contains("2.x") || branch.contains("version2") || branch.contains("musescore2") || branch.contains("musescore 2")) return false;
      if (branch.contains("3.x") || branch.contains("version3") || branch.contains("musescore3") || branch.contains("musescore 3")) return true;
      if (branch.contains("2") && !branch.contains("3")) return false;
      return true;
      }
static inline int CompatEstimate(QString branch)
      {
      branch = branch.toLower();
      if (branch.contains("2.x") || branch.contains("2x") || branch.contains("version2") || branch.contains("musescore2") || branch.contains("musescore 2")) return -2;
      if (branch.contains("3.x") || branch.contains("3x") || branch.contains("version3") || branch.contains("musescore3") || branch.contains("musescore 3")) return 2;
      if (branch.contains("2") && !branch.contains("3")) return -1;
      if (branch.contains("3") && !branch.contains("2")) return 1;
      return 0;
      }

static bool getLatestRelease(const QString& user, const QString& repo, int& release_id, QString& link, const QString filenameBase = "")
      {
      DownloadUtils release_json;
      release_json.setTarget(githubReleaseAPI(user, repo));
      release_json.download();
      QJsonDocument releases = QJsonDocument::fromJson(release_json.returnData());
      if (!releases.array().isEmpty()) {
            // By default, releases returned from GitHub are sorted from the latest commit to the oldest
            for (const auto& release : releases.array()) {
                  if (release.isObject()) {
                        const auto& release_obj = release.toObject();
                        QString release_name = release_obj.value("name").toString();
                        if (!filenameBase.isEmpty() && filenameBase != release_name)
                              continue;
                        if (isCompatibleRelease(release_obj.value("target_commitish").toString())) {
                              // get "id" and "zipball_url"
                              release_id = release_obj.value("id").toInt();
                              link = release_obj.value("zipball_url").toString();
                              return true;
                              }
                        else
                              continue;
                        }
                  else
                        continue;
                  }
            return false;
            }
      else
            return false;
      }

static inline bool isNotFoundMessage(const QJsonObject& json_obj)
      {
      return json_obj["message"].toString() == "Not Found";
      }
static inline QString filenameBaseFromPageURL(QString page_url)
      {
      Q_ASSERT(page_url.contains("/project/"));
      if (page_url.contains("/en/project/"))
            return page_url.remove("/en/project/");
      return page_url.remove("/project/");
      }
static inline QString getExtFromURL(QString& direct_link)
      {
      QString possible_ext = direct_link.split(".").last();
      // We can assume that most extensions contain only 
      // alphanumeric and plus "_", and other connector punctuation chars
      if (possible_ext.contains(QRegularExpression("\\W")))
            return {};
      return possible_ext;
      }

//---------------------------------------------------------
//   displayPluginRepo
//---------------------------------------------------------
void ResourceManager::displayPluginRepo()
      {
      // fills pluginDescriptionMap from xml
      readPluginPackages();
      // fetch plugin list from web
      DownloadUtils html(this);
      html.setTarget(pluginRepoAddr());
      html.download();
      QByteArray html_raw = html.returnData();
      QByteArray table_start("<table");
      QByteArray table_end("table>");
      int table_start_idx = html_raw.indexOf(table_start);
      int table_end_idx = html_raw.indexOf(table_end) + 6;
      QByteArray table_raw = html_raw.mid(table_start_idx, table_end_idx - table_start_idx);
      QDomDocument table_xml;
      if (!table_xml.setContent(table_raw))
            qDebug("Fail to parse the HTML table from the repo page.");
      QDomElement body = table_xml.elementsByTagName("tbody").item(0).toElement();
      QDomNodeList tr_list = body.elementsByTagName("tr");
      pluginsTable->setRowCount(tr_list.length());
      QStringList all_categories;
      for (int row=0; row<tr_list.length();row++) {
            int col = 0;
            QDomElement tr_node = tr_list.item(row).toElement();
            QDomNodeList td_list = tr_node.elementsByTagName("td");
            Q_ASSERT(td_list.length() == 4);
            QString page_url = td_list.item(0).toElement().firstChildElement("a").attribute("href");
            QString name = td_list.item(0).toElement().text();
            std::tuple<bool,bool,bool> compat = compatFromString(td_list.item(1).toElement().text());
            QString category_raw = td_list.item(2).toElement().text();
            QStringList category_list = category_raw.split(", ");
            for (QString& c : category_list) {
                  QString c2 = c.simplified();
                  if (c2.isEmpty()) { 
                        category_list.removeOne(c);
                        continue; 
                        }
                  c = c2;
                  if (!all_categories.contains(c2))
                        all_categories << c2;
                  }
            QString url_text = "<a href=\"https://musescore.org" + page_url + "\">" + td_list.item(col).toElement().text() + "</a>";
            // display a clickable hyperlink for each item
            QLabel* plugin_name = new QLabel(url_text);
            plugin_name->setTextFormat(Qt::RichText);
            plugin_name->setTextInteractionFlags(Qt::TextBrowserInteraction);
            plugin_name->setOpenExternalLinks(true);
            pluginsTable->setIndexWidget(pluginsTable->model()->index(row, col++), plugin_name);
            for (; col < 3; col++)
                  pluginsTable->setItem(row, col, new QTableWidgetItem(td_list.item(col).toElement().text()));
            if (page_url.isNull()) {
                  pluginsTable->setItem(row,col,new QTableWidgetItem("No page URL found."));
                  continue;
                  }

            QPushButton* install_button = new QPushButton(tr("Install"));
            // add categories, name and page_url to properties
            install_button->setProperty("page_url", page_url);
            install_button->setProperty("categories", category_list);
            install_button->setProperty("name", name);
            install_button->setProperty("row", row);
            QPushButton* uninstall_button = new QPushButton(tr("Uninstall"));
            uninstall_button->setProperty("page_url", page_url);
            uninstall_button->setProperty("row", row);
            uninstall_button->setEnabled(false);
            connect(uninstall_button, SIGNAL(clicked()), this, SLOT(uninstallPluginPackage()));
            pluginsTable->setIndexWidget(pluginsTable->model()->index(row, col++), install_button);
            pluginsTable->setIndexWidget(pluginsTable->model()->index(row, col), uninstall_button);
            bool installed = pluginDescriptionMap.contains(page_url);
            refreshPluginButton(row, installed ? PluginStatus::UPDATED : PluginStatus::NOT_INSTALLED);
            }
      categories->insertItems(0, all_categories);
      }

void ResourceManager::filterPluginList()
      {
      const QString category = categories->currentText();
      const QString& search = lineEdit->text();
      for (int i = 0; i < pluginsTable->rowCount(); i++) {
            // locate the "Install" button
            QPushButton* install = static_cast<QPushButton*>(pluginsTable->indexWidget(pluginsTable->model()->index(i, 3)));
            const QString& plugin_name = install->property("name").toString();
            const QStringList& category_list = install->property("categories").toStringList();
            if (plugin_name.contains(search, Qt::CaseInsensitive) && (category == "Any" || category_list.contains(category)))
                  pluginsTable->showRow(i);
            else
                  pluginsTable->hideRow(i);
            }
      }

void ResourceManager::displayPlugins()
      {
      pluginTreeWidget->clear();
      for (const QString &pkg : pluginDescriptionMap.keys()) {
            auto& desc = pluginDescriptionMap[pkg];
            auto* package_item = new QTreeWidgetItem(pluginTreeWidget);
            package_item->setData(0, Qt::DisplayRole, desc.package_name);
            package_item->setFlags(package_item->flags() | Qt::ItemIsEnabled);
            package_item->setCheckState(0, Qt::Unchecked);
            // add qmls
            for (const auto &qml : desc.qml_paths) {
                  QFileInfo f(qml);
                  auto* qml_item = new QTreeWidgetItem(package_item);
                  qml_item->setData(0, Qt::DisplayRole, f.fileName());
                  qml_item->setFlags(qml_item->flags() | Qt::ItemIsEnabled);
                  qml_item->setCheckState(0, Qt::Unchecked);
                  }
            }
      for (int i = 0; i < mscore->getPluginManager()->pluginCount(); i++) {
            auto* desc = mscore->getPluginManager()->getPluginDescription(i);
            if (isPluginLocal(*desc)) {
                  QTreeWidgetItem* item = new QTreeWidgetItem(pluginTreeWidget);
                  item->setFlags(item->flags() | Qt::ItemIsEnabled);
                  item->setCheckState(0, desc->load ? Qt::Checked : Qt::Unchecked);
                  item->setData(0, Qt::DisplayRole, QFileInfo(desc->path).completeBaseName());
                  }
            }
      }

bool ResourceManager::isPluginLocal(PluginDescription& desc)
      {
      desc.path;
      for (const auto& item : pluginDescriptionMap.values())
            for (auto& p : item.qml_paths)
                  if (p == desc.path)
                        return false;
      return true;
      }

static std::vector<PluginPackageLink> getAttachments(const QString& html_raw, void(*localHint)(QRegularExpressionMatch&, PluginPackageLink&, const QString&))
      {
      std::vector<PluginPackageLink> file_urls;
#if 0
      // This is the code that fetches the attachment table that appears at the bottom of the page.
      // Now I find it more attracting to search for inline attachment urls instaed.
      {
            int start_idx = html_raw.indexOf("<div class=\"field field--name-upload field--type-file field--label-above\">");
            if (start_idx == -1) {
                  qDebug("No attachment found.");
                  return;
            }
            QString attachments_raw = html_raw.mid(start_idx, -1);
            XmlReader xml(attachments_raw);
            while (!xml.atEnd() && xml.name() != "table") xml.readNextStartElement();
            while (!xml.atEnd() && xml.name() != "tbody") xml.readNextStartElement();
            while (!xml.atEnd() && xml.name() != "tr") xml.readNextStartElement();
            // now we've arrived at the first <tr>
            while (!xml.atEnd() && xml.name() == "tr") {
                  for (int i = 0; i < 3; i++) // enter <td>, <span>, <a>
                        xml.readNextStartElement();
                  PluginPackageLink link = { {ATTACHMENT},{xml.attributes().value("href").toString()} };
                  //link.hint = xml.readElementText();
                  link.score = CompatEstimate(xml.readElementText());
                  file_urls.push_back(link);
                  for (int i = 0; i < 3; i++) // exit <a>, <span>, <td>
                        xml.skipCurrentElement();
                  xml.readNextStartElement();
            }
      }
#endif
      // search for all links from musescore.org
      QRegularExpression musescore_link_patt("<a href=\"(https://musescore.org/sites/musescore.org/files/(\\d+\\-\\d+/)?[\\w\\-\\.]+)\".*?>(?<text>.*?)</a>");
      auto it = musescore_link_patt.globalMatch(html_raw);
      while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            // check suffix
            const QStringList suffixes = { "qml","zip","rar","7z" };
            if (!suffixes.contains(QFileInfo(match.captured(1)).suffix().toLower()))
                  continue;
            PluginPackageLink link;
            link.source = ATTACHMENT;
            link.url = match.captured(1);
            localHint(match, link, html_raw);
            file_urls.push_back(link);
            }
      return file_urls;
      }

static std::vector<PluginPackageLink> getGitHubLinks(const QString& html_raw, void(*localHint)(QRegularExpressionMatch&, PluginPackageLink&, const QString&))
      {
      std::vector<PluginPackageLink> github_urls;
      QRegularExpression github_repo_patt("<a href=\"(https?://github.com/([\\w\\-]+)/([\\w\\-]+))\".*?>(?<text>.*?)</a>");
      QRegularExpression github_archive_link("<a href=\"(https?://github.com/([\\w\\-]+)/([\\w\\-]+)/archive/(.+?))\".*?>(?<text>.*?)</a>"); // downloadable link
      QRegularExpression github_directory_patt("<a href=\"(https?://github.com/([\\w\\-]+)/([\\w\\-]+)/(tree|blob)/([\\w\\-]+).*?)\".*?>(?<text>.*?)</a>"); // with branch info
      auto it = github_archive_link.globalMatch(html_raw);
      while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            PluginPackageLink link;
            link.source = GITHUB_RELEASE;
            link.url = match.captured(1);
            link.user = match.captured(2);
            link.repo = match.captured(3);
            localHint(match, link, html_raw);
            github_urls.push_back(link);
            }
      it = github_directory_patt.globalMatch(html_raw);
      while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            PluginPackageLink link;
            link.source = GITHUB;
            link.url = match.captured(1);
            link.user = match.captured(2);
            link.repo = match.captured(3);
            link.branch = match.captured(5);
            localHint(match, link, html_raw);
            github_urls.push_back(link);
            }
      it = github_repo_patt.globalMatch(html_raw);
      while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            PluginPackageLink link;
            link.source = GITHUB;
            link.url = match.captured(1);
            link.user = match.captured(2);
            link.repo = match.captured(3);
            localHint(match, link, html_raw);
            github_urls.push_back(link);
            }
      return github_urls;
      }

static std::vector<PluginPackageLink> getLinks(const QByteArray& html_raw_array)
      {
      QString html_raw(html_raw_array);
      auto GetLocalHint = [](QRegularExpressionMatch& match, PluginPackageLink& link, const QString& html_raw) {
            int start_idx = match.capturedStart(0);
            int end_idx = match.capturedEnd();
            int newline_start = start_idx, newline_end = end_idx;
            while (newline_start > 0 && html_raw[newline_start - 1] != '\n')
                  newline_start--;
            while (newline_end < html_raw.length() && html_raw[newline_end] != '\n')
                  newline_end++;
            // TODO: first use hyper link text as hint, 
            // and if not strong enough, use the whole line 
            link.newline_index = newline_start;
            link.curr_line = html_raw.mid(newline_start, newline_end - newline_start + 1);
            link.score = CompatEstimate(link.curr_line);
            };
      std::vector<PluginPackageLink> attachments = getAttachments(html_raw, GetLocalHint);
      std::vector<PluginPackageLink> github_links = getGitHubLinks(html_raw, GetLocalHint);
      std::vector<PluginPackageLink> urls;
      urls.insert(urls.begin(), attachments.begin(), attachments.end());
      urls.insert(urls.begin(), github_links.begin(), github_links.end());
      std::set<int> begin_idx;
      for (auto& url : urls)
            begin_idx.insert(url.newline_index);
      // now, get more hints from the line above each link
      for (int i = 0; i < urls.size(); i++) {
            auto& link = urls[i];
            if (link.score >= 0) {
                  int last_idx = link.newline_index - 1;
                  int first_idx = last_idx - 1;
                  while (first_idx > 0 && html_raw[first_idx - 1] != '\n')
                        first_idx--;
                  QString last_line = html_raw.mid(first_idx, last_idx - first_idx + 1);
                  // if another url is in the last line
                  if (begin_idx.find(first_idx) != begin_idx.end())
                        continue;
                  int newscore = CompatEstimate(last_line);
                  if (newscore > link.score)
                        link.score = newscore;
                  }
            }
      return urls;
      }

static QDateTime GetLastModified(QString& url)
      {
      QNetworkAccessManager nmg;
      QEventLoop loop;
      QNetworkReply* reply = nmg.head(QNetworkRequest(url));
      QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
      loop.exec();
      QVariant last_modified = reply->header(QNetworkRequest::LastModifiedHeader);
      if (last_modified.isValid())
            return last_modified.toDateTime();
      else
            return QDateTime();
      }


void ResourceManager::scanPluginUpdate()
      {
      for (int row = 0; row < pluginsTable->rowCount(); row++) {
            QPushButton* install = static_cast<QPushButton*>(pluginsTable->indexWidget(pluginsTable->model()->index(row, 3)));
            const QString& page_url = install->property("page_url").toString();
            // if installed
            if (pluginDescriptionMap.contains(page_url)) {
                  install->setEnabled(false);
                  install->setText(tr("Pending…"));
                  PluginWorker* worker = new PluginWorker(pluginDescriptionMap[page_url], this);
                  QtConcurrent::run(&workerThreadPool, worker, &PluginWorker::checkUpdate, install);
                  }
            }
      }

//---------------------------------------------------------
//   installPluginPackage
//---------------------------------------------------------

bool ResourceManager::installPluginPackage(QString& download_pkg, PluginPackageDescription& desc)
      {
      QFileInfo f_pkg(download_pkg);
      QString suffix = f_pkg.suffix().toLower();
      QString destination_prefix = dataPath + "/plugins" + "/" + f_pkg.completeBaseName();
      QDir().mkdir(destination_prefix);
      desc.dir = destination_prefix;
      if (suffix == "zip") {
            MQZipReader zipFile(download_pkg);
            QVector<MQZipReader::FileInfo> allFiles = zipFile.fileInfoList();
            if (allFiles.size() == 0)
                  return false;
            // If zip contains multiple files in root, or a single qml, create a
            // root folder in plugin dir for them.
            // If zip contains a single directory, don't use that directory's name
            bool has_no_dir = true;
            std::set<QString> dirs;
            for (MQZipReader::FileInfo fi : allFiles) {
                  QString dir_root = fi.filePath.split('/').first();
                  dirs.insert(dir_root);
                  }
            if (dirs.size() == 1)
                  if (allFiles.size() > 1) // the element in dirs must be a dir then
                        has_no_dir = false;
            int stripped_len = 0;
            if (!has_no_dir) {
                  stripped_len = allFiles.first().filePath.length() + 1;
                  // the directory in the archive is not to be added
                  allFiles.pop_front();
                  }
            auto filePathStripper = [&](QString& filePath) {return filePath.right(filePath.size() - stripped_len); };

            // extract and copy
            for (MQZipReader::FileInfo fi : allFiles) {
                  if (fi.isDir)
                        QDir().mkdir(destination_prefix + "/" + filePathStripper(fi.filePath));
                  else if (fi.isFile) {
                        if (QFileInfo(fi.filePath).suffix().toLower() == "qml")
                              desc.qml_paths.push_back(destination_prefix + "/" + filePathStripper(fi.filePath));
                        QFile new_f(destination_prefix + "/" + filePathStripper(fi.filePath));
                        if (!new_f.open(QIODevice::WriteOnly)) {
                              qDebug("Cannot write the file.");
                              return false;
                        }
                        new_f.write(zipFile.fileData(fi.filePath));
                        new_f.setPermissions(fi.permissions);
                        new_f.close();
                        }
                  }
            zipFile.close();
            }
      else if (suffix == "qml") {
            QString destination = destination_prefix + "/" + f_pkg.fileName();
            QFileInfo check(destination);
            if (check.exists()) {
                  if (check.isDir())
                        QDir(check.absoluteFilePath()).removeRecursively();
                  if (check.isFile())
                        QFile::remove(check.absoluteFilePath());
                  }
            QFile(f_pkg.absoluteFilePath()).copy(destination);
            desc.qml_paths.push_back(destination);
            }
      else {
            qDebug("Unknown plugin suffix %s.", qPrintable(suffix));
            return false;
            }
      QFile::remove(download_pkg);
      return true;
      }

//---------------------------------------------------------
//   downloadInstallPlugin
//---------------------------------------------------------

void ResourceManager::downloadInstallPlugin()
      {
      QPushButton* button = static_cast<QPushButton*>(sender());
      button->setEnabled(false);
      button->setText(tr("Pending…"));
      PluginWorker* worker = new PluginWorker(this);
      QtConcurrent::run(&workerThreadPool, worker, &PluginWorker::downloadInstall, button);
      }

void ResourceManager::uninstallPluginPackage()
      {
      QPushButton* button = static_cast<QPushButton*>(sender());
      const QString& url = button->property("page_url").toString();
      if (!pluginDescriptionMap.contains(url))
            return;
      PluginPackageDescription& desc = pluginDescriptionMap[url];
      // remove qml files first(which may be outside the folder of this plugin)
      for (QString& path : desc.qml_paths)
            QFile::remove(path);
      // then remove the folder
      QDir d(desc.dir);
      if (!d.removeRecursively()) {
            button->setText(tr("Failed, try again"));
            return;
            }
      pluginDescriptionMap.remove(url);
      refreshPluginButton(button->property("row").toInt(), PluginStatus::NOT_INSTALLED);
      writePluginPackages();
      displayPlugins();
      }

static const std::map<PluginStatus, PluginButtonStatus> buttonStatuses = {
      {NOT_INSTALLED,{QObject::tr("Install"),true,false}},
      {INSTALL_FAILED,{QObject::tr("Install failed. Try again"),true,false}},
      {ANALYZING,{QObject::tr("Analyzing"),false,false}},
      {ANALYZE_FAILED,{QObject::tr("Analyze failed. Try again"),true,false}},
      {DOWNLOADING,{QObject::tr("Downloading"),false,false}},
      {DOWNLOAD_FAILED,{QObject::tr("Download failed. Try again"),true,false}},
      {UPDATED,{QObject::tr("Updated"),false,true}},
      {UPDATE_AVAILABLE,{QObject::tr("Update"),true,true}}
      };

void ResourceManager::refreshPluginButton(int row, bool updated/* = true*/)
      {
      // TODO: get button, then plugin url, then update status
      // install button
      QPushButton* install = static_cast<QPushButton*>(pluginsTable->indexWidget(pluginsTable->model()->index(row, 3)));
      install->disconnect();
      QPushButton* uninstall = static_cast<QPushButton*>(pluginsTable->indexWidget(pluginsTable->model()->index(row, 4)));
      const QString& page_url = install->property("page_url").toString();
      bool installed = pluginDescriptionMap.contains(page_url);
      uninstall->setEnabled(installed);
      if (installed) {
            if (updated) {
                  install->setText("Updated");
                  install->setEnabled(false);
                  }
            else {
                  install->setText("Update");
                  install->setEnabled(true);
                  connect(install, SIGNAL(clicked()), this, SLOT(updatePlugin()));
                  }
            }
      else {
            install->setText("Install");
            install->setEnabled(true);
            connect(install, SIGNAL(clicked()), this, SLOT(downloadInstallPlugin()));
            }

      }

void ResourceManager::refreshPluginButton(int row, PluginStatus status)
      {
      QPushButton* install = static_cast<QPushButton*>(pluginsTable->indexWidget(pluginsTable->model()->index(row, 3)));
      QPushButton* uninstall = static_cast<QPushButton*>(pluginsTable->indexWidget(pluginsTable->model()->index(row, 4)));
      Q_ASSERT(buttonStatuses.find(status) != buttonStatuses.end());
      const PluginButtonStatus& button_stat = buttonStatuses.at(status);
      install->setText(button_stat.display_text);
      install->setEnabled(button_stat.install_enable);
      install->disconnect();
      if (status == UPDATE_AVAILABLE)
            QObject::connect(install, SIGNAL(clicked()), this, SLOT(updatePlugin()));
      else if(button_stat.install_enable)
            QObject::connect(install, SIGNAL(clicked()), this, SLOT(downloadInstallPlugin()));
      uninstall->setEnabled(button_stat.uninstall_enable);
      }


//---------------------------------------------------------
//   writePluginPackages
//---------------------------------------------------------
void ResourceManager::writePluginPackages()
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
      for (const QString &pkg : pluginDescriptionMap.keys()) {
            auto& v = pluginDescriptionMap.value(pkg);
            xml.stag("PluginPackage");
            xml.tag("pageURL", pkg);
            xml.tag("pkgName", v.package_name);
            xml.tag("source", v.source);
            xml.tag("directLink", v.direct_link);
            xml.tag("path", v.dir);
            xml.stag("qmlPath");
            for (const QString& path : v.qml_paths) {
                  xml.tag("qml", path);
                  }
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

//---------------------------------------------------------
//   readPluginPackages
//---------------------------------------------------------
bool ResourceManager::readPluginPackages()
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
                                    if (t == "pageURL")
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
                                    pluginDescriptionMap.insert(page_url, desc);
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

PluginWorker::PluginWorker(ResourceManager* r) : r(r)
      {
      QObject::connect(this, SIGNAL(pluginStatusChanged(int, PluginStatus)), r, SLOT(refreshPluginButton(int, PluginStatus)));
      }

PluginWorker::PluginWorker(PluginPackageDescription& desc, ResourceManager* r) : desc(desc), r(r)
      {
      QObject::connect(this, SIGNAL(pluginStatusChanged(int, PluginStatus)), r, SLOT(refreshPluginButton(int, PluginStatus)));
      }

void PluginWorker::checkUpdate(QPushButton* install)
      {
      const QString& page_url = install->property("page_url").toString();
      PluginPackageDescription* desc_tmp = new PluginPackageDescription(desc);
      PluginPackageDescription desc_backup = desc;
      install->setEnabled(false);
      install->setText("Checking for update…");
      bool should_update = analyzePluginPage("https://musescore.org" + page_url);
      desc = desc_backup;
      if (should_update) {
            desc.update = desc_tmp;
            r->commitPlugin(page_url, desc);
            emit pluginStatusChanged(install->property("row").toInt(), PluginStatus::UPDATE_AVAILABLE);
            }
      else {
            delete desc_tmp;
            emit pluginStatusChanged(install->property("row").toInt(), PluginStatus::UPDATED);
            }
      }

void PluginWorker::timeconsume()
      {
      qDebug("timeconsume start");
      int member = 0;
      for (int i = 0; i < 1000000; i++)
            member = member * 2 + 1;
      qDebug("timeconsume end");
      emit finished();
      }

bool PluginWorker::analyzePluginPage(QString full_url)
      {
      bool new_plugin = desc.source == UNKNOWN;
      QString direct_link;
      DownloadUtils page(QNetworkRequest::RedirectPolicy::NoLessSafeRedirectPolicy);
      page.setTarget(full_url);
      page.download();
      QByteArray html_raw = page.returnData();
      std::vector<PluginPackageLink> urls = getLinks(html_raw);
      // choose a link with the highest score
      PluginPackageLink target;
      int score = -2;
      for (auto& item : urls) {
            if (item.score > score) {
                  target = item;
                  score = item.score;
            }
      }
      if (score == -2)
            return false;
      if (target.source == GITHUB || target.source == GITHUB_RELEASE) {
            int release_id;
            bool has_release = false;
            if (target.source == GITHUB)
                  has_release = getLatestRelease(target.user, target.repo, release_id, direct_link);
            else
                  has_release = getLatestRelease(target.user, target.repo, release_id, direct_link, QFileInfo(target.url).completeBaseName());
            if (has_release) {
                  // there is a GitHub release, change source to RELEASE
                  if (desc.source != GITHUB_RELEASE || (desc.release_id != release_id && release_id > 0)) {
                        qDebug() << "desc source original:" << int(desc.source) << " now:release";
                        qDebug() << "release_id original:" << desc.release_id << "now: " << release_id;
                        desc.release_id = release_id;
                        desc.direct_link = direct_link;
                        desc.source = GITHUB_RELEASE;
                        return true;
                  }
                  else
                        // already up-to-date
                        return false;
            }
            QString branch;
            if (target.source == GITHUB_RELEASE)
                  // for archive links, the URL suffix is the branch name
                  branch = QFileInfo(target.url).completeBaseName();
            else
                  // fetch latest commit sha on master(usually for 3.x), as info for update
                  branch = target.branch.isNull() ? "master" : target.branch;
            QString sha = getLatestCommitSha(target.user, target.repo, branch);
            // compare the sha with previous stored sha. If the same, don't update.
            if (desc.source != GITHUB || (!sha.isEmpty() && desc.latest_commit != sha)) {
                  qDebug() << "desc source original:" << int(desc.source) << " now:github";
                  qDebug() << "commit sha original:" << desc.latest_commit << " now:" << sha;
                  desc.latest_commit = sha;
                  desc.direct_link = githubLatestArchiveURL(target.user, target.repo, "master");
                  desc.source = GITHUB;
                  return true;
            }
            else
                  return false;
      }

      else if (target.source == ATTACHMENT) {
            QDateTime date_time;
            if (desc.source != ATTACHMENT || desc.direct_link != target.url || (date_time = GetLastModified(target.url)) != desc.last_modified) {
                  qDebug() << "desc source original:" << int(desc.source) << " now:attachment";
                  qDebug() << "url before:" << desc.direct_link << " now:" << target.url;
                  desc.source = ATTACHMENT;
                  desc.direct_link = target.url;
                  if (!date_time.isValid())
                        // when it's a new plugin, we can always download and get timestamp later, so skip this step.
                        if (!new_plugin)
                              date_time = GetLastModified(target.url);
                  desc.last_modified = date_time;
                  return true;
            }
            else
                  return false;
      }
      else
            return false;
      }

QString PluginWorker::download(const QString& page_url)
      {
      DownloadUtils package(QNetworkRequest::RedirectPolicy::NoLessSafeRedirectPolicy);
      package.setTarget(desc.direct_link);
      // TODO: try to get extension name from direct_link, and add it to localPath
      QString localPath = dataPath + "/plugins/" + filenameBaseFromPageURL(page_url);
      if (desc.source == GITHUB || desc.source == GITHUB_RELEASE)
            localPath += ".zip";
      else if (desc.source == ATTACHMENT)
            localPath += "." + QFileInfo(desc.direct_link).suffix();
      QDir().mkpath(dataPath + "/plugins");
      package.setLocalFile(localPath);
      package.download();
      package.saveFile();
      // update the timestamp again
      desc.last_modified = package.getHeader(QNetworkRequest::LastModifiedHeader).toDateTime();
      return localPath;
      }

void PluginWorker::updateInstall()
      {

      }

bool PluginWorker::install(QString& download_pkg)
      {
      QFileInfo f_pkg(download_pkg);
      QString suffix = f_pkg.suffix().toLower();
      QString destination_prefix = dataPath + "/plugins" + "/" + f_pkg.completeBaseName();
      QDir().mkdir(destination_prefix);
      desc.dir = destination_prefix;
      if (suffix == "zip") {
            MQZipReader zipFile(download_pkg);
            QVector<MQZipReader::FileInfo> allFiles = zipFile.fileInfoList();
            if (allFiles.size() == 0)
                  return false;
            // If zip contains multiple files in root, or a single qml, create a
            // root folder in plugin dir for them.
            // If zip contains a single directory, don't use that directory's name
            bool has_no_dir = true;
            std::set<QString> dirs;
            for (MQZipReader::FileInfo fi : allFiles) {
                  QString dir_root = fi.filePath.split('/').first();
                  dirs.insert(dir_root);
                  }
            if (dirs.size() == 1)
                  if (allFiles.size() > 1) // the element in dirs must be a dir then
                        has_no_dir = false;
            int stripped_len = 0;
            if (!has_no_dir) {
                  stripped_len = allFiles.first().filePath.length() + 1;
                  // the directory in the archive is not to be added
                  allFiles.pop_front();
                  }
            auto filePathStripper = [&](QString& filePath) {return filePath.right(filePath.size() - stripped_len); };

            // extract and copy
            for (MQZipReader::FileInfo fi : allFiles) {
                  if (fi.isDir)
                        QDir().mkdir(destination_prefix + "/" + filePathStripper(fi.filePath));
                  else if (fi.isFile) {
                        if (QFileInfo(fi.filePath).suffix().toLower() == "qml")
                              desc.qml_paths.push_back(destination_prefix + "/" + filePathStripper(fi.filePath));
                        QFile new_f(destination_prefix + "/" + filePathStripper(fi.filePath));
                        if (!new_f.open(QIODevice::WriteOnly)) {
                              qDebug("Cannot write the file.");
                              return false;
                              }
                        new_f.write(zipFile.fileData(fi.filePath));
                        new_f.setPermissions(fi.permissions);
                        new_f.close();
                        }
                  }
            zipFile.close();
            }
      else if (suffix == "qml") {
            QString destination = destination_prefix + "/" + f_pkg.fileName();
            QFileInfo check(destination);
            if (check.exists()) {
                  if (check.isDir())
                        QDir(check.absoluteFilePath()).removeRecursively();
                  if (check.isFile())
                        QFile::remove(check.absoluteFilePath());
                  }
            QFile(f_pkg.absoluteFilePath()).copy(destination);
            desc.qml_paths.push_back(destination);
            }
      else {
            qDebug("Unknown plugin suffix %s.", qPrintable(suffix));
            return false;
            }
      QFile::remove(download_pkg);
      return true;
      }

void PluginWorker::downloadInstall(QPushButton* button)
      {
      button->setEnabled(false);
      button->setText(tr("Analyzing…"));
      QString page_url = button->property("page_url").toString();
      desc.package_name = button->property("name").toString();
      analyzePluginPage("https://musescore.org" + page_url);
      if (desc.source == UNKNOWN) {
            button->setText(tr("Unknown plugin source."));
            button->setEnabled(true);
            return;
            }
      button->setText(tr("Downloading…"));
      QString localPath = download(page_url);

      if (install(localPath)) {
            // add the new description item to the map
            r->commitPlugin(page_url, desc);
            emit pluginStatusChanged(button->property("row").toInt(), PluginStatus::UPDATED);
            
      }
      else {
            emit pluginStatusChanged(button->property("row").toInt(), PluginStatus::INSTALL_FAILED);
            return;
      }
      }

}

