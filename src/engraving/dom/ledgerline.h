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

#ifndef MU_ENGRAVING_LEDGERLINE_H
#define MU_ENGRAVING_LEDGERLINE_H

#include "engravingitem.h"

namespace mu::engraving {
class Chord;

//---------------------------------------------------------
//    @@ LedgerLine
///     Graphic representation of a ledger line.
//!
//!    parent:     Chord
//!    x-origin:   Chord
//!    y-origin:   SStaff
//---------------------------------------------------------

class LedgerLine final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, LedgerLine)
    DECLARE_CLASSOF(ElementType::LEDGER_LINE)

public:
    LedgerLine(EngravingItem*);
    ~LedgerLine();
    LedgerLine& operator=(const LedgerLine&) = delete;

    LedgerLine* clone() const override { return new LedgerLine(*this); }

    PointF pagePos() const override;        ///< position in page coordinates
    Chord* chord() const { return toChord(explicitParent()); }

    double len() const { return m_len; }
    void setLen(double v) { m_len = v; }

    void setVertical(bool v) { m_vertical = v; }
    bool vertical() const { return m_vertical; }

    double measureXPos() const;

    void spatiumChanged(double /*oldValue*/, double /*newValue*/) override;

    struct LayoutData : public EngravingItem::LayoutData {
        double lineWidth = 0.0;
    };
    DECLARE_LAYOUTDATA_METHODS(LedgerLine);

private:

    double m_len = 0.0;
    bool m_vertical = false;
};
} // namespace mu::engraving
#endif
