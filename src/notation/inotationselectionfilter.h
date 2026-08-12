/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#pragma once

#include "engraving/dom/selectionfilter.h"

namespace mu::notation {
class INotationSelectionFilter
{
public:
    virtual ~INotationSelectionFilter() = default;

    virtual bool isSelectionTypeFiltered(const engraving::SelectionFilterTypesVariant& variant) const = 0;
    virtual void setSelectionTypeFiltered(const engraving::SelectionFilterTypesVariant& variant, bool filtered) = 0;

    virtual bool includeSingleNotes() const = 0;
    virtual void setIncludeSingleNotes(bool include) = 0;
};

using INotationSelectionFilterPtr = std::shared_ptr<INotationSelectionFilter>;
}
