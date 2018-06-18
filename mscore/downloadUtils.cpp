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

DownloadUtils::DownloadUtils(QWidget *parent)
   : QObject(parent)
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
      qDebug() << "here writing to file " <<  _localFile;
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

void DownloadUtils::download(bool showProgress)
      {
      QUrl url = QUrl::fromEncoded(_target.toLocal8Bit());
      QNetworkRequest request(url);
      QEventLoop loop;
      QNetworkReply* reply = manager.get(request);

      QObject::connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));
      QObject::connect(&manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(downloadFinished(QNetworkReply*)));
      QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));

      if (showProgress) {
            progressDialog = new QProgressDialog(static_cast<QWidget*>(parent()));
            progressDialog->setWindowFlags(Qt::WindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowTitleHint));
            progressDialog->setWindowModality(Qt::ApplicationModal);
            progressDialog->setCancelButtonText(tr("Cancel"));
            connect(progressDialog, SLOT(canceled()), this, SIGNAL(cancel()));
            progressDialog->setLabelText(tr("Downloading..."));
            progressDialog->setAutoClose(true);
            progressDialog->setAutoReset(true);
            QObject::connect(progressDialog, SIGNAL(canceled()), &loop, SLOT(quit()));
            progressDialog->show();
            }

      loop.exec();

      QObject::disconnect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));
      QObject::disconnect(&manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(downloadFinished(QNetworkReply*)));
      QObject::disconnect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
      }

void DownloadUtils::downloadProgress(qint64 received, qint64 total)
      {
      double curVal = (double(received)/total)*100;
      if (progressDialog && progressDialog->isVisible())
            progressDialog->setValue(curVal);
      }

}
