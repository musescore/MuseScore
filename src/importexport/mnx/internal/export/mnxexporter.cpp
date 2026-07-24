/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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
#include "mnxexporter.h"
#include "internal/shared/mnxtypesconv.h"

#include <stdexcept>

#include "engraving/dom/masterscore.h"
#include "engraving/dom/score.h"
#include "global/iapplication.h"
#include "io/path.h"
#include "modularity/ioc.h"
#include "log.h"
#include "translation.h"

using namespace mu::engraving;

namespace mu::iex::mnxio {
//---------------------------------------------------------
//   getOrAssignEID
//---------------------------------------------------------

EID MnxExporter::getOrAssignEID(EngravingObject* item)
{
    EID eid = item->eid();
    if (!eid.isValid()) {
        eid = item->assignNewEID();
    }
    return eid;
}

//---------------------------------------------------------
//   mnxEventFromCR
//---------------------------------------------------------

std::optional<mnx::sequence::Event> MnxExporter::mnxEventFromCR(const engraving::ChordRest* cr)
{
    auto pointer = muse::value(m_crToMnxEvent, cr);
    if (!pointer.empty()) {
        return mnx::sequence::Event(mnxDocument().root(), pointer);
    }
    return std::nullopt;
}

//---------------------------------------------------------
//   mnxMeasureIndexFromMeasure
//---------------------------------------------------------

size_t MnxExporter::mnxMeasureIndexFromMeasure(const engraving::Measure* measure) const
{
    IF_ASSERT_FAILED(measure) {
        throw std::logic_error("Measure is null while resolving MNX measure index.");
    }
    const size_t result = muse::value(m_measToMnxMeas, measure, muse::nidx);
    IF_ASSERT_FAILED(result != muse::nidx) {
        throw std::logic_error("Measure is not mapped to an MNX measure index.");
    }
    return result;
}

//---------------------------------------------------------
//   mnxPartStaffFromStaffIdx
//---------------------------------------------------------

std::pair<size_t, int> MnxExporter::mnxPartStaffFromStaffIdx(engraving::staff_idx_t staffIdx) const
{
    const auto it = m_staffToPartStaff.find(staffIdx);
    IF_ASSERT_FAILED(it != m_staffToPartStaff.end()) {
        throw std::logic_error("Staff index is not mapped to an MNX part/staff.");
    }
    return it->second;
}

//---------------------------------------------------------
//   mnxFermataFromFermata
//---------------------------------------------------------

mnx::Fermata MnxExporter::mnxFermataFromFermata(const Fermata* fermata)
{
    mnx::Fermata result;
    result.set_or_clear_duration(toMnxFermataDuration(fermata->fermataType()));
    result.set_or_clear_symbol(toMnxFermataSymbol(fermata->symId()));
    result.set_or_clear_orient(toMnxOrientation(fermata->placement()));
    return result;
}

//---------------------------------------------------------
//   mnxChordTargetPosition
//   calculate exact MNX position for an object targeting
//   a chord, including grace-note targets
//---------------------------------------------------------

std::optional<MnxChordTargetPosition> MnxExporter::mnxChordTargetPosition(const Chord* chord, const Measure* measure)
{
    IF_ASSERT_FAILED(chord && measure) {
        return std::nullopt;
    }

    auto positionFromTick = [measure](const Fraction& tick) {
        return MnxChordTargetPosition {
            toMnxFractionValue(tick - measure->tick()).reduced(),
            std::nullopt
        };
    };

    if (!chord->isGrace()) {
        MnxChordTargetPosition position = positionFromTick(chord->tick());
        if (!chord->graceNotes().empty()) {
            position.graceIndex = 0;
        }
        return position;
    }

    EngravingObject* parent = chord->explicitParent();
    if (!parent || !parent->isChord()) {
        LOGW() << "Skipping grace-note target with missing main chord.";
        return std::nullopt;
    }

    const Chord* mainChord = toChord(parent);
    const GraceNotesGroup& graceNotes = chord->isGraceAfter()
                                        ? mainChord->graceNotesAfter()
                                        : mainChord->graceNotesBefore();
    const auto graceIt = std::find(graceNotes.begin(), graceNotes.end(), chord);
    if (graceIt == graceNotes.end()) {
        LOGW() << "Skipping grace-note target whose grace index could not be resolved.";
        return std::nullopt;
    }

    const size_t index = static_cast<size_t>(std::distance(graceNotes.begin(), graceIt));
    MnxChordTargetPosition position = positionFromTick(chord->isGraceAfter() ? mainChord->tick() + mainChord->ticks()
                                                       : mainChord->tick());
    position.graceIndex = static_cast<unsigned>(graceNotes.size() - index);
    return position;
}

//---------------------------------------------------------
//   exportMnx
//---------------------------------------------------------

muse::Ret MnxExporter::exportMnx()
{
    LOGI() << "MNX export started: schema version=" << m_mnxDocument.mnx().version();
    // Header
    static muse::GlobalInject<muse::IApplication> application;
    auto client = m_mnxDocument.mnx().ensure_mnxdom().ensure_client();
    muse::String version = application()->version().toString();
    const muse::String revision = application()->revision();
    const muse::String build = application()->build();
    if (!revision.isEmpty()) {
        version += u" rev. " + revision;
    }
    client.set_name(application()->title().toStdString());
    client.set_version(version.toStdString());
    if (!build.isEmpty()) {
        client.set_build(build.toStdString());
    }
    if (m_score && m_score->masterScore()) {
        IFileInfoProviderPtr fileInfo = m_score->masterScore()->fileInfo();
        if (fileInfo) {
            const muse::io::path_t fileName = fileInfo->fileName();
            const std::string format = muse::io::suffix(fileName);
            if (!format.empty() || !fileName.empty()) {
                auto source = m_mnxDocument.mnx().ensure_mnxdom().ensure_source();
                if (!format.empty()) {
                    source.set_format(format);
                }
                if (!fileName.empty()) {
                    source.set_filename(fileName.toStdString());
                }
            }
        }
    }

    if (m_exportBeams) {
        mnx::MnxMetaData::Support support = m_mnxDocument.mnx().ensure_support();
        support.set_useBeams(true);
    }

    createGlobal();
    if (!createParts()) {
        const String msg = muse::TranslatableString(
            "importexport/mnx",
            "MNX export skipped because the score contains no exportable parts. (Tablature is not supported yet).").str;
        return muse::make_ret(muse::Ret::Code::NotSupported, msg);
    }
    createLayout(m_exportedStaves, "full-score");
    /// @todo Creation of all layouts and scores, including excerpts. (Deferred to a future dev cycle.)
    return muse::make_ok();
}
} // namespace mu::iex::mnxio
