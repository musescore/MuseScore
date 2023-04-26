/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __QMLPLUGINAPI_H__
#define __QMLPLUGINAPI_H__

#include <QFile>

#include "qmlplugin.h"
#include "enums.h"
#include "framework/actions/iactionsdispatcher.h"
#include "context/iglobalcontext.h"
#include "modularity/ioc.h"

#include "engraving/libmscore/score.h"
#include "engraving/libmscore/types.h"
#include "engraving/types/types.h"

#include "apitypes.h"

namespace mu::engraving {
class EngravingItem;
}

/**
 * \namespace mu::plugins::api
 * Contains items exposed to the QML plugins framework.
 */

namespace mu::plugins::api {
class EngravingItem;
class FractionWrapper;
class MsProcess;
class Score;

#define DECLARE_API_ENUM(qmlName, cppName, enumName) \
    Q_PROPERTY(mu::plugins::api::Enum * qmlName READ get_##cppName CONSTANT) \
    static Enum* cppName; \
    static Enum* get_##cppName() { \
        if (!cppName) \
        cppName = wrapEnum<enumName>(); \
        return cppName; \
    }

//---------------------------------------------------------
///   \class PluginAPI
///   \brief Main class of the plugins framework.\ Named
///   as \p MuseScore in QML.
///   \details This class is exposed to QML plugins
///   framework under \p MuseScore name and is the root
///   component of each MuseScore plugin.
//   @P scores               array[mu::engraving::Score]  all currently open scores (read only)
//---------------------------------------------------------

class PluginAPI : public QmlPlugin
{
    Q_OBJECT

    INJECT(plugins, mu::actions::IActionsDispatcher, actionsDispatcher)
    INJECT(plugins, mu::context::IGlobalContext, context)

    /** Path where the plugin is placed in menu */
    Q_PROPERTY(QString menuPath READ menuPath WRITE setMenuPath)
    /** Title of this plugin */
    Q_PROPERTY(QString title READ title WRITE setTitle)
    /** Source file path, without the file name (read only) */
    Q_PROPERTY(QString filePath READ filePath)
    /** Version of this plugin */
    Q_PROPERTY(QString version READ version WRITE setVersion)
    /** Human-readable plugin description, displayed in Plugin Manager */
    Q_PROPERTY(QString description READ description WRITE setDescription)
    /** Type may be dialog, dock, or not defined */
    Q_PROPERTY(QString pluginType READ pluginType WRITE setPluginType)
    /** Where to dock on main screen. Possible values: left, top, bottom, right */
    Q_PROPERTY(QString dockArea READ dockArea WRITE setDockArea)
    /** Whether the plugin requires an existing score to run, default is `true` */
    Q_PROPERTY(bool requiresScore READ requiresScore WRITE setRequiresScore)
    /** The name of the thumbnail that should be next to the plugin */
    Q_PROPERTY(QString thumbnailName READ thumbnailName WRITE setThumbnailName)
    /** The code of the category */
    Q_PROPERTY(QString categoryCode READ categoryCode WRITE setCategoryCode)
    /**
     * \brief Number of MIDI ticks for 1/4 note (read only)
     * \see \ref ticklength
     */
    Q_PROPERTY(int division READ division)
    /** Complete version number of MuseScore in the form: MMmmuu (read only) */
    Q_PROPERTY(int mscoreVersion READ mscoreVersion CONSTANT)
    /** 1st part of the MuseScore version (read only) */
    Q_PROPERTY(int mscoreMajorVersion READ mscoreMajorVersion CONSTANT)
    /** 2nd part of the MuseScore version (read only)*/
    Q_PROPERTY(int mscoreMinorVersion READ mscoreMinorVersion CONSTANT)
    /** 3rd part of the MuseScore version (read only) */
    Q_PROPERTY(int mscoreUpdateVersion READ mscoreUpdateVersion CONSTANT)
    /** (read-only) */
    Q_PROPERTY(qreal mscoreDPI READ mscoreDPI)
    /** Current score, if any (read only) */
    Q_PROPERTY(mu::plugins::api::Score * curScore READ curScore CONSTANT)
    /** List of currently open scores (read only).\n \since MuseScore 3.2 */
    Q_PROPERTY(QQmlListProperty<mu::plugins::api::Score> scores READ scores)

    // Should be initialized in qmlpluginapi.cpp
    /// Contains mu::engraving::ElementType enumeration values
    DECLARE_API_ENUM(Element,          elementTypeEnum,        mu::plugins::api::enums::ElementType)
    /// Contains mu::engraving::AccidentalType enumeration values
    DECLARE_API_ENUM(Accidental,       accidentalTypeEnum,     mu::plugins::api::enums::AccidentalType)
    /// Contains mu::engraving::BeamMode enumeration values
    DECLARE_API_ENUM(Beam,             beamModeEnum,           mu::plugins::api::enums::BeamMode)
    /// Contains mu::engraving::Placement enumeration values
    /// \note In MuseScore 2.X this enumeration was available as
    /// EngravingItem.ABOVE and EngravingItem.BELOW.
    DECLARE_API_ENUM(Placement,        placementEnum,          mu::plugins::api::enums::Placement)
    /// Contains mu::engraving::GlissandoType enumeration values
    DECLARE_API_ENUM(Glissando,        glissandoTypeEnum,      mu::plugins::api::enums::GlissandoType) // was probably absent in 2.X
    /// Contains mu::engraving::LayoutBreak::Type enumeration values
    DECLARE_API_ENUM(LayoutBreak,      layoutBreakTypeEnum,    mu::plugins::api::enums::LayoutBreakType)
    /// Contains mu::engraving::Lyrics::Syllabic enumeration values
    DECLARE_API_ENUM(Lyrics,           lyricsSyllabicEnum,     mu::plugins::api::enums::Syllabic)
    /// Contains mu::engraving::Direction enumeration values
    /// \note In MuseScore 2.X this enumeration was available as
    /// MScore.UP, MScore.DOWN, MScore.AUTO.
    DECLARE_API_ENUM(Direction,        directionEnum,          mu::plugins::api::enums::Direction)
    /// Contains mu::engraving::DirectionH enumeration values
    /// \note In MuseScore 2.X this enumeration was available as
    /// MScore.LEFT, MScore.RIGHT, MScore.AUTO.
    DECLARE_API_ENUM(DirectionH,       directionHEnum,         mu::plugins::api::enums::DirectionH)
    /// Contains mu::engraving::OrnamentStyle enumeration values
    /// \note In MuseScore 2.X this enumeration was available as
    /// MScore.DEFAULT, MScore.BAROQUE.
    DECLARE_API_ENUM(OrnamentStyle,    ornamentStyleEnum,      mu::plugins::api::enums::OrnamentStyle)
    /// Contains mu::engraving::GlissandoStyle enumeration values
    /// \note In MuseScore 2.X this enumeration was available as
    /// MScore.CHROMATIC, MScore.WHITE_KEYS, MScore.BLACK_KEYS,
    /// MScore.DIATONIC.
    DECLARE_API_ENUM(GlissandoStyle,   glissandoStyleEnum,     mu::plugins::api::enums::GlissandoStyle)
    /// Contains mu::engraving::Tid enumeration values
    /// \note In MuseScore 2.X this enumeration was available as
    /// TextStyleType (TextStyleType.TITLE etc.)
    DECLARE_API_ENUM(Tid,              tidEnum,                mu::plugins::api::enums::Tid)
    /// Contains mu::engraving::Align enumeration values
    /// \since MuseScore 3.3
    DECLARE_API_ENUM(Align,            alignEnum,              mu::plugins::api::enums::Align)
    /// Contains mu::engraving::NoteType enumeration values
    /// \since MuseScore 3.2.1
    DECLARE_API_ENUM(NoteType,         noteTypeEnum,           mu::plugins::api::enums::NoteType)
    /// Contains mu::engraving::PlayEventType enumeration values
    /// \since MuseScore 3.3
    DECLARE_API_ENUM(PlayEventType,    playEventTypeEnum,      mu::plugins::api::enums::PlayEventType)
    /// Contains mu::engraving::NoteHead::Type enumeration values
    /// \note In MuseScore 2.X this enumeration was available in
    /// NoteHead class (e.g. NoteHead.HEAD_QUARTER).
    DECLARE_API_ENUM(NoteHeadType,     noteHeadTypeEnum,       mu::plugins::api::enums::NoteHeadType)
    /// Contains mu::engraving::NoteHead::Scheme enumeration values
    /// \since MuseScore 3.5
    DECLARE_API_ENUM(NoteHeadScheme,   noteHeadSchemeEnum,     mu::plugins::api::enums::NoteHeadScheme)
    /// Contains mu::engraving::NoteHead::Group enumeration values
    /// \note In MuseScore 2.X this enumeration was available in
    /// NoteHead class (e.g. NoteHead.HEAD_TRIANGLE).
    DECLARE_API_ENUM(NoteHeadGroup,    noteHeadGroupEnum,      mu::plugins::api::enums::NoteHeadGroup)
    /// Contains mu::engraving::ValueType enumeration values
    /// \note In MuseScore 2.X this enumeration was available as
    /// Note.OFFSET_VAL, Note.USER_VAL
    DECLARE_API_ENUM(NoteValueType,    noteValueTypeEnum,      mu::plugins::api::enums::VeloType)
    /// Contains mu::engraving::SegmentType enumeration values
    DECLARE_API_ENUM(Segment,          segmentTypeEnum,        mu::plugins::api::enums::SegmentType)
    DECLARE_API_ENUM(Spanner,          spannerAnchorEnum,      mu::plugins::api::enums::Anchor)           // probably unavailable in 2.X
    /// Contains mu::engraving::SymId enumeration values
    /// \since MuseScore 3.5
    DECLARE_API_ENUM(SymId,            symIdEnum,              mu::plugins::api::enums::SymId)
    /// Contains mu::engraving::HarmonyType enumeration values
    /// \since MuseScore 3.6
    DECLARE_API_ENUM(HarmonyType,      harmonyTypeEnum,        mu::plugins::api::enums::HarmonyType)

    QFile logFile;

signals:
    /// Indicates that the plugin was launched.
    /// Implement \p onRun() function in your plugin to handle this signal.
    void run();

    /**
     * Notifies plugin about changes in score state.
     * Called after each user (or plugin) action which may have changed a
     * score. Implement \p onScoreStateChanged() function in your plugin to
     * handle this signal.
     *
     * \p state variable is available within the handler with following
     * fields:
     *
     * - \p selectionChanged
     * - \p excerptsChanged
     * - \p instrumentsChanged
     * - \p startLayoutTick
     * - \p endLayoutTick
     * - \p undoRedo - whether this onScoreStateChanged invocation results
     *   from user undo/redo action. It is usually not recommended to modify
     *   score from plugins in this case. Available since MuseScore 3.5.
     *
     * If a plugin modifies score in this handler, then it should:
     * 1. enclose all modifications within Score::startCmd() / Score::endCmd()
     * 2. take care of preventing an infinite recursion, as plugin-originated
     *    changes will trigger this signal again after calling Score::endCmd()
     *
     * Example:
     * \code
     * import QtQuick 2.0
     * import MuseScore 3.0
     *
     * MuseScore {
     *     menuPath: "Plugins.selectionChangeExample"
     *     pluginType: "dock"
     *     dockArea:   "left"
     *     implicitHeight: 75 // necessary for dock widget to appear with nonzero height
     *     implicitWidth: 150
     *
     *     Text {
     *        // A label which will display pitch of the currently selected note
     *        id: pitchLabel
     *        anchors.fill: parent
     *     }
     *
     *     onScoreStateChanged: {
     *         if (state.selectionChanged) {
     *             var el = curScore ? curScore.selection.elements[0] : null;
     *             if (el && el.type == EngravingItem.NOTE)
     *                 pitchLabel.text = el.pitch;
     *             else
     *                 pitchLabel.text = "no note selected";
     *         }
     *     }
     * }
     * \endcode
     * \warning This functionality is considered experimental.
     * This API may change in future versions of MuseScore.
     * \since MuseScore 3.3
     */
    void scoreStateChanged(const QMap<QString, QVariant>& state);

public:
    /// \cond MS_INTERNAL
    PluginAPI(QQuickItem* parent = 0);

    static void registerQmlTypes();

    void runPlugin() override { emit run(); }
    void endCmd(const QMap<QString, QVariant>& stateInfo) override { emit scoreStateChanged(stateInfo); }

    Score* curScore() const;
    QQmlListProperty<Score> scores();
    /// \endcond

    Q_INVOKABLE mu::plugins::api::Score* newScore(const QString& name, const QString& part, int measures);
    Q_INVOKABLE mu::plugins::api::EngravingItem* newElement(int);
    Q_INVOKABLE void removeElement(mu::plugins::api::EngravingItem* wrapped);
    Q_INVOKABLE void cmd(const QString&);
    /** \cond PLUGIN_API \private \endcond */
    Q_INVOKABLE mu::plugins::api::MsProcess* newQProcess();
    Q_INVOKABLE bool writeScore(mu::plugins::api::Score*, const QString& name, const QString& ext);
    Q_INVOKABLE mu::plugins::api::Score* readScore(const QString& name, bool noninteractive = false);
    Q_INVOKABLE void closeScore(mu::plugins::api::Score*);

    Q_INVOKABLE void log(const QString&);
    Q_INVOKABLE void logn(const QString&);
    Q_INVOKABLE void log2(const QString&, const QString&);
    Q_INVOKABLE void openLog(const QString&);
    Q_INVOKABLE void closeLog();

    Q_INVOKABLE mu::plugins::api::FractionWrapper* fraction(int numerator, int denominator) const;

    Q_INVOKABLE void quit();

private:
    mu::engraving::Score* currentScore() const;
};

#undef DECLARE_API_ENUM
} // namespace mu::plugins::api

#endif
