//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __HELP_BROWSER_H
#define __HELP_BROWSER_H

//---------------------------------------------------------
//   WebView
//---------------------------------------------------------

class WebView : public QWebView {
      Q_OBJECT
      virtual void wheelEvent(QWheelEvent*);

   public:
      WebView(QWidget* parent = 0) :QWebView(parent) {}
      };

//---------------------------------------------------------
//   HelpBrowser
//---------------------------------------------------------

class HelpBrowser : public QWidget {
      Q_OBJECT
      WebView* view;
      QWidget* toolbar;
      QUrl homePath;

   private slots:
      void homeClicked();

   public:
      HelpBrowser(QWidget* parent = 0);
      void setContent(const QString& path);
      void setContent(const QUrl& url);
      };

#endif

