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

#ifndef __HOOK_H__
#define __HOOK_H__

#include "symbol.h"

namespace Ms {
class Chord;

//---------------------------------------------------------
//   @@ Hook
//---------------------------------------------------------

class Hook final : public Symbol
{
    int _hookType { 0 };

public:
    Hook(Score* = 0);

    Hook* clone() const override { return new Hook(*this); }
    qreal mag() const override { return parent()->mag(); }
    Element* elementBase() const override;
    ElementType type() const override { return ElementType::HOOK; }
    void setHookType(int v);
    int hookType() const { return _hookType; }
    void layout() override;
    void draw(mu::draw::Painter*) const override;
    Chord* chord() const { return (Chord*)parent(); }
};
}     // namespace Ms
#endif
