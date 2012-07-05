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

#ifndef __PLUGIN_CREATOR_H__
#define __PLUGIN_CREATOR_H__

#include "ui_pluginCreator.h"

class QmlPlugin;

//---------------------------------------------------------
//   PluginCreator
//---------------------------------------------------------

class PluginCreator : public QMainWindow, public Ui::PluginCreatorBase {
      Q_OBJECT

      enum PCState { S_INIT, S_EMPTY, S_CLEAN, S_DIRTY };
      PCState state;
      bool created;

      QString path;
      QmlPlugin* item;
      QPointer<QDeclarativeView> view;
      QPointer<QDockWidget> dock;

      void setState(PCState newState);
      void closeEvent(QCloseEvent*);
      void readSettings();
      void setTitle(const QString&);

   private slots:
      void runClicked();
      void stopClicked();
      void loadPlugin();
      void savePlugin();
      void newPlugin();
      void textChanged();
      void closePlugin();
      void qmlWarnings(const QList<QDeclarativeError>&);

   signals:
      void closed();

   public:
      PluginCreator(QWidget* parent = 0);
      void writeSettings();
      void msg(const QString&);
      };

#endif
