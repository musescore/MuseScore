/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include "global/api/apiobject.h"

#include "extensions/api/v1/iapiv1object.h"

#include "qmlpluginapi.h"

namespace mu::engraving::apiv1 {
//! NOTE This API is used in `js` scripts of macros
//! It repeats the API of the qml plugin.
//! It is also available as `api.engraving.`
class EngravingApiV1 : public muse::api::ApiObject, public muse::extensions::apiv1::IApiV1Object
{
    Q_OBJECT

    Q_PROPERTY(int division READ division CONSTANT)
    Q_PROPERTY(int mscoreVersion READ mscoreVersion CONSTANT)
    Q_PROPERTY(int mscoreMajorVersion READ mscoreMajorVersion CONSTANT)
    Q_PROPERTY(int mscoreMinorVersion READ mscoreMinorVersion CONSTANT)
    Q_PROPERTY(int mscoreUpdateVersion READ mscoreUpdateVersion CONSTANT)
    Q_PROPERTY(qreal mscoreDPI READ mscoreDPI CONSTANT)

    Q_PROPERTY(apiv1::Score * curScore READ curScore CONSTANT)

    Q_PROPERTY(apiv1::Enum * Element READ elementTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * Accidental READ accidentalTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * OrnamentStyle READ ornamentStyleEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * Align READ alignEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * Placement READ placementEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * PlacementH READ placementHEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * TextPlace READ textPlaceEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * Direction READ directionEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * DirectionH READ directionHEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * Orientation READ orientationEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * AutoOnOff READ autoOnOffEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * VoiceAssignment READ voiceAssignmentEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * SpacerType READ spacerTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * LayoutBreak READ layoutBreakTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * DurationType READ durationTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * NoteValueType READ noteValueTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * Beam READ beamModeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * Glissando READ glissandoTypeEnum CONSTANT) // was probably absent in 2.X
    Q_PROPERTY(apiv1::Enum * GlissandoStyle READ glissandoStyleEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * HarmonyType READ harmonyTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * NoteHeadType READ noteHeadTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * NoteHeadScheme READ noteHeadSchemeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * NoteHeadGroup READ noteHeadGroupEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * NoteType READ noteTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * PlayEventType READ playEventTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * Segment READ segmentTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * BarLineType READ barLineTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * Tid READ tidEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * Lyrics READ lyricsSyllabicEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * Spanner READ spannerAnchorEnum CONSTANT)           // probably unavailable in 2.X
    Q_PROPERTY(apiv1::Enum * MMRestRangeBracketType READ mMRestRangeBracketTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * TupletNumberType READ tupletNumberTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * TupletBracketType READ tupletBracketTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * TripletFeelType READ tripletFeelTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * GuitarBendType READ guitarBendTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * GuitarBendShowHoldLine READ guitarBendShowHoldLineEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * ClefType READ clefTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * ClefToBarlinePosition READ clefToBarlinePositionEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * DynamicType READ dynamicTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * DynamicSpeed READ dynamicSpeedEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * LineType READ lineTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * HookType READ hookTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * KeyMode READ keyModeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * ArpeggioType READ arpeggioTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * IntervalStep READ intervalStepEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * IntervalType READ intervalTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * InstrumentLabelVisibility READ instrumentLabelVisibilityEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * OrnamentShowAccidental READ ornamentShowAccidentalEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * PartialSpannerDirection READ partialSpannerDirectionEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * ChordStylePreset READ chordStylePresetEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * AnnotationCategory READ annotationCategoryEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * PlayingTechniqueType READ playingTechniqueTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * GradualTempoChangeType READ gradualTempoChangeTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * ChangeMethod READ changeMethodEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * ChangeDirection READ changeDirectionEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * AccidentalRole READ accidentalRoleEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * AccidentalVal READ accidentalValEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * FermataType READ fermataTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * ChordLineType READ chordLineTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * SlurStyleType READ slurStyleTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * TremoloType READ tremoloTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * TremoloChordType READ tremoloChordTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * BracketType READ bracketTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * JumpType READ jumpTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * MarkerType READ markerTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * StaffGroup READ staffGroupEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * HideMode READ hideModeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * TrillType READ trillTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * VibratoType READ vibratoTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * ArticulationTextType READ articulationTextTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * LyricsDashSystemStart READ lyricsDashSystemStartEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * NoteLineEndPlacement READ noteLineEndPlacementEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * SpannerSegmentType READ spannerSegmentTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * TiePlacement READ tiePlacementEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * TieDotsPlacement READ tieDotsPlacementEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * TimeSigPlacement READ timeSigPlacementEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * TimeSigStyle READ timeSigStyleEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * TimeSigVSMargin READ timeSigVSMarginEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * NoteSpellingType READ noteSpellingTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * Key READ keyEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * UpdateMode READ updateModeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * LayoutFlag READ layoutFlagEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * ElementFlag READ elementFlagEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * SymId READ symIdEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * Cursor READ cursorEnum CONSTANT)

public:
    EngravingApiV1(muse::api::IApiEngine* e);
    ~EngravingApiV1();

    void setup(QJSValue globalObject) override;

    void setApi(PluginAPI* api);
    PluginAPI* api() const;

    //! Api V1 (qml plugin api)
    int division() const { return api()->division(); }
    int mscoreVersion() const { return api()->mscoreVersion(); }
    int mscoreMajorVersion() const { return api()->mscoreMajorVersion(); }
    int mscoreMinorVersion() const { return api()->mscoreMinorVersion(); }
    int mscoreUpdateVersion() const { return api()->mscoreUpdateVersion(); }
    qreal mscoreDPI() const { return api()->mscoreDPI(); }

    apiv1::Score* curScore() const { return api()->curScore(); }

    apiv1::Enum* elementTypeEnum() const { return api()->get_elementTypeEnum(); }
    apiv1::Enum* accidentalTypeEnum() const { return api()->get_accidentalTypeEnum(); }
    apiv1::Enum* ornamentStyleEnum() const { return api()->get_ornamentStyleEnum(); }
    apiv1::Enum* alignEnum() const { return api()->get_alignEnum(); }
    apiv1::Enum* placementEnum() const { return api()->get_placementEnum(); }
    apiv1::Enum* placementHEnum() const { return api()->get_placementHEnum(); }
    apiv1::Enum* textPlaceEnum() const { return api()->get_textPlaceEnum(); }
    apiv1::Enum* directionEnum() const { return api()->get_directionEnum(); }
    apiv1::Enum* directionHEnum() const { return api()->get_directionHEnum(); }
    apiv1::Enum* orientationEnum() const { return api()->get_orientationEnum(); }
    apiv1::Enum* autoOnOffEnum() const { return api()->get_autoOnOffEnum(); }
    apiv1::Enum* voiceAssignmentEnum() const { return api()->get_voiceAssignmentEnum(); }
    apiv1::Enum* spacerTypeEnum() const { return api()->get_spacerTypeEnum(); }
    apiv1::Enum* layoutBreakTypeEnum() const { return api()->get_layoutBreakTypeEnum(); }
    apiv1::Enum* durationTypeEnum() const { return api()->get_durationTypeEnum(); }
    apiv1::Enum* noteValueTypeEnum() const { return api()->get_noteValueTypeEnum(); }
    apiv1::Enum* beamModeEnum() const { return api()->get_beamModeEnum(); }
    apiv1::Enum* glissandoTypeEnum() const { return api()->get_glissandoTypeEnum(); }
    apiv1::Enum* glissandoStyleEnum() const { return api()->get_glissandoStyleEnum(); }
    apiv1::Enum* harmonyTypeEnum() const { return api()->get_harmonyTypeEnum(); }
    apiv1::Enum* noteHeadTypeEnum() const { return api()->get_noteHeadTypeEnum(); }
    apiv1::Enum* noteHeadSchemeEnum() const { return api()->get_noteHeadSchemeEnum(); }
    apiv1::Enum* noteHeadGroupEnum() const { return api()->get_noteHeadGroupEnum(); }
    apiv1::Enum* noteTypeEnum() const { return api()->get_noteTypeEnum(); }
    apiv1::Enum* playEventTypeEnum() const { return api()->get_playEventTypeEnum(); }
    apiv1::Enum* segmentTypeEnum() const { return api()->get_segmentTypeEnum(); }
    apiv1::Enum* barLineTypeEnum() const { return api()->get_barLineTypeEnum(); }
    apiv1::Enum* tidEnum() const { return api()->get_tidEnum(); }
    apiv1::Enum* lyricsSyllabicEnum() const { return api()->get_lyricsSyllabicEnum(); }
    apiv1::Enum* spannerAnchorEnum() const { return api()->get_spannerAnchorEnum(); }
    apiv1::Enum* mMRestRangeBracketTypeEnum() const { return api()->get_mMRestRangeBracketTypeEnum(); }
    apiv1::Enum* tupletNumberTypeEnum() const { return api()->get_tupletNumberTypeEnum(); }
    apiv1::Enum* tupletBracketTypeEnum() const { return api()->get_tupletBracketTypeEnum(); }
    apiv1::Enum* tripletFeelTypeEnum() const { return api()->get_tripletFeelTypeEnum(); }
    apiv1::Enum* guitarBendTypeEnum() const { return api()->get_guitarBendTypeEnum(); }
    apiv1::Enum* guitarBendShowHoldLineEnum() const { return api()->get_guitarBendShowHoldLineEnum(); }
    apiv1::Enum* clefTypeEnum() const { return api()->get_clefTypeEnum(); }
    apiv1::Enum* clefToBarlinePositionEnum() const { return api()->get_clefToBarlinePositionEnum(); }
    apiv1::Enum* dynamicTypeEnum() const { return api()->get_dynamicTypeEnum(); }
    apiv1::Enum* dynamicSpeedEnum() const { return api()->get_dynamicSpeedEnum(); }
    apiv1::Enum* lineTypeEnum() const { return api()->get_lineTypeEnum(); }
    apiv1::Enum* hookTypeEnum() const { return api()->get_hookTypeEnum(); }
    apiv1::Enum* keyModeEnum() const { return api()->get_keyModeEnum(); }
    apiv1::Enum* arpeggioTypeEnum() const { return api()->get_arpeggioTypeEnum(); }
    apiv1::Enum* intervalStepEnum() const { return api()->get_intervalStepEnum(); }
    apiv1::Enum* intervalTypeEnum() const { return api()->get_intervalTypeEnum(); }
    apiv1::Enum* instrumentLabelVisibilityEnum() const { return api()->get_instrumentLabelVisibilityEnum(); }
    apiv1::Enum* ornamentShowAccidentalEnum() const { return api()->get_ornamentShowAccidentalEnum(); }
    apiv1::Enum* partialSpannerDirectionEnum() const { return api()->get_partialSpannerDirectionEnum(); }
    apiv1::Enum* chordStylePresetEnum() const { return api()->get_chordStylePresetEnum(); }
    apiv1::Enum* annotationCategoryEnum() const { return api()->get_annotationCategoryEnum(); }
    apiv1::Enum* playingTechniqueTypeEnum() const { return api()->get_playingTechniqueTypeEnum(); }
    apiv1::Enum* gradualTempoChangeTypeEnum() const { return api()->get_gradualTempoChangeTypeEnum(); }
    apiv1::Enum* changeMethodEnum() const { return api()->get_changeMethodEnum(); }
    apiv1::Enum* changeDirectionEnum() const { return api()->get_changeDirectionEnum(); }
    apiv1::Enum* accidentalRoleEnum() const { return api()->get_accidentalRoleEnum(); }
    apiv1::Enum* accidentalValEnum() const { return api()->get_accidentalValEnum(); }
    apiv1::Enum* fermataTypeEnum() const { return api()->get_fermataTypeEnum(); }
    apiv1::Enum* chordLineTypeEnum() const { return api()->get_chordLineTypeEnum(); }
    apiv1::Enum* slurStyleTypeEnum() const { return api()->get_slurStyleTypeEnum(); }
    apiv1::Enum* tremoloTypeEnum() const { return api()->get_tremoloTypeEnum(); }
    apiv1::Enum* tremoloChordTypeEnum() const { return api()->get_tremoloChordTypeEnum(); }
    apiv1::Enum* bracketTypeEnum() const { return api()->get_bracketTypeEnum(); }
    apiv1::Enum* jumpTypeEnum() const { return api()->get_jumpTypeEnum(); }
    apiv1::Enum* markerTypeEnum() const { return api()->get_markerTypeEnum(); }
    apiv1::Enum* staffGroupEnum() const { return api()->get_staffGroupEnum(); }
    apiv1::Enum* hideModeEnum() const { return api()->get_hideModeEnum(); }
    apiv1::Enum* trillTypeEnum() const { return api()->get_trillTypeEnum(); }
    apiv1::Enum* vibratoTypeEnum() const { return api()->get_vibratoTypeEnum(); }
    apiv1::Enum* articulationTextTypeEnum() const { return api()->get_articulationTextTypeEnum(); }
    apiv1::Enum* lyricsDashSystemStartEnum() const { return api()->get_lyricsDashSystemStartEnum(); }
    apiv1::Enum* noteLineEndPlacementEnum() const { return api()->get_noteLineEndPlacementEnum(); }
    apiv1::Enum* spannerSegmentTypeEnum() const { return api()->get_spannerSegmentTypeEnum(); }
    apiv1::Enum* tiePlacementEnum() const { return api()->get_tiePlacementEnum(); }
    apiv1::Enum* tieDotsPlacementEnum() const { return api()->get_tieDotsPlacementEnum(); }
    apiv1::Enum* timeSigPlacementEnum() const { return api()->get_timeSigPlacementEnum(); }
    apiv1::Enum* timeSigStyleEnum() const { return api()->get_timeSigStyleEnum(); }
    apiv1::Enum* timeSigVSMarginEnum() const { return api()->get_timeSigVSMarginEnum(); }
    apiv1::Enum* noteSpellingTypeEnum() const { return api()->get_noteSpellingTypeEnum(); }
    apiv1::Enum* keyEnum() const { return api()->get_keyEnum(); }
    apiv1::Enum* updateModeEnum() const { return api()->get_updateModeEnum(); }
    apiv1::Enum* layoutFlagEnum() const { return api()->get_layoutFlagEnum(); }
    apiv1::Enum* elementFlagEnum() const { return api()->get_elementFlagEnum(); }
    apiv1::Enum* symIdEnum() const { return api()->get_symIdEnum(); }
    apiv1::Enum* cursorEnum() const { return api()->get_cursorEnum(); }

    Q_INVOKABLE apiv1::Score* newScore(const QString& name, const QString& part, int measures)
    {
        return api()->newScore(name, part, measures);
    }

    Q_INVOKABLE apiv1::EngravingItem* newElement(int type) { return api()->newElement(type); }
    Q_INVOKABLE void removeElement(apiv1::EngravingItem* wrapped) { api()->removeElement(wrapped); }
    Q_INVOKABLE void cmd(const QString& code) { api()->cmd(code); }
    Q_INVOKABLE bool writeScore(apiv1::Score* s, const QString& name, const QString& ext)
    {
        return api()->writeScore(s, name, ext);
    }

    Q_INVOKABLE apiv1::Score* readScore(const QString& name, bool noninteractive = false)
    {
        return api()->readScore(name, noninteractive);
    }

    Q_INVOKABLE void closeScore(apiv1::Score* s) { api()->closeScore(s); }

    Q_INVOKABLE void log(const QString& m) { api()->log(m); }
    Q_INVOKABLE void logn(const QString& m) { api()->logn(m); }
    Q_INVOKABLE void log2(const QString& t, const QString& m) { api()->log2(t, m); }
    Q_INVOKABLE void openLog(const QString& f) { api()->openLog(f); }
    Q_INVOKABLE void closeLog() { api()->closeLog(); }

    Q_INVOKABLE apiv1::FractionWrapper* fraction(int numerator, int denominator) const
    {
        return api()->fraction(numerator, denominator);
    }

    Q_INVOKABLE apiv1::FractionWrapper* fractionFromTicks(int ticks) const
    {
        return api()->fractionFromTicks(ticks);
    }

    Q_INVOKABLE apiv1::OrnamentIntervalWrapper* defaultOrnamentInterval() const
    {
        return api()->defaultOrnamentInterval();
    }

    Q_INVOKABLE apiv1::OrnamentIntervalWrapper* ornamentInterval(int step, int type) const
    {
        return api()->ornamentInterval(step, type);
    }

    Q_INVOKABLE apiv1::IntervalWrapper* interval(int diatonic, int chromatic) const
    {
        return api()->interval(diatonic, chromatic);
    }

    Q_INVOKABLE apiv1::IntervalWrapper* intervalFromOrnamentInterval(apiv1::OrnamentIntervalWrapper* o) const
    {
        return api()->intervalFromOrnamentInterval(o);
    }

    Q_INVOKABLE void quit() { api()->quit(); }

private:
    mutable PluginAPI* m_api = nullptr;
    mutable bool m_selfApi = false;
};
}
