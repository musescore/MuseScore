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

#include "iinspectorpopupcontroller.h"
#include "interactive/iinteractive.h"
#include "ui/imainwindow.h"

namespace muse::uicomponents {
class PopupView;
}

namespace mu::inspector {
class InspectorPopupController : public QObject, public IInspectorPopupController, public muse::Contextable
{
    Q_OBJECT

    muse::ContextInject<muse::ui::IMainWindow> mainWindow = { this };
    muse::ContextInject<muse::IInteractive> interactive = { this };

public:
    explicit InspectorPopupController(const muse::modularity::ContextPtr& iocCtx);
    ~InspectorPopupController() override;

    void init();

    void setNotationView(const QQuickItem* view) override;

    muse::uicomponents::PopupView* popup() const override;
    void setPopup(muse::uicomponents::PopupView* popup, QQuickItem* control = nullptr) override;
    muse::async::Notification popupChanged() const override;

    muse::uicomponents::DropdownView* dropdown() const override;
    void setDropdown(muse::uicomponents::DropdownView* dropdown) override;
    muse::async::Notification dropdownChanged() const override;

    muse::uicomponents::MenuView* menu() const override;
    void setMenu(muse::uicomponents::MenuView* menu) override;
    muse::async::Notification menuChanged() const override;

private slots:
    void closePopup();
    void doClosePopup();

private:
    bool eventFilter(QObject* watched, QEvent* event) override;

    void closePopupIfNeed(const QPointF& mouseGlobalPos);

    const QQuickItem* m_notationView = nullptr;

    muse::uicomponents::PopupView* m_popup = nullptr;
    QQuickItem* m_visualControl = nullptr;
    muse::async::Notification m_popupChanged;

    muse::uicomponents::DropdownView* m_dropdown = nullptr;
    muse::async::Notification m_dropdownChanged;

    muse::uicomponents::MenuView* m_menu = nullptr;
    muse::async::Notification m_menuChanged;
};
}
