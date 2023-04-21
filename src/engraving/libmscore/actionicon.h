/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#ifndef MU_LIBMSCORE_ACTIONICON_H
#define MU_LIBMSCORE_ACTIONICON_H

#include "engravingitem.h"

namespace mu::engraving {
enum class ActionIconType {
    UNDEFINED = -1,

    ACCIACCATURA,
    APPOGGIATURA,
    GRACE4,
    GRACE16,
    GRACE32,
    GRACE8_AFTER,
    GRACE16_AFTER,
    GRACE32_AFTER,

    BEAM_AUTO,
    BEAM_NONE,
    BEAM_BREAK_LEFT,
    BEAM_BREAK_INNER_8TH,
    BEAM_BREAK_INNER_16TH,
    BEAM_JOIN,

    BEAM_FEATHERED_DECELERATE,
    BEAM_FEATHERED_ACCELERATE,

    VFRAME,
    HFRAME,
    TFRAME,
    FFRAME,
    MEASURE,

    PARENTHESES,
    BRACKETS,
};

//! Dummy element, used for drag&drop
class ActionIcon final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, ActionIcon)
    DECLARE_CLASSOF(ElementType::ACTION_ICON)

public:
    ActionIcon(EngravingItem* score);
    ~ActionIcon() override = default;

    ActionIcon* clone() const override;

    ActionIconType actionType() const;
    const std::string& actionCode() const;

    void setActionType(ActionIconType val);
    void setAction(const std::string& actionCode, char16_t icon);

    double fontSize() const;
    void setFontSize(double size);

    void draw(mu::draw::Painter*) const override;
    void layout() override;

    PropertyValue getProperty(Pid) const override;
    bool setProperty(Pid, const PropertyValue&) override;

    static constexpr double DEFAULT_FONT_SIZE = 16.0;

private:
    ActionIconType m_actionType { ActionIconType::UNDEFINED };
    std::string m_actionCode;
    char16_t m_icon = 0;
    mu::draw::Font m_iconFont;
};
}

#endif // MU_LIBMSCORE_ACTIONICON_H
