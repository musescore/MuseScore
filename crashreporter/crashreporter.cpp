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

#include "crashreporter.h"

#include "ui_crashreporter.h"
#include "config.h"

#include <QDir>
#include <QFileInfo>
#include <QHttpMultiPart>
#include <QMessageBox>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QProcess>

namespace Ms {

//---------------------------------------------------------
//   version
//---------------------------------------------------------

static QString version()
      {
      QString ver(VERSION);

      const QString versionLabel(VERSION_LABEL);
      if (!versionLabel.isEmpty())
            ver.append("-").append(versionLabel);

      return ver;
      }

//---------------------------------------------------------
//   CrashReporter
//---------------------------------------------------------

CrashReporter::CrashReporter(const QString& miniDumpFile, const QUrl& uploadUrl)
   : _ui(new Ui::CrashReporter), _miniDump(miniDumpFile), _uploadUrl(uploadUrl)
      {
      _ui->setupUi(this);
      }

//---------------------------------------------------------
//   ~CrashReporter
//---------------------------------------------------------

CrashReporter::~CrashReporter()
      {
      delete _ui;
      }

//---------------------------------------------------------
//   CrashReporter::uploadReport
//---------------------------------------------------------

void CrashReporter::uploadReport()
      {
      if (!_networkManager) {
            _networkManager = new QNetworkAccessManager(this);
            connect(_networkManager, &QNetworkAccessManager::finished, this, &CrashReporter::onUploadFinished);
            }

      QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

      QHttpPart miniDumpPart;
      miniDumpPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
      const QString filename = QFileInfo(_miniDump).fileName();
      const QString contentDisposition(QString("form-data; name=\"upload_file_minidump\"; filename=\"%1\"").arg(filename));
      miniDumpPart.setHeader(QNetworkRequest::ContentDispositionHeader, contentDisposition);
      QFile* file = new QFile(_miniDump, multiPart);
      file->open(QIODevice::ReadOnly);
      miniDumpPart.setBodyDevice(file);
      multiPart->append(miniDumpPart);

      QHttpPart releasePart;
      releasePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"sentry[release]\""));
      releasePart.setBody(version().toLatin1());
      multiPart->append(releasePart);

      const QString comment(_ui->commentText->toPlainText());
      const bool hasComment = !comment.isEmpty();

      QHttpPart hasCommentPart;
      hasCommentPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"sentry[tags][hasComment]\""));
      hasCommentPart.setBody(QString::number(hasComment).toLatin1());
      multiPart->append(hasCommentPart);

      QHttpPart commentPart;
      commentPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"comments\""));
      commentPart.setBody(comment.toUtf8());
      multiPart->append(commentPart);

      QNetworkRequest request(_uploadUrl);
      // TODO: custom user-agent?

      QNetworkReply* reply = _networkManager->post(request, multiPart);
      multiPart->setParent(reply);
      }

//---------------------------------------------------------
//   CrashReporter::onUploadFinished
//---------------------------------------------------------

void CrashReporter::onUploadFinished(QNetworkReply* reply)
      {
      bool success = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) == 200;

      if (success) {
            QMessageBox::information(this, tr("Success!"), tr("Crash report uploaded successfully!"));
            if (_ui->restartCheckBox->isChecked()) {
                  static_assert(sizeof(MSCORE_EXECUTABLE) > 1,
                     "MSCORE_EXECUTABLE should be defined to make it possible to restart MuseScore"
                     );
                  const QDir appDir(qApp->applicationDirPath());
                  QProcess::startDetached(appDir.filePath(MSCORE_EXECUTABLE));
                  }
            close();
            }
      else
            QMessageBox::warning(this, tr("Error"), tr("Error while uploading crash report:\n%1").arg(reply->errorString()));

      reply->deleteLater();
      }

//---------------------------------------------------------
//   CrashReporter::on_sendReportButton_clicked
//---------------------------------------------------------

void CrashReporter::on_sendReportButton_clicked()
      {
      uploadReport();
      }

//---------------------------------------------------------
//   CrashReporter::on_cancelButton_clicked
//---------------------------------------------------------

void CrashReporter::on_cancelButton_clicked()
      {
      close();
      }

} // namespace Ms

//---------------------------------------------------------
//   main
//---------------------------------------------------------

int main(int argc, char** argv)
      {
      if (argc < 2)
            qFatal("Usage: %s <dump_file>", argv[0]);

      QString miniDump(argv[1]);
      if (!QFileInfo(miniDump).exists())
            qFatal("minidump file does not exist");

      QApplication app(argc, argv);

      static_assert(sizeof(CRASH_REPORT_URL) > 1, "Crash report URL must be valid");
      const char* crashReportUrl = CRASH_REPORT_URL;

      Ms::CrashReporter reporter(miniDump, QUrl(crashReportUrl));
      reporter.show();

      return app.exec();
      }
