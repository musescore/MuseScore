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

#include "gracenotesmetaparser.h"

#include "libmscore/chord.h"

using namespace mu::engraving;

void GraceNotesMetaParser::doParse(const Ms::EngravingItem* item, const RenderingContext& ctx, mpe::ArticulationMap& result)
{
    IF_ASSERT_FAILED(item->type() == Ms::ElementType::CHORD && ctx.isValid()) {
        return;
    }

    mpe::ArticulationType type = mpe::ArticulationType::Undefined;

    const Ms::Chord* graceChord = Ms::toChord(item);

    if (!graceChord->isChordPlayable()) {
        return;
    }

    switch (graceChord->noteType()) {
    case Ms::NoteType::ACCIACCATURA:
        type = mpe::ArticulationType::Acciaccatura;
        break;

    case Ms::NoteType::APPOGGIATURA:
    case Ms::NoteType::GRACE4:
    case Ms::NoteType::GRACE16:
    case Ms::NoteType::GRACE32:
        type = mpe::ArticulationType::PreAppoggiatura;
        break;

    case Ms::NoteType::GRACE8_AFTER:
    case Ms::NoteType::GRACE16_AFTER:
    case Ms::NoteType::GRACE32_AFTER:
        type = mpe::ArticulationType::PostAppoggiatura;
        break;
    default:
        break;
    }

    if (type == mpe::ArticulationType::Undefined) {
        return;
    }

    appendArticulationData(mpe::ArticulationMeta(type, ctx.profile->pattern(type), ctx.nominalTimestamp, ctx.nominalDuration), result);
}
