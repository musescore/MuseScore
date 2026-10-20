/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "engravingitem.h"

#include "draw/types/font.h"
#include "global/types/translatablestring.h"

#include "../types/types.h"

namespace mu::engraving {
//! Dummy element, used for drag&drop
class ActionIcon final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, ActionIcon)
    DECLARE_CLASSOF(ElementType::ACTION_ICON)

public:
    ActionIcon(DummyParentOr<EngravingItem> score);
    ~ActionIcon() override = default;

    ActionIcon* clone() const override;

    void setActionType(ActionIconType val);
    ActionIconType actionType() const;

    const muse::TranslatableString& title() const;
    char16_t icon() const;

    const muse::draw::Font& iconFont() const { return m_iconFont; }

    PropertyValue getProperty(Pid) const override;
    bool setProperty(Pid, const PropertyValue&) override;

private:
    ActionIconType m_actionType = ActionIconType::UNDEFINED;
    muse::TranslatableString m_title;
    char16_t m_icon = 0;
    muse::draw::Font m_iconFont;
};
}
