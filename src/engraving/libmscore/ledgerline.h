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

#ifndef __LEDGERLINE_H__
#define __LEDGERLINE_H__

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
    LedgerLine(Score*);
    ~LedgerLine();
    LedgerLine& operator=(const LedgerLine&) = delete;

    LedgerLine* clone() const override { return new LedgerLine(*this); }

    mu::PointF pagePos() const override;        ///< position in page coordinates
    Chord* chord() const { return toChord(explicitParent()); }

    double len() const { return _len; }
    double lineWidth() const { return _width; }
    void setLen(double v) { _len = v; }
    void setLineWidth(double v) { _width = v; }
    void setVertical(bool v) { m_vertical = v; }
    bool vertical() const { return m_vertical; }

    void draw(mu::draw::Painter*) const override;

    double measureXPos() const;
    LedgerLine* next() const { return _next; }
    void setNext(LedgerLine* l) { _next = l; }

    void spatiumChanged(double /*oldValue*/, double /*newValue*/) override;

private:
    double _width;
    double _len;
    LedgerLine* _next;
    bool m_vertical = false;
};
} // namespace mu::engraving
#endif
