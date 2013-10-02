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
#include "ui_resourceManager.h"

namespace Ms {

extern QString dataPath;
extern QString mscoreGlobalShare;

ResourceManager::ResourceManager(QWidget *parent) :
      QDialog(parent)
      {
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      QDir dir;
      dir.mkpath(dataPath + "/locale");
      baseAddr = "http://extensions.musescore.org/";
      displayPlugins();
      displayLanguages();
      languagesTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
      languagesTable->verticalHeader()->hide();
      tabs->removeTab(1);
      tabs->setCurrentIndex(0);
      }

void ResourceManager::displayPlugins()
      {
      tabs->setTabText(1, "Plugins");
      textBrowser->setText("hello");
      }

void ResourceManager::displayLanguages()
      {
      tabs->setTabText(0,tr("Languages"));
      DownloadUtils *js = new DownloadUtils(this);
      js->setTarget(baseAddr + "languages/details.json");
      js->download();
      QByteArray json = js->returnData();

      QJsonParseError err;
      QJsonDocument result = QJsonDocument::fromJson(json, &err);

      if (err.error != QJsonParseError::NoError || !result.isObject()) {
            qDebug("An error occured during parsing");
            return;
            }
      int rowCount = result.object().keys().size();
      rowCount -= 2; //version and type
      qDebug() << result.object().keys().size();
      qDebug() << result.toJson();
      languagesTable->setRowCount(rowCount);

      int row = 0;
      int col = 0;
      QPushButton* updateButtons[rowCount];
      QPushButton* temp;
      languagesTable->verticalHeader()->show();

      QCryptographicHash hash(QCryptographicHash::Sha1);

      for (QString key : result.object().keys()) {
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

            languagesTable->setItem(row,col++,new QTableWidgetItem (name));
            languagesTable->setItem(row,col++,new QTableWidgetItem (filename));
            languagesTable->setItem(row,col++,new QTableWidgetItem (tr("%1 KB").arg(fileSize)));
            updateButtons[row] = new QPushButton(tr("Update"));

            temp = updateButtons[row];
            buttonMap[temp] = "languages/" + filename;
            buttonHashMap[temp] = hashValue;
            languagesTable->setIndexWidget(languagesTable->model()->index(row,col++), temp);
            QString local = dataPath + "/locale/" + filename;

            QFileInfo fileLocal(local);
            if(!fileLocal.exists())
                  local = mscoreGlobalShare + "locale/" + filename;;

            if(verifyFile(local, hashValue)) {
                  temp->setText(tr("No update"));
                  temp->setDisabled(1);
                  }
            else {
                  connect(temp, SIGNAL(clicked()), this, SLOT(download()));
                  }
            row++;
            }
      }

void ResourceManager::download()
      {
      QPushButton *button = dynamic_cast<QPushButton*>( sender() );
      QString data = buttonMap[button];
      QString hash = buttonHashMap[button];
      button->setText(tr("Updating"));
      button->setDisabled(1);
      QString baseAddress = baseAddr + data;
      DownloadUtils *dl = new DownloadUtils(this);
      dl->setTarget(baseAddress);
      qDebug() << baseAddress;
      QString localPath = dataPath + "/locale/" + data.split('/')[1];
      dl->setLocalFile(localPath);
      dl->download();
      if( !dl->saveFile() || !verifyFile(localPath, hash)) {
            button->setText(tr("Failed, try again"));
            button->setEnabled(1);
            }
      else {
            button->setText(tr("Updated"));
            }
      }


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
}
