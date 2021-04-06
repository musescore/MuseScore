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
#include "keynavigationcontroller.h"

#include <algorithm>

#include "log.h"

using namespace mu::ui;

void KeyNavigationController::init()
{
    dispatcher()->reg(this, "nav-next-section", this, &KeyNavigationController::nextSection);
    dispatcher()->reg(this, "nav-prev-section", this, &KeyNavigationController::prevSection);
}

static void print(const std::string& title, const std::vector<IKeyNavigationSection*>& chain)
{
    LOGI() << title;
    for (size_t i = 0; i < chain.size(); ++i) {
        LOGI() << "    " << i << chain.at(i)->name() << " order: " << chain.at(i)->order();
    }
}

void KeyNavigationController::reg(IKeyNavigationSection* s)
{
    //! TODO add check on valid state

    m_chain.push_back(s);
    print("after push_back", m_chain);
    std::sort(m_chain.begin(), m_chain.end(), [this](const IKeyNavigationSection* f, const IKeyNavigationSection* s) {
        return f->order() < s->order();
    });

    print("after sort", m_chain);
}

void KeyNavigationController::unreg(IKeyNavigationSection* s)
{
    m_chain.erase(std::remove(m_chain.begin(), m_chain.end(), s), m_chain.end());
}

IKeyNavigationSection* KeyNavigationController::firstSection() const
{
    if (!m_chain.empty()) {
        return findFirstEnabledSection(m_chain.cbegin(), m_chain.cend());
    }
    return nullptr;
}

IKeyNavigationSection* KeyNavigationController::lastSection() const
{
    if (!m_chain.empty()) {
        auto last = --m_chain.cend();
        return findLastEnabledSection(last, m_chain.cbegin());
    }
    return nullptr;
}

IKeyNavigationSection* KeyNavigationController::nextSection(const IKeyNavigationSection* s) const
{
    auto it = std::find(m_chain.begin(), m_chain.end(), s);
    IF_ASSERT_FAILED(it != m_chain.end()) {
        return nullptr;
    }
    ++it;
    if (it == m_chain.end()) {
        return nullptr;
    }
    return findFirstEnabledSection(it, m_chain.end());
}

IKeyNavigationSection* KeyNavigationController::prevSection(const IKeyNavigationSection* s) const
{
    auto it = std::find(m_chain.begin(), m_chain.end(), s);
    IF_ASSERT_FAILED(it != m_chain.end()) {
        return nullptr;
    }
    if (it == m_chain.begin()) {
        return nullptr;
    }

    --it;

    return findLastEnabledSection(it, m_chain.cbegin());
}

IKeyNavigationSection* KeyNavigationController::findFirstEnabledSection(Chain::const_iterator it, Chain::const_iterator end) const
{
    for (; it != end; ++it) {
        IKeyNavigationSection* s = *it;
        if (s->enabled()) {
            return s;
        }
    }
    return nullptr;
}

IKeyNavigationSection* KeyNavigationController::findLastEnabledSection(Chain::const_iterator it, Chain::const_iterator begin) const
{
    for (; it != begin; --it) {
        IKeyNavigationSection* s = *it;
        if (s->enabled()) {
            return s;
        }
    }

    IKeyNavigationSection* s = *begin;
    if (s->enabled()) {
        return s;
    }

    return nullptr;
}

IKeyNavigationSection* KeyNavigationController::activeSection() const
{
    auto it = std::find_if(m_chain.cbegin(), m_chain.cend(), [this](const IKeyNavigationSection* s) {
        return s->active();
    });

    if (it != m_chain.cend()) {
        return *it;
    }

    return nullptr;
}

void KeyNavigationController::nextSection()
{
    LOGI() << "====";
    if (m_chain.empty()) {
        return;
    }

    IKeyNavigationSection* activeSec = activeSection();
    if (!activeSec) { // no any active
        firstSection()->setActive(true);
        return;
    }

    activeSec->setActive(false);

    IKeyNavigationSection* nextSec = nextSection(activeSec);
    if (!nextSec) { // active is last
        firstSection()->setActive(true);
        return;
    }

    nextSec->setActive(true);
}

void KeyNavigationController::prevSection()
{
    LOGI() << "====";
    if (m_chain.empty()) {
        return;
    }

    IKeyNavigationSection* activeSec = activeSection();
    if (!activeSec) { // no any active
        lastSection()->setActive(true);
        return;
    }

    activeSec->setActive(false);

    IKeyNavigationSection* prevSec = prevSection(activeSec);
    if (!prevSec) { // active is first
        lastSection()->setActive(true);
        return;
    }

    prevSec->setActive(true);
}
