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

#ifndef __QMLPLUGIN_H__
#define __QMLPLUGIN_H__

#include "config.h"

#ifdef SCRIPT_INTERFACE
#include "libmscore/mscore.h"

namespace Ms {

class MsProcess;
class Score;
class Element;
class MScore;
class MuseScoreCore;

extern int version();
extern int majorVersion();
extern int minorVersion();
extern int updateVersion();

//---------------------------------------------------------
//   QmlPlugin
//   @@ MuseScore
//   @P menuPath              QString        where the plugin is placed in menu
//   @P version               QString
//   @P description           QString
//   @P pluginType            QString
//   @P dockArea              QString
//   @P division              int            number of MIDI ticks for 1/4 note, read only
//   @P mscoreVersion         int            the complete version number of MuseScore in the form: MMmmuu, read only
//   @P mscoreMajorVersion    int            the 1st part of the MuseScore version, read only
//   @P mscoreMinorVersion    int            the 2nd part of the MuseScore version, read only
//   @P mscoreUpdateVersion   int            the 3rd part of the MuseScore version, read only
//   @P mscoreDPI             qreal          read only
//   @P curScore              Score*         the current score, if any, read only
//   @P scores                array[Score]   all currently open scores, read only
//---------------------------------------------------------

class QmlPlugin : public QDeclarativeItem {
      Q_OBJECT
      Q_PROPERTY(QString menuPath        READ menuPath WRITE setMenuPath)
      Q_PROPERTY(QString version         READ version WRITE setVersion)
      Q_PROPERTY(QString description     READ description WRITE setDescription)
      Q_PROPERTY(QString pluginType      READ pluginType WRITE setPluginType)

      Q_PROPERTY(QString dockArea        READ dockArea WRITE setDockArea)
      Q_PROPERTY(int division            READ division)
      Q_PROPERTY(int mscoreVersion       READ mscoreVersion)
      Q_PROPERTY(int mscoreMajorVersion  READ mscoreMajorVersion)
      Q_PROPERTY(int mscoreMinorVersion  READ mscoreMinorVersion)
      Q_PROPERTY(int mscoreUpdateVersion READ mscoreUpdateVersion)
      Q_PROPERTY(qreal mscoreDPI         READ mscoreDPI)
      Q_PROPERTY(Score* curScore         READ curScore)
      Q_PROPERTY(QDeclarativeListProperty<Score> scores READ scores);

      MuseScoreCore* msc;
      QString _menuPath;
      QString _pluginType;
      QString _dockArea;
      QString _version;
      QString _description;

   signals:
      void run();

   public:
      QmlPlugin(QDeclarativeItem* parent = 0);
      ~QmlPlugin();

      void setMenuPath(const QString& s)   { _menuPath = s;    }
      QString menuPath() const             { return _menuPath; }
      void setVersion(const QString& s)    { _version = s; }
      QString version() const              { return _version; }
      void setDescription(const QString& s) { _description = s; }
      QString description() const          { return _description; }

      void setPluginType(const QString& s) { _pluginType = s;    }
      QString pluginType() const           { return _pluginType; }
      void setDockArea(const QString& s)   { _dockArea = s;    }
      QString dockArea() const             { return _dockArea; }
      void runPlugin()                     { emit run();       }

      int division() const                { return MScore::division; }
      int mscoreVersion() const           { return Ms::version();      }
      int mscoreMajorVersion() const      { return majorVersion();  }
      int mscoreMinorVersion() const      { return minorVersion();  }
      int mscoreUpdateVersion() const     { return updateVersion(); }
      qreal mscoreDPI() const             { return MScore::DPI;     }

      Score* curScore() const;
      QDeclarativeListProperty<Score> scores();

      Q_INVOKABLE Score* newScore(const QString& name, const QString& part, int measures);
      Q_INVOKABLE Element* newElement(int);
      Q_INVOKABLE void cmd(const QString&);
      Q_INVOKABLE MsProcess* newQProcess();
      Q_INVOKABLE bool writeScore(Score*, const QString& name, const QString& ext);
      Q_INVOKABLE Score* readScore(const QString& name);
      };

#endif

} // namespace Ms
#endif

