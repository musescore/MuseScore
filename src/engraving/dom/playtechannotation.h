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

#ifndef MU_ENGRAVING_PLAYINGTECHNIQUEANNOTATION_H
#define MU_ENGRAVING_PLAYINGTECHNIQUEANNOTATION_H

#include "stafftextbase.h"
#include "../types/types.h"

namespace mu::engraving {
class PlayTechAnnotation final : public StaffTextBase
{
    OBJECT_ALLOCATOR(engraving, PlayTechAnnotation)
    DECLARE_CLASSOF(ElementType::PLAYTECH_ANNOTATION)

public:
    PlayTechAnnotation(Segment* parent = nullptr, PlayingTechniqueType techniqueType = PlayingTechniqueType::Natural,
                       TextStyleType tid = TextStyleType::STAFF);

    PlayingTechniqueType techniqueType() const;
    void setTechniqueType(const PlayingTechniqueType techniqueType);

    PlayTechAnnotation* clone() const override;

private:

    PropertyValue getProperty(Pid id) const override;
    bool setProperty(Pid propertyId, const PropertyValue& val) override;
    PropertyValue propertyDefault(Pid id) const override;

    PlayingTechniqueType m_techniqueType = PlayingTechniqueType::Undefined;
};
}

#endif // MU_ENGRAVING_PLAYINGTECHNIQUEANNOTATION_H
