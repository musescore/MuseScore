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

#ifndef __DOWNLOAD_UTILS_H__
#define __DOWNLOAD_UTILS_H__

namespace Ms {

class DownloadUtils : public QObject
      {
      Q_OBJECT

      QByteArray sdata;
      QNetworkAccessManager manager;
      QString _target;
      QString _localFile;

      QProgressDialog* progressDialog = nullptr;

   public:
      explicit DownloadUtils(QWidget *parent=0);

      void setTarget(const QString& t)      { _target = t; }
      void setLocalFile(const QString& t)   { _localFile = t; }
      bool saveFile();
      QByteArray returnData();

   signals:
      void done();

   public slots:
      void download(bool showProgress = false, const int timeOutMSecs = 20000);
      void downloadFinished(QNetworkReply* data);
      void downloadProgress(qint64 received, qint64 total);
      };
}

#endif

