/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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

#include "stringtunings.h"

#include "types/typesconv.h"
#include "utils.h"

#include "part.h"
#include "score.h"
#include "segment.h"
#include "undo.h"

#include "containers.h"

using namespace mu;
using namespace mu::engraving;

// STYLE
static const ElementStyle stringTuningsStyle {
    { Sid::staffTextPlacement, Pid::PLACEMENT },
    { Sid::staffTextMinDistance, Pid::MIN_DISTANCE },
};

StringTunings::StringTunings(Segment* parent, TextStyleType textStyleType)
    : StaffTextBase(ElementType::STRING_TUNINGS, parent, textStyleType, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
    initElementStyle(&stringTuningsStyle);
}

StringTunings::StringTunings(const StringTunings& s)
    : StaffTextBase(s)
{
}

StringTunings* StringTunings::clone() const
{
    return new StringTunings(*this);
}

bool StringTunings::isEditable() const
{
    return false;
}

PropertyValue StringTunings::getProperty(Pid id) const
{
    if (id == Pid::STRINGTUNINGS_STRINGS_COUNT) {
        if (staff()->isTabStaff(Fraction(0, 1))) {
            return staff()->lines(Fraction(0, 1));
        } else {
            for (Staff* _staff : staff()->staffList()) {
                if (_staff == staff()) {
                    continue;
                }

                if (_staff->score() == staff()->score() && _staff->isTabStaff(Fraction(0, 1))) {
                    return _staff->lines(Fraction(0, 1));
                }
            }
        }

        return part()->instrument(tick())->stringData()->strings();
    } else if (id == Pid::STRINGTUNINGS_PRESET) {
        return m_preset;
    }

    return StaffTextBase::getProperty(id);
}

PropertyValue StringTunings::propertyDefault(Pid id) const
{
    if (id == Pid::STRINGTUNINGS_STRINGS_COUNT) {
        return part()->instrument(tick())->stringData()->strings(); // todo
    } else if (id == Pid::STRINGTUNINGS_PRESET) {
        return String();
    }

    return StaffTextBase::propertyDefault(id);
}

bool StringTunings::setProperty(Pid id, const PropertyValue& val)
{
    if (id == Pid::STRINGTUNINGS_STRINGS_COUNT) {
        if (staff()->isTabStaff(Fraction(0, 1))) {
            staff()->staffType(Fraction(0, 1))->setLines(val.toInt());
        } else {
            for (Staff* _staff : staff()->staffList()) {
                if (_staff == staff()) {
                    continue;
                }

                if (_staff->score() == staff()->score() && _staff->isTabStaff(Fraction(0, 1))) {
                    _staff->staffType(Fraction(0, 1))->setLines(val.toInt());
                }
            }
        }
    } else if (id == Pid::STRINGTUNINGS_PRESET) {
        m_preset = val.value<String>();
    } else {
        return StaffTextBase::setProperty(id, val);
    }

    triggerLayout();
    return true;
}

const StringData* StringTunings::stringData() const
{
    return part()->instrument(tick())->stringData();
}

void StringTunings::undoStringData(const StringData& stringData)
{
    score()->undo(new ChangeStringData(part()->instrument(tick()), stringData));
    triggerLayout();
}

String StringTunings::preset() const
{
    return m_preset;
}

void StringTunings::setPreset(const String& preset)
{
    m_preset = preset;
}

void StringTunings::updateText()
{
    undoChangeProperty(Pid::TEXT, generateText(), PropertyFlags::STYLED);
}

String StringTunings::generateText() const
{
    if (!part() || !part()->instrument(tick())) {
        return String();
    }

    std::vector<instrString> stringList = part()->instrument(tick())->stringData()->stringList();
    std::vector<String> stringListOpened;
    for (size_t i = 0; i < stringList.size(); ++i) {
        if (stringList[i].open) {
            String pitchStr = pitch2string(stringList[i].pitch);
            if (pitchStr.empty()) {
                LOGE() << "Invalid get pitch name for " << stringList[i].pitch;
                continue;
            }

            stringListOpened.push_back(String(u"<sym>guitarString%1</sym> - %2").arg(String::number(i + 1), String(pitchStr[0]).toUpper()));
        }
    }

    if (stringListOpened.empty()) {
        return u"<sym>guitarString6</sym>"; // todo fork
    }

    int columnCount = 0;
    int rowCount = 0;

    if (stringListOpened.size() <= 4) {
        rowCount = stringListOpened.size();
        columnCount = 1;
    } else {
        rowCount = std::ceil(static_cast<double>(stringListOpened.size()) / 2);
        columnCount = 2;
    }

    String result;
    for (int i = 0; i < rowCount; ++i) {
        for (int j = 0; j < columnCount; ++j) {
            size_t index = i + j * rowCount;
            if (index < stringListOpened.size()) {
                result += stringListOpened[index];
            }
            result += u"\t";
        }
        result += u"\n";
    }

    return result;
}
