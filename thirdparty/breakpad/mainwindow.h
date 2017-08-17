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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <iostream>
#include <fstream>
#include <QApplication>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSslError>
#include <QList>

#define CRASH_SUBMIT_URL "https://musescore.sp.backtrace.io:6098/post?format=minidump&token=00268871877ba102d69a23a8e713fff9700acf65999b1f043ec09c5c253b9c03"

using namespace::std;

wstring str2wstr(string mystr);
string wstr2str(wstring mystr);
QMap <QString,QString> read_comma_seperated_metadata_txt_file(QString mypath);
bool launcher(wstring program);
QString get_musescore_path();

namespace Ui {
      class MainWindow;
}

class MainWindow : public QMainWindow {
      Q_OBJECT

public:
      explicit MainWindow(QWidget *parent = 0);
      ~MainWindow();

private slots:
      void on_btnRestart_clicked();
      void on_btnQuit_clicked();
      void uploadFinished(QNetworkReply *reply);
      void sslErrors(const QList<QSslError> &errors);
      void onError(QNetworkReply::NetworkError err);

private:
      Ui::MainWindow *ui;
      void sendReportQt(QString user_txt);
      QNetworkAccessManager *m_manager;
      QFile *m_file;
};

#endif // MAINWINDOW_H
