//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "keynavigationsubsection.h"

#include <algorithm>

#include "log.h"

#include "keynavigationcontrol.h"

using namespace mu::ui;

KeyNavigationSubSection::KeyNavigationSubSection(QObject* parent)
    : AbstractKeyNavigation(parent)
{
}

KeyNavigationSubSection::~KeyNavigationSubSection()
{
    if (m_section) {
        m_section->removeSubSection(this);
    }
}

QString KeyNavigationSubSection::name() const
{
    return AbstractKeyNavigation::name();
}

int KeyNavigationSubSection::order() const
{
    return AbstractKeyNavigation::order();
}

bool KeyNavigationSubSection::enabled() const
{
    return AbstractKeyNavigation::enabled();
}

bool KeyNavigationSubSection::active() const
{
    return AbstractKeyNavigation::active();
}

void KeyNavigationSubSection::setActive(bool arg)
{
    AbstractKeyNavigation::setActive(arg);
}

mu::async::Channel<bool> KeyNavigationSubSection::activeChanged() const
{
    return AbstractKeyNavigation::activeChanged();
}

const QSet<IKeyNavigationControl*>& KeyNavigationSubSection::controls() const
{
    return m_controls;
}

mu::async::Channel<SubSectionControl> KeyNavigationSubSection::forceActiveRequested() const
{
    return m_forceActiveRequested;
}

KeyNavigationSection* KeyNavigationSubSection::section() const
{
    return m_section;
}

void KeyNavigationSubSection::setSection(KeyNavigationSection* section)
{
    if (m_section == section) {
        return;
    }

    if (m_section) {
        m_section->removeSubSection(this);
        m_section->disconnect(this);
    }

    m_section = section;

    if (m_section) {
        connect(m_section, &KeyNavigationSection::destroyed, this, &KeyNavigationSubSection::onSectionDestroyed);
    }
}

void KeyNavigationSubSection::onSectionDestroyed()
{
    m_section = nullptr;
}

void KeyNavigationSubSection::componentComplete()
{
    LOGD() << "Completed: " << m_name << ", order: " << m_order;

    IF_ASSERT_FAILED(!m_name.isEmpty()) {
        return;
    }

    IF_ASSERT_FAILED(m_order > -1) {
        return;
    }

    if (m_section) {
        m_section->addSubSection(this);
    }
}

void KeyNavigationSubSection::addControl(KeyNavigationControl* control)
{
    IF_ASSERT_FAILED(control) {
        return;
    }

    m_controls.insert(control);

    control->forceActiveRequested().onReceive(this, [this](IKeyNavigationControl* c) {
        m_forceActiveRequested.send(std::make_tuple(this, c));
    });
}

void KeyNavigationSubSection::removeControl(KeyNavigationControl* control)
{
    IF_ASSERT_FAILED(control) {
        return;
    }

    m_controls.remove(control);
    control->forceActiveRequested().resetOnReceive(this);
}
