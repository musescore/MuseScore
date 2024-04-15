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
#ifndef MU_ENGRAVING_IENGRAVINGELEMENTSPROVIDER_H
#define MU_ENGRAVING_IENGRAVINGELEMENTSPROVIDER_H

#include <unordered_set>

#include "modularity/imoduleinterface.h"
#include "async/channel.h"

namespace mu::engraving {
class EngravingObject;
using EngravingObjectSet = std::unordered_set<const EngravingObject*>;
class IEngravingElementsProvider : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IEngravingElementsProvider)
public:
    virtual ~IEngravingElementsProvider() = default;

    // statistic
    virtual void clearStatistic() = 0;
    virtual void printStatistic(const std::string& title) = 0;

    // register
    virtual void reg(const mu::engraving::EngravingObject* e) = 0;
    virtual void unreg(const mu::engraving::EngravingObject* e) = 0;
    virtual const EngravingObjectSet& elements() const = 0;

    // debug draw
    virtual void select(const mu::engraving::EngravingObject* e, bool arg) = 0;
    virtual bool isSelected(const mu::engraving::EngravingObject* e) const = 0;
    virtual muse::async::Channel<const mu::engraving::EngravingObject*, bool> selectChanged() const = 0;
};
}

#endif // MU_ENGRAVING_IENGRAVINGELEMENTSPROVIDER_H
