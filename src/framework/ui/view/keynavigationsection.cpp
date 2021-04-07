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

static void printSubList(const QList<IKeyNavigationSubSection*>& list)
{
    for (const IKeyNavigationSubSection* s : list) {
        LOGI() << s->name() << " " << s->order();
    }
}

void KeyNavigationSection::addSubSection(KeyNavigationSubSection* s)
{
    m_subsections.append(s);
    std::sort(m_subsections.begin(), m_subsections.end(), [](const IKeyNavigationSubSection* f, const IKeyNavigationSubSection* s) {
        return f->order() < s->order();
    });

    printSubList(m_subsections);
}

void KeyNavigationSection::removeSubSection(KeyNavigationSubSection* s)
{
    m_subsections.removeOne(s);
}

const QList<IKeyNavigationSubSection*>& KeyNavigationSection::subsections() const
{
    return m_subsections;
}
