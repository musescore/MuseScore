/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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

#include "capo.h"

#include "segment.h"

using namespace mu::engraving;

static const ElementStyle CAPO_STYLE {
    { Sid::staffTextPlacement, Pid::PLACEMENT },
    { Sid::staffTextMinDistance, Pid::MIN_DISTANCE },
};

Capo::Capo(Segment* parent, TextStyleType textStyleType)
    : StaffTextBase(ElementType::CAPO, parent, textStyleType, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
    initElementStyle(&CAPO_STYLE);

    m_params.active = true;
    m_params.fretPosition = 1;
}

Capo* Capo::clone() const
{
    return new Capo(*this);
}

PropertyValue Capo::getProperty(Pid id) const
{
    if (id == Pid::ACTIVE) {
        return m_params.active;
    } else if (id == Pid::CAPO_FRET_POSITION) {
        return m_params.fretPosition;
    } else if (id == Pid::CAPO_IGNORED_STRINGS) {
        std::vector<int> ignoredStrings;
        for (string_idx_t string : m_params.ignoredStrings) {
            ignoredStrings.push_back(static_cast<int>(string));
        }

        return ignoredStrings;
    }

    return StaffTextBase::getProperty(id);
}

PropertyValue Capo::propertyDefault(Pid id) const
{
    if (id == Pid::ACTIVE) {
        return true;
    } else if (id == Pid::CAPO_FRET_POSITION) {
        return 1;
    } else if (id == Pid::CAPO_IGNORED_STRINGS) {
        return std::vector<int>();
    }

    return StaffTextBase::propertyDefault(id);
}

bool Capo::setProperty(Pid id, const PropertyValue& val)
{
    if (id == Pid::ACTIVE) {
        m_params.active = val.toBool();
    } else if (id == Pid::CAPO_FRET_POSITION) {
        m_params.fretPosition = val.toInt();
    } else if (id == Pid::CAPO_IGNORED_STRINGS) {
        m_params.ignoredStrings.clear();
        std::vector<int> ignoredStrings = val.value<std::vector<int>>();
        for (int string : ignoredStrings) {
            m_params.ignoredStrings.insert(static_cast<string_idx_t>(string));
        }
    } else {
        return StaffTextBase::setProperty(id, val);
    }

    triggerLayout();
    return true;
}

bool Capo::isEditable() const
{
    return false;
}

const CapoParams& Capo::params() const
{
    return m_params;
}

void Capo::setParams(const CapoParams& params)
{
    m_params = params;
}
