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


/// Class for managing downloads
/// Warning: For this class to show a progress dialog, it needs a QWidget as parent

class DownloadUtils : public QObject
      {
      Q_OBJECT

      QByteArray sdata; /// the final data received
      QNetworkAccessManager manager;
      QString _target; /// the URL of to-be-downloaded contents
      QString _localFile; /// the file to which the data will be saved when saveFile() is called.

      QProgressDialog* progressDialog = nullptr;

   public:
      explicit DownloadUtils(QWidget *parent = nullptr);

      void setTarget(const QString& t)      { _target = t; }
      void setLocalFile(const QString& t)   { _localFile = t; }
      bool saveFile();
      QByteArray returnData();

   signals:
      void done(); /// emitted when the download is complete

   public slots:
      void download(bool showProgress = false);
      void downloadFinished(QNetworkReply* data);
      void downloadProgress(qint64 received, qint64 total);
      };

} // namepace Ms

#endif

