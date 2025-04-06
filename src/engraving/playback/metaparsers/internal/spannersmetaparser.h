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

#ifndef MU_ENGRAVING_SPANNERSMETAPARSER_H
#define MU_ENGRAVING_SPANNERSMETAPARSER_H

#include "playback/metaparsers/metaparserbase.h"

namespace mu::engraving {
class SpannersMetaParser : public MetaParserBase<SpannersMetaParser>
{
public:
    static bool isAbleToParse(const EngravingItem* spannerItem);

private:
    friend MetaParserBase;

    static void doParse(const EngravingItem* item, const RenderingContext& spannerCtx, muse::mpe::ArticulationMap& result);
};
}

#endif // MU_ENGRAVING_SPANNERSMETAPARSER_H
