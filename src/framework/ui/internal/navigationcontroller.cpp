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
#include "navigationcontroller.h"

#include <algorithm>
#include <limits>

#include "log.h"

//#define NAVIGATION_LOGGING_ENABLED

#ifdef NAVIGATION_LOGGING_ENABLED
#define MYLOG() LOGI()
#else
#define MYLOG() LOGN()
#endif

using namespace mu::ui;

static const mu::UriQuery DEV_SHOW_CONTROLS_URI("musescore://devtools/keynav/controls?sync=false&modal=false");

using MoveDirection = NavigationController::MoveDirection;
using Event = INavigation::Event;

// algorithms
template<class T>
static T* findNearestEnabled(const std::set<T*>& set, const INavigation::Index& currentIndex, MoveDirection direction)
{
    T* ret = nullptr;
    for (T* v : set) {
        if (!v->enabled()) {
            continue;
        }

        switch (direction) {
        case MoveDirection::First: {
            if (!ret) {
                ret = v;
                continue;
            }

            if (v->index().row <= ret->index().row) {
                if (v->index().column <= ret->index().column) {
                    ret = v;
                    continue;
                }
            }
        } break;
        case MoveDirection::Last: {
            if (!ret) {
                ret = v;
                continue;
            }

            if (v->index().row >= ret->index().row) {
                if (v->index().column >= ret->index().column) {
                    ret = v;
                    continue;
                }
            }
        } break;
        case MoveDirection::Right: {
            if (v->index().row != currentIndex.row) {
                continue;
            }

            if (v->index().column > currentIndex.column) {
                if (!ret) {
                    ret = v;
                    continue;
                }

                if (v->index().column < ret->index().column) {
                    ret = v;
                }
            }
        } break;
        case MoveDirection::Left: {
            if (v->index().row != currentIndex.row) {
                continue;
            }

            if (v->index().column < currentIndex.column) {
                if (!ret) {
                    ret = v;
                    continue;
                }

                if (v->index().column > ret->index().column) {
                    ret = v;
                }
            }
        } break;
        case MoveDirection::Down: {
            if (v->index().column != currentIndex.column) {
                continue;
            }

            if (v->index().row > currentIndex.row) {
                if (!ret) {
                    ret = v;
                    continue;
                }

                if (v->index().row < ret->index().row) {
                    ret = v;
                }
            }
        } break;
        case MoveDirection::Up: {
            if (v->index().column != currentIndex.column) {
                continue;
            }

            if (v->index().row < currentIndex.row) {
                if (!ret) {
                    ret = v;
                    continue;
                }

                if (v->index().row > ret->index().row) {
                    ret = v;
                }
            }
        } break;
        }
    }

    return ret;
}

template<class T>
static T* firstEnabled(const std::set<T*>& set, const INavigation::Index& currentIndex, MoveDirection direction)
{
    if (set.empty()) {
        return nullptr;
    }

    IF_ASSERT_FAILED(direction == MoveDirection::Right || direction == MoveDirection::Down) {
        return nullptr;
    }

    return findNearestEnabled<T>(set, currentIndex, direction);
}

template<class T>
static T* firstEnabled(const std::set<T*>& set)
{
    if (set.empty()) {
        return nullptr;
    }

    return findNearestEnabled<T>(set, INavigation::Index(), MoveDirection::First);
}

template<class T>
static T* lastEnabled(const std::set<T*>& set, const INavigation::Index& currentIndex, MoveDirection direction)
{
    if (set.empty()) {
        return nullptr;
    }

    IF_ASSERT_FAILED(direction == MoveDirection::Left || (direction == MoveDirection::Up)) {
        return nullptr;
    }

    return findNearestEnabled<T>(set, currentIndex, direction);
}

template<class T>
static T* lastEnabled(const std::set<T*>& set)
{
    if (set.empty()) {
        return nullptr;
    }

    return findNearestEnabled<T>(set, INavigation::Index(), MoveDirection::Last);
}

template<class T>
static T* nextEnabled(const std::set<T*>& set, const INavigation::Index& currentIndex,
                      MoveDirection direction = MoveDirection::Right)
{
    if (set.empty()) {
        return nullptr;
    }

    IF_ASSERT_FAILED(direction == MoveDirection::Right || direction == MoveDirection::Down) {
        return nullptr;
    }

    return findNearestEnabled<T>(set, currentIndex, direction);
}

template<class T>
static T* prevEnabled(const std::set<T*>& set, const INavigation::Index& currentIndex, MoveDirection direction = MoveDirection::Left)
{
    if (set.empty()) {
        return nullptr;
    }

    IF_ASSERT_FAILED(direction == MoveDirection::Left || direction == MoveDirection::Up) {
        return nullptr;
    }

    return findNearestEnabled<T>(set, currentIndex, direction);
}

template<class T>
static T* findActive(const std::set<T*>& set)
{
    auto it = std::find_if(set.cbegin(), set.cend(), [](const T* s) {
        return s->active();
    });

    if (it != set.cend()) {
        return *it;
    }

    return nullptr;
}

void NavigationController::init()
{
    dispatcher()->reg(this, "nav-dev-show-controls", this, &NavigationController::devShowControls);

    dispatcher()->reg(this, "nav-next-section", this, &NavigationController::goToNextSection);
    dispatcher()->reg(this, "nav-prev-section", this, &NavigationController::goToPrevSection);
    dispatcher()->reg(this, "nav-next-panel", this, &NavigationController::goToNextPanel);
    dispatcher()->reg(this, "nav-prev-panel", this, &NavigationController::goToPrevPanel);

    dispatcher()->reg(this, "nav-trigger-control", this, &NavigationController::doTriggerControl);

    dispatcher()->reg(this, "nav-right", this, &NavigationController::onRight);
    dispatcher()->reg(this, "nav-left", this, &NavigationController::onLeft);
    dispatcher()->reg(this, "nav-up", this, &NavigationController::onUp);
    dispatcher()->reg(this, "nav-down", this, &NavigationController::onDown);
    dispatcher()->reg(this, "nav-escape", this, &NavigationController::onEscape);

    dispatcher()->reg(this, "nav-first-control", this, &NavigationController::goToFirstControl);         // typically Home key
    dispatcher()->reg(this, "nav-last-control", this, &NavigationController::goToLastControl);           // typically End key
    dispatcher()->reg(this, "nav-nextrow-control", this, &NavigationController::goToNextRowControl);     // typically PageDown key
    dispatcher()->reg(this, "nav-prevrow-control", this, &NavigationController::goToPrevRowControl);     // typically PageUp key
}

void NavigationController::reg(INavigationSection* section)
{
    //! TODO add check on valid state

    m_sections.insert(section);

    section->forceActiveRequested().onReceive(this, [this](const SectionPanelControl& ssc) {
        onForceActiveRequested(std::get<0>(ssc), std::get<1>(ssc), std::get<2>(ssc));
    });
}

void NavigationController::unreg(INavigationSection* section)
{
    m_sections.erase(section);
    section->forceActiveRequested().resetOnReceive(this);
}

const std::set<INavigationSection*>& NavigationController::sections() const
{
    return m_sections;
}

void NavigationController::devShowControls()
{
    if (!interactive()->isOpened(DEV_SHOW_CONTROLS_URI.uri()).val) {
        interactive()->open(DEV_SHOW_CONTROLS_URI);
    }
}

void NavigationController::doActivateSection(INavigationSection* s)
{
    IF_ASSERT_FAILED(s) {
        return;
    }

    for (INavigationPanel* sub : s->panels()) {
        doDeactivatePanel(sub);
    }

    s->setActive(true);
    MYLOG() << "activated section: " << s->name() << ", order: " << s->index().order();

    INavigationPanel* firstSub = firstEnabled(s->panels());
    if (firstSub) {
        doActivatePanel(firstSub);
    }
}

void NavigationController::doDeactivateSection(INavigationSection* s)
{
    IF_ASSERT_FAILED(s) {
        return;
    }

    for (INavigationPanel* sub : s->panels()) {
        doDeactivatePanel(sub);
    }

    s->setActive(false);
}

void NavigationController::doActivatePanel(INavigationPanel* sub)
{
    IF_ASSERT_FAILED(sub) {
        return;
    }

    for (INavigationControl* ctr : sub->controls()) {
        doDeactivateControl(ctr);
    }

    sub->setActive(true);
    MYLOG() << "activated subsection: " << sub->name() << ", order: " << sub->index().order();

    INavigationControl* firstCtr = firstEnabled(sub->controls());
    if (firstCtr) {
        doActivateControl(firstCtr);
    }
}

void NavigationController::doDeactivatePanel(INavigationPanel* sub)
{
    IF_ASSERT_FAILED(sub) {
        return;
    }

    for (INavigationControl* ctr : sub->controls()) {
        ctr->setActive(false);
    }

    sub->setActive(false);
}

void NavigationController::doActivateControl(INavigationControl* c)
{
    IF_ASSERT_FAILED(c) {
        return;
    }

    c->setActive(true);
    MYLOG() << "activated control: " << c->name() << ", row: " << c->index().row << ", column: " << c->index().column;
}

void NavigationController::doDeactivateControl(INavigationControl* c)
{
    IF_ASSERT_FAILED(c) {
        return;
    }

    c->setActive(false);
}

void NavigationController::doActivateFirst()
{
    if (m_sections.empty()) {
        return;
    }

    INavigationSection* first = firstEnabled(m_sections);
    if (first) {
        doActivateSection(first);
    }
}

void NavigationController::doActivateLast()
{
    if (m_sections.empty()) {
        return;
    }

    INavigationSection* last = lastEnabled(m_sections);
    if (last) {
        doActivateSection(last);
    }
}

INavigationSection* NavigationController::activeSection() const
{
    if (m_sections.empty()) {
        return nullptr;
    }
    return findActive(m_sections);
}

INavigationPanel* NavigationController::activePanel() const
{
    INavigationSection* activeSec = activeSection();
    if (!activeSec) {
        return nullptr;
    }
    return findActive(activeSec->panels());
}

INavigationControl* NavigationController::activeControl() const
{
    INavigationPanel* activeSub = activePanel();
    if (!activeSub) {
        return nullptr;
    }
    return findActive(activeSub->controls());
}

void NavigationController::goToNextSection()
{
    MYLOG() << "====";
    if (m_sections.empty()) {
        return;
    }

    INavigationSection* activeSec = findActive(m_sections);
    if (!activeSec) { // no any active
        doActivateFirst();
        return;
    }

    doDeactivateSection(activeSec);

    INavigationSection* nextSec = nextEnabled(m_sections, activeSec->index());
    if (!nextSec) { // active is last
        nextSec = firstEnabled(m_sections); // the first to be the next
    }

    doActivateSection(nextSec);
}

void NavigationController::goToPrevSection()
{
    MYLOG() << "====";
    if (m_sections.empty()) {
        return;
    }

    INavigationSection* activeSec = findActive(m_sections);
    if (!activeSec) { // no any active
        doActivateLast();
        return;
    }

    doDeactivateSection(activeSec);

    INavigationSection* prevSec = prevEnabled(m_sections, activeSec->index());
    if (!prevSec) { // active is first
        prevSec = lastEnabled(m_sections); // the last to be the prev
    }

    doActivateSection(prevSec);
}

void NavigationController::goToNextPanel()
{
    MYLOG() << "====";
    INavigationSection* activeSec = activeSection();
    if (!activeSec) {
        doActivateFirst();
        return;
    }

    INavigationPanel* activeSubSec = findActive(activeSec->panels());
    if (!activeSubSec) { // no any active
        INavigationPanel* first = firstEnabled(activeSec->panels());
        if (first) {
            doActivatePanel(first);
        }
        return;
    }

    doDeactivatePanel(activeSubSec);

    INavigationPanel* nextSubSec = nextEnabled(activeSec->panels(), activeSubSec->index());
    if (!nextSubSec) { // active is last
        nextSubSec = firstEnabled(activeSec->panels()); // the first to be the next
    }

    doActivatePanel(nextSubSec);
}

void NavigationController::goToPrevPanel()
{
    MYLOG() << "====";
    INavigationSection* activeSec = activeSection();
    if (!activeSec) {
        doActivateLast();
        return;
    }

    INavigationPanel* activeSubSec = findActive(activeSec->panels());
    if (!activeSubSec) { // no any active
        INavigationPanel* lastSub = lastEnabled(activeSec->panels());
        if (lastSub) {
            doActivatePanel(lastSub);
        }
        return;
    }

    doDeactivatePanel(activeSubSec);

    INavigationPanel* prevSubSec = prevEnabled(activeSec->panels(), activeSubSec->index());
    if (!prevSubSec) { // active is first
        prevSubSec = lastEnabled(activeSec->panels()); // the last to be the prev
    }

    doActivatePanel(prevSubSec);
}

void NavigationController::onRight()
{
    INavigationPanel* activeSubSec = activePanel();
    if (!activeSubSec) {
        return;
    }

    INavigation::EventPtr e = Event::make(Event::Right);
    activeSubSec->onEvent(e);
    if (e->accepted) {
        return;
    }

    INavigationControl* activeCtrl = findActive(activeSubSec->controls());
    if (activeCtrl) {
        activeCtrl->onEvent(e);
        if (e->accepted) {
            return;
        }
    }

    INavigationPanel::Direction direction = activeSubSec->direction();
    switch (direction) {
    case INavigationPanel::Direction::Horizontal: {
        goToControl(MoveDirection::Right, activeSubSec);
    } break;
    case INavigationPanel::Direction::Vertical: {
        // noop
    } break;
    case INavigationPanel::Direction::Both: {
        goToControl(MoveDirection::Right, activeSubSec);
    } break;
    }
}

void NavigationController::onLeft()
{
    INavigationPanel* activeSubSec = activePanel();
    if (!activeSubSec) {
        return;
    }

    INavigation::EventPtr e = Event::make(Event::Left);
    activeSubSec->onEvent(e);
    if (e->accepted) {
        return;
    }

    INavigationControl* activeCtrl = findActive(activeSubSec->controls());
    if (activeCtrl) {
        activeCtrl->onEvent(e);
        if (e->accepted) {
            return;
        }
    }

    INavigationPanel::Direction direction = activeSubSec->direction();
    switch (direction) {
    case INavigationPanel::Direction::Horizontal: {
        goToControl(MoveDirection::Left, activeSubSec);
    } break;
    case INavigationPanel::Direction::Vertical: {
        // noop
    } break;
    case INavigationPanel::Direction::Both: {
        goToControl(MoveDirection::Left, activeSubSec);
    } break;
    }
}

void NavigationController::onDown()
{
    INavigationPanel* activeSubSec = activePanel();
    if (!activeSubSec) {
        return;
    }

    INavigation::EventPtr e = Event::make(Event::Down);
    activeSubSec->onEvent(e);
    if (e->accepted) {
        return;
    }

    INavigationControl* activeCtrl = findActive(activeSubSec->controls());
    if (activeCtrl) {
        activeCtrl->onEvent(e);
        if (e->accepted) {
            return;
        }
    }

    INavigationPanel::Direction direction = activeSubSec->direction();
    switch (direction) {
    case INavigationPanel::Direction::Horizontal: {
        // noop
    } break;
    case INavigationPanel::Direction::Vertical: {
        goToControl(MoveDirection::Down, activeSubSec);
    } break;
    case INavigationPanel::Direction::Both: {
        goToControl(MoveDirection::Down, activeSubSec);
    } break;
    }
}

void NavigationController::onUp()
{
    INavigationPanel* activeSubSec = activePanel();
    if (!activeSubSec) {
        return;
    }

    INavigation::EventPtr e = Event::make(Event::Up);
    activeSubSec->onEvent(e);
    if (e->accepted) {
        return;
    }

    INavigationControl* activeCtrl = findActive(activeSubSec->controls());
    if (activeCtrl) {
        activeCtrl->onEvent(e);
        if (e->accepted) {
            return;
        }
    }

    INavigationPanel::Direction direction = activeSubSec->direction();
    switch (direction) {
    case INavigationPanel::Direction::Horizontal: {
        // noop
    } break;
    case INavigationPanel::Direction::Vertical: {
        goToControl(MoveDirection::Up, activeSubSec);
    } break;
    case INavigationPanel::Direction::Both: {
        goToControl(MoveDirection::Up, activeSubSec);
    } break;
    }
}

void NavigationController::onEscape()
{
    MYLOG() << "====";
    INavigationPanel* activeSubSec = activePanel();
    if (!activeSubSec) {
        return;
    }

    INavigation::EventPtr e = Event::make(Event::Escape);
    activeSubSec->onEvent(e);
    if (e->accepted) {
        return;
    }

    INavigationControl* activeCtrl = findActive(activeSubSec->controls());
    if (activeCtrl) {
        activeCtrl->onEvent(e);
    }
}

void NavigationController::goToFirstControl()
{
    MYLOG() << "====";
    goToControl(MoveDirection::First);
}

void NavigationController::goToLastControl()
{
    MYLOG() << "====";
    goToControl(MoveDirection::Last);
}

void NavigationController::goToNextRowControl()
{
    MYLOG() << "====";
    INavigationPanel* activeSubSec = activePanel();
    if (!activeSubSec) {
        return;
    }

    INavigationControl* activeControl = findActive(activeSubSec->controls());
    if (activeControl) {
        doDeactivateControl(activeControl);
    }

    INavigationControl* toControl = nullptr;
    if (activeControl) {
        INavigation::Index index = activeControl->index();
        index.column = 0;
        toControl = nextEnabled(activeSubSec->controls(), index, MoveDirection::Down);
        if (!toControl) { // last row
            toControl = firstEnabled(activeSubSec->controls());
        }
    } else {
        toControl = firstEnabled(activeSubSec->controls());
    }

    if (toControl) {
        doActivateControl(toControl);
    }
}

void NavigationController::goToPrevRowControl()
{
    MYLOG() << "====";
    INavigationPanel* activeSubSec = activePanel();
    if (!activeSubSec) {
        return;
    }

    INavigationControl* activeControl = findActive(activeSubSec->controls());
    if (activeControl) {
        doDeactivateControl(activeControl);
    }

    INavigationControl* toControl = nullptr;
    if (activeControl) {
        INavigation::Index index = activeControl->index();
        index.column = 0;
        toControl = prevEnabled(activeSubSec->controls(), index, MoveDirection::Up);
        if (!toControl) { // first row
            toControl = lastEnabled(activeSubSec->controls());
        }
    } else {
        toControl = lastEnabled(activeSubSec->controls());
    }

    if (toControl) {
        doActivateControl(toControl);
    }
}

void NavigationController::goToControl(MoveDirection direction, INavigationPanel* activeSubSec)
{
    MYLOG() << "direction: " << direction;
    if (!activeSubSec) {
        activeSubSec = activePanel();
    }

    if (!activeSubSec) {
        return;
    }

    INavigationControl* activeControl = findActive(activeSubSec->controls());
    if (activeControl) {
        MYLOG() << "current activated control: " << activeControl->name()
                << ", row: " << activeControl->index().row
                << ", column: " << activeControl->index().column;

        doDeactivateControl(activeControl);
    }

    INavigationControl* toControl = nullptr;

    switch (direction) {
    case MoveDirection::First: {
        toControl = firstEnabled(activeSubSec->controls());
    } break;
    case MoveDirection::Last: {
        toControl = lastEnabled(activeSubSec->controls());
    } break;
    case MoveDirection::Right: {
        if (!activeControl) { // no any active
            toControl = firstEnabled(activeSubSec->controls(), INavigation::Index(), direction);
        } else {
            toControl = nextEnabled(activeSubSec->controls(), activeControl->index(), direction);
            if (!toControl) { // active is last
                INavigation::Index index = activeControl->index();
                index.column = -1;
                toControl = firstEnabled(activeSubSec->controls(), index, direction); // the first to be the next
            }
        }
    } break;
    case MoveDirection::Down: {
        if (!activeControl) { // no any active
            toControl = firstEnabled(activeSubSec->controls(), INavigation::Index(), direction);
        } else {
            toControl = nextEnabled(activeSubSec->controls(), activeControl->index(), direction);
            if (!toControl) { // active is last
                INavigation::Index index = activeControl->index();
                index.row = -1;
                toControl = firstEnabled(activeSubSec->controls(), index, direction); // the first to be the next
            }
        }
    } break;
    case MoveDirection::Left: {
        if (!activeControl) { // no any active
            toControl = lastEnabled(activeSubSec->controls(), INavigation::Index(), direction);
        } else {
            toControl = prevEnabled(activeSubSec->controls(), activeControl->index(), direction);
            if (!toControl) { // active is first
                INavigation::Index index = activeControl->index();
                index.column = std::numeric_limits<int>::max();
                toControl = lastEnabled(activeSubSec->controls(), index, direction); // the last to be the next
            }
        }
    } break;
    case MoveDirection::Up: {
        if (!activeControl) { // no any active
            toControl = lastEnabled(activeSubSec->controls(), INavigation::Index(), direction);
        } else {
            toControl = prevEnabled(activeSubSec->controls(), activeControl->index(), direction);
            if (!toControl) { // active is first
                INavigation::Index index = activeControl->index();
                index.row = std::numeric_limits<int>::max();
                toControl = lastEnabled(activeSubSec->controls(), index, direction); // the last to be the next
            }
        }
    } break;
    }

    if (toControl) {
        doActivateControl(toControl);
    }
}

void NavigationController::doTriggerControl()
{
    MYLOG() << "====";
    INavigationControl* activeCtrl= activeControl();
    if (!activeCtrl) {
        return;
    }

    MYLOG() << "triggered control: " << activeCtrl->name();
    activeCtrl->trigger();
}

void NavigationController::onForceActiveRequested(INavigationSection* sec, INavigationPanel* sub, INavigationControl* ctrl)
{
    if (m_sections.empty()) {
        return;
    }

    INavigationSection* activeSec = findActive(m_sections);
    if (activeSec && activeSec != sec) {
        doDeactivateSection(activeSec);
    }

    sec->setActive(true);
    MYLOG() << "activated section: " << sec->name() << ", order: " << sec->index().order();

    INavigationPanel* activeSub = findActive(sec->panels());
    if (activeSub && activeSub != sub) {
        doDeactivatePanel(activeSub);
    }

    sub->setActive(true);
    MYLOG() << "activated subsection: " << sub->name() << ", order: " << sub->index().order();

    INavigationControl* activeCtrl = findActive(sub->controls());
    if (activeCtrl && activeCtrl != ctrl) {
        activeCtrl->setActive(false);
    }

    ctrl->setActive(true);
    MYLOG() << "activated control: " << ctrl->name() << ", row: " << ctrl->index().row << ", column: " << ctrl->index().column;
}
