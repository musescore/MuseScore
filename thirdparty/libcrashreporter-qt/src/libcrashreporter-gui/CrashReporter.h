/*
 * libcrashreporter-qt
 *
 * Copyright (C) 2010-2011  Christian Muehlhaeuser <muesli@tomahawk-player.org>
 * Copyright (C) 2016  Teo Mrnjavac <teo@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */


#ifndef CRASHREPORTER_H
#define CRASHREPORTER_H

#include <QDialog>
#include <QFile>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QByteArray>

namespace Ui
{
    class CrashReporter;
}

#ifdef Q_OS_LINUX
class BacktraceGenerator;
#endif

class CrashReporter : public QDialog
{
    Q_OBJECT

public:
    CrashReporter( const QUrl& url, const QStringList& argv );
    virtual ~CrashReporter( );

    void setLogo(const QPixmap& logo);
    void setText(const QString& text);
    void setBottomText(const QString& text);

    void setReportData(const QByteArray& name, const QByteArray& content);
    void setReportData(const QByteArray& name, const QByteArray& content, const QByteArray& contentType, const QByteArray& fileName);

private:
    Ui::CrashReporter* m_ui;
#ifdef Q_OS_LINUX
    BacktraceGenerator* m_btg;
#endif

    QString m_minidump_file_path;
    QNetworkRequest* m_request;
    QNetworkReply* m_reply;
    QUrl m_url;

    QMap < QByteArray, QByteArray > m_formContents;
    QMap < QByteArray, QByteArray > m_formContentTypes;
    QMap < QByteArray, QByteArray > m_formFileNames;


public slots:
    void send();

private slots:
    void onDone();
    void onProgress( qint64 done, qint64 total );
    void onFail( int error, const QString& errorString );
    void onSendButton();
};

#endif // CRASHREPORTER_H
