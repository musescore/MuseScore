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

class Element;
class MScore;

/**
 * \namespace Ms::PluginAPI
 * Contains items exposed to the QML plugins framework.
 */

namespace PluginAPI {

class Element;
class FractionWrapper;
class MsProcess;
class Score;

#define DECLARE_API_ENUM(qmlName, cppName) \
      Q_PROPERTY(Ms::PluginAPI::Enum* qmlName READ get_##cppName CONSTANT) \
      static Enum* cppName; \
      static Enum* get_##cppName() { return cppName; }

//---------------------------------------------------------
///   \class PluginAPI
///   \brief Main class of the plugins framework.\ Named
///   as \p MuseScore in QML.
///   \details This class is exposed to QML plugins
///   framework under \p MuseScore name and is the root
///   component of each MuseScore plugin.
//   @P scores               array[Ms::Score]  all currently open scores (read only)
//---------------------------------------------------------

class PluginAPI : public Ms::QmlPlugin {
      Q_OBJECT
      /** Path where the plugin is placed in menu */
      Q_PROPERTY(QString menuPath        READ menuPath WRITE setMenuPath)
      /** Source file path, without the file name (read only) */
      Q_PROPERTY(QString filePath        READ filePath)
      /** Version of this plugin */
      Q_PROPERTY(QString version         READ version WRITE setVersion)
      /** Human-readable plugin description, displayed in Plugin Manager */
      Q_PROPERTY(QString description     READ description WRITE setDescription)
      /** Type may be dialog, dock, or not defined */
      Q_PROPERTY(QString pluginType      READ pluginType WRITE setPluginType)
      /** Where to dock on main screen. Possible values: left, top, bottom, right */
      Q_PROPERTY(QString dockArea        READ dockArea WRITE setDockArea)
      /** Whether the plugin requires an existing score to run, default is `true` */
      Q_PROPERTY(bool requiresScore      READ requiresScore WRITE setRequiresScore)
      /** Number of MIDI ticks for 1/4 note (read only) */
      Q_PROPERTY(int division            READ division)
      /** Complete version number of MuseScore in the form: MMmmuu (read only) */
      Q_PROPERTY(int mscoreVersion       READ mscoreVersion       CONSTANT)
      /** 1st part of the MuseScore version (read only) */
      Q_PROPERTY(int mscoreMajorVersion  READ mscoreMajorVersion  CONSTANT)
      /** 2nd part of the MuseScore version (read only)*/
      Q_PROPERTY(int mscoreMinorVersion  READ mscoreMinorVersion  CONSTANT)
      /** 3rd part of the MuseScore version (read only) */
      Q_PROPERTY(int mscoreUpdateVersion READ mscoreUpdateVersion CONSTANT)
      /** (read-only) */
      Q_PROPERTY(qreal mscoreDPI         READ mscoreDPI)
      /** Current score, if any (read only) */
      Q_PROPERTY(Ms::PluginAPI::Score* curScore     READ curScore)
      /** List of currently open scores (read only).\n \since MuseScore 3.2 */
      Q_PROPERTY(QQmlListProperty<Ms::PluginAPI::Score> scores READ scores)

      // Should be initialized in qmlpluginapi.cpp
      /// Contains Ms::ElementType enumeration values
      DECLARE_API_ENUM( Element,          elementTypeEnum         )
      /// Contains Ms::AccidentalType enumeration values
      DECLARE_API_ENUM( Accidental,       accidentalTypeEnum      )
      /// Contains Ms::Beam::Mode enumeration values
      DECLARE_API_ENUM( Beam,             beamModeEnum            )
      /// Contains Ms::Placement enumeration values
      /// \note In MuseScore 2.X this enumeration was available as
      /// Element.ABOVE and Element.BELOW.
      DECLARE_API_ENUM( Placement,        placementEnum           )
      /// Contains Ms::GlissandoType enumeration values
      DECLARE_API_ENUM( Glissando,        glissandoTypeEnum       ) // was probably absent in 2.X
      /// Contains Ms::LayoutBreak::Type enumeration values
      DECLARE_API_ENUM( LayoutBreak,      layoutBreakTypeEnum     )
      /// Contains Ms::Lyrics::Syllabic enumeration values
      DECLARE_API_ENUM( Lyrics,           lyricsSyllabicEnum      )
      /// Contains Ms::Direction enumeration values
      /// \note In MuseScore 2.X this enumeration was available as
      /// MScore.UP, MScore.DOWN, MScore.AUTO.
      DECLARE_API_ENUM( Direction,        directionEnum           )
      /// Contains Ms::MScore::DirectionH enumeration values
      /// \note In MuseScore 2.X this enumeration was available as
      /// MScore.LEFT, MScore.RIGHT, MScore.AUTO.
      DECLARE_API_ENUM( DirectionH,       directionHEnum          )
      /// Contains Ms::MScore::OrnamentStyle enumeration values
      /// \note In MuseScore 2.X this enumeration was available as
      /// MScore.DEFAULT, MScore.BAROQUE.
      DECLARE_API_ENUM( OrnamentStyle,    ornamentStyleEnum       )
      /// Contains Ms::GlissandoStyle enumeration values
      /// \note In MuseScore 2.X this enumeration was available as
      /// MScore.CHROMATIC, MScore.WHITE_KEYS, MScore.BLACK_KEYS,
      /// MScore.DIATONIC.
      DECLARE_API_ENUM( GlissandoStyle,   glissandoStyleEnum      )
      /// Contains Ms::Tid enumeration values
      /// \note In MuseScore 2.X this enumeration was available as
      /// TextStyleType (TextStyleType.TITLE etc.)
      DECLARE_API_ENUM( Tid,              tidEnum                 )
      /// Contains Ms::NoteType enumeration values 
      /// \since MuseScore 3.2.1
      DECLARE_API_ENUM( NoteType,         noteTypeEnum            )
      /// Contains Ms::NoteHead::Type enumeration values
      /// \note In MuseScore 2.X this enumeration was available in
      /// NoteHead class (e.g. NoteHead.HEAD_QUARTER).
      DECLARE_API_ENUM( NoteHeadType,     noteHeadTypeEnum        )
      /// Contains Ms::NoteHead::Group enumeration values
      /// \note In MuseScore 2.X this enumeration was available in
      /// NoteHead class (e.g. NoteHead.HEAD_TRIANGLE).
      DECLARE_API_ENUM( NoteHeadGroup,    noteHeadGroupEnum       )
      /// Contains Ms::Note::ValueType enumeration values
      /// \note In MuseScore 2.X this enumeration was available as
      /// Note.OFFSET_VAL, Note.USER_VAL
      DECLARE_API_ENUM( NoteValueType,    noteValueTypeEnum       )
      /// Contains Ms::SegmentType enumeration values
      DECLARE_API_ENUM( Segment,          segmentTypeEnum         )
      DECLARE_API_ENUM( Spanner,          spannerAnchorEnum       ) // probably unavailable in 2.X

      QFile logFile;

      static void initEnums();

   signals:
      /// Indicates that the plugin was launched.
      /// Implement \p onRun() function in your plugin to handle this signal.
      void run();

   public:
      /// \cond MS_INTERNAL
      PluginAPI(QQuickItem* parent = 0);

      static void registerQmlTypes();

      void runPlugin() override            { emit run();       }

      Score* curScore() const;
      QQmlListProperty<Score> scores();
      /// \endcond

      Q_INVOKABLE Ms::PluginAPI::Score* newScore(const QString& name, const QString& part, int measures);
      Q_INVOKABLE Ms::PluginAPI::Element* newElement(int);
      Q_INVOKABLE void cmd(const QString&);
      /** \cond PLUGIN_API \private \endcond */
      Q_INVOKABLE Ms::PluginAPI::MsProcess* newQProcess();
      Q_INVOKABLE bool writeScore(Ms::PluginAPI::Score*, const QString& name, const QString& ext);
      Q_INVOKABLE Ms::PluginAPI::Score* readScore(const QString& name, bool noninteractive = false);
      Q_INVOKABLE void closeScore(Ms::PluginAPI::Score*);

      Q_INVOKABLE void log(const QString&);
      Q_INVOKABLE void logn(const QString&);
      Q_INVOKABLE void log2(const QString&, const QString&);
      Q_INVOKABLE void openLog(const QString&);
      Q_INVOKABLE void closeLog();

      Q_INVOKABLE Ms::PluginAPI::FractionWrapper* fraction(int numerator, int denominator) const;
      };

#undef DECLARE_API_ENUM
} // namespace PluginAPI
} // namespace Ms
#endif
