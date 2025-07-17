/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#pragma once

#include <QQuickItem>

#include "extensions/api/v1/ipluginapiv1.h"

#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"
#include "context/iglobalcontext.h"
#include "global/iapplication.h"

#include "enums.h"
#include "apitypes.h"
#include "cursor.h"

namespace mu::engraving {
class EngravingItem;
}

/**
 * \namespace mu::plugins::api
 * Contains items exposed to the QML plugins framework.
 */

namespace mu::engraving::apiv1 {
class EngravingItem;
class FractionWrapper;
class OrnamentIntervalWrapper;
class MsProcess;
class Score;

#define DECLARE_API_ENUM(qmlName, cppName, enumName) \
    /** Accessed using qmlName.VALUE*/ \
    Q_PROPERTY(mu::engraving::apiv1::Enum * qmlName READ get_##cppName CONSTANT) \
    static Enum* cppName; \
    static Enum* get_##cppName() { \
        if (!cppName) { \
            cppName = wrapEnum<enumName>(); \
            cppName->freeze(); \
        } \
        return cppName; \
    }

#define DECLARE_API_ENUM2(qmlName, cppName, enumName1, enumName2) \
    /** Accessed using qmlName.VALUE*/ \
    Q_PROPERTY(mu::engraving::apiv1::Enum * qmlName READ get_##cppName CONSTANT) \
    static Enum* cppName; \
    static Enum* get_##cppName() { \
        if (!cppName) { \
            cppName = wrapEnum<enumName1>(); \
            cppName->add(QMetaEnum::fromType<enumName2>()); \
            cppName->freeze(); \
        } \
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

class PluginAPI : public QQuickItem, public muse::extensions::apiv1::IPluginApiV1, public muse::Injectable
{
    Q_OBJECT

    /// Path where the plugin is placed in menu
    Q_PROPERTY(QString menuPath READ menuPath WRITE setMenuPath)
    /// Title of this plugin
    Q_PROPERTY(QString title READ title WRITE setTitle)
    // /// Source file path, without the file name (read only)
    // Q_PROPERTY(QString filePath READ filePath)
    /// Version of this plugin
    Q_PROPERTY(QString version READ version WRITE setVersion)
    /// Human-readable plugin description, displayed in Plugin Manager
    Q_PROPERTY(QString description READ description WRITE setDescription)
    /// Type may be dialog or not defined
    Q_PROPERTY(QString pluginType READ pluginType WRITE setPluginType)
    /// Where to dock on main screen. Possible values: left, top, bottom, right
    Q_PROPERTY(QString dockArea READ dockArea WRITE setDockArea)
    /// Whether the plugin requires an existing score to run, default is `true`
    Q_PROPERTY(bool requiresScore READ requiresScore WRITE setRequiresScore)
    /// The name of the thumbnail that should be next to the plugin
    Q_PROPERTY(QString thumbnailName READ thumbnailName WRITE setThumbnailName)
    /// The code of the category
    Q_PROPERTY(QString categoryCode READ categoryCode WRITE setCategoryCode)
    /// \brief Number of MIDI ticks for 1/4 note (read only)
    /// \see \ref ticklength
    Q_PROPERTY(int division READ division CONSTANT)
    /// Complete version number of MuseScore in the form: MMmmuu (read only)
    Q_PROPERTY(int mscoreVersion READ mscoreVersion CONSTANT)
    /// 1st part of the MuseScore version (read only)
    Q_PROPERTY(int mscoreMajorVersion READ mscoreMajorVersion CONSTANT)
    /// 2nd part of the MuseScore version (read only)
    Q_PROPERTY(int mscoreMinorVersion READ mscoreMinorVersion CONSTANT)
    /// 3rd part of the MuseScore version (read only)
    Q_PROPERTY(int mscoreUpdateVersion READ mscoreUpdateVersion CONSTANT)
    /// (read-only)
    Q_PROPERTY(qreal mscoreDPI READ mscoreDPI CONSTANT)
    /// Current score, if any (read only)
    Q_PROPERTY(mu::engraving::apiv1::Score * curScore READ curScore CONSTANT)
    /// List of currently open scores (read only).\n \since MuseScore 3.2
    Q_PROPERTY(QQmlListProperty<mu::engraving::apiv1::Score> scores READ scores)

public:
    muse::Inject<muse::actions::IActionsDispatcher> actionsDispatcher = { this };
    muse::Inject<mu::context::IGlobalContext> context = { this };
    muse::Inject<muse::IApplication> application = { this };

public:
    // Should be initialized in qmlpluginapi.cpp
    /// Contains mu::engraving::ElementType enumeration values
    DECLARE_API_ENUM(Element,          elementTypeEnum,        mu::engraving::apiv1::enums::ElementType)
    /// Contains mu::engraving::AccidentalType enumeration values
    DECLARE_API_ENUM(Accidental,       accidentalTypeEnum,     mu::engraving::apiv1::enums::AccidentalType)
    /// Contains mu::engraving::AccidentalBracket enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(AccidentalBracket, accidentalBracketEnum, mu::engraving::apiv1::enums::AccidentalBracket)
    /// Contains mu::engraving::OrnamentStyle enumeration values
    /// \note In MuseScore 2.X this enumeration was available as
    /// MScore.DEFAULT, MScore.BAROQUE.
    DECLARE_API_ENUM(OrnamentStyle,    ornamentStyleEnum,      mu::engraving::apiv1::enums::OrnamentStyle)
    /// Contains mu::engraving::Align enumeration values
    /// \since MuseScore 3.3
    DECLARE_API_ENUM(Align,            alignEnum,              mu::engraving::apiv1::enums::Align)
    /// Contains mu::engraving::Placement enumeration values
    /// \note In MuseScore 2.X this enumeration was available as
    /// EngravingItem.ABOVE and EngravingItem.BELOW.
    DECLARE_API_ENUM(Placement,        placementEnum,          mu::engraving::apiv1::enums::Placement)
    /// Contains mu::engraving::PlacementH enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(PlacementH, placementHEnum, mu::engraving::apiv1::enums::PlacementH)
    /// Contains mu::engraving::TextPlace enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(TextPlace, textPlaceEnum, mu::engraving::apiv1::enums::TextPlace)
    /// Contains mu::engraving::Direction enumeration values
    /// \note In MuseScore 2.X this enumeration was available as
    /// MScore.UP, MScore.DOWN, MScore.AUTO.
    DECLARE_API_ENUM(Direction, directionEnum, mu::engraving::apiv1::enums::Direction)
    /// Contains mu::engraving::DirectionH enumeration values
    /// \note In MuseScore 2.X this enumeration was available as
    /// MScore.LEFT, MScore.RIGHT, MScore.AUTO.
    DECLARE_API_ENUM(DirectionH, directionHEnum, mu::engraving::apiv1::enums::DirectionH)
    /// Contains mu::engraving::Orientation enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(Orientation, orientationEnum, mu::engraving::apiv1::enums::Orientation)
    /// Contains mu::engraving::AutoOnOff enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(AutoOnOff, autoOnOffEnum, mu::engraving::apiv1::enums::AutoOnOff)
    /// Contains mu::engraving::VoiceAssignment enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(VoiceAssignment, voiceAssignmentEnum, mu::engraving::apiv1::enums::VoiceAssignment)
    /// Contains mu::engraving::SpacerType enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(SpacerType, spacerTypeEnum, mu::engraving::apiv1::enums::SpacerType)
    /// Contains mu::engraving::LayoutBreakType enumeration values
    DECLARE_API_ENUM(LayoutBreak, layoutBreakTypeEnum, mu::engraving::apiv1::enums::LayoutBreakType)
    /// Contains mu::engraving::DurationType enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(DurationType, durationTypeEnum, mu::engraving::apiv1::enums::DurationType)
    /// Contains mu::engraving::ValueType enumeration values
    /// \note In MuseScore 2.X this enumeration was available as
    /// Note.OFFSET_VAL, Note.USER_VAL
    DECLARE_API_ENUM(NoteValueType,    noteValueTypeEnum,      mu::engraving::apiv1::enums::VeloType)
    /// Contains mu::engraving::BeamMode enumeration values
    DECLARE_API_ENUM(Beam, beamModeEnum, mu::engraving::apiv1::enums::BeamMode)
    /// Contains mu::engraving::GlissandoType enumeration values
    DECLARE_API_ENUM(Glissando, glissandoTypeEnum, mu::engraving::apiv1::enums::GlissandoType) // was probably absent in 2.X
    /// Contains mu::engraving::GlissandoStyle enumeration values
    /// \note In MuseScore 2.X this enumeration was available as
    /// MScore.CHROMATIC, MScore.WHITE_KEYS, MScore.BLACK_KEYS,
    /// MScore.DIATONIC.
    DECLARE_API_ENUM(GlissandoStyle, glissandoStyleEnum, mu::engraving::apiv1::enums::GlissandoStyle)
    /// Contains mu::engraving::HarmonyType enumeration values
    /// \since MuseScore 3.5
    DECLARE_API_ENUM(HarmonyType, harmonyTypeEnum, mu::engraving::apiv1::enums::HarmonyType)
    /// Contains mu::engraving::HarmonyVoicing enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(HarmonyVoicing, harmonyVoicingEnum, mu::engraving::apiv1::enums::HarmonyVoicing)
    /// Contains mu::engraving::HDuration enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(HDuration, hDurationEnum, mu::engraving::apiv1::enums::HDuration)
    /// Contains mu::engraving::FrameType enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(FrameType, frameTypeEnum, mu::engraving::apiv1::enums::FrameType)
    /// Contains mu::engraving::VerticalAlignment enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(VerticalAlignment, verticalAlignmentEnum, mu::engraving::apiv1::enums::VerticalAlignment)
    /// Contains mu::engraving::TremoloBarType enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(TremoloBarType, tremoloBarTypeEnum, mu::engraving::apiv1::enums::TremoloBarType)
    /// Contains mu::engraving::PreferSharpFlat enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(PreferSharpFlat, preferSharpFlatEnum, mu::engraving::apiv1::enums::PreferSharpFlat)
    /// Contains mu::engraving::NoteHead::Type enumeration values
    /// \note In MuseScore 2.X this enumeration was available in
    /// NoteHead class (e.g. NoteHead.HEAD_QUARTER).
    DECLARE_API_ENUM(NoteHeadType, noteHeadTypeEnum, mu::engraving::apiv1::enums::NoteHeadType)
    /// Contains mu::engraving::NoteHeadScheme enumeration values
    /// \since MuseScore 3.5
    DECLARE_API_ENUM(NoteHeadScheme, noteHeadSchemeEnum, mu::engraving::apiv1::enums::NoteHeadScheme)
    /// Contains mu::engraving::NoteHeadGroup enumeration values
    /// \note In MuseScore 2.X this enumeration was available in
    /// NoteHead class (e.g. NoteHead.HEAD_TRIANGLE).
    DECLARE_API_ENUM(NoteHeadGroup, noteHeadGroupEnum, mu::engraving::apiv1::enums::NoteHeadGroup)
    /// Contains mu::engraving::NoteType enumeration values
    /// \since MuseScore 3.2.1
    DECLARE_API_ENUM(NoteType, noteTypeEnum, mu::engraving::apiv1::enums::NoteType)
    /// Contains mu::engraving::PlayEventType enumeration values
    /// \since MuseScore 3.3
    DECLARE_API_ENUM(PlayEventType, playEventTypeEnum, mu::engraving::apiv1::enums::PlayEventType)
    /// Contains mu::engraving::SegmentType enumeration values
    DECLARE_API_ENUM(Segment,          segmentTypeEnum,        mu::engraving::apiv1::enums::SegmentType)
    /// Contains mu::engraving::BarLineType enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(BarLineType, barLineTypeEnum, mu::engraving::apiv1::enums::BarLineType)
    /// Contains mu::engraving::Tid enumeration values
    /// \note In MuseScore 2.X this enumeration was available as
    /// TextStyleType (TextStyleType.TITLE etc.)
    DECLARE_API_ENUM(Tid, tidEnum, mu::engraving::apiv1::enums::Tid)
    /// Contains mu::engraving::LyricsSyllabic enumeration values
    DECLARE_API_ENUM(Lyrics, lyricsSyllabicEnum, mu::engraving::apiv1::enums::Syllabic)
    /// Contains mu::engraving::Spanner::Anchor enumeration values
    DECLARE_API_ENUM(Spanner,          spannerAnchorEnum,      mu::engraving::apiv1::enums::Anchor)           // probably unavailable in 2.X
    /// Contains mu::engraving::MMRestRangeBracketType enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(MMRestRangeBracketType, mMRestRangeBracketTypeEnum, mu::engraving::apiv1::enums::MMRestRangeBracketType)
    /// Contains mu::engraving::TupletNumberType enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(TupletNumberType, tupletNumberTypeEnum, mu::engraving::apiv1::enums::TupletNumberType)
    /// Contains mu::engraving::TupletBracketType enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(TupletBracketType, tupletBracketTypeEnum, mu::engraving::apiv1::enums::TupletBracketType)
    /// Contains mu::engraving::TripletFeelType enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(TripletFeelType, tripletFeelTypeEnum, mu::engraving::apiv1::enums::TripletFeelType)
    /// Contains mu::engraving::GuitarBendType enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(GuitarBendType, guitarBendTypeEnum, mu::engraving::apiv1::enums::GuitarBendType)
    /// Contains mu::engraving::GuitarBendShowHoldLine enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(GuitarBendShowHoldLine, guitarBendShowHoldLineEnum, mu::engraving::apiv1::enums::GuitarBendShowHoldLine)
    /// Contains mu::engraving::ClefType enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(ClefType, clefTypeEnum, mu::engraving::apiv1::enums::ClefType)
    /// Contains mu::engraving::ClefToBarlinePosition enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(ClefToBarlinePosition, clefToBarlinePositionEnum, mu::engraving::apiv1::enums::ClefToBarlinePosition)
    /// Contains mu::engraving::DynamicType enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(DynamicType, dynamicTypeEnum, mu::engraving::apiv1::enums::DynamicType)
    /// Contains mu::engraving::DynamicSpeed enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(DynamicSpeed, dynamicSpeedEnum, mu::engraving::apiv1::enums::DynamicSpeed)
    /// Contains mu::engraving::LineType enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(LineType, lineTypeEnum, mu::engraving::apiv1::enums::LineType)
    /// Contains mu::engraving::HookType enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(HookType, hookTypeEnum, mu::engraving::apiv1::enums::HookType)
    /// Contains mu::engraving::KeyMode enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(KeyMode, keyModeEnum, mu::engraving::apiv1::enums::KeyMode)
    /// Contains mu::engraving::ArpeggioType enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(ArpeggioType, arpeggioTypeEnum, mu::engraving::apiv1::enums::ArpeggioType)
    /// Contains mu::engraving::IntervalStep enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(IntervalStep, intervalStepEnum, mu::engraving::apiv1::enums::IntervalStep)
    /// Contains mu::engraving::IntervalType enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(IntervalType, intervalTypeEnum, mu::engraving::apiv1::enums::IntervalType)
    /// Contains mu::engraving::InstrumentLabelVisibility enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(InstrumentLabelVisibility, instrumentLabelVisibilityEnum, mu::engraving::apiv1::enums::InstrumentLabelVisibility)
    /// Contains mu::engraving::OrnamentShowAccidental enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(OrnamentShowAccidental, ornamentShowAccidentalEnum, mu::engraving::apiv1::enums::OrnamentShowAccidental)
    /// Contains mu::engraving::PartialSpannerDirection enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(PartialSpannerDirection, partialSpannerDirectionEnum, mu::engraving::apiv1::enums::PartialSpannerDirection)
    /// Contains mu::engraving::ChordStylePreset enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(ChordStylePreset, chordStylePresetEnum, mu::engraving::apiv1::enums::ChordStylePreset)
    /// Contains mu::engraving::AnnotationCategory enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(AnnotationCategory, annotationCategoryEnum, mu::engraving::apiv1::enums::AnnotationCategory)
    /// Contains mu::engraving::PlayingTechniqueType enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(PlayingTechniqueType, playingTechniqueTypeEnum, mu::engraving::apiv1::enums::PlayingTechniqueType)
    /// Contains mu::engraving::GradualTempoChangeType enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(GradualTempoChangeType, gradualTempoChangeTypeEnum, mu::engraving::apiv1::enums::GradualTempoChangeType)
    /// Contains mu::engraving::ChangeMethod enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(ChangeMethod, changeMethodEnum, mu::engraving::apiv1::enums::ChangeMethod)
    /// Contains mu::engraving::ChangeDirection enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(ChangeDirection, changeDirectionEnum, mu::engraving::apiv1::enums::ChangeDirection)
    /// Contains mu::engraving::AccidentalRole enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(AccidentalRole, accidentalRoleEnum, mu::engraving::apiv1::enums::AccidentalRole)
    /// Contains mu::engraving::AccidentalVal enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(AccidentalVal, accidentalValEnum, mu::engraving::apiv1::enums::AccidentalVal)
    /// Contains mu::engraving::FermataType enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(FermataType, fermataTypeEnum, mu::engraving::apiv1::enums::FermataType)
    /// Contains mu::engraving::ChordLineType enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(ChordLineType, chordLineTypeEnum, mu::engraving::apiv1::enums::ChordLineType)
    /// Contains mu::engraving::SlurStyleType enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(SlurStyleType, slurStyleTypeEnum, mu::engraving::apiv1::enums::SlurStyleType)
    /// Contains mu::engraving::TremoloType enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(TremoloType, tremoloTypeEnum, mu::engraving::apiv1::enums::TremoloType)
    /// Contains mu::engraving::TremoloChordType enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(TremoloChordType, tremoloChordTypeEnum, mu::engraving::apiv1::enums::TremoloChordType)
    /// Contains mu::engraving::BracketType enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(BracketType, bracketTypeEnum, mu::engraving::apiv1::enums::BracketType)
    /// Contains mu::engraving::JumpType enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(JumpType, jumpTypeEnum, mu::engraving::apiv1::enums::JumpType)
    /// Contains mu::engraving::MarkerType enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(MarkerType, markerTypeEnum, mu::engraving::apiv1::enums::MarkerType)
    /// Contains mu::engraving::MeasureNumberMode enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(MeasureNumberMode, measureNumberModeEnum, mu::engraving::apiv1::enums::MeasureNumberMode)
    /// Contains mu::engraving::StaffGroup enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(StaffGroup, staffGroupEnum, mu::engraving::apiv1::enums::StaffGroup)
    /// Contains mu::engraving::HideMode enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(HideMode, hideModeEnum, mu::engraving::apiv1::enums::HideMode)
    /// Contains mu::engraving::OttavaType enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(OttavaType, ottavaTypeEnum, mu::engraving::apiv1::enums::OttavaType)
    /// Contains mu::engraving::HairpinType enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(HairpinType, hairpinTypeEnum, mu::engraving::apiv1::enums::HairpinType)
    /// Contains mu::engraving::TrillType enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(TrillType, trillTypeEnum, mu::engraving::apiv1::enums::TrillType)
    /// Contains mu::engraving::VibratoType enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(VibratoType, vibratoTypeEnum, mu::engraving::apiv1::enums::VibratoType)
    /// Contains mu::engraving::ArticulationTextType enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(ArticulationTextType, articulationTextTypeEnum, mu::engraving::apiv1::enums::ArticulationTextType)
    /// Contains mu::engraving::LyricsDashSystemStart enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(LyricsDashSystemStart, lyricsDashSystemStartEnum, mu::engraving::apiv1::enums::LyricsDashSystemStart)
    /// Contains mu::engraving::NoteLineEndPlacement enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(NoteLineEndPlacement, noteLineEndPlacementEnum, mu::engraving::apiv1::enums::NoteLineEndPlacement)
    /// Contains mu::engraving::SpannerSegmentType enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(SpannerSegmentType, spannerSegmentTypeEnum, mu::engraving::apiv1::enums::SpannerSegmentType)
    /// Contains mu::engraving::TiePlacement enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(TiePlacement, tiePlacementEnum, mu::engraving::apiv1::enums::TiePlacement)
    /// Contains mu::engraving::TieDotsPlacement enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(TieDotsPlacement, tieDotsPlacementEnum, mu::engraving::apiv1::enums::TieDotsPlacement)
    /// Contains mu::engraving::TimeSigType enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(TimeSigType, timeSigTypeEnum, mu::engraving::apiv1::enums::TimeSigType)
    /// Contains mu::engraving::TimeSigPlacement enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(TimeSigPlacement, timeSigPlacementEnum, mu::engraving::apiv1::enums::TimeSigPlacement)
    /// Contains mu::engraving::TimeSigStyle enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(TimeSigStyle, timeSigStyleEnum, mu::engraving::apiv1::enums::TimeSigStyle)
    /// Contains mu::engraving::TimeSigVSMargin enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(TimeSigVSMargin, timeSigVSMarginEnum, mu::engraving::apiv1::enums::TimeSigVSMargin)
    /// Contains mu::engraving::NoteSpellingType enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(NoteSpellingType, noteSpellingTypeEnum, mu::engraving::apiv1::enums::NoteSpellingType)
    /// Contains mu::engraving::Key enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(Key, keyEnum, mu::engraving::apiv1::enums::Key)
    /// Contains mu::engraving::UpdateMode enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(UpdateMode, updateModeEnum, mu::engraving::apiv1::enums::UpdateMode)
    /// Contains mu::engraving::LayoutFlag enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(LayoutFlag, layoutFlagEnum, mu::engraving::apiv1::enums::LayoutFlag)
    /// Contains mu::engraving::LayoutMode enumeration values
    /// \since MuseScore 4.6
    DECLARE_API_ENUM(LayoutMode, layoutModeEnum, mu::engraving::apiv1::enums::LayoutMode)
    /// Contains mu::engraving::SymId enumeration values
    /// \since MuseScore 3.5
    DECLARE_API_ENUM(SymId, symIdEnum, mu::engraving::apiv1::enums::SymId)

    DECLARE_API_ENUM2(Cursor, cursorEnum, Cursor::RewindMode, Cursor::InputStateMode)

signals:
    /// Indicates that the plugin was launched.
    /// Implement \p onRun() function in your plugin to handle this signal.
    void run();

    void closeRequested();

    /// Notifies plugin about changes in score state.
    /// Called after each user (or plugin) action which may have changed a
    /// score. Implement \p onScoreStateChanged() function in your plugin to
    /// handle this signal.
    ///
    /// \p state variable is available within the handler with following
    /// fields:
    ///
    /// - \p selectionChanged
    /// - \p excerptsChanged
    /// - \p instrumentsChanged
    /// - \p startLayoutTick
    /// - \p endLayoutTick
    /// - \p undoRedo - whether this onScoreStateChanged invocation results
    ///   from user undo/redo action. It is usually not recommended to modify
    ///   score from plugins in this case. Available since MuseScore 3.5.
    ///
    /// If a plugin modifies score in this handler, then it should:
    /// 1. enclose all modifications within Score::startCmd() / Score::endCmd()
    /// 2. take care of preventing an infinite recursion, as plugin-originated
    ///    changes will trigger this signal again after calling Score::endCmd()
    ///
    /// Example:
    /// \code
    /// import QtQuick 2.0
    /// import MuseScore 3.0
    ///
    /// MuseScore {
    ///     pluginType: "dialog"
    ///     implicitHeight: 75 // necessary for dock widget to appear with nonzero height
    ///     implicitWidth: 150
    ///
    ///     Text {
    ///        // A label which will display pitch of the currently selected note
    ///        id: pitchLabel
    ///        anchors.fill: parent
    ///     }
    ///
    ///     onScoreStateChanged: {
    ///         if (state.selectionChanged) {
    ///             var el = curScore ? curScore.selection.elements[0] : null;
    ///             if (el && el.type == EngravingItem.NOTE)
    ///                 pitchLabel.text = el.pitch;
    ///             else
    ///                 pitchLabel.text = "no note selected";
    ///         }
    ///     }
    /// }
    /// \endcode
    /// \warning This functionality is considered experimental.
    /// This API may change in future versions of MuseScore.
    /// \since MuseScore 3.3
    void scoreStateChanged(const QMap<QString, QVariant>& state);

public:
    /// \cond MS_INTERNAL
    PluginAPI(QQuickItem* parent = 0);

    static void registerQmlTypes();

    void setup(QQmlEngine* e) override;
    void runPlugin() override { emit run(); }
    muse::async::Notification closeRequest() const override { return m_closeRequested; }

    void endCmd(const QMap<QString, QVariant>& stateInfo) { emit scoreStateChanged(stateInfo); }

    apiv1::Score* curScore() const;
    QQmlListProperty<apiv1::Score> scores();
    // /// \endcond

    Q_INVOKABLE apiv1::Score* newScore(const QString& name, const QString& part, int measures);
    Q_INVOKABLE apiv1::EngravingItem* newElement(int);
    Q_INVOKABLE void removeElement(apiv1::EngravingItem* wrapped);
    Q_INVOKABLE void cmd(const QString&);
    /// \cond PLUGIN_API \private \endcond
    Q_INVOKABLE apiv1::MsProcess* newQProcess();
    Q_INVOKABLE bool writeScore(apiv1::Score*, const QString& name, const QString& ext);
    Q_INVOKABLE apiv1::Score* readScore(const QString& name, bool noninteractive = false);
    Q_INVOKABLE void closeScore(apiv1::Score*);

    Q_INVOKABLE void log(const QString&);
    Q_INVOKABLE void logn(const QString&);
    Q_INVOKABLE void log2(const QString&, const QString&);
    Q_INVOKABLE void openLog(const QString&);
    Q_INVOKABLE void closeLog();

    Q_INVOKABLE apiv1::FractionWrapper* fraction(int numerator, int denominator) const;
    Q_INVOKABLE apiv1::FractionWrapper* fractionFromTicks(int ticks) const;

    Q_INVOKABLE apiv1::OrnamentIntervalWrapper* ornamentInterval(int step, int type) const;

    Q_INVOKABLE apiv1::IntervalWrapper* interval(int chromatic, int diatonic) const;
    Q_INVOKABLE apiv1::IntervalWrapper* intervalFromOrnamentInterval(apiv1::OrnamentIntervalWrapper* o) const;

    Q_INVOKABLE void quit();

    QString pluginType() const;
    void setPluginType(const QString& newPluginType);

    QString menuPath() const;
    void setMenuPath(const QString& newMenuPath);

    QString title() const;
    void setTitle(const QString& newTitle);

    QString version() const;
    void setVersion(const QString& newVersion);

    QString description() const;
    void setDescription(const QString& newDescription);

    QString dockArea() const;
    void setDockArea(const QString& newDockArea);

    bool requiresScore() const;
    void setRequiresScore(bool newRequiresScore);

    QString thumbnailName() const;
    void setThumbnailName(const QString& newThumbnailName);

    QString categoryCode() const;
    void setCategoryCode(const QString& newCategoryCode);

    int division() const;
    int mscoreVersion() const;
    int mscoreMajorVersion() const;
    int mscoreMinorVersion() const;
    int mscoreUpdateVersion() const;
    qreal mscoreDPI() const;
    OrnamentIntervalWrapper* defaultOrnamentInterval() const;

private:
    mu::engraving::Score* currentScore() const;

    QString m_pluginType;
    QString m_title;
    QString m_version;
    QString m_description;
    bool m_requiresScore = true;
    QString m_thumbnailName;
    QString m_categoryCode;
    muse::async::Notification m_closeRequested;
};

#undef DECLARE_API_ENUM2
#undef DECLARE_API_ENUM
}
