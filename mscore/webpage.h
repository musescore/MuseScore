//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer and others
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

#ifndef __WEBPAGE_H__
#define __WEBPAGE_H__

#include "musescore.h"
#include "scoreview.h"

namespace Ms {

class MuseScore;

//---------------------------------------------------------
//   MyNetworkAccessManager
//---------------------------------------------------------

class MyNetworkAccessManager: public QNetworkAccessManager
      {
      Q_OBJECT

   public:
      MyNetworkAccessManager(QObject *parent) : QNetworkAccessManager(parent) {}

   protected:
      QNetworkReply * createRequest(Operation op,
                                    const QNetworkRequest & req,
                                    QIODevice * outgoingData = 0);
      };

// Derive from QWebPage, because a WebPage handles
// plugin creation

//---------------------------------------------------------
//   MyWebPage
//---------------------------------------------------------

class MyWebPage: public QWebPage
      {
      Q_OBJECT

   protected:
      QObject *createPlugin(
         const QString &classid,
         const QUrl &url,
         const QStringList &paramNames,
         const QStringList & paramValues);

   public:
      MyWebPage(QObject *parent = 0);
      };

//---------------------------------------------------------
//   MyWebView
//    Derive a new class from QWebView for convenience.
//    Otherwise you'd always have to create a QWebView
//    and a MyWebPage and assign the MyWebPage object
//    to the QWebView. This class does that for you
//    automatically.
//---------------------------------------------------------

class MyWebView: public QWebView
      {
      Q_OBJECT

      MyWebPage m_page;
      QProgressBar* progressBar;

   public slots:
      void link(const QUrl& url);
      void setBusy();
      void stopBusy(bool val);

#ifndef QT_NO_OPENSSL
      void ignoreSSLErrors(QNetworkReply *reply, QList<QSslError> sslErrors);
#endif
     
   public:
      MyWebView(QWidget *parent = 0);
      ~MyWebView();
      MyWebPage* webPage() { return &m_page;}
      virtual QSize sizeHint () const;
      };

//---------------------------------------------------------
//   WebPage
//---------------------------------------------------------

class WebPageDockWidget : public QDockWidget
      {
      Q_OBJECT

      MyWebView* web;

   public slots:
      void addToJavascript();
      void saveOnlineFinished();

   public:
      WebPageDockWidget(MuseScore* mscore, QWidget* parent = 0);
      Q_INVOKABLE void load();
      Q_INVOKABLE bool saveCurrentScoreOnline(QString action, QVariantMap parameters, QString fileFieldName);
      Q_INVOKABLE bool setCurrentScoreSource(QString source);
      Q_INVOKABLE QObject* currentScore();

      QUrl webUrl();
      };

class CookieJar : public QNetworkCookieJar
      {
      Q_OBJECT

    public:
      CookieJar(QString path, QObject *parent = 0);  //load cookie
      ~CookieJar();  //save cookies

    private:
      QString file; // where to save cookies
      };

#if 0
//---------------------------------------------------------
//   WebScoreView
//---------------------------------------------------------

class WebScoreView : public ScoreView
      {
      Q_OBJECT
      QNetworkAccessManager* networkManager;

   private slots:
      void networkFinished(QNetworkReply*);

   public:
      WebScoreView(QWidget* parent = 0);
      WebScoreView(const WebScoreView&);
      void setScore(const QString&);
      };

Q_DECLARE_METATYPE(WebScoreView);
#endif
}
#endif

