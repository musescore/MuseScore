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
#include "navigationcontrol.h"
#include "navigationpanel.h"

#include "log.h"

using namespace mu::ui;
using namespace mu::accessibility;

NavigationControl::NavigationControl(QObject* parent)
    : AbstractNavigation(parent)
{
}

NavigationControl::~NavigationControl()
{
    if (m_panel) {
        m_panel->removeControl(this);
    }
    accessibilityController()->destroyed(this);
}

void NavigationControl::componentComplete()
{
    AbstractNavigation::componentComplete();
    accessibilityController()->created(m_panel, this);
}

QString NavigationControl::name() const
{
    return AbstractNavigation::name();
}

const INavigation::Index& NavigationControl::index() const
{
    return AbstractNavigation::index();
}

mu::async::Channel<INavigation::Index> NavigationControl::indexChanged() const
{
    return AbstractNavigation::indexChanged();
}

bool NavigationControl::enabled() const
{
    return AbstractNavigation::enabled();
}

mu::async::Channel<bool> NavigationControl::enabledChanged() const
{
    return AbstractNavigation::enabledChanged();
}

bool NavigationControl::active() const
{
    return AbstractNavigation::active();
}

void NavigationControl::setActive(bool arg)
{
    AbstractNavigation::setActive(arg);
    if (arg) {
        accessibilityController()->focused(this);
    }
}

mu::async::Channel<bool> NavigationControl::activeChanged() const
{
    return AbstractNavigation::activeChanged();
}

void NavigationControl::onEvent(EventPtr e)
{
    AbstractNavigation::onEvent(e);
}

void NavigationControl::trigger()
{
    emit triggered();
}

mu::async::Channel<INavigationControl*> NavigationControl::activeRequested() const
{
    return m_forceActiveRequested;
}

void NavigationControl::requestActive()
{
    m_forceActiveRequested.send(this);
}

void NavigationControl::setPanel(NavigationPanel* panel)
{
    TRACEFUNC;
    if (m_panel == panel) {
        return;
    }

    if (m_panel) {
        m_panel->removeControl(this);
        m_panel->disconnect(this);
    }

    m_panel = panel;

    if (m_panel) {
        m_panel->addControl(this);
        connect(m_panel, &NavigationPanel::destroyed, this, &NavigationControl::onPanelDestroyed);
    }

    emit panelChanged(m_panel);
}

void NavigationControl::onPanelDestroyed()
{
    m_panel = nullptr;
}

NavigationPanel* NavigationControl::panel_property() const
{
    return m_panel;
}

INavigationPanel* NavigationControl::panel() const
{
    return m_panel;
}

IAccessibility::Role NavigationControl::accessibleRole() const
{
    return IAccessibility::Role::Button;
}

QString NavigationControl::accessibleName() const
{
    return name();
}

bool NavigationControl::accessibleState(State st) const
{
    switch (st) {
    case State::Undefined: return false;
    case State::Disabled: return !enabled();
    case State::Active: return active();
    case State::Focused: return active();
    default: {
        LOGW() << "not handled state: " << static_cast<int>(st);
    }
    }

    return false;
}
