/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "textbase.h"

namespace mu::engraving {
class PlayCountText final : public TextBase
{
    OBJECT_ALLOCATOR(engraving, PlayCountText)
    DECLARE_CLASSOF(ElementType::PLAY_COUNT_TEXT)

public:
    PlayCountText* clone() const override { return new PlayCountText(*this); }

    Segment* segment() const { return toSegment(parent()); }
    BarLine* barline() const;

    bool isEditable() const override { return true; }
    void endEdit(EditData&) override;
    bool allowTimeAnchor() const override { return false; }
    RectF drag(EditData& ed) override;

    String playCountCustomText() const { return m_playCountCustomText; }
    void setPlayCountCustomText(const String& v) { m_playCountCustomText = v; }

    void setPlayCountTextSetting(const AutoCustomHide& v) { m_playCountTextSetting = v; }
    AutoCustomHide playCountTextSetting() const { return m_playCountTextSetting; }

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid propertyId) const override;

private:
    friend class Factory;
    PlayCountText(Segment* parent, TextStyleType tid = TextStyleType::REPEAT_PLAY_COUNT);

    AutoCustomHide m_playCountTextSetting = AutoCustomHide::AUTO;
    String m_playCountCustomText = u"";
};
} // namespace mu::engraving
