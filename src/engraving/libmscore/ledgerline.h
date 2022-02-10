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

namespace Ms {
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
    qreal _width;
    qreal _len;
    LedgerLine* _next;
    bool vertical { false };

public:
    LedgerLine(Score*);
    ~LedgerLine();
    LedgerLine& operator=(const LedgerLine&) = delete;

    LedgerLine* clone() const override { return new LedgerLine(*this); }

    mu::PointF pagePos() const override;        ///< position in page coordinates
    Chord* chord() const { return toChord(explicitParent()); }

    qreal len() const { return _len; }
    qreal lineWidth() const { return _width; }
    void setLen(qreal v) { _len = v; }
    void setLineWidth(qreal v) { _width = v; }

    void layout() override;
    void draw(mu::draw::Painter*) const override;

    qreal measureXPos() const;
    LedgerLine* next() const { return _next; }
    void setNext(LedgerLine* l) { _next = l; }

    void writeProperties(XmlWriter& xml) const override;
    bool readProperties(XmlReader&) override;
    void spatiumChanged(qreal /*oldValue*/, qreal /*newValue*/) override;
};
}     // namespace Ms
#endif
