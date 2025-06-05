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
#include "navigationsection.h"

#include <QQuickWindow>
#include <QApplication>

#include "log.h"

using namespace muse;
using namespace muse::ui;

NavigationSection::NavigationSection(QObject* parent)
    : AbstractNavigation(parent)
{
}

NavigationSection::~NavigationSection()
{
    navigationController()->unreg(this);
}

void NavigationSection::componentComplete()
{
    //! NOTE Reg after set properties.
    IF_ASSERT_FAILED(!m_name.isEmpty()) {
        return;
    }

    IF_ASSERT_FAILED(order() > -1) {
        return;
    }

    navigationController()->reg(this);
}

QString NavigationSection::name() const
{
    return AbstractNavigation::name();
}

const INavigation::Index& NavigationSection::index() const
{
    return AbstractNavigation::index();
}

void NavigationSection::setIndex(const Index& index)
{
    AbstractNavigation::setIndex(index);
}

async::Channel<INavigation::Index> NavigationSection::indexChanged() const
{
    return AbstractNavigation::indexChanged();
}

bool NavigationSection::enabled() const
{
    if (!AbstractNavigation::enabled()) {
        return false;
    }

    bool enbl = false;
    for (INavigationPanel* p : m_panels) {
        if (p->enabled()) {
            enbl = true;
            break;
        }
    }

    return enbl;
}

async::Channel<bool> NavigationSection::enabledChanged() const
{
    return AbstractNavigation::enabledChanged();
}

bool NavigationSection::active() const
{
    return AbstractNavigation::active();
}

void NavigationSection::setActive(bool arg)
{
    AbstractNavigation::setActive(arg);
}

async::Channel<bool> NavigationSection::activeChanged() const
{
    return AbstractNavigation::activeChanged();
}

void NavigationSection::onEvent(EventPtr e)
{
    AbstractNavigation::onEvent(e);
}

QWindow* NavigationSection::window() const
{
    return AbstractNavigation::window();
}

QQuickItem* muse::ui::NavigationSection::visualItem() const
{
    return AbstractNavigation::visualItem();
}

void NavigationSection::addPanel(NavigationPanel* panel)
{
    TRACEFUNC;
    IF_ASSERT_FAILED(panel) {
        return;
    }

    m_panels.insert(panel);

    if (m_panelsListChanged.isConnected()) {
        m_panelsListChanged.notify();
    }
}

void NavigationSection::removePanel(NavigationPanel* panel)
{
    TRACEFUNC;
    IF_ASSERT_FAILED(panel) {
        return;
    }

    m_panels.erase(panel);

    if (m_panelsListChanged.isConnected()) {
        m_panelsListChanged.notify();
    }
}

void NavigationSection::setOnActiveRequested(const OnActiveRequested& func)
{
    m_onActiveRequested = func;
}

void NavigationSection::requestActive(INavigationPanel* panel, INavigationControl* control, bool enableHighlight,
                                      INavigation::ActivationType activationType)
{
    if (m_onActiveRequested) {
        m_onActiveRequested(this, panel, control, enableHighlight, activationType);
    }
}

INavigationSection::Type NavigationSection::type() const
{
    return static_cast<INavigationSection::Type>(m_type);
}

NavigationSection::QmlType NavigationSection::type_property() const
{
    return m_type;
}

void NavigationSection::setType(QmlType type)
{
    if (m_type == type) {
        return;
    }

    m_type = type;
    emit typeChanged(m_type);
}

const std::set<INavigationPanel*>& NavigationSection::panels() const
{
    return m_panels;
}

async::Notification NavigationSection::panelsListChanged() const
{
    return m_panelsListChanged;
}
