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

#include "log.h"

using namespace mu::ui;

KeyNavigationSubSection::KeyNavigationSubSection(QObject* parent)
    : QObject(parent)
{
}

KeyNavigationSection* KeyNavigationSubSection::section() const
{
    return m_section;
}

void KeyNavigationSubSection::setSection(KeyNavigationSection* section)
{
    m_section = section;
    if (m_section) {
        m_section->addSubSection(this);
    }
}

void KeyNavigationSubSection::setName(QString name)
{
    m_name = name;
}

QString KeyNavigationSubSection::name() const
{
    return m_name;
}

void KeyNavigationSubSection::setOrder(int order)
{
    m_order = order;
}

int KeyNavigationSubSection::order() const
{
    return m_order;
}

void KeyNavigationSubSection::setEnabled(bool enabled)
{
    if (m_enabled == enabled) {
        return;
    }

    m_enabled = enabled;
    emit enabledChanged(m_enabled);
}

bool KeyNavigationSubSection::enabled() const
{
    return m_enabled;
}

void KeyNavigationSubSection::setActive(bool active)
{
    if (m_active == active) {
        return;
    }

    m_active = active;
    emit activeChanged(m_active);
}

bool KeyNavigationSubSection::active() const
{
    return m_active;
}

void KeyNavigationSubSection::classBegin()
{
}

void KeyNavigationSubSection::componentComplete()
{
    //! NOTE Reg after set properties.
    LOGD() << "Completed: " << m_name << ", order: " << m_order;

    IF_ASSERT_FAILED(!m_name.isEmpty()) {
        return;
    }

    IF_ASSERT_FAILED(m_order > -1) {
        return;
    }
}
