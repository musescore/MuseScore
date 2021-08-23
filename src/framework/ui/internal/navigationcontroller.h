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
#ifndef MU_UI_NAVIGATIONCONTROLLER_H
#define MU_UI_NAVIGATIONCONTROLLER_H

#include <QObject>
#include <QList>

#include "../inavigationcontroller.h"
#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"
#include "actions/actionable.h"
#include "async/asyncable.h"
#include "global/iinteractive.h"

namespace mu::ui {
class NavigationController : public QObject, public INavigationController, public actions::Actionable, public async::Asyncable
{
    INJECT(ui, actions::IActionsDispatcher, dispatcher)
    INJECT(ui, framework::IInteractive, interactive)

public:
    NavigationController() = default;

    enum MoveDirection {
        First = 0,
        Last,
        Right,
        Left,
        Up,
        Down
    };

    void reg(INavigationSection* section) override;
    void unreg(INavigationSection* section) override;

    const std::set<INavigationSection*>& sections() const override;

    bool requestActivateByName(const std::string& section, const std::string& panel, const std::string& control) override;

    INavigationSection* activeSection() const override;
    INavigationPanel* activePanel() const override;
    INavigationControl* activeControl() const override;

    async::Notification navigationChanged() const override;

    void init();

private:

    bool eventFilter(QObject* watched, QEvent* event) override;

    void goToNextSection();
    void goToPrevSection(bool isActivateLastPanel = false);
    void goToNextPanel();
    void goToPrevPanel();

    void goToFirstControl();
    void goToLastControl();
    void goToNextRowControl();
    void goToPrevRowControl();

    void goToControl(MoveDirection direction, INavigationPanel* activePanel = nullptr);

    void onLeft();
    void onRight();
    void onUp();
    void onDown();
    void onEscape();

    void doTriggerControl();
    void onActiveRequested(INavigationSection* sect, INavigationPanel* panel, INavigationControl* ctrl);

    void doActivateSection(INavigationSection* sect, bool isActivateLastPanel = false);
    void doDeactivateSection(INavigationSection* sect);
    void doActivatePanel(INavigationPanel* panel);
    void doDeactivatePanel(INavigationPanel* panel);
    void doActivateControl(INavigationControl* ctrl);
    void doDeactivateControl(INavigationControl* ctrl);

    void doActivateFirst();
    void doActivateLast();

    void resetActive();

    std::set<INavigationSection*> m_sections;
    async::Notification m_navigationChanged;
};
}

#endif // MU_UI_NAVIGATIONCONTROLLER_H
