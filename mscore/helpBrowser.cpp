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

#include "helpBrowser.h"
#include "icons.h"
#include "help.h"

namespace Ms {


//---------------------------------------------------------
//   HelpBrowser
//---------------------------------------------------------

HelpBrowser::HelpBrowser(QWidget* parent)
   : QWidget(parent)
      {
      view    = new HelpView(helpEngine);
      toolbar = new QWidget;
      toolbar->setSizePolicy(QSizePolicy::Expanding,
      QSizePolicy::Fixed);
      QVBoxLayout* l = new QVBoxLayout;
      l->addWidget(toolbar);
      l->addWidget(view);
      setLayout(l);
      QHBoxLayout* bl = new QHBoxLayout;

      QToolButton* home = new QToolButton;
      home->setIcon(QIcon(*icons[int(Icons::goHome_ICON)]));
      bl->addWidget(home);
      connect(home, SIGNAL(clicked()), SLOT(homeClicked()));

      bl->addStretch(2);

      QToolButton* previous = new QToolButton;
      previous->setIcon(QIcon(*icons[int(Icons::goPrevious_ICON)]));
      bl->addWidget(previous);
      connect(previous, SIGNAL(clicked()), view, SLOT(backward()));
      connect(view, SIGNAL(backwardAvailable(bool)), previous, SLOT(setEnabled(bool)));

      QToolButton* next = new QToolButton;
      next->setIcon(QIcon(*icons[int(Icons::goNext_ICON)]));
      bl->addWidget(next);
      connect(next, SIGNAL(clicked()), view, SLOT(forward())
      connect(view, SIGNAL(forwardAvailable(bool)), next, SLOT(setEnabled(bool)));

      bl->addStretch(10);

//      QToolButton* reload = new QToolButton;
//      QAction * reloadAction = view->pageAction(QWebPage::Reload);
      //for an unknown reason setting icon on the QToolButton doesn't work here...
//      reloadAction->setIcon(QIcon(*icons[int(Icons::viewRefresh_ICON)]));
//      reload->setDefaultAction(reloadAction);
//      bl->addWidget(reload);

      toolbar->setLayout(bl);
      }

//---------------------------------------------------------
//   setContent
//---------------------------------------------------------

void HelpBrowser::setContent(const QString& path)
      {
      homePath = QUrl::fromLocalFile(path);
      view->setSource(homePath);
      }

void HelpBrowser::setContent(const QUrl& url)
      {
      homePath = url;
      printf("HelpBroser::setContent: <%s>\n", qPrintable(url.toString()));
      view->setSource(url);
      }

//---------------------------------------------------------
//   homeClicked
//---------------------------------------------------------

void HelpBrowser::homeClicked()
      {
      view->setSource(homePath);
      }

//---------------------------------------------------------
//   loadResource
//---------------------------------------------------------

QVariant HelpView::loadResource(int type, const QUrl& name)
      {
      if (name.scheme() == "qthelp")
            return QVariant(helpEngine->fileData(name));
      return QTextBrowser::loadResource(type, name);
      }

}

