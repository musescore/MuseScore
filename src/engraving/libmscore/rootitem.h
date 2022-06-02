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
#ifndef MU_ENGRAVING_ROOTITEM_H
#define MU_ENGRAVING_ROOTITEM_H

#include "engravingitem.h"

#include "compat/dummyelement.h"

namespace mu::engraving {
class Score;
}

namespace mu::engraving {
class RootItem : public mu::engraving::EngravingItem
{
public:
    RootItem(mu::engraving::Score* score);
    ~RootItem() override;

    compat::DummyElement* dummy() const;
    void init();

    EngravingObject* scanParent() const override;

    mu::engraving::EngravingItem* clone() const override { return nullptr; }
    mu::engraving::PropertyValue getProperty(mu::engraving::Pid) const override { return mu::engraving::PropertyValue(); }
    bool setProperty(mu::engraving::Pid, const mu::engraving::PropertyValue&) override { return false; }

private:

    mu::engraving::AccessibleItem* createAccessible() override;

    mu::engraving::Score* m_score = nullptr;
    compat::DummyElement* m_dummy = nullptr;
};
}

#endif // MU_ENGRAVING_ROOTITEM_H
