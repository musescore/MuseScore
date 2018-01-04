//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2017 Nikolaos Hatzopoulos (nickhatz@csu.fullerton.edu)
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

#include "mainwindow.h"
#include "ui_mainwindow.h"

//-----------------------------------------------------------------------------
// get_musescore_path() : finds the path of MuseScore.exe
// MuseScore.exe can be found in the current directroy (for production)
// or under the mscore folder (during development build)
//-----------------------------------------------------------------------------

QString get_musescore_path()
      {
      QString crashreporter_path = QCoreApplication::applicationDirPath();
      QString res = crashreporter_path+"/MuseScore.exe";
      QString res_empty = "";
      QFileInfo fileInfo(res);
      if (fileInfo.exists() && fileInfo.isFile())
            return res;

      res = crashreporter_path + "/../../mscore/MuseScore.exe";
      QFileInfo fileInfo2(res);
      if (fileInfo2.exists() && fileInfo2.isFile())
            return res;

      return res_empty;
      }

//-----------------------------------------------------------------------------
// read_comma_seperated_metadata_txt_file :
// Read a comma seperated file with only two columns: key and parameter
// We asuming the key will never being defined with comma
// though if a parameter has a comma then the parameter will not come
// in seperate parts example the line: mykey,parmetere1,test will then split as:
// key=>mykey parameter=>parameter1,test
//-----------------------------------------------------------------------------

QMap <QString,QString> read_comma_seperated_metadata_txt_file(QString mypath)
      {
      QMap <QString,QString> mymap;
      QFile file(mypath);
      QStringList fields;
      QString line;
      QString mypar;
      int i;

      if(!file.open(QIODevice::ReadOnly)) {
            QMessageBox::information(0, "error", file.errorString());
      }

      QTextStream in(&file);

      while(!in.atEnd()) {
            line = in.readLine();
            fields = line.split(',');
            if (fields.count()==2)
                  mymap.insert(fields.at(0),fields.at(1));
            if (fields.count()>2) {
                  mypar = "";
                  for(i=1; i<fields.count()-1; i++) {
                        mypar += fields.at(i)+QString(",");
                  }
                  mypar += fields.at(fields.count()-1);
                  mymap.insert(fields.at(0),mypar);
            }
      }

      file.close();
      return mymap;
      }

//-----------------------------------------------------------------------------
// MainWindow Constructor
//-----------------------------------------------------------------------------

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
      {
      ui->setupUi(this);
      m_manager = new QNetworkAccessManager(this);
      connect(m_manager, &QNetworkAccessManager::finished, this, &MainWindow::uploadFinished);

      }
//-----------------------------------------------------------------------------
// MainWindow Destructor
//-----------------------------------------------------------------------------

MainWindow::~MainWindow()
      {
      delete ui;
      }

//-----------------------------------------------------------------------------
// sslErrors : It reports Errors regarding SSL network transmitions
//-----------------------------------------------------------------------------

void MainWindow::sslErrors(const QList<QSslError> &sslErrors)
      {
      #ifndef QT_NO_SSL
      foreach (const QSslError &error, sslErrors)
            fprintf(stderr, "SSL error: %s\n", qPrintable(error.errorString()));
      #else
      Q_UNUSED(sslErrors);
      #endif
      }

//-----------------------------------------------------------------------------
// uploadFinished : Actions needed during an upload finished
//-----------------------------------------------------------------------------

void MainWindow::uploadFinished(QNetworkReply *reply)
      {
      if (!reply->error()) {
            m_file->close();
            m_file->deleteLater();
            reply->deleteLater();
      }
      }

//-----------------------------------------------------------------------------
// onError : Error messages from netwokr respond
//-----------------------------------------------------------------------------

void MainWindow::onError(QNetworkReply::NetworkError err)
      {
      qDebug() << " SOME ERROR!";
      qDebug() << err;
      }

//-----------------------------------------------------------------------------
// sendRpoertQt : Uploads the minidump and metadata files to the crash server
//-----------------------------------------------------------------------------

void MainWindow::sendReportQt(QString user_txt)
      {
      QString minidump_path;
      QString metadata_path;

      if (QCoreApplication::arguments().count()==3) {

            minidump_path = QCoreApplication::arguments().at(1);
            metadata_path = QCoreApplication::arguments().at(2);

            QStringList filePathList = minidump_path.split('/');
            QString minidump_filename = filePathList.at(filePathList.count()-1);

            qDebug() << "minidump file: " << minidump_path;

            QString url = CRASH_SUBMIT_URL;

            QString boundary = "--" + QString::number(
                              qrand() * (90000000000) / (RAND_MAX + 1) + 10000000000, 16);

            QNetworkRequest request(url);

            // According to RFC2046 section 5.1: http://www.ietf.org/rfc/rfc2046.txt
            // We need to define our boundary stirng in the header request ( request.setHeader )
            // and repeate the same boundary in between the multi parts ( multiPart->setBoundary )

            request.setHeader(QNetworkRequest::ContentTypeHeader,
                              "multipart/form-data; boundary=" + boundary);

            QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
            multiPart->setBoundary(boundary.toUtf8());

            QMap <QString,QString> metadata;
            metadata = read_comma_seperated_metadata_txt_file(metadata_path);

            QMap<QString, QString>::iterator it;
            QHttpPart textToken;

            // metadata input

            for (it=metadata.begin(); it!=metadata.end(); ++it) {
                  textToken.setHeader(QNetworkRequest::ContentDispositionHeader,
                                      QVariant("form-data; name=\""+it.key()+"\""));
                  textToken.setBody(QByteArray(it.value().toUtf8()));
                  multiPart->append(textToken);
            }

            // user text input

            textToken.setHeader(QNetworkRequest::ContentDispositionHeader,
                                QVariant("form-data; name=\"user_text_input\""));
            textToken.setBody(QByteArray(user_txt.toUtf8()));
            multiPart->append(textToken);

            //FILE

            m_file = new QFile(minidump_path);
            m_file->open(QIODevice::ReadOnly);

            QHttpPart filePart;
            filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
            QString contentDisposition = QString("form-data; name=\"upload_file_minidump\"; filename=\""+minidump_filename+"\"");
            filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(contentDisposition));
            filePart.setBodyDevice(m_file);
            m_file->setParent(multiPart); // we cannot delete the file now, so delete it with the multiPart

            multiPart->append(filePart);

            QNetworkReply *reply = m_manager->post(request,multiPart);

            connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onError(QNetworkReply::NetworkError)));
            //connect(reply, SIGNAL(finished()),this, SLOT(uploadFinished()));

            #ifndef QT_NO_SSL
            connect(reply, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(sslErrors(QList<QSslError>)));
            #endif
            QEventLoop loop;
            connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
            loop.exec();

            int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

            QNetworkReply::NetworkError e = reply->error();
            qDebug() << "status code: " << statusCode;
            qDebug() << "reply error: " << e;
            qDebug() << "reply: " << reply->readAll();

      }

      }

//-----------------------------------------------------------------------------
// on_btnQuit_clicked : Quit button functionality
//-----------------------------------------------------------------------------

void MainWindow::on_btnQuit_clicked()
      {

      if (ui->checkBox->isChecked()) {
            QString user_txt = ui->plainTextEdit->toPlainText();
            sendReportQt(user_txt);

      }

      //close();
      QApplication::quit();

      }

//-----------------------------------------------------------------------------
// on_btnRestart_clicked : Restart button functionality
//-----------------------------------------------------------------------------

void MainWindow::on_btnRestart_clicked()
      {

      if (ui->checkBox->isChecked()) {
            QString user_txt = ui->plainTextEdit->toPlainText();
            sendReportQt(user_txt);
      }

      QString mscore_path = get_musescore_path();

      // Start a process that is not attached to the parent process
      // otherwise the parent process kills the child
      QProcess *process;
      process->startDetached(mscore_path);

      //close();
      QApplication::quit();

      }
