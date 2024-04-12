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
#ifndef INOTATIONELEMENTS_H
#define INOTATIONELEMENTS_H

#include <vector>

#include "modularity/imoduleinterface.h"
#include "notationtypes.h"

namespace mu::notation {
class INotationElements
{
public:
    virtual ~INotationElements() = default;

    virtual mu::engraving::Score* msScore() const = 0;

    virtual EngravingItem* search(const std::string& searchText) const = 0;
    virtual std::vector<EngravingItem*> elements(const FilterElementsOptions& elementOptions = FilterElementsOptions()) const = 0;

    virtual Measure* measure(const int measureIndex) const = 0;

    virtual PageList pages() const = 0;
    virtual const Page* pageByPoint(const muse::PointF& point) const = 0;
};

using INotationElementsPtr = std::shared_ptr<INotationElements>;
}

#endif // INOTATIONELEMENTS_H
