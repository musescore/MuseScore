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

wstring str2wstr(string mystr){
    wstring res(mystr.begin(), mystr.end());
    return res;
}

string wstr2str(wstring mystr){
    string res(mystr.begin(), mystr.end());
    return res;
}

pair<wstring,wstring> line2strings(string line){
        string str1, str2;
        string *mystr;
        bool myflag;
        int i;

        myflag = false;
        str1.empty();
        str2.empty();

        mystr = &str1;

        for (i = 0; i < line.size(); i++){
                if ( line[i] == ',' && myflag == false){
                        mystr = &str2;
                        myflag = true;
                }
                else{
                        if (line[i] != '\n'){
                                *mystr += line[i];
                        }

                }
        }

        return pair<wstring,wstring>(str2wstr(str1),str2wstr(str2));

}


map <wstring,wstring> read_csv(string mypath){
        string line;
        pair<wstring,wstring> mykeyval;
        ifstream myfile(mypath);
        map <wstring,wstring> mymap;

        if (myfile.is_open()){
                while ( getline (myfile,line) ){
                        cout << line << '\n';
                        mykeyval = line2strings(line);
                        //cout << "str1: " << mykeyval.first << " str2: " << mykeyval.second << endl;
                        mymap.insert(mykeyval);

                }
                myfile.close();

        }
        else{
            cout << "Unable to open file";
        }

        return mymap;
}

void sendReport(QString user_txt){
    QString minidump_path;
    QString metadata_path;
    map<wstring, wstring> parameters;
    map<wstring, wstring> files;

    if ( QCoreApplication::arguments().count() == 3 ){
        //cout << argv[1] << endl;
        minidump_path = QCoreApplication::arguments().at(1);
        metadata_path = QCoreApplication::arguments().at(2);
        cout << "Minidump path: " << minidump_path.toStdString() << endl;
        parameters = read_csv(metadata_path.toStdString());
        parameters.insert(pair<wstring, wstring>(L"user_crash_input", user_txt.toStdWString()));

        wstring response;
        int mytimeout, my_error;

        // Add any attributes to the parameters map.
        // Attributes such as uname.sysname, uname.version, cpu.count are
        // extracted from minidump files automatically.
        //parameters.insert(pair<wstring, wstring>(L"product_name", L"foo"));
        //parameters.insert(pair<wstring, wstring>(L"version", L"0.1.0"));
        files.insert(pair<wstring, wstring>(L"upload_file_minidump", minidump_path.toStdWString()));

        wstring url = L"https://musescore.sp.backtrace.io:6098/post?format=minidump&token=00268871877ba102d69a23a8e713fff9700acf65999b1f043ec09c5c253b9c03";

        google_breakpad::HTTPUpload *test_upload;
        test_upload->SendRequest(url,
                                   parameters,
                                   files,
                                   &mytimeout,
                                   &response,
                                   &my_error);

        cout << wstr2str(response) << endl;

        //QMessageBox::information(this,"Message","Report has send!!");

    }
}

QString MainWindow::http_attribute_encode(QString attribute_name, QString input) {
    // result structure follows RFC 5987
    bool need_utf_encoding = false;
    QString result = "";
    QByteArray input_c = input.toLocal8Bit();
    char c;
    for (int i = 0; i < input_c.length(); i++) {
        c = input_c.at(i);
        if (c == '\\' || c == '/' || c == '\0' || c < ' ' || c > '~') {
            // ignore and request utf-8 version
            need_utf_encoding = true;
        }
        else if (c == '"') {
            result += "\\\"";
        }
        else {
            result += c;
        }
    }

    if (result.length() == 0) {
        need_utf_encoding = true;
    }

    if (!need_utf_encoding) {
        // return simple version
        return QString("%1=\"%2\"").arg(attribute_name, result);
    }

    QString result_utf8 = "";
    for (int i = 0; i < input_c.length(); i++) {
        c = input_c.at(i);
        if (
                (c >= '0' && c <= '9')
                || (c >= 'A' && c <= 'Z')
                || (c >= 'a' && c <= 'z')
                ) {
            result_utf8 += c;
        }
        else {
            result_utf8 += "%" + QString::number(static_cast<unsigned char>(input_c.at(i)), 16).toUpper();
        }
    }

    // return enhanced version with UTF-8 support
    return QString("%1=\"%2\"; %1*=utf-8''%3").arg(attribute_name, result, result_utf8);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, &QNetworkAccessManager::finished, this, &MainWindow::uploadFinished);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::sslErrors(const QList<QSslError> &sslErrors)
{
#ifndef QT_NO_SSL
    foreach (const QSslError &error, sslErrors)
        fprintf(stderr, "SSL error: %s\n", qPrintable(error.errorString()));
#else
    Q_UNUSED(sslErrors);
#endif
}

void MainWindow::uploadFinished(QNetworkReply *reply)
{
    if (!reply->error())
    {
        m_file->close();
        m_file->deleteLater();
        reply->deleteLater();
    }
}

void MainWindow::sendReportQt(){
    QString minidump_path;
    QString metadata_path;

    if ( QCoreApplication::arguments().count() == 3 ){
        //cout << argv[1] << endl;
        minidump_path = QCoreApplication::arguments().at(1);
        metadata_path = QCoreApplication::arguments().at(2);

        qDebug() << "minidump file: " << minidump_path;

        //parameters = read_csv(metadata_path.toStdString());
        //parameters.insert(pair<wstring, wstring>(L"user_crash_input", user_txt.toStdWString()));

        QString url = "https://musescore.sp.backtrace.io:6098/post?format=minidump&token=00268871877ba102d69a23a8e713fff9700acf65999b1f043ec09c5c253b9c03";



        /*bool first = true;
        foreach (QString key, input->vars.keys()) {
            if (!first) {
                request_content.append("&");
            }
            first = false;

            request_content.append(QUrl::toPercentEncoding(key));
            request_content.append("=");
            request_content.append(QUrl::toPercentEncoding(input->vars.value(key)));
        }*/

        QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

        QHttpPart textPart;
        textPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"name\""));
        textPart.setBody("toto");

        m_file = new QFile(minidump_path);
        m_file->open(QIODevice::ReadOnly);

        QHttpPart filePart;
        filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
        filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"upload_file_minidump\"; filename=\""+minidump_path+"\""));

        filePart.setBodyDevice(m_file);
        m_file->setParent(multiPart); // we cannot delete the file now, so delete it with the multiPart

        //multiPart->append(textPart);
        multiPart->append(filePart);

        QNetworkRequest request(url);
        //request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        //request.setHeader(QNetworkRequest::ContentTypeHeader, "multipart/form-data; boundary=" + boundary);

        QNetworkReply *reply = m_manager->post(request,multiPart);

    #ifndef QT_NO_SSL
        connect(reply, SIGNAL(sslErrors(QList<QSslError>)), SLOT(sslErrors(QList<QSslError>)));
    #endif
        QEventLoop loop;
        connect(reply, SIGNAL(finished()),&loop, SLOT(quit()));
        loop.exec();
    }

}


void MainWindow::on_pushButton_clicked(){

    if ( ui->checkBox->isChecked() ){
        QString user_txt = ui->plainTextEdit->toPlainText();
        sendReport(user_txt);
        //sendReportQt();

    }

    close();

}

void MainWindow::on_pushButton_2_clicked(){

    if ( ui->checkBox->isChecked() ){
        QString user_txt = ui->plainTextEdit->toPlainText();
        sendReport(user_txt);
        //sendReportQt();

    }

    // TODO: restart MuseScore

}
