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

#include "../qmlplugin.h"
#include "enums.h"
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

      Enum* elementTypeEnum;
      Q_PROPERTY(Ms::PluginAPI::Enum* Element MEMBER elementTypeEnum)
      Enum* accidentalTypeEnum;
      Q_PROPERTY(Ms::PluginAPI::Enum* Accidental MEMBER accidentalTypeEnum)
      Enum* beamModeEnum;
      Q_PROPERTY(Ms::PluginAPI::Enum* Beam MEMBER beamModeEnum)
      Enum* placementEnum;
      Q_PROPERTY(Ms::PluginAPI::Enum* Placement MEMBER placementEnum) // was Element.ABOVE and Element.BELOW in 2.X
      Enum* glissandoTypeEnum;
      Q_PROPERTY(Ms::PluginAPI::Enum* Glissando MEMBER glissandoTypeEnum) // was probably absent in 2.X
      Enum* layoutBreakTypeEnum;
      Q_PROPERTY(Ms::PluginAPI::Enum* LayoutBreak MEMBER layoutBreakTypeEnum)
      Enum* lyricsSyllabicEnum;
      Q_PROPERTY(Ms::PluginAPI::Enum* Lyrics MEMBER lyricsSyllabicEnum)
      Enum* directionEnum;
      Q_PROPERTY(Ms::PluginAPI::Enum* Direction MEMBER directionEnum) // was in MScore class in 2.X
      Enum* directionHEnum;
      Q_PROPERTY(Ms::PluginAPI::Enum* DirectionH MEMBER directionHEnum) // was in MScore class in 2.X
      Enum* ornamentStyleEnum;
      Q_PROPERTY(Ms::PluginAPI::Enum* OrnamentStyle MEMBER ornamentStyleEnum) // was in MScore class in 2.X
      Enum* glissandoStyleEnum;
      Q_PROPERTY(Ms::PluginAPI::Enum* GlissandoStyle MEMBER glissandoStyleEnum) // was in MScore class in 2.X
      Enum* tidEnum;
      Q_PROPERTY(Ms::PluginAPI::Enum* Tid MEMBER tidEnum) // was TextStyleType in 2.X
      Enum* noteHeadTypeEnum;
      Q_PROPERTY(Ms::PluginAPI::Enum* NoteHeadType MEMBER noteHeadTypeEnum) // was in NoteHead class in 2.X
      Enum* noteHeadGroupEnum;
      Q_PROPERTY(Ms::PluginAPI::Enum* NoteHeadGroup MEMBER noteHeadGroupEnum) // was in NoteHead class in 2.X
      Enum* noteValueTypeEnum; // or velo type?
      Q_PROPERTY(Ms::PluginAPI::Enum* NoteValueType MEMBER noteValueTypeEnum) // was in Note class in 2.X
      Enum* segmentTypeEnum;
      Q_PROPERTY(Ms::PluginAPI::Enum* Segment MEMBER segmentTypeEnum);
      Enum* spannerAnchorEnum;
      Q_PROPERTY(Ms::PluginAPI::Enum* Spanner MEMBER spannerAnchorEnum); // probably unavailable in 2.X

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
      Q_INVOKABLE Ms::PluginAPI::Element* newElement(int);
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
