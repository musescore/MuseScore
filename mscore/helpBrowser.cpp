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

namespace Ms {

//---------------------------------------------------------
//   HelpBrowser
//---------------------------------------------------------

HelpBrowser::HelpBrowser(QWidget* parent)
   : QWidget(parent)
      {
      view    = new WebView;
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
      previous->setDefaultAction(view->pageAction(QWebPage::Back));
      previous->setIcon(QIcon(*icons[int(Icons::goPrevious_ICON)]));
      bl->addWidget(previous);

      QToolButton* next = new QToolButton;
      next->setDefaultAction(view->pageAction(QWebPage::Forward));
      next->setIcon(QIcon(*icons[int(Icons::goNext_ICON)]));
      bl->addWidget(next);

      bl->addStretch(10);

      QToolButton* reload = new QToolButton;
      QAction * reloadAction = view->pageAction(QWebPage::Reload);
      //for an unknown reason setting icon on the QToolButton doesn't work here...
      reloadAction->setIcon(QIcon(*icons[int(Icons::viewRefresh_ICON)]));
      reload->setDefaultAction(reloadAction);
      bl->addWidget(reload);

      toolbar->setLayout(bl);
      }

//---------------------------------------------------------
//   setContent
//---------------------------------------------------------

void HelpBrowser::setContent(const QString& path)
      {
      homePath = QUrl::fromLocalFile(path);
      view->setUrl(homePath);
      }

void HelpBrowser::setContent(const QUrl& url)
      {
      homePath = url;
      view->setUrl(url);
      }

//---------------------------------------------------------
//   homeClicked
//---------------------------------------------------------

void HelpBrowser::homeClicked()
      {
      view->setUrl(homePath);
      }

//---------------------------------------------------------
//   wheelEvent
//---------------------------------------------------------

void WebView::wheelEvent(QWheelEvent* event)
      {
      static int deltaSum = 0;
      deltaSum += event->delta();
      int step = deltaSum / 120;
      deltaSum %= 120;

      if (event->modifiers() & Qt::ControlModifier) {
            qreal _mag = zoomFactor();

            if (step > 0) {
                  for (int i = 0; i < step; ++i)
                        _mag *= 1.1;
                  }
            else {
                  for (int i = 0; i < -step; ++i)
                        _mag /= 1.1;
                  }
            setZoomFactor(_mag);
            event->accept();
            }
      else
            event->ignore();
      QWebView::wheelEvent(event);
      }
}

