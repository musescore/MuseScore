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
#include "preferences.h"

namespace Ms {

UpdateChecker::UpdateChecker()
      {
      manager = new QNetworkAccessManager(this);
      connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(onRequestFinished(QNetworkReply*)));
      }

UpdateChecker::~UpdateChecker()
      {
      delete manager;
      }

void UpdateChecker::onRequestFinished(QNetworkReply* reply)
{
    if(reply->error() != QNetworkReply::NoError){
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
    QString releaseType;

    while (!reader.atEnd() && !reader.hasError()) {
        QXmlStreamReader::TokenType token = reader.readNext();
        if(token == QXmlStreamReader::StartDocument) {
            continue;
        }
        if(token == QXmlStreamReader::StartElement) {
            if(reader.name() == "version") {
                version = parseText(reader);
            }else if (reader.name() == "revision") {
                upgradeRevision = parseText(reader);
            }else if (reader.name() == "downloadUrl") {
                downloadUrl = parseText(reader);
            }else if (reader.name() == "infoUrl") {
                infoUrl = parseText(reader);
            }else if (reader.name() == "description") {
                description = parseText(reader);
            }
        }
    }

    if (reader.error() != QXmlStreamReader::NoError)
        qDebug() << reader.error() << reader.errorString();

    QString message = tr("An update for MuseScore is available: <a href=\"%1\">MuseScore %2 r.%3</a>").arg(downloadUrl).arg(version).arg(upgradeRevision);
//    qDebug("revision %s", revision.toLatin1().constData());
    if(!version.isEmpty() &&  upgradeRevision > revision ){
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Update Available"));
        msgBox.setText(message);
        msgBox.setTextFormat(Qt::RichText);
        msgBox.exec();
    }else if(manual){
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("No Update Available"));
        msgBox.setText(tr("No Update Available"));
        msgBox.setTextFormat(Qt::RichText);
        msgBox.exec();
    }
}

QString UpdateChecker::parseText(QXmlStreamReader& reader){
    QString result;
    reader.readNext();
    if(reader.tokenType() == QXmlStreamReader::Characters)
      result = reader.text().toString();
    return result;
}

void UpdateChecker::check(QString rev, bool m)
{
    manual = m;
    #if defined(Q_OS_WIN)
    os = "win";
    #endif
    #if defined(Q_OS_MAC)
    os = "mac";
    #endif
    if(qApp->applicationName() == "MuseScore"){ //avoid nightly cymbals
          if(MuseScore::unstable()){
                  release = "pre";
          }else{
                  release = "stable";
          }
    }else{
        release = "nightly";
    }
//    qDebug("release type: %s", release.toLatin1().constData());
    if(!os.isEmpty() && !release.isEmpty() && release != "nightly"){
        revision =  rev;
        manager->get(QNetworkRequest(QUrl("http://update.musescore.org/update_"+os +"_" + release +".xml")));
    }
}

//---------------------------------------------------------
//   default period
//---------------------------------------------------------

int UpdateChecker::defaultPeriod()
      {
        int result = 24;
        if(qApp->applicationName() == "MuseScore"){ //avoid nightly cymbals
            if(MuseScore::unstable()){
                  result = 24;
            }else{
                  result = 7*24;
            }
        }
        return result;
        }

bool UpdateChecker::hasToCheck(){
    QSettings s;
    s.beginGroup("Update");
    QDateTime lastUpdate = s.value("lastUpdateDate", QDateTime::currentDateTime()).value<QDateTime>();

//    qDebug("preferences.checkUpdateStartup: %d" , preferences.checkUpdateStartup);
//    qDebug("lastupdate: %s", lastUpdate.toString("dd.MM.yyyy hh:mm:ss.zzz").toLatin1().constData());

    if(preferences.checkUpdateStartup < 0 ){ //Never
      return false;
    }else if (preferences.checkUpdateStartup == 0) { // factory
      preferences.checkUpdateStartup = UpdateChecker::defaultPeriod();
      preferences.dirty = true;
    }
    s.endGroup();
    return QDateTime::currentDateTime() > lastUpdate.addSecs(3600 * preferences.checkUpdateStartup);
}
}

