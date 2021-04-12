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
#include "keynavigationcontrol.h"

using namespace mu::ui;

KeyNavigationControl::KeyNavigationControl(QObject* parent)
    : AbstractKeyNavigation(parent)
{
}

KeyNavigationControl::~KeyNavigationControl()
{
    if (m_subsection) {
        m_subsection->removeControl(this);
    }
}

QString KeyNavigationControl::name() const
{
    return AbstractKeyNavigation::name();
}

const IKeyNavigation::Index& KeyNavigationControl::index() const
{
    return AbstractKeyNavigation::index();
}

bool KeyNavigationControl::enabled() const
{
    return AbstractKeyNavigation::enabled();
}

bool KeyNavigationControl::active() const
{
    return AbstractKeyNavigation::active();
}

void KeyNavigationControl::setActive(bool arg)
{
    AbstractKeyNavigation::setActive(arg);
}

mu::async::Channel<bool> KeyNavigationControl::activeChanged() const
{
    return AbstractKeyNavigation::activeChanged();
}

void KeyNavigationControl::trigger()
{
    emit triggered();
}

mu::async::Channel<IKeyNavigationControl*> KeyNavigationControl::forceActiveRequested() const
{
    return m_forceActiveRequested;
}

void KeyNavigationControl::forceActive()
{
    m_forceActiveRequested.send(this);
}

void KeyNavigationControl::setSubSection(KeyNavigationSubSection* subsection)
{
    if (m_subsection == subsection) {
        return;
    }

    if (m_subsection) {
        m_subsection->removeControl(this);
        m_subsection->disconnect(this);
    }

    m_subsection = subsection;

    if (m_subsection) {
        connect(m_subsection, &KeyNavigationSubSection::destroyed, this, &KeyNavigationControl::onSubSectionDestroyed);
    }

    emit subsectionChanged(m_subsection);
}

void KeyNavigationControl::onSubSectionDestroyed()
{
    m_subsection = nullptr;
}

KeyNavigationSubSection* KeyNavigationControl::subsection() const
{
    return m_subsection;
}

void KeyNavigationControl::componentComplete()
{
    if (m_subsection) {
        m_subsection->addControl(this);
    }
}
