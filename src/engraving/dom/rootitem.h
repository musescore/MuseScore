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
#ifndef MU_ENGRAVING_ROOTITEM_H
#define MU_ENGRAVING_ROOTITEM_H

#include "engravingitem.h"

#include "../compat/dummyelement.h"

namespace mu::engraving {
class Score;

class RootItem : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, RootItem)
public:
    RootItem(Score* score);
    ~RootItem() override;

    compat::DummyElement* dummy() const;
    void init();

    EngravingObject* scanParent() const override;

    EngravingItem* clone() const override { return nullptr; }
    PropertyValue getProperty(Pid) const override { return PropertyValue(); }
    bool setProperty(Pid, const PropertyValue&) override { return false; }

private:

#ifndef ENGRAVING_NO_ACCESSIBILITY
    AccessibleItemPtr createAccessible() override;
#endif

    Score* m_score = nullptr;
    compat::DummyElement* m_dummy = nullptr;
};
}

#endif // MU_ENGRAVING_ROOTITEM_H
