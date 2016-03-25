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

namespace Ms {

//---------------------------------------------------------
//   HelpView
//---------------------------------------------------------

class HelpView : public QTextBrowser {
      Q_OBJECT
      QHelpEngine* helpEngine;

   public:
      HelpView(QHelpEngine* he, QWidget* parent = 0) : QTextBrowser(parent), helpEngine(he) {}
      QVariant loadResource(int type, const QUrl& name);
      };

//---------------------------------------------------------
//   HelpBrowser
//---------------------------------------------------------

class HelpBrowser : public QWidget {
      Q_OBJECT
      HelpView* view;
      QWidget* toolbar;
      QUrl homePath;

   private slots:
      void homeClicked();

   public:
      HelpBrowser(QWidget* parent = 0);
      void setContent(const QString& path);
      void setContent(const QUrl& url);
      };


} // namespace Ms
#endif

