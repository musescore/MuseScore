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
#include "keynavigationsection.h"

#include <algorithm>

#include "log.h"

#include "keynavigationsubsection.h"

using namespace mu::ui;

KeyNavigationSection::KeyNavigationSection(QObject* parent)
    : AbstractKeyNavigation(parent)
{
}

KeyNavigationSection::~KeyNavigationSection()
{
    keyNavigationController()->unreg(this);
}

void KeyNavigationSection::componentComplete()
{
    //! NOTE Reg after set properties.
    LOGD() << "Completed: " << m_name << ", order: " << m_order;

    IF_ASSERT_FAILED(!m_name.isEmpty()) {
        return;
    }

    IF_ASSERT_FAILED(m_order > -1) {
        return;
    }

    keyNavigationController()->reg(this);
}

QString KeyNavigationSection::name() const
{
    return AbstractKeyNavigation::name();
}

int KeyNavigationSection::order() const
{
    return AbstractKeyNavigation::order();
}

bool KeyNavigationSection::enabled() const
{
    return AbstractKeyNavigation::enabled();
}

bool KeyNavigationSection::active() const
{
    return AbstractKeyNavigation::active();
}

void KeyNavigationSection::setActive(bool arg)
{
    AbstractKeyNavigation::setActive(arg);
}

mu::async::Channel<bool> KeyNavigationSection::activeChanged() const
{
    return AbstractKeyNavigation::activeChanged();
}

void KeyNavigationSection::addSubSection(KeyNavigationSubSection* sub)
{
    IF_ASSERT_FAILED(sub) {
        return;
    }

    m_subsections.insert(sub);

    sub->forceActiveRequested().onReceive(this, [this](const SubSectionControl& subcon) {
        m_forceActiveRequested.send(std::make_tuple(this, std::get<0>(subcon), std::get<1>(subcon)));
    });
}

mu::async::Channel<SectionSubSectionControl> KeyNavigationSection::forceActiveRequested() const
{
    return m_forceActiveRequested;
}

void KeyNavigationSection::removeSubSection(KeyNavigationSubSection* sub)
{
    IF_ASSERT_FAILED(sub) {
        return;
    }

    m_subsections.remove(sub);
    sub->forceActiveRequested().resetOnReceive(this);
}

const QSet<IKeyNavigationSubSection*>& KeyNavigationSection::subsections() const
{
    return m_subsections;
}
