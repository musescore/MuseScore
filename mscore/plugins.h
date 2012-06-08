//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __PLUGINS_H__
#define __PLUGINS_H__

#include "musescore.h"
#include "libmscore/score.h"

class Cursor;

//---------------------------------------------------------
//   QmlPlugin
//---------------------------------------------------------

class QmlPlugin : public QDeclarativeItem {
      Q_OBJECT
      Q_PROPERTY(QString menuPath READ menuPath WRITE setMenuPath)
      Q_PROPERTY(QString pluginType READ pluginType WRITE setPluginType)
      Q_PROPERTY(QString dockArea READ dockArea WRITE setDockArea)
      Q_PROPERTY(int mscoreVersion READ mscoreVersion)
      Q_PROPERTY(Score* curScore READ curScore)
      Q_PROPERTY(QDeclarativeListProperty<Score> scores READ scores);

      QString _menuPath;
      QString _pluginType;
      QString _dockArea;

   signals:
      void run();

   public:
      QmlPlugin(QDeclarativeItem* parent = 0);
      ~QmlPlugin();

      void setMenuPath(const QString& s)   { _menuPath = s;    }
      QString menuPath() const             { return _menuPath; }
      void setPluginType(const QString& s) { _pluginType = s;    }
      QString pluginType() const           { return _pluginType; }
      void setDockArea(const QString& s)   { _dockArea = s;    }
      QString dockArea() const             { return _dockArea; }
      void runPlugin()                     { emit run();       }

      int mscoreVersion() const;
      Score* curScore() const;
      QDeclarativeListProperty<Score> scores();

      Q_INVOKABLE Score* newScore(const QString& name, const QString& part, int measures);
      Q_INVOKABLE Cursor* newCursor();
      Q_INVOKABLE Element* newElement(int);
      Q_INVOKABLE void cmd(const QString&);
      };

#endif

