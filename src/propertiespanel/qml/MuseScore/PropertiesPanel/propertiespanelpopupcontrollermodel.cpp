/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include "propertiespanelpopupcontrollermodel.h"

#include "uicomponents/qml/Muse/UiComponents/popupview.h"

using namespace mu::propertiespanel;
using namespace muse::uicomponents;

PropertiesPanelPopupControllerModel::PropertiesPanelPopupControllerModel(QObject* parent)
    : QObject(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
}

void PropertiesPanelPopupControllerModel::classBegin()
{
    init();
}

void PropertiesPanelPopupControllerModel::init()
{
    popupController()->popupChanged().onNotify(this, [this]() {
        emit isAnyPopupOpenChanged();
    });

    popupController()->dropdownChanged().onNotify(this, [this]() {
        emit isAnyPopupOpenChanged();
    });

    popupController()->menuChanged().onNotify(this, [this]() {
        emit isAnyPopupOpenChanged();
    });
}

bool PropertiesPanelPopupControllerModel::isAnyPopupOpen() const
{
    auto controller = popupController();
    return controller->popup() || controller->dropdown() || controller->menu();
}

void PropertiesPanelPopupControllerModel::setNotationView(const QQuickItem* view)
{
    popupController()->setNotationView(view);
}

void PropertiesPanelPopupControllerModel::setPopup(PopupView* popup, QQuickItem* control)
{
    popupController()->setPopup(popup, control);
}

void PropertiesPanelPopupControllerModel::setDropdown(DropdownView* dropdown)
{
    popupController()->setDropdown(dropdown);
}

void PropertiesPanelPopupControllerModel::setMenu(MenuView* menu)
{
    popupController()->setMenu(menu);
}

void PropertiesPanelPopupControllerModel::repositionPopupIfNeed()
{
    if (popupController()->popup()) {
        popupController()->popup()->repositionWindowIfNeed();
    }
}
