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
#ifndef MU_DIAGNOSTICS_IENGRAVINGELEMENTSPROVIDER_H
#define MU_DIAGNOSTICS_IENGRAVINGELEMENTSPROVIDER_H

#include <unordered_set>
#include "modularity/imoduleexport.h"
#include "async/channel.h"

namespace Ms {
class EngravingObject;
}

namespace mu::diagnostics {
using EngravingObjectList = std::unordered_set<const Ms::EngravingObject*>;
class IEngravingElementsProvider : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IEngravingElementsProvider)
public:
    virtual ~IEngravingElementsProvider() = default;

    // statistic
    virtual void clearStatistic() = 0;
    virtual void printStatistic(const std::string& title) = 0;

    // register
    virtual void reg(const Ms::EngravingObject* e) = 0;
    virtual void unreg(const Ms::EngravingObject* e) = 0;
    virtual const EngravingObjectList& elements() const = 0;

    // debug draw
    virtual void select(const Ms::EngravingObject* e, bool arg) = 0;
    virtual bool isSelected(const Ms::EngravingObject* e) const = 0;
    virtual async::Channel<const Ms::EngravingObject*, bool> selectChanged() const = 0;
};
}

#endif // MU_DIAGNOSTICS_IENGRAVINGELEMENTSPROVIDER_H
