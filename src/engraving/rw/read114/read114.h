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

#ifndef MU_ENGRAVING_READ114_H
#define MU_ENGRAVING_READ114_H

#include "../ireader.h"

namespace mu::engraving::read114 {
class Read114 : public rw::IReader
{
public:
    //---------------------------------------------------------
    //   read114
    //    import old version <= 1.3 files
    //---------------------------------------------------------
    Err readScore(Score* masterScore, XmlReader& e, rw::ReadInOutData* out) override;

    bool pasteStaff(XmlReader& e, Segment* dst, staff_idx_t dstStaff, Fraction scale) override;
    void pasteSymbols(XmlReader& e, ChordRest* dst) override;

    void readTremoloCompat(compat::TremoloCompat* item, XmlReader& xml) override;

private:
    void doReadItem(EngravingItem* item, XmlReader& xml) override;
};
}

#endif // MU_ENGRAVING_READ114_H
