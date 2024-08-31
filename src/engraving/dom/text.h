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

#ifndef MU_ENGRAVING_TEXT_H
#define MU_ENGRAVING_TEXT_H

#include "textbase.h"

namespace mu::engraving {
//---------------------------------------------------------
//   Text
//---------------------------------------------------------

class Text final : public TextBase
{
    OBJECT_ALLOCATOR(engraving, Text)
    DECLARE_CLASSOF(ElementType::TEXT)

public:

    Text* clone() const override { return new Text(*this); }

    PropertyValue propertyDefault(Pid id) const override;
    PropertyValue getProperty(Pid id) const override;

    static String readXmlText(XmlReader& r, Score* score);

    void setLayoutToParentWidth(bool v) { m_layoutToParentWidth = v; }

    bool hasVoiceAssignmentProperties() const override;
    VoiceAssignment voiceAssignment() const;

private:
    friend class Factory;
    Text(EngravingItem* parent, TextStyleType tid = TextStyleType::DEFAULT);
};
} // namespace mu::engraving

#endif
