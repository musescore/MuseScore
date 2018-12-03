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
#include "tourhandler.h"

namespace Ms {

//---------------------------------------------------------
//   showStartcenter
//---------------------------------------------------------

void MuseScore::showStartcenter(bool show)
      {
      QAction* a = getAction("startcenter");
      if (show && startcenter == nullptr) {
            startcenter = new Startcenter;
            startcenter->addAction(a);
            startcenter->readSettings();
            connect(startcenter, SIGNAL(closed(bool)), a, SLOT(setChecked(bool)));
            connect(startcenter, SIGNAL(closed(bool)), tourHandler(), SLOT(showWelcomeTour()), Qt::QueuedConnection);
            }
      if (!startcenter)
            return;
      if (show)
            startcenter->setVisible(true);
      else
            startcenter->close();
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

#ifdef USE_WEBENGINE
      if (!noWebView) {
            _webView = new MyWebView(this);
            _webView->setMaximumWidth(200);  

            MyWebEnginePage* page = new MyWebEnginePage(this);
            MyWebUrlRequestInterceptor* wuri = new MyWebUrlRequestInterceptor(page);
            page->profile()->setRequestInterceptor(wuri);
            _webView->setPage(page);

            auto extendedVer = QString(VERSION) + "." + QString(BUILD_NUMBER);
            _webView->setUrl(QUrl(QString("https://connect2.musescore.com/?version=%1").arg(extendedVer)));

            horizontalLayout->addWidget(_webView);
            }
#endif

      if (enableExperimental)
// right now don’t know how it use in WebEngine @handrok
//            QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
//      QWebSettings::globalSettings()->setAttribute(QWebSettings::PluginsEnabled, false);
      recentPage->setBoldTitle(false);
      updateRecentScores();

      setFocus();
      }

//---------------------------------------------------------
//   ~Startcenter
//---------------------------------------------------------

Startcenter::~Startcenter() {
//      if (_webView)
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
      mscore->tourHandler()->delayWelcomeTour();
      close();
      getAction("file-new")->trigger();
      }

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void Startcenter::closeEvent(QCloseEvent* event)
      {
      AbstractDialog::closeEvent(event);
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
      mscore->tourHandler()->delayWelcomeTour();
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

//---------------------------------------------------------
//   keyPressEvent
//---------------------------------------------------------

void Startcenter::keyPressEvent(QKeyEvent *event)
      {
      if(event->key() == Qt::Key_Escape)
            event->ignore(); // will handle it on key release.
      else
            AbstractDialog::keyPressEvent(event);
      }

//---------------------------------------------------------
//   keyReleaseEvent
//---------------------------------------------------------

void Startcenter::keyReleaseEvent(QKeyEvent *event)
      {
      if(event->key() == Qt::Key_Escape) {
            close();
            }
      else
            AbstractDialog::keyReleaseEvent(event);
      }

#ifdef USE_WEBENGINE
 
//---------------------------------------------------------
//   MyWebView
//---------------------------------------------------------

MyWebView::MyWebView(QWidget *parent):
      QWebEngineView(parent)
      {
      if (!enableExperimental)
            setContextMenuPolicy(Qt::NoContextMenu);
      }

//---------------------------------------------------------
//   ~MyWebView
//---------------------------------------------------------

MyWebView::~MyWebView()
      {
      disconnect(this, SIGNAL(loadFinished(bool)), this, SLOT(stopBusy(bool)));
      }

//---------------------------------------------------------
//   sizeHint
//---------------------------------------------------------

QSize MyWebView::sizeHint() const
      {
      return QSize(200 , 600);
      }


bool MyWebEnginePage::acceptNavigationRequest(const QUrl & url, QWebEnginePage::NavigationType type, bool isMainFrame)
      {
      qDebug() << "acceptNavigationRequest(" << url << "," << type << "," << isMainFrame << ")";

      if (type == QWebEnginePage::NavigationTypeLinkClicked)
      {
            QString path(url.path());
            QFileInfo fi(path);
            if (fi.suffix() == "mscz" || fi.suffix() == "xml"
                  || fi.suffix() == "musicxml" || fi.suffix() == "mxl") {
                  mscore->loadFile(url);
                  QAction* a = getAction("startcenter");
                  a->setChecked(false);
                  mscore->showStartcenter(false);
            }
            else
                  QDesktopServices::openUrl(url);

            return false;
      }
      return true;
      }


#endif //USE_WEBENGINE
}

