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
    dispatcher()->reg(this, "nav-next-section", this, &KeyNavigationController::goToNextSection);
    dispatcher()->reg(this, "nav-prev-section", this, &KeyNavigationController::goToPrevSection);
    dispatcher()->reg(this, "nav-next-subsection", this, &KeyNavigationController::goToNextSubSection);
    dispatcher()->reg(this, "nav-prev-subsection", this, &KeyNavigationController::goToPrevSubSection);
    dispatcher()->reg(this, "nav-right", this, &KeyNavigationController::goToNextControl);
    dispatcher()->reg(this, "nav-left", this, &KeyNavigationController::goToPrevControl);
    dispatcher()->reg(this, "nav-trigger", this, &KeyNavigationController::doTriggerControl);
}

void KeyNavigationController::reg(IKeyNavigationSection* s)
{
    //! TODO add check on valid state

    m_sections.push_back(s);
    std::sort(m_sections.begin(), m_sections.end(), [](const IKeyNavigationSection* f, const IKeyNavigationSection* s) {
        return f->order() < s->order();
    });

    s->forceActiveRequested().onReceive(this, [this](const SectionSubSectionControl& ssc) {
        onForceActiveRequested(std::get<0>(ssc), std::get<1>(ssc), std::get<2>(ssc));
    });
}

void KeyNavigationController::unreg(IKeyNavigationSection* s)
{
    m_sections.erase(std::remove(m_sections.begin(), m_sections.end(), s), m_sections.end());
    s->forceActiveRequested().resetOnReceive(this);
}

void KeyNavigationController::doActivateSection(IKeyNavigationSection* s)
{
    IF_ASSERT_FAILED(s) {
        return;
    }

    for (IKeyNavigationSubSection* sub : s->subsections()) {
        doDeactivateSubSection(sub);
    }

    s->setActive(true);

    IKeyNavigationSubSection* firstSub = firstEnabled(s->subsections());
    if (firstSub) {
        doActivateSubSection(firstSub);
    }

    LOGD() << "activateSection: " << s->name();
}

void KeyNavigationController::doDeactivateSection(IKeyNavigationSection* s)
{
    IF_ASSERT_FAILED(s) {
        return;
    }

    for (IKeyNavigationSubSection* sub : s->subsections()) {
        doDeactivateSubSection(sub);
    }

    s->setActive(false);
}

void KeyNavigationController::doActivateSubSection(IKeyNavigationSubSection* sub)
{
    IF_ASSERT_FAILED(sub) {
        return;
    }

    for (IKeyNavigationControl* ctr : sub->controls()) {
        ctr->setActive(false);
    }

    sub->setActive(true);

    IKeyNavigationControl* firstCtr = firstEnabled(sub->controls());
    if (firstCtr) {
        firstCtr->setActive(true);
    }

    LOGD() << "activateSubSection: " << sub->name();
}

void KeyNavigationController::doDeactivateSubSection(IKeyNavigationSubSection* sub)
{
    IF_ASSERT_FAILED(sub) {
        return;
    }

    for (IKeyNavigationControl* ctr : sub->controls()) {
        ctr->setActive(false);
    }

    sub->setActive(false);
}

void KeyNavigationController::goToNextSection()
{
    LOGI() << "====";
    if (m_sections.empty()) {
        return;
    }

    IKeyNavigationSection* activeSec = findActive(m_sections);
    if (!activeSec) { // no any active
        IKeyNavigationSection* first = firstEnabled(m_sections);
        if (first) {
            doActivateSection(first);
        }
        return;
    }

    doDeactivateSection(activeSec);

    IKeyNavigationSection* nextSec = nextEnabled(m_sections, activeSec);
    if (!nextSec) { // active is last
        IKeyNavigationSection* first = firstEnabled(m_sections);
        if (first) {
            doActivateSection(first);
        }
        return;
    }

    doActivateSection(nextSec);
}

void KeyNavigationController::goToPrevSection()
{
    LOGI() << "====";
    if (m_sections.empty()) {
        return;
    }

    IKeyNavigationSection* activeSec = findActive(m_sections);
    if (!activeSec) { // no any active
        IKeyNavigationSection* last = lastEnabled(m_sections);
        if (last) {
            doActivateSection(last);
        }
        return;
    }

    doDeactivateSection(activeSec);

    IKeyNavigationSection* prevSec = prevEnabled(m_sections, activeSec);
    if (!prevSec) { // active is first
        IKeyNavigationSection* last = lastEnabled(m_sections);
        if (last) {
            doActivateSection(last);
        }
        return;
    }

    doActivateSection(prevSec);
}

const QList<IKeyNavigationSubSection*>& KeyNavigationController::subsectionsOfActiveSection(bool doActiveIfNoAnyActive)
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

const QList<IKeyNavigationControl*>& KeyNavigationController::controlsOfActiveSubSection(bool doActiveIfNoAnyActive)
{
    static const QList<IKeyNavigationControl*> null;

    const QList<IKeyNavigationSubSection*>& subsections = subsectionsOfActiveSection(doActiveIfNoAnyActive);
    if (subsections.isEmpty()) {
        return null;
    }

    IKeyNavigationSubSection* activeSubSec = findActive(subsections);
    if (!activeSubSec) { // no any active
        if (!doActiveIfNoAnyActive) {
            return null;
        }

        activeSubSec = firstEnabled(subsections);
        if (!activeSubSec) {
            return null;
        }
        activeSubSec->setActive(true);
    }

    return activeSubSec->controls();
}

void KeyNavigationController::goToNextSubSection()
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
            doActivateSubSection(firstSub);
        }
        return;
    }

    doDeactivateSubSection(activeSubSec);

    IKeyNavigationSubSection* nextSubSec = nextEnabled(subsections, activeSubSec);
    if (!nextSubSec) { // active is last
        IKeyNavigationSubSection* firstSub = firstEnabled(subsections);
        if (firstSub) {
            doActivateSubSection(firstSub);
        }
        return;
    }

    doActivateSubSection(nextSubSec);
}

void KeyNavigationController::goToPrevSubSection()
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
            doActivateSubSection(lastSub);
        }
        return;
    }

    doDeactivateSubSection(activeSubSec);

    IKeyNavigationSubSection* prevSubSec = prevEnabled(subsections, activeSubSec);
    if (!prevSubSec) { // active is first
        IKeyNavigationSubSection* lastSub = lastEnabled(subsections);
        if (lastSub) {
            doActivateSubSection(lastSub);
        }
        return;
    }

    doActivateSubSection(prevSubSec);
}

void KeyNavigationController::goToNextControl()
{
    LOGI() << "====";

    const QList<IKeyNavigationControl*>& controls = controlsOfActiveSubSection();
    if (controls.isEmpty()) {
        return;
    }

    IKeyNavigationControl* activeControl = findActive(controls);
    if (!activeControl) { // no any active
        IKeyNavigationControl* first = firstEnabled(controls);
        if (first) {
            first->setActive(true);
        }
        return;
    }

    activeControl->setActive(false);

    IKeyNavigationControl* nextControl = nextEnabled(controls, activeControl);
    if (!nextControl) { // active is last
        IKeyNavigationControl* first = firstEnabled(controls);
        if (first) {
            first->setActive(true);
        }
        return;
    }

    nextControl->setActive(true);
}

void KeyNavigationController::goToPrevControl()
{
    LOGI() << "====";

    const QList<IKeyNavigationControl*>& controls = controlsOfActiveSubSection();
    if (controls.isEmpty()) {
        return;
    }

    IKeyNavigationControl* activeControl = findActive(controls);
    if (!activeControl) { // no any active
        IKeyNavigationControl* last = lastEnabled(controls);
        if (last) {
            last->setActive(true);
        }
        return;
    }

    activeControl->setActive(false);

    IKeyNavigationControl* prevControl = prevEnabled(controls, activeControl);
    if (!prevControl) { // active is first
        IKeyNavigationControl* last = lastEnabled(controls);
        if (last) {
            last->setActive(true);
        }
        return;
    }

    prevControl->setActive(true);
}

void KeyNavigationController::doTriggerControl()
{
    LOGI() << "====";
    const QList<IKeyNavigationControl*>& controls = controlsOfActiveSubSection();
    if (controls.isEmpty()) {
        return;
    }

    IKeyNavigationControl* activeControl = findActive(controls);
    if (!activeControl) {
        return;
    }

    activeControl->trigger();
}

void KeyNavigationController::onForceActiveRequested(IKeyNavigationSection* sec, IKeyNavigationSubSection* sub, IKeyNavigationControl* ctrl)
{
    if (m_sections.empty()) {
        return;
    }

    IKeyNavigationSection* activeSec = findActive(m_sections);
    if (activeSec && activeSec != sec) {
        doDeactivateSection(activeSec);
    }
    activeSec = sec;
    activeSec->setActive(true);

    IKeyNavigationSubSection* activeSub = findActive(activeSec->subsections());
    if (activeSub && activeSub != sub) {
        doDeactivateSubSection(activeSub);
    }
    activeSub = sub;
    activeSub->setActive(true);

    IKeyNavigationControl* activeCtrl = findActive(activeSub->controls());
    if (activeCtrl && activeCtrl != ctrl) {
        activeCtrl->setActive(false);
    }

    activeCtrl = ctrl;
    activeCtrl->setActive(true);
}
