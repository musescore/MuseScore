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
#ifndef MU_ENGRAVING_APIV1_ENGRAVINGAPIV1_H
#define MU_ENGRAVING_APIV1_ENGRAVINGAPIV1_H

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

    Q_PROPERTY(apiv1::Enum * Element READ elementEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * Accidental READ accidentalTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * Beam READ beamModeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * Placement READ placementEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * Glissando READ glissandoTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * LayoutBreak READ layoutBreakTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * Lyrics READ lyricsSyllabicEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * Direction READ directionEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * DirectionH READ directionHEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * OrnamentStyle READ ornamentStyleEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * GlissandoStyle READ glissandoStyleEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * Tid READ tidEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * Align READ alignEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * NoteType READ noteTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * PlayEventType READ playEventTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * NoteHeadType READ noteHeadTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * NoteHeadScheme READ noteHeadSchemeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * NoteHeadGroup READ noteHeadGroupEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * NoteValueType READ noteValueTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * Segment READ segmentTypeEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * Spanner READ spannerAnchorEnum CONSTANT)                   // probably unavailable in 2.X
    Q_PROPERTY(apiv1::Enum * SymId READ symIdEnum CONSTANT)
    Q_PROPERTY(apiv1::Enum * HarmonyType READ harmonyTypeEnum CONSTANT)
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

    apiv1::Enum* elementEnum() const { return api()->get_elementTypeEnum(); }
    apiv1::Enum* accidentalTypeEnum() const { return api()->get_accidentalTypeEnum(); }
    apiv1::Enum* beamModeEnum() const { return api()->get_beamModeEnum(); }
    apiv1::Enum* placementEnum() const { return api()->get_placementEnum(); }
    apiv1::Enum* glissandoTypeEnum() const { return api()->get_glissandoTypeEnum(); }
    apiv1::Enum* layoutBreakTypeEnum() const { return api()->get_layoutBreakTypeEnum(); }
    apiv1::Enum* lyricsSyllabicEnum() const { return api()->get_lyricsSyllabicEnum(); }
    apiv1::Enum* directionEnum() const { return api()->get_directionEnum(); }
    apiv1::Enum* directionHEnum() const { return api()->get_directionHEnum(); }
    apiv1::Enum* ornamentStyleEnum() const { return api()->get_ornamentStyleEnum(); }
    apiv1::Enum* glissandoStyleEnum() const { return api()->get_glissandoStyleEnum(); }
    apiv1::Enum* tidEnum() const { return api()->get_tidEnum(); }
    apiv1::Enum* alignEnum() const { return api()->get_alignEnum(); }
    apiv1::Enum* noteTypeEnum() const { return api()->get_noteTypeEnum(); }
    apiv1::Enum* playEventTypeEnum() const { return api()->get_playEventTypeEnum(); }
    apiv1::Enum* noteHeadTypeEnum() const { return api()->get_noteHeadTypeEnum(); }
    apiv1::Enum* noteHeadSchemeEnum() const { return api()->get_noteHeadSchemeEnum(); }
    apiv1::Enum* noteHeadGroupEnum() const { return api()->get_noteHeadGroupEnum(); }
    apiv1::Enum* noteValueTypeEnum() const { return api()->get_noteValueTypeEnum(); }
    apiv1::Enum* segmentTypeEnum() const { return api()->get_segmentTypeEnum(); }
    apiv1::Enum* spannerAnchorEnum() const { return api()->get_spannerAnchorEnum(); }
    apiv1::Enum* symIdEnum() const { return api()->get_symIdEnum(); }
    apiv1::Enum* harmonyTypeEnum() const { return api()->get_harmonyTypeEnum(); }
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

    Q_INVOKABLE void quit() { api()->quit(); }

private:
    mutable PluginAPI* m_api = nullptr;
    mutable bool m_selfApi = false;
};
}

#endif // MU_ENGRAVING_APIV1_ENGRAVINGAPIV1_H
