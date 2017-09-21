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
#include "libmscore/cursor.h"

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
//   @P menuPath             QString           where the plugin is placed in menu
//   @P filePath             QString           source file path, without the file name (read only)
//   @P version              QString           version of this plugin
//   @P description          QString           human readable description, displayed in Plugin Manager
//   @P pluginType           QString           type may be dialog, dock, or not defined.
//   @P dockArea             QString           where to dock on main screen. left,top,bottom, right(default)
//   @P requiresScore        bool              whether the plugin requires an existing score to run
//   @P division             int               number of MIDI ticks for 1/4 note (read only)
//   @P mscoreVersion        int               complete version number of MuseScore in the form: MMmmuu (read only)
//   @P mscoreMajorVersion   int               1st part of the MuseScore version (read only)
//   @P mscoreMinorVersion   int               2nd part of the MuseScore version (read only)
//   @P mscoreUpdateVersion  int               3rd part of the MuseScore version (read only)
//   @P mscoreDPI            qreal             (read only)
//   @P curScore             Ms::Score*        current score, if any (read only)
//   @P scores               array[Ms::Score]  all currently open scores (read only)
//---------------------------------------------------------

class QmlPlugin : public QQuickItem {
      Q_OBJECT
      Q_PROPERTY(QString menuPath        READ menuPath WRITE setMenuPath)
      Q_PROPERTY(QString filePath        READ filePath)
      Q_PROPERTY(QString version         READ version WRITE setVersion)
      Q_PROPERTY(QString description     READ description WRITE setDescription)
      Q_PROPERTY(QString pluginType      READ pluginType WRITE setPluginType)

      Q_PROPERTY(QString dockArea        READ dockArea WRITE setDockArea)
      Q_PROPERTY(bool requiresScore      READ requiresScore WRITE setRequiresScore)
      Q_PROPERTY(int division            READ division)
      Q_PROPERTY(int mscoreVersion       READ mscoreVersion)
      Q_PROPERTY(int mscoreMajorVersion  READ mscoreMajorVersion)
      Q_PROPERTY(int mscoreMinorVersion  READ mscoreMinorVersion)
      Q_PROPERTY(int mscoreUpdateVersion READ mscoreUpdateVersion)
      Q_PROPERTY(qreal mscoreDPI         READ mscoreDPI)
      Q_PROPERTY(Ms::Score* curScore     READ curScore)
//TODO-ws      Q_PROPERTY(QQmlListProperty<Ms::Score> scores READ scores)

      MuseScoreCore* msc;
      QString _menuPath;
      QString _pluginType;
      QString _dockArea;
      bool    _requiresScore;
      QString _version;
      QString _description;
      QFile logFile;

   protected:
      QString _filePath;            // the path of the source file, without file name
   signals:
      void run();

   public:
      QmlPlugin(QQuickItem* parent = 0);
      ~QmlPlugin();

      void setMenuPath(const QString& s)   { _menuPath = s;    }
      QString menuPath() const             { return _menuPath; }
      void setVersion(const QString& s)    { _version = s; }
      QString version() const              { return _version; }
      void setDescription(const QString& s) { _description = s; }
      QString description() const          { return _description; }
      void setFilePath(const QString s)   { _filePath = s;        }
      QString filePath() const            { return _filePath;     }

      void setPluginType(const QString& s) { _pluginType = s;    }
      QString pluginType() const           { return _pluginType; }
      void setDockArea(const QString& s)   { _dockArea = s;    }
      QString dockArea() const             { return _dockArea; }
      void runPlugin()                     { emit run();       }
      void setRequiresScore(bool b)        { _requiresScore = b;    }
      bool requiresScore() const           { return _requiresScore; }

      int division() const                { return MScore::division; }
      int mscoreVersion() const           { return Ms::version();      }
      int mscoreMajorVersion() const      { return majorVersion();  }
      int mscoreMinorVersion() const      { return minorVersion();  }
      int mscoreUpdateVersion() const     { return updateVersion(); }
      qreal mscoreDPI() const             { return DPI;     }

      Score* curScore() const;
      QQmlListProperty<Score> scores();

      Q_INVOKABLE Ms::Score* newScore(const QString& name, const QString& part, int measures);
      Q_INVOKABLE Ms::ElementW* newElement(int);
      Q_INVOKABLE void cmd(const QString&);
      Q_INVOKABLE Ms::MsProcess* newQProcess();
      Q_INVOKABLE bool writeScore(Ms::Score*, const QString& name, const QString& ext);
      Q_INVOKABLE Ms::Score* readScore(const QString& name, bool noninteractive = false);
      Q_INVOKABLE void closeScore(Ms::Score*);

      Q_INVOKABLE void log(const QString&);
      Q_INVOKABLE void logn(const QString&);
      Q_INVOKABLE void log2(const QString&, const QString&);
      Q_INVOKABLE void openLog(const QString&);
      Q_INVOKABLE void closeLog();
      };


} // namespace Ms
#endif
#endif
