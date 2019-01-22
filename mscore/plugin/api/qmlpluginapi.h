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

#ifndef __QMLPLUGINAPI_H__
#define __QMLPLUGINAPI_H__

#include "config.h"

#include "plugin/qmlplugin.h"
#include "libmscore/mscore.h"
#include "libmscore/utils.h"

namespace Ms {

class MsProcess;
class Element;
class MScore;

namespace PluginAPI {

class Element;
class FractionWrapper;
class Score;

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
//   @P curScore             Ms::PluginAPI::Score*        current score, if any (read only)
//   @P scores               array[Ms::Score]  all currently open scores (read only)
//---------------------------------------------------------

class PluginAPI : public Ms::QmlPlugin {
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
      Q_PROPERTY(Ms::PluginAPI::Score* curScore     READ curScore)
//TODO-ws      Q_PROPERTY(QQmlListProperty<Ms::Score> scores READ scores)

      QFile logFile;

   signals:
      void run();

   public:
      PluginAPI(QQuickItem* parent = 0);

      static void registerQmlTypes();

      void runPlugin() override            { emit run();       }

      Score* curScore() const;
      QQmlListProperty<Score> scores();

      Q_INVOKABLE Ms::PluginAPI::Score* newScore(const QString& name, const QString& part, int measures);
//       Q_INVOKABLE Ms::PluginAPI::Element* newElement(int);
      Q_INVOKABLE Ms::PluginAPI::Element* newElement(const QString& name);
      Q_INVOKABLE void cmd(const QString&);
      Q_INVOKABLE Ms::MsProcess* newQProcess();
      Q_INVOKABLE bool writeScore(Ms::PluginAPI::Score*, const QString& name, const QString& ext);
      Q_INVOKABLE Ms::PluginAPI::Score* readScore(const QString& name, bool noninteractive = false);
      Q_INVOKABLE void closeScore(Ms::PluginAPI::Score*);

      Q_INVOKABLE void log(const QString&);
      Q_INVOKABLE void logn(const QString&);
      Q_INVOKABLE void log2(const QString&, const QString&);
      Q_INVOKABLE void openLog(const QString&);
      Q_INVOKABLE void closeLog();

      //@ creates a new fraction with the given numerator and denominator
      Q_INVOKABLE Ms::PluginAPI::FractionWrapper* fraction(int numerator, int denominator) const;
      };

} // namespace PluginAPI
} // namespace Ms
#endif
