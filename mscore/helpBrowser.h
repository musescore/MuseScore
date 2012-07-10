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
//   HelpBrowser
//---------------------------------------------------------

class HelpBrowser : public QWidget {
      Q_OBJECT
      QWebView* view;
      QWidget* toolbar;
      QString homePath;

   private slots:
      void homeClicked();

   public:
      HelpBrowser(QWidget* parent = 0);
      void setContent(const QString& path);
      };

#endif

