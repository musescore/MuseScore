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
#include <limits>

#include "log.h"

using namespace mu::ui;

// algorithms
enum class NavDirection {
    Forward,
    Back
};

template<class T>
static T* findNearestEnabled(const QSet<T*>& set, const T* from, NavDirection direction)
{
    int fromOrder = from ? from->order() : (direction == NavDirection::Forward ? -1 : std::numeric_limits<int>::max());
    T* ret = nullptr;
    for (T* v : set) {
        if (!v->enabled()) {
            continue;
        }

        if (v == from) {
            continue;
        }

        switch (direction) {
        case NavDirection::Forward: {
            if (v->order() >= fromOrder) {
                if (!ret) {
                    ret = v;
                    continue;
                }

                if (v->order() < ret->order()) {
                    ret = v;
                }
            }
        } break;
        case NavDirection::Back: {
            if (v->order() <= fromOrder) {
                if (!ret) {
                    ret = v;
                    continue;
                }

                if (v->order() > ret->order()) {
                    ret = v;
                }
            }
        } break;
        }
    }

    return ret;
}

template<class T>
static T* firstEnabled(const QSet<T*>& set)
{
    if (set.empty()) {
        return nullptr;
    }
    return findNearestEnabled<T>(set, nullptr, NavDirection::Forward);
}

template<class T>
static T* lastEnabled(const QSet<T*>& set)
{
    return findNearestEnabled<T>(set, nullptr, NavDirection::Back);
}

template<class T>
static T* nextEnabled(const QSet<T*>& set, const T* s)
{
    return findNearestEnabled<T>(set, s, NavDirection::Forward);
}

template<class T>
static T* prevEnabled(const QSet<T*>& set, const T* s)
{
    return findNearestEnabled<T>(set, s, NavDirection::Back);
}

template<class T>
static T* findActive(const QSet<T*>& set)
{
    auto it = std::find_if(set.cbegin(), set.cend(), [](const T* s) {
        return s->active();
    });

    if (it != set.cend()) {
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

    m_sections.insert(s);

    s->forceActiveRequested().onReceive(this, [this](const SectionSubSectionControl& ssc) {
        onForceActiveRequested(std::get<0>(ssc), std::get<1>(ssc), std::get<2>(ssc));
    });
}

void KeyNavigationController::unreg(IKeyNavigationSection* s)
{
    m_sections.remove(s);
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

const QSet<IKeyNavigationSubSection*>& KeyNavigationController::subsectionsOfActiveSection(bool doActiveIfNoAnyActive)
{
    static const QSet<IKeyNavigationSubSection*> null;

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

const QSet<IKeyNavigationControl*>& KeyNavigationController::controlsOfActiveSubSection(bool doActiveIfNoAnyActive)
{
    static const QSet<IKeyNavigationControl*> null;

    const QSet<IKeyNavigationSubSection*>& subsections = subsectionsOfActiveSection(doActiveIfNoAnyActive);
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

    const QSet<IKeyNavigationSubSection*>& subsections = subsectionsOfActiveSection();
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

    const QSet<IKeyNavigationSubSection*>& subsections = subsectionsOfActiveSection();
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

    const QSet<IKeyNavigationControl*>& controls = controlsOfActiveSubSection();
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

    const QSet<IKeyNavigationControl*>& controls = controlsOfActiveSubSection();
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
    const QSet<IKeyNavigationControl*>& controls = controlsOfActiveSubSection();
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
