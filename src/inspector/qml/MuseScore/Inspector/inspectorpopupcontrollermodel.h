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

#include <QObject>
#include <qqmlintegration.h>
#include <QQmlParserStatus>

#include "modularity/ioc.h"
#include "internal/iinspectorpopupcontroller.h"

#include "async/asyncable.h"

Q_MOC_INCLUDE(< QQuickItem >)
Q_MOC_INCLUDE("uicomponents/qml/Muse/UiComponents/popupview.h")
Q_MOC_INCLUDE("uicomponents/qml/Muse/UiComponents/dropdownview.h")
Q_MOC_INCLUDE("uicomponents/qml/Muse/UiComponents/menuview.h")

class QQuickItem;

namespace mu::inspector {
class InspectorPopupControllerModel : public QObject, public QQmlParserStatus, public muse::Injectable, public muse::async::Asyncable
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    QML_ELEMENT;

    Q_PROPERTY(bool isAnyPopupOpen READ isAnyPopupOpen NOTIFY isAnyPopupOpenChanged)

    muse::Inject<IInspectorPopupController> popupController  = { this };

public:
    explicit InspectorPopupControllerModel(QObject* parent = nullptr);

    bool isAnyPopupOpen() const;

    Q_INVOKABLE void setNotationView(const QQuickItem* view);
    Q_INVOKABLE void setPopup(muse::uicomponents::PopupView* popup, QQuickItem* control = nullptr);
    Q_INVOKABLE void setDropdown(muse::uicomponents::DropdownView* dropdown);
    Q_INVOKABLE void setMenu(muse::uicomponents::MenuView* menu);
    Q_INVOKABLE void repositionPopupIfNeed();

signals:
    void isAnyPopupOpenChanged();

private:
    void classBegin() override;
    void componentComplete() override {}
};
}
