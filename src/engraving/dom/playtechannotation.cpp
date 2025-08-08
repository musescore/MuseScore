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

#include "playtechannotation.h"

#include "segment.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

static const ElementStyle annotationStyle {
    { Sid::staffTextPlacement, Pid::PLACEMENT },
    { Sid::staffTextMinDistance, Pid::MIN_DISTANCE },
};

PlayTechAnnotation::PlayTechAnnotation(Segment* parent, PlayingTechniqueType techniqueType, TextStyleType tid)
    : StaffTextBase(ElementType::PLAYTECH_ANNOTATION, parent, tid, ElementFlag::MOVABLE | ElementFlag::ON_STAFF),
    m_techniqueType(techniqueType)
{
    initElementStyle(&annotationStyle);
}

PlayingTechniqueType PlayTechAnnotation::techniqueType() const
{
    return m_techniqueType;
}

void PlayTechAnnotation::setTechniqueType(const PlayingTechniqueType techniqueType)
{
    m_techniqueType = techniqueType;
}

PlayTechAnnotation* PlayTechAnnotation::clone() const
{
    return new PlayTechAnnotation(*this);
}

bool PlayTechAnnotation::isHandbellsSymbol() const
{
    return static_cast<int>(m_techniqueType) >= static_cast<int>(PlayingTechniqueType::HandbellsSwing)
           && static_cast<int>(m_techniqueType) <= static_cast<int>(PlayingTechniqueType::HandbellsDamp);
}

PropertyValue PlayTechAnnotation::getProperty(Pid id) const
{
    switch (id) {
    case Pid::PLAY_TECH_TYPE:
        return m_techniqueType;
    default:
        return StaffTextBase::getProperty(id);
    }
}

bool PlayTechAnnotation::setProperty(Pid propertyId, const PropertyValue& val)
{
    switch (propertyId) {
    case Pid::PLAY_TECH_TYPE:
        m_techniqueType = PlayingTechniqueType(val.toInt());
        break;
    default:
        if (!StaffTextBase::setProperty(propertyId, val)) {
            return false;
        }
        break;
    }

    triggerLayout();
    return true;
}

PropertyValue PlayTechAnnotation::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::TEXT_STYLE:
        return TextStyleType::STAFF;
    case Pid::PLAY_TECH_TYPE:
        return PlayingTechniqueType::Natural;
    default:
        return StaffTextBase::propertyDefault(id);
    }
}
