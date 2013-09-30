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

#include "downloadUtils.h"

namespace Ms {

DownloadUtils::DownloadUtils(QWidget *parent) :QObject(parent)
      {

      }

bool DownloadUtils::saveFile()
      {
      QFile localFile(_localFile);
      if (!localFile.open(QIODevice::WriteOnly))
            {
            qDebug() << "can't access";
            return false;
            }
      qDebug() << "here writing to file "<< sdata << " at " << _localFile;
      localFile.write(sdata);
      localFile.close();
      return true;
      }

void DownloadUtils::downloadFinished(QNetworkReply *data)
      {
      sdata = data->readAll();
      emit done();
      }

QByteArray DownloadUtils::returnData()
      {
      return sdata;
      }

void DownloadUtils::download()
      {
      QUrl url = QUrl::fromEncoded(_target.toLocal8Bit());
      QNetworkRequest request(url);
      QEventLoop loop;
      QObject::connect(manager.get(request), SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));
      QObject::connect(&manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(downloadFinished(QNetworkReply*)));
      QObject::connect(manager.get(request), SIGNAL(finished()), &loop, SLOT(quit()));
      loop.exec();
      }

void DownloadUtils::downloadProgress(qint64 received, qint64 total)
      {
      qDebug() << (double(received)/total)*100 << "%";
      }

}
