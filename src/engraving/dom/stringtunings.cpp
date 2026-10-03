/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore Limited and others
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
#include "pitchspelling.h"

#include "part.h"
#include "score.h"
#include "segment.h"
#include "staff.h"
#include "../editing/editpart.h"

#include "containers.h"
#include "translation.h"

using namespace mu;
using namespace mu::engraving;

String mu::engraving::stringTuningPitchName(int pitch, bool useFlat, NoteSpellingType spelling, bool withOctave)
{
    if (!pitchIsValid(pitch)) {
        return String();
    }

    const int tpc = pitch2tpc(pitch, Key::C, useFlat ? Prefer::FLATS : Prefer::SHARPS);

    String step;
    String accidental;
    tpc2name(tpc, spelling, NoteCaseType::CAPITAL, step, accidental);

    if (spelling == NoteSpellingType::GERMAN && tpc == TPC_B_B) {
        accidental.clear();
    }

    String result = convertPitchStringFlatsAndSharpsToUnicode(step + accidental);

    if (withOctave) {
        const int octave = (pitch / PITCH_DELTA_OCTAVE) - 1;
        result += String::number(octave);
    }

    return result;
}

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

    const NoteSpellingType spelling = style().styleV(Sid::chordSymbolSpelling).value<NoteSpellingType>();

    String elementName = score() ? score()->getTextStyleUserName(TextStyleType::STRING_TUNINGS).translated()
                         : TConv::translatedUserName(TextStyleType::STRING_TUNINGS);
    String info;

    const std::vector<instrString>& stringList = stringData->stringList();
    int numOfStrings = static_cast<int>(stringList.size());
    for (int i = 0; i < numOfStrings; ++i) {
        string_idx_t index = numOfStrings - i - 1;
        if (muse::contains(m_visibleStrings, index)) {
            const instrString& stringInfo = stringList[index];
            String pitchStr = stringTuningPitchName(stringInfo.pitch, stringInfo.useFlat, spelling, true);
            if (pitchStr.empty()) {
                LOGE() << "Invalid get pitch name for " << stringInfo.pitch;
                continue;
            }

            info += muse::mtrc("engraving", "String %1").arg(String::number(i + 1)) + u", "
                    + muse::mtrc("engraving", "Value %1").arg(pitchStr) + u"; ";
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

void StringTunings::triggerLayout() const
{
    if (!part()) {
        return StaffTextBase::triggerLayout();
    }

    Fraction startTick = tick();

    const std::map<int, StringTunings*>& allStringTuningsOnThisPart = part()->stringTunings();
    auto iterOfNextStringTuningOnThisPart = allStringTuningsOnThisPart.upper_bound(startTick.ticks());
    StringTunings* nextStringTuning = iterOfNextStringTuningOnThisPart != allStringTuningsOnThisPart.end()
                                      ? iterOfNextStringTuningOnThisPart->second : nullptr;

    Fraction endTick = nextStringTuning ? nextStringTuning->tick() : score()->endTick();

    score()->setLayout(startTick, endTick, staffIdx(), staffIdx(), this);
}

String StringTunings::generateText() const
{
    const StringData* stringData = this->stringData();
    if (!stringData || stringData->isNull()) {
        return u"";
    }

    const NoteSpellingType spelling = style().styleV(Sid::chordSymbolSpelling).value<NoteSpellingType>();

    auto guitarStringSymbol = [](int i) { return String(u"<sym>guitarString") + String::number(i) + u"</sym>"; };

    const std::vector<instrString>& stringList = stringData->stringList();
    std::vector<String> visibleStringList;
    int numOfStrings = static_cast<int>(stringList.size());
    for (int i = 0; i < numOfStrings; ++i) {
        string_idx_t index = numOfStrings - i - 1;
        if (muse::contains(m_visibleStrings, index)) {
            const instrString& stringInfo = stringList[index];
            String pitchStr = stringTuningPitchName(stringInfo.pitch, stringInfo.useFlat, spelling, false);
            if (pitchStr.empty()) {
                LOGE() << "Invalid get pitch name for " << stringInfo.pitch;
                continue;
            }

            visibleStringList.emplace_back(String(guitarStringSymbol(i + 1) + u" \u2013 " + pitchStr) + u"  ");
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
