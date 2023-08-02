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

#ifndef MU_ENGRAVING_STRING_TUNINGS_H
#define MU_ENGRAVING_STRING_TUNINGS_H

#include "stafftextbase.h"
#include "stringdata.h"

namespace mu::engraving {
class StringTunings final : public StaffTextBase
{
    OBJECT_ALLOCATOR(engraving, StringTunings)
    DECLARE_CLASSOF(ElementType::STRING_TUNINGS)

    String m_preset;

public:
    explicit StringTunings(Segment* parent, TextStyleType textStyleType = TextStyleType::STAFF);
    StringTunings(const StringTunings& s);

    StringTunings* clone() const override;
    bool isEditable() const override;

    PropertyValue getProperty(Pid id) const override;
    PropertyValue propertyDefault(Pid id) const override;
    bool setProperty(Pid id, const PropertyValue& val) override;

    const StringData* stringData() const;
    void setStringData(const StringData& stringData);

    String preset() const;
    void setPreset(const String& preset);

    void updateText();

private:
    String generateText() const;
};
}

#endif // MU_ENGRAVING_STRING_TUNINGS_H
