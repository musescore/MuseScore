/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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

#include <stdexcept>

#include "engraving/dom/score.h"
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
//   exportMnx
//---------------------------------------------------------

muse::Ret MnxExporter::exportMnx()
{
    LOGI() << "MNX export started: schema version=" << m_mnxDocument.mnx().version();
    // Header
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
