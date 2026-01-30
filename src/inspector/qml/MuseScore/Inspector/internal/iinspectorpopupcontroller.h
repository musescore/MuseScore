/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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

#include "modularity/ioc.h"
#include "async/notification.h"

class QQuickItem;

namespace muse::uicomponents {
class PopupView;
class DropdownView;
class MenuView;
}

namespace mu::inspector {
class IInspectorPopupController : MODULE_CONTEXT_INTERFACE
{
    INTERFACE_ID(IInspectorPopupController)

public:
    virtual void setNotationView(const QQuickItem* view) = 0;

    virtual muse::uicomponents::PopupView* popup() const = 0;
    virtual void setPopup(muse::uicomponents::PopupView* popup, QQuickItem* control = nullptr) = 0;
    virtual muse::async::Notification popupChanged() const = 0;

    virtual muse::uicomponents::DropdownView* dropdown() const = 0;
    virtual void setDropdown(muse::uicomponents::DropdownView* dropdown) = 0;
    virtual muse::async::Notification dropdownChanged() const = 0;

    virtual muse::uicomponents::MenuView* menu() const = 0;
    virtual void setMenu(muse::uicomponents::MenuView* menu) = 0;
    virtual muse::async::Notification menuChanged() const = 0;
};
}
