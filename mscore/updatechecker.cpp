//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2010 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "updatechecker.h"
#include "musescore.h"
#include "libmscore/mscore.h"
#include "preferences.h"
#include "resourceManager.h"
#include "extension.h"
#include "libmscore/utils.h"

namespace Ms {

//---------------------------------------------------------
//   default period
//---------------------------------------------------------

static int defaultPeriod()
      {
      int result = 24;
      if(qApp->applicationName() == "MuseScore2"){ //avoid nightly cymbals
            if (MuseScore::unstable())
                  result = 24;
            else
                  result = 24; // yes, it's again the same but let's keep the logic for now
            }
      return result;
      }

UpdateChecker::UpdateChecker(QObject* parent)
      : UpdateCheckerBase(parent)
      {
      manager = new QNetworkAccessManager(this);
      connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(onRequestFinished(QNetworkReply*)));
      }

void UpdateChecker::onRequestFinished(QNetworkReply* reply)
      {
      if (reply->error() != QNetworkReply::NoError) {
            qDebug("Error while checking update [%s]", reply->errorString().toLatin1().constData());
            return;
            }

      QSettings s;
      s.beginGroup("Update");
      s.setValue("lastUpdateDate", QDateTime::currentDateTime());
      s.endGroup();

      QByteArray data = reply->readAll();
      QXmlStreamReader reader(data);
      QString version;
      QString upgradeRevision;
      QString downloadUrl;
      QString infoUrl;
      QString description;

      while (!reader.atEnd() && !reader.hasError()) {
            QXmlStreamReader::TokenType token = reader.readNext();
            if(token == QXmlStreamReader::StartDocument) {
                  continue;
                  }
            if(token == QXmlStreamReader::StartElement) {
                  if(reader.name() == "version")
                        version = parseText(reader);
                  else if (reader.name() == "revision")
                        upgradeRevision = parseText(reader);
                  else if (reader.name() == "downloadUrl") {
                        downloadUrl = parseText(reader);
#if defined(FOR_WINSTORE)
                        downloadUrl = QString("%1?package=appx&version=%2").arg(downloadUrl).arg(_currentVersion);
#endif
                        }
                  else if (reader.name() == "infoUrl")
                        infoUrl = parseText(reader);
                  else if (reader.name() == "description")
                        description = parseText(reader);
                  }
            }

      if (reader.error() != QXmlStreamReader::NoError)
            qDebug() << reader.error() << reader.errorString();

      QString message = tr("An update for MuseScore is available: %1MuseScore %2 r.%3%4")
                  .arg("<a href=\"" + downloadUrl + "\">")
                  .arg(version)
                  .arg(upgradeRevision)
                  .arg("</a>");
//    qDebug("revision %s", revision.toLatin1().constData());
      if (!version.isEmpty() &&  version > _currentVersion ) {
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Update Available"));
            msgBox.setText(message);
            msgBox.setTextFormat(Qt::RichText);
            msgBox.exec();
            }
      else if (manual) {
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("No Update Available"));
            msgBox.setText(tr("No Update Available"));
            msgBox.setTextFormat(Qt::RichText);
            msgBox.exec();
            }
      }

QString UpdateChecker::parseText(QXmlStreamReader& reader)
      {
      QString result;
      reader.readNext();
      if (reader.tokenType() == QXmlStreamReader::Characters)
            result = reader.text().toString();
      return result;
      }

bool UpdateChecker::getUpdatePrefValue()
      {
      return preferences.getBool(PREF_UI_APP_STARTUP_CHECKUPDATE);
      }

QString UpdateChecker::getUpdatePrefString()
      {
      return "lastUpdateDate";
      }

void UpdateChecker::check(QString currentVersion, bool m)
      {
      manual = m;
#if defined(Q_OS_WIN)
      os = "win";
#endif
#if defined(Q_OS_MAC)
      os = "mac";
#endif
      bool check = true;
      if (MuseScore::unstable()) {
            release = "nightly";
            QString buildNumber = QString("%1").arg(BUILD_NUMBER);
            if (!buildNumber.isEmpty())
                  _currentVersion = QString("%1.%2").arg(currentVersion).arg(BUILD_NUMBER);
            else {
                  _currentVersion = currentVersion;
                  check = false;
                  }
            }
      else {
            release = "stable";
             _currentVersion =  currentVersion;
            }
      if (MScore::debugMode)
            qDebug("release type: %s", release.toLatin1().constData());
      if (!os.isEmpty() && !release.isEmpty() && check)
            manager->get(QNetworkRequest(QUrl("http://update.musescore.org/update_" + os +"_" + release +".xml")));
      }

UpdateCheckerBase::UpdateCheckerBase(QObject* parent)
      : QObject(parent)
      {

      }

//---------------------------------------------------------
//   default hasToCheck
//---------------------------------------------------------

bool UpdateCheckerBase::hasToCheck()
      {
      if(!getUpdatePrefValue())
            return false;
//disable embedded updating for both stable/unstable Mac builds and stable Win builds
#if !defined(Q_OS_MAC) && (!defined(Q_OS_WIN) || defined(MSCORE_UNSTABLE))
      QSettings s;
      s.beginGroup("Update");
      QDateTime now = QDateTime::currentDateTime();
      QDateTime lastUpdate = s.value(getUpdatePrefString(), now).value<QDateTime>();

      if (MScore::debugMode) {
            qDebug(QString("preferences." + getUpdatePrefString() + ": %d").toStdString().c_str() , getUpdatePrefValue());
            qDebug(QString("last update for " + getUpdatePrefString() + ": %s").toStdString().c_str(), qPrintable(lastUpdate.toString("dd.MM.yyyy hh:mm:ss.zzz")));
            }
      s.endGroup();
      return now == lastUpdate || now > lastUpdate.addSecs(3600 * defaultPeriod()) ;
#else
      return true;
#endif
      }

ExtensionsUpdateChecker::ExtensionsUpdateChecker(QObject* parent)
      : UpdateCheckerBase(parent)
      {

      }

void ExtensionsUpdateChecker::check()
      {
      DownloadUtils *js = new DownloadUtils();
      js->setTarget(ResourceManager::baseAddr() + "extensions/details.json");
      js->download();
      QByteArray json = js->returnData();

      // parse the json file
      QJsonParseError err;
      QJsonDocument result = QJsonDocument::fromJson(json, &err);
      if (err.error != QJsonParseError::NoError || !result.isObject()) {
            qDebug("An error occurred during parsing");
            return;
            }

      QStringList extensions = result.object().keys();
      for (QString key : extensions) {
            if (!result.object().value(key).isObject())
                  continue;
            QJsonObject value = result.object().value(key).toObject();
            QString version = value.value("version").toString();

            // get the installed version of the extension if any
            if (Extension::isInstalled(key)) {
                  QString installedVersion = Extension::getLatestVersion(key);
                  if (compareVersion(installedVersion, version)) {
                        QMessageBox msgBox;
                        msgBox.setWindowTitle(tr("Extension Updates Available"));
                        msgBox.setText(tr("One or more installed extensions have updates available in Help / Resource Manager..."));
                        msgBox.setTextFormat(Qt::RichText);
                        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
                        msgBox.setDefaultButton(QMessageBox::Ok);
                        msgBox.setEscapeButton(QMessageBox::Cancel);
                        int ret = msgBox.exec();
                        switch (ret) {
                              case QMessageBox::Ok: {
                                    ResourceManager r(static_cast<QWidget*>(this->parent()));
                                    r.selectExtensionsTab();
                                    r.exec();
                                    break;
                                    }
                              case QMessageBox::Cancel: {
                                    break;
                                    }
                              default:
                                    qWarning() << "undefined action in ExtensionsUpdateChecker::check" << ret;
                              }
                        QSettings s;
                        s.beginGroup("Update");
                        s.setValue(getUpdatePrefString(), QDateTime::currentDateTime());
                        s.endGroup();
                        break;
                        }
                  }
            }
      }

bool ExtensionsUpdateChecker::getUpdatePrefValue()
      {
      return preferences.getBool(PREF_UI_APP_STARTUP_CHECK_EXTENSIONS_UPDATE);
      }

QString ExtensionsUpdateChecker::getUpdatePrefString()
      {
      return "lastExtensionsUpdateDate";
      }

}

