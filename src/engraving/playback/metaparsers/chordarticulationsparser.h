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

namespace Ms {
class Chord;
}

namespace mu::engraving {
class ChordArticulationsParser : public MetaParserBase<ChordArticulationsParser>
{
public:
    void buildChordArticulationMap(const Ms::Chord* chord, const PlaybackContext& ctx, mpe::ArticulationMap& result) const;

protected:
    friend MetaParserBase;

    void doParse(const Ms::EngravingItem* item, const PlaybackContext& ctx, mpe::ArticulationMetaMap& result) const;

private:
    void parseSpanners(const Ms::Chord* chord, const PlaybackContext& ctx, mpe::ArticulationMetaMap& result) const;
    void parseArticulationSymbols(const Ms::Chord* chord, const PlaybackContext& ctx, mpe::ArticulationMetaMap& result) const;
    void parseAnnotations(const Ms::Chord* chord, const PlaybackContext& ctx, mpe::ArticulationMetaMap& result) const;
    void parseTremolo(const Ms::Chord* chord, const PlaybackContext& ctx, mpe::ArticulationMetaMap& result) const;
    void parseArpeggio(const Ms::Chord* chord, const PlaybackContext& ctx, mpe::ArticulationMetaMap& result) const;
    void parseGraceNotes(const Ms::Chord* chord, const PlaybackContext& ctx, mpe::ArticulationMetaMap& result) const;
};
}

#endif // CHORDARTICULATIONSPARSER_H
