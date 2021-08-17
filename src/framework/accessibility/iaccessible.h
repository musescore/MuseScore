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
#ifndef MU_ACCESSIBILITY_IACCESSIBLE_H
#define MU_ACCESSIBILITY_IACCESSIBLE_H

#include <utility>
#include <QString>
#include <QRect>
#include "async/channel.h"

class QWindow;

namespace mu::accessibility {
class IAccessible
{
public:

    virtual ~IAccessible() = default;

    //! NOTE Please sync with ui::MUAccessible::Role (src/framework/ui/view/qmlaccessible.h)
    enum Role {
        NoRole = 0,
        Application,
        Dialog,
        Panel,
        StaticText,
        EditableText,
        Button,
        CheckBox,
        RadioButton,
        ComboBox,
        ListItem,

        // Custom roles
        Information, // just text

        // Score roles
        ElementOnScore
    };

    enum class State {
        Undefined = 0,
        Enabled,
        Active,
        Focused,
        Selected
    };

    enum class Property {
        Undefined = 0,
        Parent,
        Name,
        Description
    };

    virtual const IAccessible* accessibleParent() const = 0;
    virtual size_t accessibleChildCount() const = 0;
    virtual const IAccessible* accessibleChild(size_t i) const = 0;

    virtual IAccessible::Role accessibleRole() const = 0;
    virtual QString accessibleName() const = 0;
    virtual QString accessibleDescription() const = 0;
    virtual bool accessibleState(State st) const = 0;
    virtual QRect accessibleRect() const = 0;

    virtual async::Channel<IAccessible::Property> accessiblePropertyChanged() const = 0;
    virtual async::Channel<IAccessible::State, bool> accessibleStateChanged() const = 0;
};
}

#endif // MU_ACCESSIBILITY_IACCESSIBLE_H
