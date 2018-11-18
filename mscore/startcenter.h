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

#ifndef __STARTCENTER_H__
#define __STARTCENTER_H__

#include "config.h"
#include "abstractdialog.h"
#include "ui_startcenter.h"

namespace Ms {

#ifdef USE_WEBENGINE

class MyWebUrlRequestInterceptor : public QWebEngineUrlRequestInterceptor {
    Q_OBJECT

    public:
      MyWebUrlRequestInterceptor(QObject* p = Q_NULLPTR)
            : QWebEngineUrlRequestInterceptor(p) {}

      void interceptRequest(QWebEngineUrlRequestInfo& info)
            {
            info.setHttpHeader("Accept-Language",
                  QString("%1;q=0.8,en-US;q=0.6,en;q=0.4").arg(mscore->getLocaleISOCode()).toUtf8());
            }
      };

//---------------------------------------------------------
//   MyWebEnginePage
//---------------------------------------------------------

class MyWebEnginePage : public QWebEnginePage {
    Q_OBJECT

    public:
      MyWebEnginePage(QObject* parent = Q_NULLPTR)
            : QWebEnginePage(parent) {}

      bool acceptNavigationRequest(const QUrl& url, QWebEnginePage::NavigationType type, bool isMainFrame);
      };

//---------------------------------------------------------
//   MyWebEngineView
//---------------------------------------------------------

class MyWebView : public QWebEngineView {
    Q_OBJECT

   public slots:

   public:
      MyWebView(QWidget* parent = 0);
      ~MyWebView();
      virtual QSize sizeHint() const;
      };

#endif //USE_WEBENGINE

//---------------------------------------------------------
//   Startcenter
//---------------------------------------------------------

class Startcenter : public AbstractDialog, public Ui::Startcenter {
      Q_OBJECT
#ifdef USE_WEBENGINE
      MyWebView* _webView;
#endif
      virtual void closeEvent(QCloseEvent*);

    private slots:
      void loadScore(QString);
      void newScore();
      void openScoreClicked();

    protected:
      virtual void retranslate() { retranslateUi(this); }

    signals:
      void closed(bool);

    public:
      Startcenter();
      ~Startcenter();
      void updateRecentScores();
      void writeSettings();
      void readSettings();
      virtual void keyReleaseEvent(QKeyEvent*);
      };
}
#endif //__STARTCENTER_H__
