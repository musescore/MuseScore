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

namespace Ms {
class Score;
}

namespace mu::engraving {
class RootItem : public Ms::EngravingItem
{
public:
    RootItem(Ms::Score* score);
    ~RootItem() override;

    compat::DummyElement* dummy() const;
    void init();

    EngravingObject* scanParent() const override;
    EngravingObject* scanChild(int n) const override;
    int scanChildCount() const override;

    Ms::EngravingItem* clone() const override { return nullptr; }
    mu::engraving::PropertyValue getProperty(Ms::Pid) const override { return mu::engraving::PropertyValue(); }
    bool setProperty(Ms::Pid, const mu::engraving::PropertyValue&) override { return false; }

private:

    mu::engraving::AccessibleItem* createAccessible() override;

    Ms::Score* m_score = nullptr;
    compat::DummyElement* m_dummy = nullptr;
};
}

#endif // MU_ENGRAVING_ROOTITEM_H
