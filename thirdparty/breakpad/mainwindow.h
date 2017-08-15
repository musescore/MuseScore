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
#include "common/windows/http_upload.h"
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSslError>
#include <QList>

using namespace::std;

wstring str2wstr(string mystr);
string wstr2str(wstring mystr);
pair<wstring,wstring> line2strings(string line);
map <wstring,wstring> read_csv(string mypath);
void sendReport(QString user_txt);

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_2_clicked();
    void on_pushButton_clicked();
    void uploadFinished(QNetworkReply *reply);
    void sslErrors(const QList<QSslError> &errors);
    void onError(QNetworkReply::NetworkError err);

private:
    Ui::MainWindow *ui;
    void sendReportQt();
    QNetworkAccessManager *m_manager;
    QFile *m_file;
};

#endif // MAINWINDOW_H
