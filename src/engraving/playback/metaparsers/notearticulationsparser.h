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

#ifndef MU_ENGRAVING_NOTEARTICULATIONSPARSER_H
#define MU_ENGRAVING_NOTEARTICULATIONSPARSER_H

#include "types/types.h"
#include "metaparserbase.h"

namespace Ms {
class Note;
}

namespace mu::engraving {
class NoteArticulationsParser : public MetaParserBase<NoteArticulationsParser>
{
public:
    static void buildNoteArticulationMap(const Ms::Note* note, const RenderingContext& ctx, mpe::ArticulationMap& result);

protected:
    friend MetaParserBase;

    static void doParse(const Ms::EngravingItem* item, const RenderingContext& ctx, mpe::ArticulationMap& result);

private:
    static mpe::ArticulationType articulationTypeByNotehead(const NoteHeadGroup noteheadGroup);

    static void parsePersistentMeta(const RenderingContext& ctx, mpe::ArticulationMap& result);
    static void parseGhostNote(const Ms::Note* note, const RenderingContext& ctx, mpe::ArticulationMap& result);
    static void parseNoteHead(const Ms::Note* note, const RenderingContext& ctx, mpe::ArticulationMap& result);
    static void parseSpanners(const Ms::Note* note, const RenderingContext& ctx, mpe::ArticulationMap& result);
};
}

#endif // MU_ENGRAVING_NOTEARTICULATIONSPARSER_H
