//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2015 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "help.h"
#include "helpBrowser.h"
#include "musescore.h"
#include "scoreview.h"

namespace Ms {

//---------------------------------------------------------
//   HelpQuery
//---------------------------------------------------------

HelpQuery::HelpQuery(QWidget* parent)
   : QWidgetAction(parent)
      {
      mapper = new QSignalMapper(this);

      w = new QWidget(parent);
      QHBoxLayout* layout = new QHBoxLayout;

      QLabel* label = new QLabel;
      label->setText(tr("Search for: "));
      layout->addWidget(label);

      entry = new QLineEdit;
      layout->addWidget(entry);

      QToolButton* button = new QToolButton;
      button->setText("x");
      layout->addWidget(button);

      w->setLayout(layout);
      setDefaultWidget(w);

      emptyState = true;

      connect(button, SIGNAL(clicked()), entry, SLOT(clear()));
      connect(entry, SIGNAL(textChanged(const QString&)), SLOT(textChanged(const QString&)));
      connect(entry, SIGNAL(returnPressed()), SLOT(returnPressed()));
      connect(mapper, SIGNAL(mapped(QObject*)), SLOT(actionTriggered(QObject*)));
      }

//---------------------------------------------------------
//   setFocus
//---------------------------------------------------------

void HelpQuery::setFocus()
      {
      entry->clear();
      entry->setFocus();
      }

//---------------------------------------------------------
//   textChanged
//---------------------------------------------------------

void HelpQuery::textChanged(const QString& ss)
      {
      QString s = ss.toLower();
      QWidget* menu = static_cast<QWidget*>(parent());
      if (s.isEmpty()) {
            if (!emptyState) {   // restore old menu entries
                  QList<QAction*> al = menu->actions();
                  for (QAction* a : al) {
                        if (a != this)
                              menu->removeAction(a);
                        }
                  for (QAction* a : actions) {
                        if (a != this)
                              menu->addAction(a);
                        }
                  }
            emptyState = true;
            return;
            }
      if (emptyState)
            actions = menu->actions();
      for (QAction* a : menu->actions()) {
            if (a != this)
                  menu->removeAction(a);
            }
      emptyState = false;
      if (!mscore->helpEngine())
            return;
      QMap<QString,QUrl>list = mscore->helpEngine()->linksForIdentifier(s);
//      QMap<QString,QUrl>list = mscore->helpEngine()->indexModel()->linksForKeyword(s);
      int k = 0;
      for (auto i = list.begin(); i != list.end(); ++i) {
            QAction* action = new QAction(i.key(), this);
            action->setData(i.value());
// printf("add action <%s> <%s>\n", qPrintable(i.key()), qPrintable(i.value().toString()));
            menu->addAction(action);
            connect(action, SIGNAL(triggered()), mapper, SLOT(map()));
            mapper->setMapping(action, action);
            if (++k > 10)
                  break;
            }
      }

//---------------------------------------------------------
//   actionTriggered
//---------------------------------------------------------

void HelpQuery::actionTriggered(QObject* obj)
      {
      QAction* action = static_cast<QAction*>(obj);
      if (action->data().isNull())
            return;
      QUrl url = action->data().toUrl();
      if (url.isValid())
            mscore->showHelp(url);
      else
            qDebug("actionTriggered: bad url");
      entry->clear();
      }

//---------------------------------------------------------
//   return pressed
//---------------------------------------------------------

void HelpQuery::returnPressed()
      {
      QHelpEngine* he = mscore->helpEngine();
      if (!he)
            return;
      QMap<QString,QUrl>list = he->linksForIdentifier(entry->text().toLower());
      if (!list.isEmpty()) {
            mscore->showHelp(list.begin().value());
            }
      entry->clear();
      }

//---------------------------------------------------------
//   showHelp
//    show local help
//---------------------------------------------------------

void MuseScore::showHelp(const QUrl& url)
      {
      qDebug("showHelp <%s>", qPrintable(url.toString()));

      if (!_helpEngine)
            return;

      QAction* a = getAction("help");
      a->blockSignals(true);
      a->setChecked(true);
      a->blockSignals(false);

      if (!helpBrowser) {
            helpBrowser = new HelpBrowser;
            manualDock = new QDockWidget(tr("Manual"), 0);
            manualDock->setObjectName("Manual");

            manualDock->setWidget(helpBrowser);
            Qt::DockWidgetArea area = Qt::RightDockWidgetArea;
            addDockWidget(area, manualDock);
            }
      manualDock->show();
      helpBrowser->setContent(url);
      }

void MuseScore::showHelp(QString s)
      {
      qDebug("showHelp <%s>", qPrintable(s));
      s = s.toLower();
      if (!s.isEmpty()) {
            QString help = QString("https://musescore.org/redirect/help?tag=%1&locale=%2").arg(s).arg(getLocaleISOCode());
            help += QString("&utm_source=desktop&utm_medium=contextual&utm_content=%1&utm_term=%2&utm_campaign=MuseScore%3").arg(rev.trimmed()).arg(s).arg(QString(VERSION));
            QDesktopServices::openUrl(QUrl(help));
            }
#if 0
      if (!_helpEngine) {
            qDebug("no help available");
            return;
            }
      s = s.toLower();
      qDebug("showHelp <%s>", qPrintable(s));
      QMap<QString,QUrl>list = _helpEngine->linksForIdentifier(s);
      if (!list.isEmpty())
            showHelp(*list.begin());
      else {
            qDebug("help for <%s> not found", qPrintable(s));
            QMap<QString,QUrl>list = _helpEngine->linksForIdentifier("manual");
            if (!list.isEmpty())
                  showHelp(*list.begin());
            }
#endif
      }

//---------------------------------------------------------
//   showContextHelp
//---------------------------------------------------------

void MuseScore::showContextHelp()
      {
      QString s;
      QWidget* w = qApp->widgetAt(globalX, globalY);
      while (w) {
            if (!w->statusTip().isEmpty()) {
                  s = w->statusTip();
                  break;
                  }
            w = w->parentWidget();
            }
      if (w && s == "scoreview") {
            ScoreView* canvas = static_cast<ScoreView*>(w);
            QPoint pt = w->mapFromGlobal(QPoint(globalX, globalY));
            QPointF p = canvas->toLogical(pt);
            Element* e = canvas->elementNear(p);
            if (e)
                  s = QString("element:%1").arg(e->name());
            }
      showHelp(s);
      }

}  // end namespace Ms

