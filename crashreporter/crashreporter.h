//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 Werner Schweer and others
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

#ifndef __CRASHREPORTER_H__
#define __CRASHREPORTER_H__

#include <QNetworkAccessManager>
#include <QWidget>
#include <QUrl>

namespace Ui {
      class CrashReporter;
      }

namespace Ms {

//---------------------------------------------------------
//   CrashReporter
//---------------------------------------------------------

class CrashReporter : public QWidget {
      Q_OBJECT

      Ui::CrashReporter* _ui;
      QString _miniDump;
      QUrl _uploadUrl;
      QNetworkAccessManager* _networkManager = nullptr;

      void uploadReport();

   private slots:
      void on_sendReportButton_clicked();
      void on_cancelButton_clicked();
      void onUploadFinished(QNetworkReply*);

   public:
      CrashReporter(const QString& miniDumpFile, const QUrl& uploadUrl);
      CrashReporter(const CrashReporter&) = delete;
      CrashReporter& operator=(const CrashReporter&) = delete;
      ~CrashReporter();
      };

} // namespace Ms

#endif
