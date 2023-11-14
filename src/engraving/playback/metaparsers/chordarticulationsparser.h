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

#ifndef CHORDARTICULATIONSPARSER_H
#define CHORDARTICULATIONSPARSER_H

#include "metaparserbase.h"

namespace mu::engraving {
class Chord;

class ChordArticulationsParser : public MetaParserBase<ChordArticulationsParser>
{
public:
    static void buildChordArticulationMap(const Chord* chord, const RenderingContext& ctx, mpe::ArticulationMap& result);

protected:
    friend MetaParserBase;

    static void doParse(const EngravingItem* item, const RenderingContext& ctx, mpe::ArticulationMap& result);

private:
    static void parseSpanners(const Chord* chord, const RenderingContext& ctx, mpe::ArticulationMap& result);
    static void parseBends(const Chord* chord, const RenderingContext& ctx, mpe::ArticulationMap& result);
    static void parseArticulationSymbols(const Chord* chord, const RenderingContext& ctx, mpe::ArticulationMap& result);
    static void parseAnnotations(const Chord* chord, const RenderingContext& ctx, mpe::ArticulationMap& result);
    static void parseTremolo(const Chord* chord, const RenderingContext& ctx, mpe::ArticulationMap& result);
    static void parseArpeggio(const Chord* chord, const RenderingContext& ctx, mpe::ArticulationMap& result);
    static void parseGraceNotes(const Chord* chord, const RenderingContext& ctx, mpe::ArticulationMap& result);
    static void parseChordLine(const Chord* chord, const RenderingContext& ctx, mpe::ArticulationMap& result);
};
}

#endif // CHORDARTICULATIONSPARSER_H
