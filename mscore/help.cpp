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
#include "musescore.h"

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
      QMap<QString,QUrl>list = mscore->helpEngine()->linksForIdentifier(entry->text().toLower());
      if (!list.isEmpty()) {
            mscore->showHelp(list.begin().value());
            }
      entry->clear();
      }

}  // end namespace Ms

