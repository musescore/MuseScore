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

#include "resourceManager.h"
#include "musescore.h"
#include "extension.h"
#include "libmscore/utils.h"
#include "stringutils.h"
#include "ui_resourceManager.h"
#include "thirdparty/qzip/qzipreader_p.h"

namespace Ms {

extern QString dataPath;
extern QString mscoreGlobalShare;
extern QString localeName;

ResourceManager::ResourceManager(QWidget *parent) :
      QDialog(parent)
      {
      setObjectName("ResourceManager");
      setupUi(this);
      
#ifndef NDEBUG
      QPushButton* check_update = new QPushButton();
      check_update->setText("Check Update");
      bottom_layout->addWidget(check_update);
      connect(check_update, SIGNAL(clicked()), this, SLOT(scanPluginUpdate()));
#endif // !NDEBUG

      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      QDir dir;
      dir.mkpath(dataPath + "/locale");
      displayExtensions();
      displayLanguages();
      displayPluginRepo();
      displayPlugins();
      QObject::connect(lineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(filterPluginList()));
      QObject::connect(categories, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ResourceManager::filterPluginList);
      languagesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
      languagesTable->verticalHeader()->hide();
      extensionsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
      extensionsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
      extensionsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
      extensionsTable->verticalHeader()->hide();
      extensionsTable->setColumnWidth(1, 50);
      extensionsTable->setColumnWidth(1, 100);
      pluginsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
      pluginsTable->verticalHeader()->hide();
      MuseScore::restoreGeometry(this);
      }

//---------------------------------------------------------
//   ExtensionFileSize
//---------------------------------------------------------

ExtensionFileSize::ExtensionFileSize(const int i)
   : QTableWidgetItem(stringutils::convertFileSizeToHumanReadable(i), QTableWidgetItem::UserType)
     , _size(i)
      {}

//---------------------------------------------------------
//   operator<
//---------------------------------------------------------

bool ExtensionFileSize::operator<(const QTableWidgetItem& nextItem) const
      {
      if (nextItem.type() != type())
            return false;
      return getSize() < static_cast<const ExtensionFileSize&>(nextItem).getSize();
      }

//---------------------------------------------------------
//   LanguageFileSize
//---------------------------------------------------------

LanguageFileSize::LanguageFileSize(const double d)
   : QTableWidgetItem(ResourceManager::tr("%1 KB").arg(d), QTableWidgetItem::UserType)
     , _size(d)
      {}

//---------------------------------------------------------
//   operator<
//---------------------------------------------------------

bool LanguageFileSize::operator<(const QTableWidgetItem& nextItem) const
      {
      if (nextItem.type() != type())
            return false;
      return getSize() < static_cast<const LanguageFileSize&>(nextItem).getSize();
      }

//---------------------------------------------------------
//   selectLanguagesTab
//---------------------------------------------------------

void ResourceManager::selectLanguagesTab()
      {
      tabs->setCurrentIndex(tabs->indexOf(languages));
      }

//---------------------------------------------------------
//   selectExtensionsTab
//---------------------------------------------------------

void ResourceManager::selectExtensionsTab()
      {
      tabs->setCurrentIndex(tabs->indexOf(extensions));
      }


//---------------------------------------------------------
//   displayExtensions
//---------------------------------------------------------

void ResourceManager::displayExtensions()
      {
      DownloadUtils js(this);
      js.setTarget(baseAddr() + "extensions/details.json");
      js.download();
      QByteArray json = js.returnData();

      // parse the json file
      QJsonParseError err;
      QJsonDocument result = QJsonDocument::fromJson(json, &err);
      if (err.error != QJsonParseError::NoError || !result.isObject()) {
            qDebug("An error occurred during parsing");
            return;
            }
      int rowCount = result.object().keys().size();
      rowCount -= 2; //version and type
      extensionsTable->setRowCount(rowCount);

      int row = 0;
      int col = 0;
      QPushButton* buttonInstall;
      QPushButton* buttonUninstall;
      extensionsTable->verticalHeader()->show();

      QStringList exts = result.object().keys();
      for (QString key : exts) {
            if (!result.object().value(key).isObject())
                  continue;
            QJsonObject value = result.object().value(key).toObject();
            col = 0;
            QString test = value.value("file_name").toString();
            if (test.length() == 0)
                  continue;

            QString filename = value.value("file_name").toString();
            QString name = value.value("name").toString();
            int fileSize = value.value("file_size").toInt();
            QString hashValue = value.value("hash").toString();
            QString version = value.value("version").toString();

            extensionsTable->setItem(row, col++, new QTableWidgetItem(name));
            extensionsTable->setItem(row, col++, new QTableWidgetItem(version));
            extensionsTable->setItem(row, col++, new ExtensionFileSize(fileSize));
            buttonInstall = new QPushButton(tr("Install"));
            buttonUninstall = new QPushButton(tr("Uninstall"));

            connect(buttonInstall, SIGNAL(clicked()), this, SLOT(downloadExtension()));
            connect(buttonUninstall, SIGNAL(clicked()), this, SLOT(uninstallExtension()));
            buttonInstall->setProperty("path", "extensions/" + filename);
            buttonInstall->setProperty("hash", hashValue);
            buttonInstall->setProperty("rowId", row);
            buttonUninstall->setProperty("extensionId", key);
            buttonUninstall->setProperty("rowId", row);

            // get the installed version of the extension if any
            if (Extension::isInstalled(key)) {
                  buttonUninstall->setDisabled(false);
                  QString installedVersion = Extension::getLatestVersion(key);
                  if (compareVersion(installedVersion, version)) {
                        buttonInstall->setText(tr("Update"));
                        }
                  else {
                        buttonInstall->setText(tr("Updated"));
                        buttonInstall->setDisabled(true);
                        }
                  }
            else {
                  buttonUninstall->setDisabled(true);
                  }
            extensionsTable->setIndexWidget(extensionsTable->model()->index(row, col++), buttonInstall);
            extensionsTable->setIndexWidget(extensionsTable->model()->index(row, col++), buttonUninstall);
            row++;
            }
      }

//---------------------------------------------------------
//   displayLanguages
//---------------------------------------------------------

void ResourceManager::displayLanguages()
      {
      // Download details.json
      DownloadUtils js(this);
      js.setTarget(baseAddr() + "languages/details.json");
      js.download();
      QByteArray json = js.returnData();
      qDebug() << json;

      // parse the json file
      QJsonParseError err;
      QJsonDocument result = QJsonDocument::fromJson(json, &err);
      if (err.error != QJsonParseError::NoError || !result.isObject()) {
            qDebug("An error occurred during parsing");
            return;
            }
      int rowCount = result.object().keys().size();
      rowCount -= 2; //version and type
      languagesTable->setRowCount(rowCount);

      int row = 0;
      int col = 0;
#if (!defined (_MSCVER) && !defined (_MSC_VER))
      QPushButton* updateButtons[rowCount];
#else
      // MSVC does not support VLA. Replace with std::vector. If profiling determines that the
      //    heap allocation is slow, an optimization might be used.
      std::vector<QPushButton*> updateButtons(rowCount);
#endif
      QPushButton* temp;
      languagesTable->verticalHeader()->show();

      // move current language to first row
      QStringList langs = result.object().keys();
      QString lang = mscore->getLocaleISOCode();
      int index = langs.indexOf(lang);
      if (index < 0 && lang.size() > 2) {
            lang = lang.left(2);
            index = langs.indexOf(lang);
            }
      if (index >= 0) {
            QString l = langs.takeAt(index);
            langs.prepend(l);
            }

      for (QString key : langs) {
            if (!result.object().value(key).isObject())
                  continue;
            QJsonObject value = result.object().value(key).toObject();
            col = 0;
            QString test = value.value("file_name").toString();
            if (test.length() == 0)
                  continue;

            QString filename = value.value("file_name").toString();
            QString name = value.value("name").toString();
            double fileSize = value.value("file_size").toString().toDouble();
            QString hashValue = value.value("hash").toString();

            languagesTable->setItem(row, col++, new QTableWidgetItem(name));
            languagesTable->setItem(row, col++, new QTableWidgetItem(filename));
            languagesTable->setItem(row, col++, new LanguageFileSize(fileSize));
            updateButtons[row] = new QPushButton(tr("Update"));

            temp = updateButtons[row];
            languageButtonMap[temp] = "languages/" + filename;
            languageButtonHashMap[temp] = hashValue;

            languagesTable->setIndexWidget(languagesTable->model()->index(row, col++), temp);

            // get hash mscore and instruments
            QJsonObject mscoreObject = value.value("mscore").toObject();
            QString hashMscore = mscoreObject.value("hash").toString();
            QString filenameMscore = mscoreObject.value("file_name").toString();

            bool verifyMScore = verifyLanguageFile(filenameMscore, hashMscore);

            QJsonObject instrumentsObject = value.value("instruments").toObject();
            QString hashInstruments = instrumentsObject.value("hash").toString();
            QString filenameInstruments = instrumentsObject.value("file_name").toString();

            bool verifyInstruments = verifyLanguageFile(filenameInstruments, hashInstruments);

            QJsonObject toursObject = value.value("tours").toObject();
            QString hashTours = toursObject.value("hash").toString();
            QString filenameTours = toursObject.value("file_name").toString();

            bool verifyTours = verifyLanguageFile(filenameTours, hashTours);

            if (verifyMScore && verifyInstruments && verifyTours) { // compare local file with distant hash
                  temp->setText(tr("Updated"));
                  temp->setDisabled(1);
                  }
            else {
                  connect(temp, SIGNAL(clicked()), this, SLOT(downloadLanguage()));
                  }
            row++;
            }
      }

static inline std::tuple<bool, bool, bool> compatFromString(const QString& raw) {
      return std::make_tuple(raw.contains("1.x"), raw.contains("2.x"), raw.contains("3.x"));
      }

//---------------------------------------------------------
//   displayPluginRepo
//---------------------------------------------------------
void ResourceManager::displayPluginRepo() {
      // fills pluginDescriptionMap from xml
      readPluginPackages();
      // fetch plugin list from web
      DownloadUtils html(this);
      html.setTarget(pluginAddr());
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
            refreshPluginButton(row);
            }
      categories->insertItems(0,all_categories);
      }

void ResourceManager::filterPluginList()
      {
      const QString category = categories->currentText();
      const QString& search = lineEdit->text();
      for (int i = 0; i < pluginsTable->rowCount(); i++) {
            // locate the "Install" button
            QPushButton* install = static_cast<QPushButton*>(pluginsTable->indexWidget(pluginsTable->model()->index(i, 3)));
            const QString& plugin_name = install->property("name").toString();
            const QStringList& categories = install->property("categories").toStringList();
            if (plugin_name.contains(search, Qt::CaseInsensitive) && (category == "Any" || categories.contains(category)))
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

//---------------------------------------------------------
//   verifyLanguageFile
//---------------------------------------------------------

bool ResourceManager::verifyLanguageFile(QString filename, QString hash)
      {
      QString local = dataPath + "/locale/" + filename;
      QString global = mscoreGlobalShare + "locale/" + filename;
      QFileInfo fileLocal(local);
      QFileInfo fileGlobal(global);
      if (!fileLocal.exists() || (fileLocal.lastModified() <= fileGlobal.lastModified()))
            local = mscoreGlobalShare + "locale/" + filename;

      return verifyFile(local, hash);
      }

//---------------------------------------------------------
//   GitHub utility functions
//---------------------------------------------------------
static inline QString githubLatestReleaseAPI(const QString &user, const QString &repo) { return "https://api.github.com/repos/" + user + "/" + repo + "/releases"; }
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
      if (branch == "master" || branch=="3.x") return true;
      if (branch.contains("2.x") || branch.contains("version2") || branch.contains("musescore2") || branch.contains("musescore 2")) return false;
      if (branch.contains("3.x") || branch.contains("version3") || branch.contains("musescore3") || branch.contains("musescore 3")) return true;
      if (branch.contains("2") && !branch.contains("3")) return false;
      return true;
      }
static inline int CompatEstimate(QString branch)
      {
      branch = branch.toLower();
      if (branch.contains("2.x") || branch.contains("version2") || branch.contains("musescore2") || branch.contains("musescore 2")) return -2;
      if (branch.contains("3.x") || branch.contains("version3") || branch.contains("musescore3") || branch.contains("musescore 3")) return 2;
      if (branch.contains("2") && !branch.contains("3")) return -1;
      if (branch.contains("3") && !branch.contains("2")) return 1;
      return 0;
      }
static bool getLatestRelease(QJsonDocument& releases, int& release_id, QString& link) {
      // By default, releases returned from GitHub are sorted from the latest commit to the oldest
      for (const auto& release : releases.array()) {
            if (release.isObject()) {
                  const auto& release_obj = release.toObject();
                  if (isCompatibleRelease(release_obj.value("target_commitish").toString())) {
                        // get "id" and "zipball_url"
                        release_id = release_obj.value("id").toInt();
                        link = release_obj.value("zipball_url").toString();
                        return true;
                        }
                  else return false;
                  }
            else return false;
            }
      }

static inline bool isNotFoundMessage(const QJsonObject& json_obj)
      {
      return json_obj["message"].toString() == "Not Found";
      }

static std::vector<std::pair<QString, QString>> getAttachments(const QByteArray& html_raw, bool all = false)
      {
      std::vector<std::pair<QString, QString>> file_urls;
      if (!all) {
            int start_idx = html_raw.indexOf("<div class=\"field field--name-upload field--type-file field--label-above\">");
            if (start_idx == -1)
                  ; // TODO: no attachments found
            QByteArray attachments_raw = html_raw.mid(start_idx, -1);
            XmlReader xml(attachments_raw);
            while (!xml.atEnd() && xml.name() != "table") xml.readNextStartElement();
            while (!xml.atEnd() && xml.name() != "tbody") xml.readNextStartElement();
            while (!xml.atEnd() && xml.name() != "tr") xml.readNextStartElement();
            // now we've arrived at the first <tr>
            while (!xml.atEnd() && xml.name() == "tr") {
                  for (int i = 0; i < 3; i++) // enter <td>, <span>, <a>
                        xml.readNextStartElement();
                  file_urls.push_back(make_pair(xml.readElementText(), xml.attributes().value("href").toString()));
                  for (int i = 0; i < 3; i++) // exit <a>, <span>, <td>
                        xml.skipCurrentElement();
                  xml.readNextStartElement();
                  }
            }
      else {
            // search for all links from musescore.org
            QRegularExpression musescore_link_patt("https://musescore.org/sites/musescore.org/files/\\d+\\-\\d+/([\\w\\-]+)");
            auto it = musescore_link_patt.globalMatch(html_raw);
            while (it.hasNext()) {
                  QRegularExpressionMatch match = it.next();
                  file_urls.push_back(make_pair(match.captured(1), match.captured(0)));
                  }
            }
      return file_urls;
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
//   analyzePluginPage
//---------------------------------------------------------
bool ResourceManager::analyzePluginPage(QString url, PluginPackageDescription& desc)
      {
      bool should_update = false;
      QString direct_link;
      DownloadUtils page(QNetworkRequest::RedirectPolicy::NoLessSafeRedirectPolicy, this);
      page.setTarget(url);
      page.download();
      QByteArray html_raw = page.returnData();
      // first of all, check if GitHub repo links exist
      QRegularExpression github_repo_patt("https?://github.com/([\\w\\-]+)/([\\w\\-]+)");
      QRegularExpressionMatch github_match = github_repo_patt.match(html_raw);
      if (github_match.hasMatch()) {
            QString user = github_match.captured(1);
            QString repo = github_match.captured(2);
            // check if there's a GitHub release
            DownloadUtils release_json(this);
            release_json.setTarget(githubLatestReleaseAPI(user, repo));
            release_json.download();
            QJsonDocument result = QJsonDocument::fromJson(release_json.returnData());
            if (!result.array().isEmpty()) {
                  // there is a GitHub release
                  should_update = desc.source != GITHUB_RELEASE;
                  desc.source = GITHUB_RELEASE;
                  int release_id;
                  QString direct_link;
                  if (getLatestRelease(result, release_id, direct_link)) {
                        if (should_update || (desc.release_id != release_id && release_id > 0)) {
                              desc.release_id = release_id;
                              desc.direct_link = direct_link;
                              return true;
                              }
                        else
                              return false;
                        }
                  }
                  // else, fetch latest commit sha on master(usually for 3.x), as info for update
            should_update = desc.source != GITHUB;
            desc.source = GITHUB;
            QString sha = getLatestCommitSha(user, repo, "master");
            // compare the sha with previous stored sha. If the same, don't update.
            if (should_update || desc.latest_commit != sha) {
                  desc.latest_commit = sha;
                  desc.direct_link = githubLatestArchiveURL(user, repo, "master");
                  return true;
                  }
            else
                  return false;
            }
      else {
            // no github repo links exist
            // first fetch links in attachments
            std::vector<std::pair<QString, QString>> attachment_urls = getAttachments(html_raw, false);
            if (attachment_urls.empty())
                  attachment_urls = getAttachments(html_raw, true);
            std::vector<std::pair<QString, QString>> archives, qmls, selected;
            //std::pair<QString, QString> selected; // the finally selected qml file or archive
            QStringList suffixes = { "qml","zip","rar","7z" };
            std::pair<QString, QString> target;
            int score = -2;
            for (auto& item : attachment_urls) {
                  QFileInfo f(item.first);
                  if (suffixes.contains(f.suffix().toLower())) {
                        int curr_score = CompatEstimate(item.first); // add other text in that line
                        if (curr_score > score) {
                              target = item;
                              score = curr_score;
                              }
                        }
                  }
            if (score >= 0) {
                  should_update = desc.source != ATTACHMENT;
                  QDateTime date_time;
                  if (desc.source != ATTACHMENT || desc.direct_link != target.second || (date_time = GetLastModified(target.second)) != desc.last_modified) {
                        desc.source = ATTACHMENT;
                        desc.direct_link = target.second;
                        if (!date_time.isValid())
                              // TODO: we didn't download the file before, optimize: fill desc.last_modified when downloading the file.
                              date_time = GetLastModified(target.second);
                        desc.last_modified = date_time;
                        return true;
                        }
                  else
                        return false;
                  }

            qDebug("Unknown plugin source");
            desc.source = UNKNOWN;

            return false;
            }
      }

void ResourceManager::updatePlugin()
      {
      QPushButton* install = static_cast<QPushButton*>(sender());

      const QString& page_url = install->property("page_url").toString();
      PluginPackageDescription& desc = pluginDescriptionMap[page_url];
      Q_ASSERT(desc.update != nullptr);
      if (desc.update->source == UNKNOWN) {
            install->setText("Failed Try again.");
            install->setEnabled(true);
            return;
            }
      install->setText(tr("Downloading..."));
      QString localPath = downloadPluginPackage(*desc.update, page_url);
      if (installPluginPackage(localPath, *desc.update)) {
            PluginPackageDescription* p_update = desc.update;
            desc = *desc.update;
            delete(p_update);
            desc.update = nullptr;
            refreshPluginButton(install->property("row").toInt(), true);
            writePluginPackages();
            }
      }

void ResourceManager::checkPluginUpdate(QPushButton* install)
      {
      const QString& page_url = install->property("page_url").toString();
      if (pluginDescriptionMap.contains(page_url)) {
            auto& desc = pluginDescriptionMap[page_url];
            PluginPackageDescription* desc_tmp = new PluginPackageDescription(desc);
            bool should_update = analyzePluginPage("https://musescore.org" + page_url, *desc_tmp);
            if (should_update)
                  desc.update = desc_tmp;
            else
                  delete desc_tmp;
            refreshPluginButton(install->property("row").toInt(), !should_update);
            }
      }

void ResourceManager::scanPluginUpdate()
      {
      for (int row = 0; row < pluginsTable->rowCount(); row++) {
            QPushButton* install = static_cast<QPushButton*>(pluginsTable->indexWidget(pluginsTable->model()->index(row, 3)));
            const QString& page_url = install->property("page_url").toString();
            // if installed
            if (pluginDescriptionMap.contains(page_url))
                  checkPluginUpdate(install);
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
//   downloadLanguage
//---------------------------------------------------------

void ResourceManager::downloadLanguage()
      {
      QPushButton *button = static_cast<QPushButton*>( sender() );
      QString dta  = languageButtonMap[button];
      QString hash = languageButtonHashMap[button];
      button->setText(tr("Updating"));
      button->setDisabled(true);
      QString baseAddress = baseAddr() + dta;
      DownloadUtils dl(this);
      dl.setTarget(baseAddress);
      QString localPath = dataPath + "/locale/" + dta.split('/')[1];
      dl.setLocalFile(localPath);
      dl.download();
      if (!dl.saveFile() || !verifyFile(localPath, hash)) {
            button->setText(tr("Failed, try again"));
            button->setEnabled(1);
            }
      else {
            // unzip and delete
            MQZipReader zipFile(localPath);
            QFileInfo zfi(localPath);
            QString destinationDir(zfi.absolutePath());
            QVector<MQZipReader::FileInfo> allFiles = zipFile.fileInfoList();
            bool result = true;
            foreach (MQZipReader::FileInfo fi, allFiles) {
                  const QString absPath = destinationDir + "/" + fi.filePath;
                  if (fi.isFile) {
                        QFile f(absPath);
                        if (!f.open(QIODevice::WriteOnly)) {
                              result = false;
                              break;
                              }
                        f.write(zipFile.fileData(fi.filePath));
                        f.setPermissions(fi.permissions);
                        f.close();
                        }
                  }
            zipFile.close();
            if (result) {
                  QFile::remove(localPath);
                  button->setText(tr("Updated"));
                  //  retranslate the UI if current language is updated
                  if (dta == languageButtonMap.first())
                        setMscoreLocale(localeName);
                  }
            else {
                  button->setText(tr("Failed, try again"));
                  button->setEnabled(1);
                  }
            }
      }

//---------------------------------------------------------
//   downloadExtension
//---------------------------------------------------------

void ResourceManager::downloadExtension()
      {
      QPushButton* button = static_cast<QPushButton*>(sender());
      QString path  = button->property("path").toString();
      QString hash = button->property("hash").toString();
      button->setText(tr("Updating"));
      button->setDisabled(true);
      QString baseAddress = baseAddr() + path;
      DownloadUtils dl(this);
      dl.setTarget(baseAddress);
      QString localPath = QDir::tempPath() + "/" + path.split('/')[1];
      QFile::remove(localPath);
      dl.setLocalFile(localPath);
      dl.download(true);
      bool saveFileRes = dl.saveFile();
      bool verifyFileRes = saveFileRes && verifyFile(localPath, hash);
      if(!verifyFileRes) {
            QFile::remove(localPath);
            button->setText(tr("Failed, try again"));
            button->setEnabled(true);

            QMessageBox msgBox;
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setTextFormat(Qt::RichText);
            msgBox.setWindowTitle(tr("Extensions Installation Failed"));
            if (!saveFileRes) //failed to save file on disk
                  msgBox.setText(tr("Unable to save the extension file on disk"));
            else //failed to verify package, so size or hash sum is incorrect
                  msgBox.setText(tr("Unable to download, save and verify the package.\nCheck your internet connection."));
            msgBox.exec();
            }
      else {
            bool result = mscore->importExtension(localPath);
            if (result) {
                  QFile::remove(localPath);
                  button->setText(tr("Updated"));
                  // find uninstall button and make it visible
                  int rowId = button->property("rowId").toInt();
                  QPushButton* uninstallButton = static_cast<QPushButton*>(extensionsTable->indexWidget(extensionsTable->model()->index(rowId, 4)));
                  uninstallButton->setDisabled(false);
                  }
            else {
                  button->setText(tr("Failed, try again"));
                  button->setEnabled(1);
                  }
            }
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

QString ResourceManager::downloadPluginPackage(PluginPackageDescription& desc, const QString& page_url)
      {
      DownloadUtils package(QNetworkRequest::RedirectPolicy::NoLessSafeRedirectPolicy, this);
      package.setTarget(desc.direct_link);
      // TODO: try to get extension name from direct_link, and add it to localPath
      QString localPath = dataPath + "/plugins/" + filenameBaseFromPageURL(page_url);
      if (desc.source == GITHUB || desc.source == GITHUB_RELEASE)
            localPath += ".zip";
      else if (desc.source == ATTACHMENT)
            localPath += "."+QFileInfo(desc.direct_link).suffix();
      QDir().mkpath(dataPath + "/plugins");
      package.setLocalFile(localPath);
      package.download();
      // TODO: if extension hasn't been known yet, get file type from http response
      // for now, use zip for GITHUB; and get ext name from direct_link for musescore.org
      package.saveFile();
      return localPath;
      }

//---------------------------------------------------------
//   downloadInstallPlugin
//---------------------------------------------------------

void ResourceManager::downloadInstallPlugin()
      {
      QPushButton* button = static_cast<QPushButton*>(sender());
      button->setEnabled(false);
      button->setText(tr("Analyzing..."));
      QString page_url = button->property("page_url").toString();
      PluginPackageDescription new_package;
      new_package.package_name = button->property("name").toString();
      QObject* p = button->parent();
      analyzePluginPage("https://musescore.org" + page_url, new_package);
      if (new_package.source == UNKNOWN) {
            button->setText("Failed Try again.");
            button->setEnabled(true);
            return;
            }
      button->setText(tr("Downloading..."));
      QString localPath = downloadPluginPackage(new_package, page_url);

      if (installPluginPackage(localPath, new_package)) {
            // add the new description item to the map
            pluginDescriptionMap[page_url] = new_package;
            refreshPluginButton(button->property("row").toInt());
            writePluginPackages();
            displayPlugins();
            }
      else {
            button->setText("Bad archive. Try again");
            button->setEnabled(true);
            return;
            }
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
            button->setText("Failed, try again");
            return;
            }
      pluginDescriptionMap.remove(url);
      refreshPluginButton(button->property("row").toInt());
      writePluginPackages();
      displayPlugins();
      }


void ResourceManager::refreshPluginButton(int row, bool updated/* = true*/)
      {
      // TODO: get button, then plugin url, then update status
      // install button
      QPushButton* install = static_cast<QPushButton*>(pluginsTable->indexWidget(pluginsTable->model()->index(row, 3)));
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
                                                else e.unknown();
                                                }
                                          }
                                    else if (t == "GitHubReleaseID")
                                          desc.release_id = e.readInt();
                                    else if (t == "GitHubLatestSha")
                                          desc.latest_commit = e.readElementText();
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

//---------------------------------------------------------
//   uninstallExtension
//---------------------------------------------------------

void ResourceManager::uninstallExtension()
      {
      QPushButton* uninstallButton = static_cast<QPushButton*>(sender());
      QString extensionId = uninstallButton->property("extensionId").toString();
      if (mscore->uninstallExtension(extensionId)) {
            // find uninstall button and make it visible
            int rowId = uninstallButton->property("rowId").toInt();
            QPushButton* installButton = static_cast<QPushButton*>(extensionsTable->indexWidget(extensionsTable->model()->index(rowId, 3)));
            installButton->setText("Install");
            installButton->setDisabled(false);
            uninstallButton->setDisabled(true);
            }
      }

//---------------------------------------------------------
//   verifyFile
//---------------------------------------------------------

bool ResourceManager::verifyFile(QString path, QString hash)
      {
      QFile file(path);
      QCryptographicHash localHash(QCryptographicHash::Sha1);
      if(file.open(QIODevice::ReadOnly)) {
            localHash.reset();
            localHash.addData(file.readAll());
            QString hashValue2 = QString(localHash.result().toHex());
            if(hash == hashValue2)
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void ResourceManager::hideEvent(QHideEvent* event)
      {
      MuseScore::saveGeometry(this);
      QWidget::hideEvent(event);
      }

}

