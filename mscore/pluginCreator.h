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

namespace Ms {

class QmlPlugin;
class HelpBrowser;

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
      HelpBrowser* helpBrowser;
      QDockWidget* manualDock;
      QPointer<QQuickView> view;
      QPointer<QDockWidget> dock;

      void setState(PCState newState);
      virtual void closeEvent(QCloseEvent*);
      void readSettings();
      void setTitle(const QString&);
      QString manualPath();

   private slots:
      void runClicked();
      void stopClicked();
      void loadPlugin();
      void load();
      void savePlugin();
      void newPlugin();
      void textChanged();
      void closePlugin();
      void showManual();
      void qmlWarnings(const QList<QQmlError>&);

   signals:
      void closed(bool);

   public:
      PluginCreator(QWidget* parent = 0);
      void writeSettings();
      void msg(const QString&);
      };


} // namespace Ms
#endif
