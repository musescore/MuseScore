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
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      QDir dir;
      dir.mkpath(dataPath + "/locale");
      displayExtensions();
      displayLanguages();
      languagesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
      languagesTable->verticalHeader()->hide();
      extensionsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
      extensionsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
      extensionsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
      extensionsTable->verticalHeader()->hide();
      extensionsTable->setColumnWidth(1, 50);
      extensionsTable->setColumnWidth(1, 100);
      MuseScore::restoreGeometry(this);
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

      QStringList extensions = result.object().keys();
      for (QString key : extensions) {
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
            extensionsTable->setItem(row, col++, new QTableWidgetItem(stringutils::convertFileSizeToHumanReadable(fileSize)));
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
      js.download();
      js.setTarget(baseAddr() + "languages/details.json");
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
      QPushButton* updateButtons[rowCount];
      QPushButton* temp;
      languagesTable->verticalHeader()->show();

      // move current language to first row
      QStringList languages = result.object().keys();
      QString lang = mscore->getLocaleISOCode();
      int index = languages.indexOf(lang);
      if (index < 0 &&  lang.size() > 2) {
            lang = lang.left(2);
            index = languages.indexOf(lang);
            }
      if (index >= 0) {
            QString l = languages.takeAt(index);
            languages.prepend(l);
            }

      for (QString key : languages) {
            if (!result.object().value(key).isObject())
                  continue;
            QJsonObject value = result.object().value(key).toObject();
            col = 0;
            QString test = value.value("file_name").toString();
            if(test.length() == 0)
                  continue;

            QString filename = value.value("file_name").toString();
            QString name = value.value("name").toString();
            QString fileSize = value.value("file_size").toString();
            QString hashValue = value.value("hash").toString();

            languagesTable->setItem(row, col++, new QTableWidgetItem(name));
            languagesTable->setItem(row, col++, new QTableWidgetItem(filename));
            languagesTable->setItem(row, col++, new QTableWidgetItem(tr("%1 kB").arg(fileSize)));
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

            if (verifyMScore && verifyInstruments) { // compare local file with distant hash
                  temp->setText(tr("Updated"));
                  temp->setDisabled(1);
                  }
            else {
                  connect(temp, SIGNAL(clicked()), this, SLOT(downloadLanguage()));
                  }
            row++;
            }
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
      if(!fileLocal.exists() || (fileLocal.lastModified() <= fileGlobal.lastModified()) )
            local = mscoreGlobalShare + "locale/" + filename;

      return verifyFile(local, hash);
      }


//---------------------------------------------------------
//   downloadLanguage
//---------------------------------------------------------

void ResourceManager::downloadLanguage()
      {
      QPushButton *button = static_cast<QPushButton*>( sender() );
      QString data = languageButtonMap[button];
      QString hash = languageButtonHashMap[button];
      button->setText(tr("Updating"));
      button->setDisabled(true);
      QString baseAddress = baseAddr() + data;
      DownloadUtils dl(this);
      dl.setTarget(baseAddress);
      QString localPath = dataPath + "/locale/" + data.split('/')[1];
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
                  if (data == languageButtonMap.first())
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
      QString data = button->property("path").toString();
      QString hash = button->property("hash").toString();
      button->setText(tr("Updating"));
      button->setDisabled(true);
      QString baseAddress = baseAddr() + data;
      DownloadUtils dl(this);
      dl.setTarget(baseAddress);
      QString localPath = QDir::tempPath() + QDir::separator() + data.split('/')[1];
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

