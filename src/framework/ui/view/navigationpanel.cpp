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
#include "navigationpanel.h"

#include <algorithm>

#include "navigationsection.h"
#include "log.h"

using namespace mu::ui;
using namespace mu::accessibility;

NavigationPanel::NavigationPanel(QObject* parent)
    : AbstractNavigation(parent)
{
}

NavigationPanel::~NavigationPanel()
{
    accessibilityController()->unreg(this);
    if (m_section) {
        m_section->removePanel(this);
    }
}

QString NavigationPanel::name() const
{
    return AbstractNavigation::name();
}

const INavigation::Index& NavigationPanel::index() const
{
    return AbstractNavigation::index();
}

mu::async::Channel<INavigation::Index> NavigationPanel::indexChanged() const
{
    return AbstractNavigation::indexChanged();
}

bool NavigationPanel::enabled() const
{
    if (!AbstractNavigation::enabled()) {
        return false;
    }

    bool enbl = false;
    for (INavigationControl* c : m_controls) {
        if (c->enabled()) {
            enbl = true;
            break;
        }
    }
    return enbl;
}

mu::async::Channel<bool> NavigationPanel::enabledChanged() const
{
    return AbstractNavigation::enabledChanged();
}

bool NavigationPanel::active() const
{
    return AbstractNavigation::active();
}

void NavigationPanel::setActive(bool arg)
{
    AbstractNavigation::setActive(arg);
    accessibilityController()->actived(this, arg);
}

mu::async::Channel<bool> NavigationPanel::activeChanged() const
{
    return AbstractNavigation::activeChanged();
}

void NavigationPanel::onEvent(EventPtr e)
{
    AbstractNavigation::onEvent(e);
}

void NavigationPanel::setDirection(QmlDirection direction)
{
    if (m_direction == direction) {
        return;
    }

    m_direction = direction;
    emit directionChanged(m_direction);
}

NavigationPanel::QmlDirection NavigationPanel::direction_property() const
{
    return m_direction;
}

INavigationPanel::Direction NavigationPanel::direction() const
{
    return static_cast<Direction>(m_direction);
}

const std::set<INavigationControl*>& NavigationPanel::controls() const
{
    return m_controls;
}

mu::async::Notification NavigationPanel::controlsListChanged() const
{
    return m_controlsListChanged;
}

mu::async::Channel<PanelControl> NavigationPanel::activeRequested() const
{
    return m_forceActiveRequested;
}

INavigationSection* NavigationPanel::section() const
{
    return m_section;
}

NavigationSection* NavigationPanel::section_property() const
{
    return m_section;
}

void NavigationPanel::setSection(INavigationSection* section)
{
    setSection_property(dynamic_cast<NavigationSection*>(section));
}

void NavigationPanel::setSection_property(NavigationSection* section)
{
    TRACEFUNC;
    if (m_section == section) {
        return;
    }

    if (m_section) {
        m_section->removePanel(this);
        m_section->disconnect(this);
    }

    m_section = section;

    if (m_section) {
        m_section->addPanel(this);
        connect(m_section, &NavigationSection::destroyed, this, &NavigationPanel::onSectionDestroyed);
    }

    emit sectionChanged(m_section);
}

void NavigationPanel::onSectionDestroyed()
{
    m_section = nullptr;
}

void NavigationPanel::componentComplete()
{
    accessibilityController()->reg(this);
}

void NavigationPanel::addControl(NavigationControl* control)
{
    TRACEFUNC;
    IF_ASSERT_FAILED(control) {
        return;
    }

    m_controls.insert(control);

    control->activeRequested().onReceive(this, [this](INavigationControl* c) {
        m_forceActiveRequested.send(std::make_tuple(this, c));
    });

    if (m_controlsListChanged.isConnected()) {
        m_controlsListChanged.notify();
    }

    //! TODO Maybe need sorting
    m_accessibleControls.push_back(control);
}

void NavigationPanel::removeControl(NavigationControl* control)
{
    TRACEFUNC;
    IF_ASSERT_FAILED(control) {
        return;
    }

    m_controls.erase(control);
    control->activeRequested().resetOnReceive(this);

    if (m_controlsListChanged.isConnected()) {
        m_controlsListChanged.notify();
    }

    m_accessibleControls.erase(std::remove(m_accessibleControls.begin(), m_accessibleControls.end(), control), m_accessibleControls.end());
}

const IAccessibility* NavigationPanel::accessibleParent() const
{
    return accessibilityController()->rootItem();
}

mu::async::Notification NavigationPanel::accessibleParentChanged() const
{
    static async::Notification notification;
    return notification;
}

size_t NavigationPanel::accessibleChildCount() const
{
    return m_accessibleControls.size();
}

const IAccessibility* NavigationPanel::accessibleChild(size_t i) const
{
    IF_ASSERT_FAILED(i < m_accessibleControls.size()) {
        return nullptr;
    }
    return m_accessibleControls.at(i);
}

IAccessibility::Role NavigationPanel::accessibleRole() const
{
    return IAccessibility::Role::Panel;
}

QString NavigationPanel::accessibleName() const
{
    QString dirc;
    switch (m_direction) {
    case Horizontal: dirc = "Horizontal";
        break;
    case Vertical: dirc = "Vertical";
        break;
    case Both: dirc = "Both";
        break;
    }

    return name() + " direction " + dirc;
}

bool NavigationPanel::accessibleState(State st) const
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

QRect NavigationPanel::accessibleRect() const
{
    return QRect();
}
