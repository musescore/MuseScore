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
#include "preferences.h"
#include "resourceManager.h"
#include "downloadUtils.h"
#include "plugin/pluginUpdater.h"
#include "qmlpluginengine.h"
#include "thirdparty/qzip/qzipreader_p.h"
#include "ui_resourceManager.h"

namespace Ms {

static int PluginStatus_id = qRegisterMetaType<PluginStatus>("PluginStatus");

QThreadPool ResourceManager::workerThreads;

static inline std::tuple<bool, bool, bool> compatFromString(const QString& raw) {
      return std::make_tuple(raw.contains("1.x"), raw.contains("2.x"), raw.contains("3.x"));
      }

//---------------------------------------------------------
//   GitHub utility functions
//---------------------------------------------------------
static inline QString githubReleaseAPI(const QString &user, const QString &repo) {
      return "https://api.github.com/repos/" + user + "/" + repo + "/releases";
      }
static inline QString githubCommitAPI(const QString &user, const QString &repo, const QString &ref = "") {
      return "https://api.github.com/repos/" + user + "/" + repo + "/commits" + (ref.isEmpty() ? "" : ('/' + ref));
      }
static inline QString githubLatestArchiveURL(const QString &user, const QString &repo, const QString &branch = "master") {
      //return "https://api.github.com/repos/" + user + "/" + repo + "/zip/" + branch;
      return "https://github.com/" + user + '/' + repo + "/archive/" + branch + ".zip";
      }
static QString getLatestCommitSha(const QString& user, const QString &repo, const QString &branch = "master") {
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
static bool isCompatibleRelease(const QString& branch) {
      if (branch == "master" || branch == "3.x") return true;
      if (branch.contains("2.x") || branch.contains("version2") || branch.contains("musescore2") || branch.contains("musescore 2")) return false;
      if (branch.contains("3.x") || branch.contains("version3") || branch.contains("musescore3") || branch.contains("musescore 3")) return true;
      if (branch.contains("2") && !branch.contains("3")) return false;
      return true;
      }
static int CompatEstimate(QString branch) {
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

static inline bool isNotFoundMessage(const QJsonObject& json_obj) {
      return json_obj["message"].toString() == "Not Found";
      }
static inline QString filenameBaseFromPageURL(QString page_url) {
      Q_ASSERT(page_url.contains("/project/"));
      if (page_url.contains("/en/project/"))
            return page_url.remove("/en/project/");
      return page_url.remove("/project/");
      }
static inline QString getExtFromURL(QString& direct_link) {
      QString possible_ext = direct_link.split(".").last();
      // We can assume that most extensions contain only 
      // alphanumeric and plus "_", and other connector punctuation chars
      if (possible_ext.contains(QRegularExpression("\\W")))
            return {};
      return possible_ext;
      }

//---------------------------------------------------------
//   parsePluginRepo
//---------------------------------------------------------

void ResourceManager::parsePluginRepo(QByteArray html_raw)
      {
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
      pluginsTable->clearSpans();
      pluginsTable->setRowCount(tr_list.length());
      pluginsTable->setItem(0, 0, nullptr);
      QStringList all_categories;
      for (int row = 0; row < tr_list.length(); row++) {
            int col = 0;
            QDomElement tr_node = tr_list.item(row).toElement();
            QDomNodeList td_list = tr_node.elementsByTagName("td");
            Q_ASSERT(td_list.length() == 4);
            QString page_url = td_list.item(0).toElement().firstChildElement("a").attribute("href");
            QString name = td_list.item(0).toElement().text();
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
            plugin_name->setWordWrap(true);
            pluginsTable->setIndexWidget(pluginsTable->model()->index(row, col++), plugin_name); // column 0
            pluginsTable->setItem(row, col++, new QTableWidgetItem(td_list.item(2).toElement().text())); // column 1
            if (page_url.isNull()) {
                  pluginsTable->setItem(row,col,new QTableWidgetItem(tr("No page URL found.")));
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
            bool installed = mscore->getPluginManager()->isPackageInstalled(page_url);
            refreshPluginButton(row, installed ? PluginStatus::UPDATED : PluginStatus::NOT_INSTALLED);
            }
      categories->insertItems(0, all_categories);
      scanPluginUpdate();
      }

//---------------------------------------------------------
//   filterPluginList
//---------------------------------------------------------

void ResourceManager::filterPluginList()
      {
      const QString category = categories->currentText();
      const QString& search = lineEdit->text();
      for (int i = 0; i < pluginsTable->rowCount(); i++) {
            // locate the "Install" button
            QPushButton* install = static_cast<QPushButton*>(pluginsTable->indexWidget(pluginsTable->model()->index(i, 2)));
            const QString& plugin_name = install->property("name").toString();
            const QStringList& category_list = install->property("categories").toStringList();
            if (plugin_name.contains(search, Qt::CaseInsensitive) && (category == "Any" || category_list.contains(category)))
                  pluginsTable->showRow(i);
            else
                  pluginsTable->hideRow(i);
            }
      }

//---------------------------------------------------------
//   getAttachments
//   Find musescore.com attachment links from HTML contents
//   - localHint used to calculate score for each link
//---------------------------------------------------------

static std::vector<PluginPackageLink> getAttachments(const QString& html_raw, void(*localHint)(QRegularExpressionMatch&, PluginPackageLink&, const QString&))
      {
      std::vector<PluginPackageLink> file_urls;
#if 0
      // This is the code that fetches the attachment table that appears at the bottom of the page.
      // Now I find it more attracting to search for inline attachment urls instead.
      int start_idx = html_raw.indexOf("<div class=\"field field--name-upload field--type-file field--label-above\">");
      if (start_idx == -1) {
            qDebug(tr("No attachment found."));
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

//---------------------------------------------------------
//   getGitHubLinks
//   Find GitHub links from HTML contents.
//---------------------------------------------------------

static std::vector<PluginPackageLink> getGitHubLinks(const QString& html_raw, void(*localHint)(QRegularExpressionMatch&, PluginPackageLink&, const QString&))
      {
      std::vector<PluginPackageLink> github_urls;
      static const QRegularExpression github_repo_patt("<a href=\"(https?://github.com/([\\w\\-]+)/([\\w\\-]+))\".*?>(?<text>.*?)</a>");
      static const QRegularExpression github_archive_link("<a href=\"(https?://github.com/([\\w\\-]+)/([\\w\\-]+)/(archive|zipball|tarball)/(.+?))\".*?>(?<text>.*?)</a>"); // downloadable link
      static const QRegularExpression github_directory_patt("<a href=\"(https?://github.com/([\\w\\-]+)/([\\w\\-]+)/(tree|blob)/([\\w\\-]+).*?)\".*?>(?<text>.*?)</a>"); // with branch info
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

//---------------------------------------------------------
//   getLinks
//---------------------------------------------------------

static std::vector<PluginPackageLink> getLinks(const QByteArray& html_raw_array)
      {
      QString html_raw(html_raw_array);
      auto GetLocalHint = [](QRegularExpressionMatch& match, PluginPackageLink& link, const QString& html_raw) {
            // `match` must have a group called `text`
            int start_idx = match.capturedStart(0);
            int end_idx = match.capturedEnd();
            int newline_start = start_idx, newline_end = end_idx;
            while (newline_start > 0 && html_raw[newline_start - 1] != '\n')
                  newline_start--;
            while (newline_end < html_raw.length() && html_raw[newline_end] != '\n')
                  newline_end++;
            link.newline_index = newline_start;
            link.curr_line = html_raw.mid(newline_start, newline_end - newline_start + 1);
            // use hyper link text or the whole line as the hint
            link.score = std::max(CompatEstimate(match.captured("text")), CompatEstimate(link.curr_line));
            };
      std::vector<PluginPackageLink> attachments = getAttachments(html_raw, GetLocalHint);
      std::vector<PluginPackageLink> github_links = getGitHubLinks(html_raw, GetLocalHint);
      std::vector<PluginPackageLink> urls;
      urls.insert(urls.begin(), attachments.begin(), attachments.end());
      urls.insert(urls.begin(), github_links.begin(), github_links.end());
      std::set<int> begin_idx;
      for (auto& url : urls)
            begin_idx.insert(url.newline_index);
      // now, get more hints from the line above each link, and update the score if we have clearer hints
      for (size_t i = 0; i < urls.size(); i++) {
            auto& link = urls[i];
            if (link.score >= -1) {
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

//---------------------------------------------------------
//   scanPluginUpdate
//---------------------------------------------------------

void ResourceManager::scanPluginUpdate()
      {
      for (int row = 0; row < pluginsTable->rowCount(); row++) {
            QPushButton* install = static_cast<QPushButton*>(pluginsTable->indexWidget(pluginsTable->model()->index(row, 2)));
            const QString& page_url = install->property("page_url").toString();
            // if installed
            if (mscore->getPluginManager()->isPackageInstalled(page_url)) {
                  install->setEnabled(false);
                  install->setText(tr("Pending…"));
                  PluginWorker* worker = new PluginWorker(mscore->getPluginManager()->getPackageDescription(page_url), this);
                  QtConcurrent::run(&workerThreads, worker, &PluginWorker::checkUpdate, install);
                  }
            }
      }

//---------------------------------------------------------
//   updatePlugin
//---------------------------------------------------------
void ResourceManager::updatePlugin()
      {
      QPushButton* button = static_cast<QPushButton*>(sender());
      const PluginPackageDescription& desc = mscore->getPluginManager()->getPackageDescription(button->property("page_url").toString());
      button->setEnabled(false);
      button->setText(tr("Pending…"));
      QPushButton* uninstall = static_cast<QPushButton*>(pluginsTable->indexWidget(pluginsTable->model()->index(button->property("row").toInt(), 3)));
      uninstall->setEnabled(false);
      PluginWorker* worker = new PluginWorker(desc, this);
      QtConcurrent::run(&workerThreads, worker, &PluginWorker::updateInstall, button);
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
      QtConcurrent::run(&workerThreads, worker, &PluginWorker::downloadInstall, button);
      }

//---------------------------------------------------------
//   uninstallPluginPackage
//---------------------------------------------------------

void ResourceManager::uninstallPluginPackage()
      {
      QPushButton* button = static_cast<QPushButton*>(sender());
      const QString& url = button->property("page_url").toString();
      if (!mscore->getPluginManager()->uninstallPluginPackage(url)) {
            button->setText(tr("Uninstall failed. Try again"));
            return;
            }
      refreshPluginButton(button->property("row").toInt(), PluginStatus::NOT_INSTALLED);
      }

static const std::map<PluginStatus, PluginButtonStatus> buttonStatuses = {
      {NOT_INSTALLED, {QObject::tr("Install"), true, false}},
      {INSTALL_FAILED, {QObject::tr("Install failed. Try again"), true, false}},
      {INSTALL_FAILED_INVALID, {QObject::tr("Install failed. Invalid plugin."), false, false}},
      {ANALYZING, {QObject::tr("Analyzing…"), false, false}},
      {ANALYZE_FAILED, {QObject::tr("Analyze failed. Try again"), true, false}},
      {DOWNLOADING, {QObject::tr("Downloading…"), false, false}},
      {DOWNLOAD_FAILED, {QObject::tr("Download failed. Try again"), true, false}},
      {UPDATED, {QObject::tr("Updated"), false, true}},
      {UPDATE_AVAILABLE, {QObject::tr("Update"), true, true}}
      };

std::map<PluginPackageSource, QString> PluginPackageSourceVerboseStr{
      {UNKNOWN, QObject::tr("Unknown")},
      {GITHUB, QObject::tr("GitHub")},
      {GITHUB_RELEASE, QObject::tr("GitHub Release")},
      {ATTACHMENT, QObject::tr("Attachment")}
};

//---------------------------------------------------------
//   refreshPluginButton
//---------------------------------------------------------

void ResourceManager::refreshPluginButton(int row, PluginStatus status)
      {
      QPushButton* install = static_cast<QPushButton*>(pluginsTable->indexWidget(pluginsTable->model()->index(row, 2)));
      QPushButton* uninstall = static_cast<QPushButton*>(pluginsTable->indexWidget(pluginsTable->model()->index(row, 3)));
      Q_ASSERT(buttonStatuses.find(status) != buttonStatuses.end());
      const PluginButtonStatus& button_stat = buttonStatuses.at(status);
      uninstall->setText(tr("Uninstall"));
      install->setText(button_stat.display_text);
      install->setEnabled(button_stat.install_enable);
      install->disconnect();
      if (status == UPDATE_AVAILABLE)
            QObject::connect(install, SIGNAL(clicked()), this, SLOT(updatePlugin()));
      else if(button_stat.install_enable)
            QObject::connect(install, SIGNAL(clicked()), this, SLOT(downloadInstallPlugin()));
      uninstall->setEnabled(button_stat.uninstall_enable);
      }


PluginWorker::PluginWorker(ResourceManager* r) : r(r)
      {
      QObject::connect(this, SIGNAL(extensionMetaAvailable(QByteArray)), r, SLOT(parseExtensions(QByteArray)));
      QObject::connect(this, SIGNAL(languageMetaAvailable(QByteArray)), r, SLOT(parseLanguages(QByteArray)));
      QObject::connect(this, SIGNAL(pluginRepoAvailable(QByteArray)), r, SLOT(parsePluginRepo(QByteArray)));
      QObject::connect(this, SIGNAL(pluginStatusChanged(int, PluginStatus)), r, SLOT(refreshPluginButton(int, PluginStatus)));
      QObject::connect(r, &QDialog::finished, this, &PluginWorker::detached, Qt::DirectConnection);
      QObject::connect(this, SIGNAL(pluginInstalled(const QString, PluginPackageDescription*)), mscore->getPluginManager(), SLOT(commitPlugin(const QString, PluginPackageDescription*)));
      QObject::connect(this, SIGNAL(updateAvailable(const QString, PluginPackageDescription*)), mscore->getPluginManager(), SLOT(updatePluginPackage(const QString, PluginPackageDescription*)));
      }

PluginWorker::PluginWorker(const PluginPackageDescription& desc, ResourceManager* r) : desc(desc), r(r)
      {
      QObject::connect(this, SIGNAL(extensionMetaAvailable(QByteArray)), r, SLOT(parseExtensions(QByteArray)));
      QObject::connect(this, SIGNAL(languageMetaAvailable(QByteArray)), r, SLOT(parseLanguages(QByteArray)));
      QObject::connect(this, SIGNAL(pluginRepoAvailable(QByteArray)), r, SLOT(parsePluginRepo(QByteArray)));
      QObject::connect(this, SIGNAL(pluginStatusChanged(int, PluginStatus)), r, SLOT(refreshPluginButton(int, PluginStatus)));
      QObject::connect(r, &QDialog::finished, this, &PluginWorker::detached, Qt::DirectConnection);
      QObject::connect(this, SIGNAL(pluginInstalled(const QString, PluginPackageDescription*)), mscore->getPluginManager(), SLOT(commitPlugin(const QString, PluginPackageDescription*)));
      QObject::connect(this, SIGNAL(updateAvailable(const QString, PluginPackageDescription*)), mscore->getPluginManager(), SLOT(updatePluginPackage(const QString, PluginPackageDescription*)));
      }

//---------------------------------------------------------
//   fetchExtensions
//---------------------------------------------------------

void PluginWorker::fetchExtensions()
      {
      DownloadUtils js;
      js.setTarget(ResourceManager::baseAddr() + "extensions/details.json");
      QObject::connect(r, &QDialog::finished, &js, &DownloadUtils::cancel);
      js.download();
      QByteArray json = js.returnData();
      emit extensionMetaAvailable(json);
      deleteLater();
      }

//---------------------------------------------------------
//   fetchLanguages
//---------------------------------------------------------

void PluginWorker::fetchLanguages()
      {
      // Download details.json
      DownloadUtils js;
      js.setTarget(ResourceManager::baseAddr() + "languages/details.json");
      QObject::connect(r, &QDialog::finished, &js, &DownloadUtils::cancel);
      js.download();
      QByteArray json = js.returnData();
      qDebug() << json;
      emit languageMetaAvailable(json);
      deleteLater();
      }

//---------------------------------------------------------
//   fetchPluginRepo
//---------------------------------------------------------

void PluginWorker::fetchPluginRepo()
      {
      // fetch plugin list from web
      DownloadUtils html;
      html.setTarget(ResourceManager::pluginRepoAddr());
      QObject::connect(r, &QDialog::finished, &html, &DownloadUtils::cancel);
      html.download();
      QByteArray html_raw = html.returnData();
      emit pluginRepoAvailable(html_raw);
      deleteLater();
      }

//---------------------------------------------------------
//   checkUpdate
//---------------------------------------------------------

void PluginWorker::checkUpdate(QPushButton* install)
      {
      const QString& page_url = install->property("page_url").toString();
      int pluginRow = install->property("row").toInt();
      PluginPackageDescription* desc_tmp = new PluginPackageDescription(desc);
      PluginPackageDescription desc_backup = desc;
      install->setEnabled(false);
      install->setText(tr("Checking for update…"));
      bool should_update = analyzePluginPage("https://musescore.org" + page_url);
      // very hack here. should be improved.
      *desc_tmp = desc;
      desc = desc_backup;
      // if the user deletes the plugin when checking for update
      if (!mscore->getPluginManager()->isPackageInstalled(page_url))
            return;
      if (should_update) {
            desc.update = desc_tmp;
            emit updateAvailable(page_url, new PluginPackageDescription(desc));
            emit pluginStatusChanged(pluginRow, PluginStatus::UPDATE_AVAILABLE);
            }
      else {
            delete desc_tmp;
            emit pluginStatusChanged(pluginRow, PluginStatus::UPDATED);
            }
      deleteLater();
      }

//---------------------------------------------------------
//   detached
//---------------------------------------------------------

void PluginWorker::detached()
      {
      r = nullptr;
      }

static QString getPackageDescriptionText(const QString& html_raw) {
      static const QRegularExpression start("<div class=\"field field--name-body field--type-text-with-summary field--label-hidden field__item\">");
      static const QRegularExpression end("<p><a href=\"/.*?project/issues/[\\w\\-]+\">Issue Tracker</a></p>");
      auto startMatch = start.match(html_raw);
      if (!startMatch.hasMatch())
            return QString();
      int startIdx = startMatch.capturedEnd(0);
      auto endMatch = end.match(html_raw);
      if (!endMatch.hasMatch())
            return QString();
      int endIdx = endMatch.capturedStart(0);
      QString desc = html_raw.mid(startIdx, endIdx - startIdx).trimmed();
      // remove </div>
      if (desc.endsWith("</div>"))
            desc = desc.left(desc.size() - QString("</div>").size());
      static const QRegularExpression localHref("(<a href=\")(/.*?\")");
      desc.replace(localHref, "\\1https://musescore.org\\2");
      return desc;
}

//---------------------------------------------------------
//   analyzePluginPage
//---------------------------------------------------------

bool PluginWorker::analyzePluginPage(QString full_url)
      {
      bool new_plugin = desc.source == UNKNOWN;
      QString direct_link;
      DownloadUtils page(QNetworkRequest::RedirectPolicy::NoLessSafeRedirectPolicy);
      page.setTarget(full_url);
      page.download();
      QByteArray html_raw = page.returnData();
      std::vector<PluginPackageLink> urls = getLinks(html_raw);
      QString desc_text = getPackageDescriptionText(QString(html_raw));
      desc.desc_text = desc_text;
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
            else {
                  // fetch latest commit sha on master(usually for 3.x), as info for update
                  branch = target.branch.isNull() ? "master" : target.branch;
                  }
            QString sha = getLatestCommitSha(target.user, target.repo, branch);
            // compare the sha with previous stored sha. If the same, don't update.
            if (desc.source != GITHUB || (!sha.isEmpty() && desc.latest_commit != sha)) {
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

//---------------------------------------------------------
//   download
//---------------------------------------------------------

QString PluginWorker::download(const QString& page_url, bool update/* = false*/)
      {
      PluginPackageDescription* src = update ? desc.update : &desc;
      DownloadUtils package(QNetworkRequest::RedirectPolicy::NoLessSafeRedirectPolicy);
      package.setTarget(src->direct_link);
      QString localPath = QFileInfo(preferences.getString(PREF_APP_PATHS_MYPLUGINS), filenameBaseFromPageURL(page_url)).absoluteFilePath();
      if (src->source == GITHUB || src->source == GITHUB_RELEASE)
            localPath += ".zip";
      else if (src->source == ATTACHMENT)
            localPath += "." + QFileInfo(src->direct_link).suffix();
      package.setLocalFile(localPath);
      package.download();
      QDir().mkpath(preferences.getString(PREF_APP_PATHS_MYPLUGINS));
      package.saveFile();
      // update the timestamp again
      src->last_modified = package.getHeader(QNetworkRequest::LastModifiedHeader).toDateTime();
      return localPath;
      }

//---------------------------------------------------------
//   updateInstall
//   Triggered when "Update" is clicked
//---------------------------------------------------------

void PluginWorker::updateInstall(QPushButton* button)
      {
      const QString& page_url = button->property("page_url").toString();
      const int buttonRow = button->property("row").toInt();
      Q_ASSERT(desc.update != nullptr);
      if (desc.update->source == UNKNOWN) {
            button->setText(tr("Unknown plugin source."));
            return;
            }
      emit pluginStatusChanged(buttonRow, PluginStatus::DOWNLOADING);
      QString localPath = download(page_url, true);
      PluginStatus res;
      if (install(localPath, &res)) {
            // add the new description item to the map
            PluginPackageDescription* p_update = desc.update;
            desc = *desc.update;
            desc.update = nullptr;
            delete p_update;
            emit pluginInstalled(page_url, new PluginPackageDescription(desc));
            emit pluginStatusChanged(buttonRow, PluginStatus::UPDATED);
            }
      else
            emit pluginStatusChanged(buttonRow, res);
      deleteLater();
      }

//---------------------------------------------------------
//   install
//---------------------------------------------------------

bool PluginWorker::install(QString& download_pkg, PluginStatus* result/*= nullptr*/)
      {
      QFileInfo f_pkg(download_pkg);
      QString suffix = f_pkg.suffix().toLower();
      QString destination_prefix = QFileInfo(preferences.getString(PREF_APP_PATHS_MYPLUGINS), f_pkg.completeBaseName()).absoluteFilePath();
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
            bool has_qml = false;
            std::set<QString> dirs;
            for (MQZipReader::FileInfo fi : allFiles) {
                  QString dir_root = fi.filePath.split('/').first();
                  dirs.insert(dir_root);
                  if (fi.isFile && QFileInfo(fi.filePath).suffix() == "qml")
                        has_qml = true;
                  }
            if (!has_qml) {
                  // invalid plugin
                  if (result)
                        *result = INSTALL_FAILED_INVALID;
                  QFile::remove(download_pkg);
                  return false;
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
                  if (fi.filePath.startsWith("__MACOSX"))
                        continue;
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
//   downloadInstall
//   Triggered when "Install" is clicked.
//---------------------------------------------------------

void PluginWorker::downloadInstall(QPushButton* button)
      {
      button->setEnabled(false);
      const QString page_url = button->property("page_url").toString();
      const int buttonRow = button->property("row").toInt();
      emit pluginStatusChanged(buttonRow, PluginStatus::ANALYZING);
      desc.package_name = button->property("name").toString();
      analyzePluginPage("https://musescore.org" + page_url);
      if (desc.source == UNKNOWN) {
            button->setText(tr("Unknown plugin source."));
            button->setEnabled(true);
            return;
            }
      if (r)
            emit pluginStatusChanged(buttonRow, PluginStatus::DOWNLOADING);
      QString localPath = download(page_url);
      PluginStatus result;
      if (install(localPath, &result)) {
            // add the new description item to the map
            emit pluginInstalled(page_url, new PluginPackageDescription(desc));
            emit pluginStatusChanged(buttonRow, PluginStatus::UPDATED);
            }
      else
            emit pluginStatusChanged(buttonRow, result);
      deleteLater();
      }

}

