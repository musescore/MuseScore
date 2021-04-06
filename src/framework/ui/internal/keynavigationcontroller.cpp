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

// algorithms
template<class T>
static T* findFirstEnabled(typename QList<T*>::const_iterator it, typename QList<T*>::const_iterator end)
{
    for (; it != end; ++it) {
        T* s = *it;
        if (s->enabled()) {
            return s;
        }
    }
    return nullptr;
}

template<class T>
static T* findLastEnabled(typename QList<T*>::const_iterator it, typename QList<T*>::const_iterator begin)
{
    for (; it != begin; --it) {
        T* s = *it;
        if (s->enabled()) {
            return s;
        }
    }

    T* s = *begin;
    if (s->enabled()) {
        return s;
    }

    return nullptr;
}

template<class T>
static T* firstEnabled(const QList<T*>& list)
{
    if (list.empty()) {
        return nullptr;
    }
    return findFirstEnabled<T>(list.cbegin(), list.cend());
}

template<class T>
static T* lastEnabled(const QList<T*>& list)
{
    if (list.empty()) {
        return nullptr;
    }
    auto last = --list.cend();
    return findLastEnabled<T>(last, list.cbegin());
}

template<class T>
static T* nextEnabled(const QList<T*>& list, const T* s)
{
    auto it = std::find(list.begin(), list.end(), s);
    IF_ASSERT_FAILED(it != list.end()) {
        return nullptr;
    }

    ++it;

    if (it == list.end()) {
        return nullptr;
    }

    return findFirstEnabled<T>(it, list.end());
}

template<class T>
static T* prevEnabled(const QList<T*>& list, const T* s)
{
    auto it = std::find(list.begin(), list.end(), s);
    IF_ASSERT_FAILED(it != list.end()) {
        return nullptr;
    }
    if (it == list.begin()) {
        return nullptr;
    }

    --it;

    return findLastEnabled<T>(it, list.cbegin());
}

template<class T>
static T* findActive(const QList<T*>& list)
{
    auto it = std::find_if(list.cbegin(), list.cend(), [](const T* s) {
        return s->active();
    });

    if (it != list.cend()) {
        return *it;
    }

    return nullptr;
}

void KeyNavigationController::init()
{
    dispatcher()->reg(this, "nav-next-section", this, &KeyNavigationController::nextSection);
    dispatcher()->reg(this, "nav-prev-section", this, &KeyNavigationController::prevSection);
    dispatcher()->reg(this, "nav-next-subsection", this, &KeyNavigationController::nextSubSection);
    dispatcher()->reg(this, "nav-prev-subsection", this, &KeyNavigationController::prevSubSection);
}

void KeyNavigationController::reg(IKeyNavigationSection* s)
{
    //! TODO add check on valid state

    m_sections.push_back(s);
    std::sort(m_sections.begin(), m_sections.end(), [this](const IKeyNavigationSection* f, const IKeyNavigationSection* s) {
        return f->order() < s->order();
    });
}

void KeyNavigationController::unreg(IKeyNavigationSection* s)
{
    m_sections.erase(std::remove(m_sections.begin(), m_sections.end(), s), m_sections.end());
}

void KeyNavigationController::activateSection(IKeyNavigationSection* s)
{
    IF_ASSERT_FAILED(s) {
        return;
    }

    for (IKeyNavigationSubSection* sub : s->subsections()) {
        sub->setActive(false);
    }

    s->setActive(true);

    IKeyNavigationSubSection* firstSub = firstEnabled(s->subsections());
    if (firstSub) {
        firstSub->setActive(true);
    }
}

void KeyNavigationController::deactivateSection(IKeyNavigationSection* s)
{
    IF_ASSERT_FAILED(s) {
        return;
    }

    for (IKeyNavigationSubSection* sub : s->subsections()) {
        sub->setActive(false);
    }

    s->setActive(false);
}

void KeyNavigationController::nextSection()
{
    LOGI() << "====";
    if (m_sections.empty()) {
        return;
    }

    IKeyNavigationSection* activeSec = findActive(m_sections);
    if (!activeSec) { // no any active
        IKeyNavigationSection* first = firstEnabled(m_sections);
        if (first) {
            activateSection(first);
        }
        return;
    }

    deactivateSection(activeSec);

    IKeyNavigationSection* nextSec = nextEnabled(m_sections, activeSec);
    if (!nextSec) { // active is last
        IKeyNavigationSection* first = firstEnabled(m_sections);
        if (first) {
            activateSection(first);
        }
        return;
    }

    activateSection(nextSec);
}

void KeyNavigationController::prevSection()
{
    LOGI() << "====";
    if (m_sections.empty()) {
        return;
    }

    IKeyNavigationSection* activeSec = findActive(m_sections);
    if (!activeSec) { // no any active
        IKeyNavigationSection* last = lastEnabled(m_sections);
        if (last) {
            activateSection(last);
        }
        return;
    }

    deactivateSection(activeSec);

    IKeyNavigationSection* prevSec = prevEnabled(m_sections, activeSec);
    if (!prevSec) { // active is first
        IKeyNavigationSection* last = lastEnabled(m_sections);
        if (last) {
            activateSection(last);
        }
        return;
    }

    activateSection(prevSec);
}

const QList<IKeyNavigationSubSection*>& KeyNavigationController::subsectionsOfActiveSection(bool doActiveIfNoAnyActive) const
{
    static const QList<IKeyNavigationSubSection*> null;

    if (m_sections.empty()) {
        return null;
    }

    IKeyNavigationSection* activeSec = findActive(m_sections);
    if (!activeSec) { // no any active
        if (!doActiveIfNoAnyActive) {
            return null;
        }

        activeSec = firstEnabled(m_sections);
        if (!activeSec) {
            return null;
        }
        activeSec->setActive(true);
    }

    return activeSec->subsections();
}

void KeyNavigationController::nextSubSection()
{
    LOGI() << "====";

    const QList<IKeyNavigationSubSection*>& subsections = subsectionsOfActiveSection();
    if (subsections.isEmpty()) {
        return;
    }

    IKeyNavigationSubSection* activeSubSec = findActive(subsections);
    if (!activeSubSec) { // no any active
        IKeyNavigationSubSection* firstSub = firstEnabled(subsections);
        if (firstSub) {
            firstSub->setActive(true);
        }
        return;
    }

    activeSubSec->setActive(false);

    IKeyNavigationSubSection* nextSubSec = nextEnabled(subsections, activeSubSec);
    if (!nextSubSec) { // active is last
        IKeyNavigationSubSection* firstSub = firstEnabled(subsections);
        if (firstSub) {
            firstSub->setActive(true);
        }
        return;
    }

    nextSubSec->setActive(true);
}

void KeyNavigationController::prevSubSection()
{
    LOGI() << "====";

    const QList<IKeyNavigationSubSection*>& subsections = subsectionsOfActiveSection();
    if (subsections.isEmpty()) {
        return;
    }

    IKeyNavigationSubSection* activeSubSec = findActive(subsections);
    if (!activeSubSec) { // no any active
        IKeyNavigationSubSection* lastSub = lastEnabled(subsections);
        if (lastSub) {
            lastSub->setActive(true);
        }
        return;
    }

    activeSubSec->setActive(false);

    IKeyNavigationSubSection* prevSubSec = prevEnabled(subsections, activeSubSec);
    if (!prevSubSec) { // active is first
        IKeyNavigationSubSection* lastSub = lastEnabled(subsections);
        if (lastSub) {
            lastSub->setActive(true);
        }
        return;
    }

    prevSubSec->setActive(true);
}
