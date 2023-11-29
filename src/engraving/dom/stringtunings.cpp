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
#include "translation.h"

using namespace mu;
using namespace mu::engraving;

// STYLE
static const ElementStyle STRING_TUNINGS_STYLE {
    { Sid::staffTextPlacement, Pid::PLACEMENT },
    { Sid::staffTextMinDistance, Pid::MIN_DISTANCE },
};

StringTunings::StringTunings(Segment* parent, TextStyleType textStyleType)
    : StaffTextBase(ElementType::STRING_TUNINGS, parent, textStyleType, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
    initElementStyle(&STRING_TUNINGS_STYLE);
}

StringTunings::StringTunings(const StringTunings& s)
    : StaffTextBase(s)
{
    m_preset = s.m_preset;
    m_visibleStrings = s.m_visibleStrings;
    m_stringData = s.m_stringData;

    m_noStringVisible = s.m_noStringVisible;
    m_stringsNumber = s.m_stringsNumber;
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
        Fraction tick = this->tick();
        if (staff()->isTabStaff(tick)) {
            return staff()->lines(tick);
        } else {
            for (const Staff* _staff : staff()->staffList()) {
                if (_staff == staff()) {
                    continue;
                }

                if (_staff->score() == staff()->score() && _staff->isTabStaff(tick)) {
                    return _staff->lines(tick);
                }
            }
        }

        if (m_stringsNumber.has_value()) {
            return m_stringsNumber.value();
        }

        return stringData()->strings();
    } else if (id == Pid::STRINGTUNINGS_PRESET) {
        return m_preset;
    } else if (id == Pid::STRINGTUNINGS_VISIBLE_STRINGS) {
        std::vector<int> visibleStrings;
        for (string_idx_t string : m_visibleStrings) {
            visibleStrings.emplace_back(static_cast<int>(string));
        }

        return visibleStrings;
    }

    return StaffTextBase::getProperty(id);
}

PropertyValue StringTunings::propertyDefault(Pid id) const
{
    if (id == Pid::STRINGTUNINGS_STRINGS_COUNT) {
        return stringData()->strings();
    } else if (id == Pid::STRINGTUNINGS_PRESET) {
        return String();
    } else if (id == Pid::STRINGTUNINGS_VISIBLE_STRINGS) {
        return {};
    }

    return StaffTextBase::propertyDefault(id);
}

bool StringTunings::setProperty(Pid id, const PropertyValue& val)
{
    if (id == Pid::STRINGTUNINGS_STRINGS_COUNT) {
        Fraction tick = this->tick();
        if (staff()->isTabStaff(tick)) {
            staff()->staffType(tick)->setLines(val.toInt());
        } else {
            for (Staff* _staff : staff()->staffList()) {
                if (_staff == staff()) {
                    continue;
                }

                if (_staff->score() == staff()->score() && _staff->isTabStaff(tick)) {
                    _staff->staffType(tick)->setLines(val.toInt());
                }
            }
        }

        m_stringsNumber = val.toInt();
    } else if (id == Pid::STRINGTUNINGS_PRESET) {
        m_preset = val.value<String>();
    } else if (id == Pid::STRINGTUNINGS_VISIBLE_STRINGS) {
        m_visibleStrings.clear();
        std::vector<int> ignoredStrings = val.value<std::vector<int> >();
        for (int string : ignoredStrings) {
            m_visibleStrings.emplace_back(static_cast<string_idx_t>(string));
        }
    } else {
        return StaffTextBase::setProperty(id, val);
    }

    triggerLayout();
    return true;
}

String StringTunings::accessibleInfo() const
{
    const StringData* stringData = this->stringData();
    if (stringData->isNull()) {
        return String();
    }

    String elementName = score() ? score()->getTextStyleUserName(TextStyleType::STRING_TUNINGS).translated()
                         : TConv::translatedUserName(TextStyleType::STRING_TUNINGS);
    String info;

    const std::vector<instrString>& stringList = stringData->stringList();
    int numOfStrings = static_cast<int>(stringList.size());
    for (int i = 0; i < numOfStrings; ++i) {
        string_idx_t index = numOfStrings - i - 1;
        if (mu::contains(m_visibleStrings, index)) {
            const instrString str = stringList[index];
            String pitchStr = pitch2string(str.pitch, str.useFlat);
            if (pitchStr.empty()) {
                LOGE() << "Invalid get pitch name for " << str.pitch;
                continue;
            }

            info += mtrc("engraving", "String %1").arg(String::number(i + 1)) + u", "
                    + mtrc("engraving", "Value %1").arg(pitchStr) + u"; ";
        }
    }

    return String(u"%1: %2").arg(elementName, info);
}

String StringTunings::screenReaderInfo() const
{
    return accessibleInfo();
}

const StringData* StringTunings::stringData() const
{
    return &m_stringData;
}

void StringTunings::setStringData(const StringData& stringData)
{
    m_stringData.set(stringData);
    triggerLayout();
}

void StringTunings::undoStringData(const StringData& stringData)
{
    score()->undo(new ChangeStringData(this, stringData));
    triggerLayout();
}

const String& StringTunings::preset() const
{
    return m_preset;
}

void StringTunings::setPreset(const String& preset)
{
    m_preset = preset;
}

const std::vector<string_idx_t>& StringTunings::visibleStrings() const
{
    return m_visibleStrings;
}

void StringTunings::setVisibleStrings(const std::vector<string_idx_t>& visibleStrings)
{
    m_visibleStrings = visibleStrings;
}

void StringTunings::updateText()
{
    String updatedText = generateText();
    undoChangeProperty(Pid::TEXT, updatedText, PropertyFlags::STYLED);

    if (updatedText.empty()) {
        m_noStringVisible = true;
    } else {
        m_noStringVisible = false;
    }
}

bool StringTunings::noStringVisible() const
{
    return m_noStringVisible;
}

String StringTunings::generateText() const
{
    const StringData* stringData = this->stringData();
    if (!stringData || stringData->isNull()) {
        return u"";
    }

    auto guitarStringSymbol = [](int i) { return String(u"<sym>guitarString") + String::number(i) + u"</sym>"; };

    const std::vector<instrString>& stringList = stringData->stringList();
    std::vector<String> visibleStringList;
    int numOfStrings = static_cast<int>(stringList.size());
    for (int i = 0; i < numOfStrings; ++i) {
        string_idx_t index = numOfStrings - i - 1;
        if (mu::contains(m_visibleStrings, index)) {
            const instrString str = stringList[index];
            String pitchStr = pitch2string(str.pitch, str.useFlat);
            if (pitchStr.empty()) {
                LOGE() << "Invalid get pitch name for " << str.pitch;
                continue;
            }

            Char accidental;
            if (pitchStr.size() > 1) {
                Char sym(pitchStr[1]);
                if (!sym.isDigit()) {
                    accidental = sym;
                }
            }

            visibleStringList.emplace_back(String(guitarStringSymbol(i + 1) + u" \u2013 "
                                                  + String(pitchStr[0]).toUpper() + accidental) + u"  ");
        }
    }

    if (visibleStringList.empty()) {
        return u"";
    }

    size_t columnCount = 0;
    size_t rowCount = 0;

    if (visibleStringList.size() < 4) {
        rowCount = visibleStringList.size();
        columnCount = 1;
    } else {
        rowCount = std::ceil(static_cast<double>(visibleStringList.size()) / 2);
        columnCount = 2;
    }

    String result;
    for (size_t i = 0; i < rowCount; ++i) {
        for (size_t j = 0; j < columnCount; ++j) {
            size_t index = i + j * rowCount;
            if (index < visibleStringList.size()) {
                result += visibleStringList[index];
            }
            result += u"\t";
        }
        result += u"\n";
    }

    return result;
}
