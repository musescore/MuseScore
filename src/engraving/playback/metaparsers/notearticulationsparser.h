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

#ifndef MU_ENGRAVING_NOTEARTICULATIONSPARSER_H
#define MU_ENGRAVING_NOTEARTICULATIONSPARSER_H

#include "../types/types.h"
#include "metaparserbase.h"

namespace mu::engraving {
class Note;

class NoteArticulationsParser : public MetaParserBase<NoteArticulationsParser>
{
public:
    static void buildNoteArticulationMap(const Note* note, const RenderingContext& ctx, muse::mpe::ArticulationMap& result);

    static void parsePersistentMeta(const RenderingContext& ctx, muse::mpe::ArticulationMap& result);
    static void parseGhostNote(const Note* note, const RenderingContext& ctx, muse::mpe::ArticulationMap& result);
    static void parseNoteHead(const Note* note, const RenderingContext& ctx, muse::mpe::ArticulationMap& result);
    static void parseSpanners(const Note* note, const RenderingContext& ctx, muse::mpe::ArticulationMap& result);

protected:
    friend MetaParserBase;

    static void doParse(const EngravingItem* item, const RenderingContext& ctx, muse::mpe::ArticulationMap& result);

private:
    static muse::mpe::ArticulationType articulationTypeByNoteheadGroup(const NoteHeadGroup noteheadGroup);
};
}

#endif // MU_ENGRAVING_NOTEARTICULATIONSPARSER_H
