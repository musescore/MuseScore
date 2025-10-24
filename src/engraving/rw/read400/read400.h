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
#pragma once

#include "../ireader.h"

namespace mu::engraving {
class Score;
class XmlReader;
}

namespace mu::engraving::read400 {
class ReadContext;
class Read400 : public rw::IReader
{
public:

    muse::Ret readScoreFile(Score* score, XmlReader& e, rw::ReadInOutData* data) override;

    static bool readScoreTag(Score* score, XmlReader& e, ReadContext& ctx);

    bool pasteStaff(XmlReader& e, Segment* dst, staff_idx_t dstStaff, Fraction scale) override;
    void pasteSymbols(XmlReader& e, ChordRest* dst) override;
    void readTremoloCompat(compat::TremoloCompat* item, XmlReader& xml) override;

private:
    void preparePasteDurationElement(Score* score, const Fraction& tick, const Fraction& ticks, const track_idx_t track);
    void doReadItem(EngravingItem* item, XmlReader& xml) override;
};
}
