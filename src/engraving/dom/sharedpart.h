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
#pragma once

#include "part.h"

namespace mu::engraving {
using SharedTracksMap = std::map<track_idx_t, track_idx_t>;

class SharedPart final : public Part
{
    OBJECT_ALLOCATOR(engraving, SharedPart)
    DECLARE_CLASSOF(ElementType::SHARED_PART)

public:
    SharedPart(Score* score);

    void addOriginPart(Part* p);
    void removeOriginPart(Part* p);
    const std::vector<Part*> originParts() const { return m_originParts; }

    String partName() const override;

private:
    std::vector<Part*> m_originParts;
    std::map<Fraction, SharedTracksMap> m_trackMaps;
};
}
