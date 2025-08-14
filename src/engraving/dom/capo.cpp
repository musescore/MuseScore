/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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

#include "translation.h"

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
    } else if (id == Pid::CAPO_GENERATE_TEXT) {
        return m_shouldAutomaticallyGenerateText;
    } else if (id == Pid::CAPO_TRANSPOSE_MODE) {
        return m_params.transposeMode;
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
    } else if (id == Pid::CAPO_GENERATE_TEXT) {
        return true;
    } else if (id == Pid::CAPO_TRANSPOSE_MODE) {
        return (int)CapoParams::TransposeMode::PLAYBACK_ONLY;
    }

    return StaffTextBase::propertyDefault(id);
}

bool Capo::setProperty(Pid id, const PropertyValue& val)
{
    if (id == Pid::ACTIVE) {
        bool active = val.toBool();
        switch (m_params.transposeMode) {
        case CapoParams::TransposeMode::NOTATION_ONLY:
            m_params.transition = CapoParams::Transition::PB_TO_NOTATION;
            break;
        case CapoParams::TransposeMode::TAB_ONLY:
            m_params.transition = CapoParams::Transition::PB_TO_TAB;
            break;
        case CapoParams::TransposeMode::PLAYBACK_ONLY:
        default:
            break;
        }
        m_params.active = active;
    } else if (id == Pid::CAPO_FRET_POSITION) {
        m_params.fretPosition = val.toInt();
        switch (m_params.transposeMode) {
        case CapoParams::TransposeMode::NOTATION_ONLY:
            m_params.transition = CapoParams::Transition::UPDATE_NOTES;
            break;
        case CapoParams::TransposeMode::TAB_ONLY:
            m_params.transition = CapoParams::Transition::UPDATE_FRETS;
            break;
        case CapoParams::TransposeMode::PLAYBACK_ONLY:
            break;
        }
    } else if (id == Pid::CAPO_IGNORED_STRINGS) {
        m_params.ignoredStrings.clear();
        std::vector<int> ignoredStrings = val.value<std::vector<int> >();
        for (int string : ignoredStrings) {
            m_params.ignoredStrings.insert(static_cast<string_idx_t>(string));
        }
    } else if (id == Pid::CAPO_GENERATE_TEXT) {
        m_shouldAutomaticallyGenerateText = val.toBool();

        if (!m_shouldAutomaticallyGenerateText) {
            setXmlText(m_customText);
        }
    } else if (id == Pid::CAPO_TRANSPOSE_MODE) {
        auto newMode = (CapoParams::TransposeMode)val.toInt();
        switch (newMode) {
        case CapoParams::TransposeMode::PLAYBACK_ONLY:
            if (CapoParams::TransposeMode::TAB_ONLY == m_params.transposeMode) {
                m_params.transition = CapoParams::Transition::TAB_TO_PB;
            } else if (CapoParams::TransposeMode::NOTATION_ONLY == m_params.transposeMode) {
                m_params.transition = CapoParams::Transition::NOTATION_TO_PB;
            } else {
            }

            break;
        case CapoParams::TransposeMode::NOTATION_ONLY:
            if (CapoParams::TransposeMode::PLAYBACK_ONLY == m_params.transposeMode) {
                m_params.transition = CapoParams::Transition::PB_TO_NOTATION;
            } else if (CapoParams::TransposeMode::TAB_ONLY == m_params.transposeMode) {
                m_params.transition = CapoParams::Transition::TAB_TO_NOTATION;
            } else {
            }
            break;
        case CapoParams::TransposeMode::TAB_ONLY:
            if (CapoParams::TransposeMode::PLAYBACK_ONLY == m_params.transposeMode) {
                m_params.transition = CapoParams::Transition::PB_TO_TAB;
            } else if (CapoParams::TransposeMode::NOTATION_ONLY == m_params.transposeMode) {
                m_params.transition = CapoParams::Transition::NOTATION_TO_TAB;
            } else {
            }
            break;
        default:
            break;
        }
        m_params.transposeMode = newMode;
    } else {
        return StaffTextBase::setProperty(id, val);
    }

    triggerLayout();
    return true;
}

void Capo::setXmlText(const String& text)
{
    if (!m_shouldAutomaticallyGenerateText) {
        m_customText = text;
    }

    StaffTextBase::setXmlText(text);
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

bool Capo::shouldAutomaticallyGenerateText() const
{
    return m_shouldAutomaticallyGenerateText;
}

muse::String Capo::generateText(size_t stringCount) const
{
    if (!m_params.active || m_params.fretPosition == 0) {
        return muse::mtrc("engraving", "No capo");
    }

    if (m_params.ignoredStrings.empty()) {
        return muse::mtrc("engraving", "Capo %1").arg(m_params.fretPosition);
    }

    StringList stringsToApply;

    for (string_idx_t idx = 0; idx < stringCount; ++idx) {
        if (muse::contains(m_params.ignoredStrings, idx)) {
            continue;
        }

        stringsToApply.emplace_back(String::number(idx + 1));
    }

    if (stringsToApply.empty()) {
        return muse::mtrc("engraving", "No capo");
    }

    String text = muse::mtrc("engraving", "Partial capo:\nFret %1 on strings %2")
                  .arg(m_params.fretPosition)
                  .arg(stringsToApply.join(u", "));

    return text;
}
