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

//---------------------------------------------------------
//   HelpBrowser
//---------------------------------------------------------

HelpBrowser::HelpBrowser(QWidget* parent)
   : QWidget(parent)
      {
      view    = new WebView;
      toolbar = new QWidget;
      QVBoxLayout* l = new QVBoxLayout;
      l->addWidget(toolbar);
      l->addWidget(view);
      setLayout(l);
      QHBoxLayout* bl = new QHBoxLayout;

      QToolButton* home = new QToolButton;
      home->setIcon(QIcon(":/data/home.png"));
      bl->addWidget(home);
      connect(home, SIGNAL(clicked()), SLOT(homeClicked()));

      bl->addStretch(2);

      QToolButton* previous = new QToolButton;
      previous->setDefaultAction(view->pageAction(QWebPage::Back));
      bl->addWidget(previous);

      QToolButton* next = new QToolButton;
      bl->addWidget(next);
      next->setDefaultAction(view->pageAction(QWebPage::Forward));

      bl->addStretch(10);

      QToolButton* reload = new QToolButton;
      reload->setDefaultAction(view->pageAction(QWebPage::Reload));
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

