//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2014-2016 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================


#include "musescore.h"
#include "libmscore/mscore.h"
#include "startcenter.h"
#include "scoreBrowser.h"

namespace Ms {

//---------------------------------------------------------
//   showStartcenter
//---------------------------------------------------------

void MuseScore::showStartcenter(bool val)
      {
      QAction* a = getAction("startcenter");
      if (val && startcenter == nullptr) {
            startcenter = new Startcenter;
            startcenter->addAction(a);
            startcenter->readSettings();
            connect(startcenter, SIGNAL(closed(bool)), a, SLOT(setChecked(bool)));
            connect(startcenter, SIGNAL(rejected()), a, SLOT(toggle()));
            }
      startcenter->setVisible(val);
      }

//---------------------------------------------------------
//   Startcenter
//---------------------------------------------------------

Startcenter::Startcenter()
 : AbstractDialog(0)
      {
      setObjectName("Startcenter");
      setupUi(this);
      setBackgroundRole(QPalette::Base);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      setWindowModality(Qt::ApplicationModal);
      connect(recentPage,  &ScoreBrowser::scoreActivated, this, &Startcenter::loadScore);
      connect(openScore, SIGNAL(clicked()), this, SLOT(openScoreClicked()));
      connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
      setStyleSheet(QString("QPushButton { background-color: %1 }").arg(openScore->palette().color(QPalette::Base).name()));

      //TODO init webview
      /*if (!noWebView) {
            _webView = new MyWebView(this);
            _webView->setUrl(QUrl(QString("https://connect2.musescore.com/?version=%1").arg(VERSION)));
            horizontalLayout->addWidget(_webView);
            }

      if (enableExperimental)
            QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
      QWebSettings::globalSettings()->setAttribute(QWebSettings::PluginsEnabled, false);*/
      recentPage->setBoldTitle(false);
      updateRecentScores();
      }

//---------------------------------------------------------
//   ~Startcenter
//---------------------------------------------------------

Startcenter::~Startcenter() {
//TODO      if (_webView)
//            delete _webView;
      }

//---------------------------------------------------------
//   loadScore
//---------------------------------------------------------

void Startcenter::loadScore(QString s)
      {
      if (s.endsWith("Create_New_Score.mscz")) {
            newScore();
            }
      else {
            mscore->openScore(s);
            close();
            }
      }

//---------------------------------------------------------
//   newScore
//---------------------------------------------------------

void Startcenter::newScore()
      {
      close();
      getAction("file-new")->trigger();
      }

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void Startcenter::closeEvent(QCloseEvent*)
      {
      emit closed(false);
      }

//---------------------------------------------------------
//   updateRecentScores
//---------------------------------------------------------

void Startcenter::updateRecentScores()
      {
      QFileInfoList fil = mscore->recentScores();
      QFileInfo newScore(":/data/Create_New_Score.mscz");
      fil.prepend(newScore);
      recentPage->setScores(fil);
      recentPage->selectFirst();
      }

//---------------------------------------------------------
//   openScoreClicked
//---------------------------------------------------------

void Startcenter::openScoreClicked()
      {
      close();
      getAction("file-open")->trigger();
      }

//---------------------------------------------------------
//   writeSettings
//---------------------------------------------------------

void Startcenter::writeSettings()
      {
      MuseScore::saveGeometry(this);
      }

//---------------------------------------------------------
//   readSettings
//---------------------------------------------------------

void Startcenter::readSettings()
      {
      MuseScore::restoreGeometry(this);
      }

#if 0
//---------------------------------------------------------
//   MyNetworkAccessManager
//---------------------------------------------------------

QNetworkReply* MyNetworkAccessManager::createRequest(Operation op,
                                          const QNetworkRequest & req,
                                          QIODevice * outgoingData)
      {
      QNetworkRequest new_req(req);
      new_req.setRawHeader("Accept-Language",  QString("%1;q=0.8,en-US;q=0.6,en;q=0.4").arg(mscore->getLocaleISOCode()).toLatin1());
      return QNetworkAccessManager::createRequest(op, new_req, outgoingData);
      }

//---------------------------------------------------------
//   MyWebView
//---------------------------------------------------------

MyWebView::MyWebView(QWidget *parent):
   QWebEngineView(parent)
      {
      page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
      QNetworkAccessManager* networkManager = new MyNetworkAccessManager(this);
#ifndef QT_NO_OPENSSL
      connect(networkManager,SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)),this, SLOT(ignoreSSLErrors(QNetworkReply*,QList<QSslError>)));
#endif

      connect(this, SIGNAL(loadFinished(bool)), SLOT(stopBusy(bool)));
      connect(this, SIGNAL(linkClicked(const QUrl&)), SLOT(link(const QUrl&)));

      QWebFrame* frame = page()->mainFrame();
      connect(frame, SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(addToJavascript()));

      page()->setNetworkAccessManager(networkManager);

      setZoomFactor(guiScaling);

      if (!enableExperimental)
            setContextMenuPolicy(Qt::NoContextMenu);

      //set cookie jar for persistent cookies
      CookieJar* jar = new CookieJar(QString(dataPath + "/cookie_store.txt"));
      page()->networkAccessManager()->setCookieJar(jar);

      page()->currentFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
      page()->currentFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAsNeeded);
      }

//---------------------------------------------------------
//   ~MyWebView
//---------------------------------------------------------

MyWebView::~MyWebView()
      {
      disconnect(this, SIGNAL(loadFinished(bool)), this, SLOT(stopBusy(bool)));
      }

#ifndef QT_NO_OPENSSL
/**
Slot connected to the sslErrors signal of QNetworkAccessManager
When this slot is called, call ignoreSslErrors method of QNetworkReply
*/
void MyWebView::ignoreSSLErrors(QNetworkReply *reply, QList<QSslError> sslErrors)
      {
      foreach (const QSslError &error, sslErrors)
            qDebug("Ignore SSL error: %d %s", error.error(), qPrintable(error.errorString()));
      reply->ignoreSslErrors(sslErrors);
      }
#endif

//---------------------------------------------------------
//   stopBusy
//---------------------------------------------------------

void MyWebView::stopBusy(bool val)
      {
      if (!val) {
            setVisible(false);
            }
      setCursor(Qt::ArrowCursor);
      }

//---------------------------------------------------------
//   setBusy
//---------------------------------------------------------

void MyWebView::setBusy()
      {
      setCursor(Qt::WaitCursor);
      }

//---------------------------------------------------------
//   link
//---------------------------------------------------------

void MyWebView::link(const QUrl& url)
      {
      QString path(url.path());
      QFileInfo fi(path);
      if (fi.suffix() == "mscz" || fi.suffix() == "xml" || fi.suffix() == "mxl") {
            mscore->loadFile(url);
            QAction* a = getAction("startcenter");
            a->setChecked(false);
            mscore->showStartcenter(false);
            }
      else
            QDesktopServices::openUrl(url);
      }

//---------------------------------------------------------
//   addToJavascript
//---------------------------------------------------------

void MyWebView::addToJavascript()
      {
      QWebFrame* frame = page()->mainFrame();
      frame->addToJavaScriptWindowObject("mscore", mscore);
      }

//---------------------------------------------------------
//   sizeHint
//---------------------------------------------------------

QSize MyWebView::sizeHint() const
      {
      return QSize(200 * guiScaling, 600 * guiScaling);
      }

//---------------------------------------------------------
//   CookieJar
//
//   Once the QNetworkCookieJar object is deleted, all cookies it held will be
//   discarded as well. If you want to save the cookies, you should derive from
//   this class and implement the saving to disk to your own storage format.
//   (From QNetworkCookieJar documentation.)
//---------------------------------------------------------

CookieJar::CookieJar(QString path, QObject *parent)
    : QNetworkCookieJar(parent)
      {
      _file = path;
      load();
      }

//---------------------------------------------------------
//   save
//---------------------------------------------------------

void CookieJar::load()
      {
      QFile cookieFile(_file);
      if (cookieFile.exists() && cookieFile.open(QIODevice::ReadOnly)) {
            QList<QNetworkCookie> list;
            QByteArray line;

            while(!(line = cookieFile.readLine()).isNull()) {
                  list.append(QNetworkCookie::parseCookies(line));
                  }
            setAllCookies(list);
            }
      else {
            if (MScore::debugMode)
                  qDebug() << "Can't open "<<  _file << " to read cookies";
            }
      }

//---------------------------------------------------------
//   setCookiesFromUrl
//---------------------------------------------------------

bool CookieJar::setCookiesFromUrl(const QList<QNetworkCookie>& cookieList, const QUrl& url)
      {
      bool res = QNetworkCookieJar::setCookiesFromUrl(cookieList, url);
      save();
      return res;
      }

//---------------------------------------------------------
//   save
//---------------------------------------------------------

void CookieJar::save()
      {
      QList <QNetworkCookie> cookieList = allCookies();
      QDateTime now = QDateTime::currentDateTime();
      for (int i = cookieList.count() - 1; i >= 0; --i) {
            if (cookieList.at(i).isSessionCookie() || cookieList.at(i).expirationDate() < now)
                  cookieList.removeAt(i);
            }

      QFile file(_file);
      if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            if (MScore::debugMode)
                  qDebug() << "Can't open "<< _file << " to save cookies";
            return;
            }

      QTextStream out(&file);
      for(int i = 0 ; i < cookieList.size() ; i++) {
            //get cookie data
            QNetworkCookie cookie = cookieList.at(i);
            if (!cookie.isSessionCookie()) {
                  QByteArray line =  cookie.toRawForm(QNetworkCookie::Full);
                  out << line << "\n";
                  }
            }
      file.close();
      }
#endif
}

