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

#ifndef __HELP_H__
#define __HELP_H__

namespace Ms {

class HelpEngine;

//---------------------------------------------------------
//   HelpQuery
//---------------------------------------------------------

class HelpQuery : public QWidgetAction {
      Q_OBJECT

      QWidget* w;
      QLineEdit* entry;
      QList<QAction*> actions;
      QSignalMapper* mapper;

      bool emptyState;

   private slots:
      void textChanged(const QString&);
      void actionTriggered(QObject*);
      void returnPressed();

   public slots:
      void setFocus();

   public:
      HelpQuery(QWidget* parent);
      };


}     // end namespace Ms


#endif
