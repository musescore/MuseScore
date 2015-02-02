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

static QHelpEngineCore* help;

//---------------------------------------------------------
//   HelpQuery
//---------------------------------------------------------

HelpQuery::HelpQuery(QWidget* parent)
   : QWidgetAction(parent)
      {
      if (!help) {
            QString s = getSharePath() + "manual/doc.qhc";
            printf("====<%s>\n", qPrintable(s));
            help = new QHelpEngineCore(s);
            if (!help->setupData()) {
                  delete help;
                  help = 0;
                  printf("cannot setup data for help engine\n");
                  }
            }

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
      QMenu* menu = static_cast<QMenu*>(parent);
      connect(menu, SIGNAL(triggered(QAction*)), SLOT(actionTriggered(QAction*)));
      }

//---------------------------------------------------------
//   textChanged
//---------------------------------------------------------

void HelpQuery::textChanged(const QString& s)
      {
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
      for (const QChar& c : s) {
            QAction* action = new QAction(QString(c), this);
            action->setData(QString(c));
            menu->addAction(action);
            }
      emptyState = false;


      QMap<QString,QUrl>list = help->linksForIdentifier(s);
      printf("=====links %d\n", list.size());
      for (const QUrl& s : list)
            printf("   %s\n", qPrintable(s.toString()));
      }

//---------------------------------------------------------
//   actionTriggered
//---------------------------------------------------------

void HelpQuery::actionTriggered(QAction* action)
      {
      if (action->data().isNull())
            return;
      printf("action <%s>\n", qPrintable(action->data().toString()));
      }


}  // end namespace Ms

