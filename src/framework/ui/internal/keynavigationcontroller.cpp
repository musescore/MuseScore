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

using MoveDirection = KeyNavigationController::MoveDirection;

// algorithms
template<class T>
static IKeyNavigation::Index determinateExtremeIndex(const QSet<T*>& set, MoveDirection direction)
{
    IKeyNavigation::Index index;
    switch (direction) {
    case MoveDirection::Right: {
        index.column = -1;

        // find min row
        index.row = std::numeric_limits<int>::max();
        for (T* v : set) {
            if (v->index().row < index.row) {
                index.row = v->index().row;
            }
        }
    } break;
    case MoveDirection::Left: {
        index.column = std::numeric_limits<int>::max();

        // find max row
        index.row = -1;
        for (T* v : set) {
            if (v->index().row > index.row) {
                index.row = v->index().row;
            }
        }
    } break;
    case MoveDirection::Down: {
        index.row = -1;

        // find min column
        index.column = std::numeric_limits<int>::max();
        for (T* v : set) {
            if (v->index().column < index.column) {
                index.column = v->index().column;
            }
        }
    } break;
    case MoveDirection::Up: {
        index.row = std::numeric_limits<int>::max();

        // find max row
        index.column = -1;
        for (T* v : set) {
            if (v->index().column > index.column) {
                index.column = v->index().row;
            }
        }
    } break;
    }

    return index;
}

template<class T>
static T* findNearestEnabled(const QSet<T*>& set, const IKeyNavigation::Index& currentIndex, MoveDirection direction)
{
    T* ret = nullptr;
    for (T* v : set) {
        if (!v->enabled()) {
            continue;
        }

        switch (direction) {
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
static T* firstEnabled(const QSet<T*>& set,
                       const IKeyNavigation::Index& currentIndex = IKeyNavigation::Index(),
                       MoveDirection direction = MoveDirection::Right)
{
    if (set.empty()) {
        return nullptr;
    }

    IF_ASSERT_FAILED(direction == MoveDirection::Right || direction == MoveDirection::Down) {
        return nullptr;
    }

    if (currentIndex.column < 0 && currentIndex.row < 0) {
        return findNearestEnabled<T>(set, determinateExtremeIndex(set, direction), direction);
    }

    return findNearestEnabled<T>(set, currentIndex, direction);
}

template<class T>
static T* lastEnabled(const QSet<T*>& set,
                      const IKeyNavigation::Index& currentIndex = IKeyNavigation::Index(),
                      MoveDirection direction = MoveDirection::Left)
{
    if (set.empty()) {
        return nullptr;
    }

    IF_ASSERT_FAILED(direction == MoveDirection::Left || direction == MoveDirection::Up) {
        return nullptr;
    }

    if (currentIndex.column < 0 && currentIndex.row < 0) {
        return findNearestEnabled<T>(set, determinateExtremeIndex(set, direction), direction);
    }

    return findNearestEnabled<T>(set, currentIndex, direction);
}

template<class T>
static T* nextEnabled(const QSet<T*>& set, const IKeyNavigation::Index& currentIndex, MoveDirection direction = MoveDirection::Right)
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
static T* prevEnabled(const QSet<T*>& set, const IKeyNavigation::Index& currentIndex, MoveDirection direction = MoveDirection::Left)
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
    dispatcher()->reg(this, "nav-right", this, &KeyNavigationController::onRight);
    dispatcher()->reg(this, "nav-left", this, &KeyNavigationController::onLeft);
    dispatcher()->reg(this, "nav-up", this, &KeyNavigationController::onUp);
    dispatcher()->reg(this, "nav-down", this, &KeyNavigationController::onDown);
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
    LOGD() << "activated section: " << s->name() << ", order: " << s->index().order();

    IKeyNavigationSubSection* firstSub = firstEnabled(s->subsections());
    if (firstSub) {
        doActivateSubSection(firstSub);
    }
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
        doDeactivateControl(ctr);
    }

    sub->setActive(true);
    LOGD() << "activated subsection: " << sub->name() << ", order: " << sub->index().order();

    IKeyNavigationControl* firstCtr = firstEnabled(sub->controls());
    if (firstCtr) {
        doActivateControl(firstCtr);
    }
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

void KeyNavigationController::doActivateControl(IKeyNavigationControl* c)
{
    IF_ASSERT_FAILED(c) {
        return;
    }

    c->setActive(true);
    LOGD() << "activated control: " << c->name() << ", row: " << c->index().row << ", column: " << c->index().column;
}

void KeyNavigationController::doDeactivateControl(IKeyNavigationControl* c)
{
    IF_ASSERT_FAILED(c) {
        return;
    }

    c->setActive(false);
}

void KeyNavigationController::doActivateFirst()
{
    if (m_sections.empty()) {
        return;
    }

    IKeyNavigationSection* first = firstEnabled(m_sections);
    if (first) {
        doActivateSection(first);
    }
}

void KeyNavigationController::doActivateLast()
{
    if (m_sections.empty()) {
        return;
    }

    IKeyNavigationSection* last = lastEnabled(m_sections);
    if (last) {
        doActivateSection(last);
    }
}

IKeyNavigationSection* KeyNavigationController::activeSection() const
{
    if (m_sections.empty()) {
        return nullptr;
    }
    return findActive(m_sections);
}

IKeyNavigationSubSection* KeyNavigationController::activeSubSection() const
{
    IKeyNavigationSection* activeSec = activeSection();
    if (!activeSec) {
        return nullptr;
    }
    return findActive(activeSec->subsections());
}

IKeyNavigationControl* KeyNavigationController::activeControl() const
{
    IKeyNavigationSubSection* activeSub = activeSubSection();
    if (!activeSub) {
        return nullptr;
    }
    return findActive(activeSub->controls());
}

void KeyNavigationController::goToNextSection()
{
    LOGI() << "====";
    if (m_sections.empty()) {
        return;
    }

    IKeyNavigationSection* activeSec = findActive(m_sections);
    if (!activeSec) { // no any active
        doActivateFirst();
        return;
    }

    doDeactivateSection(activeSec);

    IKeyNavigationSection* nextSec = nextEnabled(m_sections, activeSec->index());
    if (!nextSec) { // active is last
        nextSec = firstEnabled(m_sections); // the first to be the next
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
        doActivateLast();
        return;
    }

    doDeactivateSection(activeSec);

    IKeyNavigationSection* prevSec = prevEnabled(m_sections, activeSec->index());
    if (!prevSec) { // active is first
        prevSec = lastEnabled(m_sections); // the last to be the prev
    }

    doActivateSection(prevSec);
}

void KeyNavigationController::goToNextSubSection()
{
    LOGI() << "====";

    IKeyNavigationSection* activeSec = activeSection();
    if (!activeSec) {
        doActivateFirst();
        return;
    }

    IKeyNavigationSubSection* activeSubSec = findActive(activeSec->subsections());
    if (!activeSubSec) { // no any active
        IKeyNavigationSubSection* first = firstEnabled(activeSec->subsections());
        if (first) {
            doActivateSubSection(first);
        }
        return;
    }

    doDeactivateSubSection(activeSubSec);

    IKeyNavigationSubSection* nextSubSec = nextEnabled(activeSec->subsections(), activeSubSec->index());
    if (!nextSubSec) { // active is last
        nextSubSec = firstEnabled(activeSec->subsections()); // the first to be the next
    }

    doActivateSubSection(nextSubSec);
}

void KeyNavigationController::goToPrevSubSection()
{
    LOGI() << "====";
    IKeyNavigationSection* activeSec = activeSection();
    if (!activeSec) {
        doActivateLast();
        return;
    }

    IKeyNavigationSubSection* activeSubSec = findActive(activeSec->subsections());
    if (!activeSubSec) { // no any active
        IKeyNavigationSubSection* lastSub = lastEnabled(activeSec->subsections());
        if (lastSub) {
            doActivateSubSection(lastSub);
        }
        return;
    }

    doDeactivateSubSection(activeSubSec);

    IKeyNavigationSubSection* prevSubSec = prevEnabled(activeSec->subsections(), activeSubSec->index());
    if (!prevSubSec) { // active is first
        prevSubSec = lastEnabled(activeSec->subsections()); // the last to be the prev
    }

    doActivateSubSection(prevSubSec);
}

void KeyNavigationController::onRight()
{
    IKeyNavigationSubSection* activeSubSec = activeSubSection();
    if (!activeSubSec) {
        return;
    }

    IKeyNavigationSubSection::Direction direction = activeSubSec->direction();
    switch (direction) {
    case IKeyNavigationSubSection::Direction::Horizontal: {
        goToControl(MoveDirection::Right, activeSubSec);
    } break;
    case IKeyNavigationSubSection::Direction::Vertical: {
        NOT_IMPLEMENTED;
    } break;
    case IKeyNavigationSubSection::Direction::Both: {
        goToControl(MoveDirection::Right, activeSubSec);
    } break;
    }
}

void KeyNavigationController::onLeft()
{
    IKeyNavigationSubSection* activeSubSec = activeSubSection();
    if (!activeSubSec) {
        return;
    }

    IKeyNavigationSubSection::Direction direction = activeSubSec->direction();
    switch (direction) {
    case IKeyNavigationSubSection::Direction::Horizontal: {
        goToControl(MoveDirection::Left, activeSubSec);
    } break;
    case IKeyNavigationSubSection::Direction::Vertical: {
        NOT_IMPLEMENTED;
    } break;
    case IKeyNavigationSubSection::Direction::Both: {
        goToControl(MoveDirection::Left, activeSubSec);
    } break;
    }
}

void KeyNavigationController::onDown()
{
    IKeyNavigationSubSection* activeSubSec = activeSubSection();
    if (!activeSubSec) {
        return;
    }

    IKeyNavigationSubSection::Direction direction = activeSubSec->direction();
    switch (direction) {
    case IKeyNavigationSubSection::Direction::Horizontal: {
        NOT_IMPLEMENTED;
    } break;
    case IKeyNavigationSubSection::Direction::Vertical: {
        goToControl(MoveDirection::Down, activeSubSec);
    } break;
    case IKeyNavigationSubSection::Direction::Both: {
        goToControl(MoveDirection::Down, activeSubSec);
    } break;
    }
}

void KeyNavigationController::onUp()
{
    IKeyNavigationSubSection* activeSubSec = activeSubSection();
    if (!activeSubSec) {
        return;
    }

    IKeyNavigationSubSection::Direction direction = activeSubSec->direction();
    switch (direction) {
    case IKeyNavigationSubSection::Direction::Horizontal: {
        NOT_IMPLEMENTED;
    } break;
    case IKeyNavigationSubSection::Direction::Vertical: {
        goToControl(MoveDirection::Up, activeSubSec);
    } break;
    case IKeyNavigationSubSection::Direction::Both: {
        goToControl(MoveDirection::Up, activeSubSec);
    } break;
    }
}

void KeyNavigationController::goToControl(MoveDirection direction, IKeyNavigationSubSection* activeSubSec)
{
    LOGI() << "direction: " << direction;

    if (!activeSubSec) {
        activeSubSec = activeSubSection();
    }

    if (!activeSubSec) {
        return;
    }

    IKeyNavigationControl* activeControl = findActive(activeSubSec->controls());
    if (activeControl) {
        doDeactivateControl(activeControl);
    }

    IKeyNavigationControl* toControl = nullptr;

    switch (direction) {
    case MoveDirection::Right: {
        if (!activeControl) { // no any active
            toControl = firstEnabled(activeSubSec->controls(), IKeyNavigation::Index(), direction);
        } else {
            toControl = nextEnabled(activeSubSec->controls(), activeControl->index(), direction);
            if (!toControl) { // active is last
                IKeyNavigation::Index index = activeControl->index();
                index.column = -1;
                toControl = firstEnabled(activeSubSec->controls(), index, direction); // the first to be the next
            }
        }
    } break;
    case MoveDirection::Down: {
        if (!activeControl) { // no any active
            toControl = firstEnabled(activeSubSec->controls(), IKeyNavigation::Index(), direction);
        } else {
            toControl = nextEnabled(activeSubSec->controls(), activeControl->index(), direction);
            if (!toControl) { // active is last
                IKeyNavigation::Index index = activeControl->index();
                index.row = -1;
                toControl = firstEnabled(activeSubSec->controls(), index, direction); // the first to be the next
            }
        }
    } break;
    case MoveDirection::Left: {
        if (!activeControl) { // no any active
            toControl = lastEnabled(activeSubSec->controls(), IKeyNavigation::Index(), direction);
        } else {
            toControl = prevEnabled(activeSubSec->controls(), activeControl->index(), direction);
            if (!toControl) { // active is first
                IKeyNavigation::Index index = activeControl->index();
                index.column = std::numeric_limits<int>::max();
                toControl = lastEnabled(activeSubSec->controls(), index, direction); // the last to be the next
            }
        }
    } break;
    case MoveDirection::Up: {
        if (!activeControl) { // no any active
            toControl = lastEnabled(activeSubSec->controls(), IKeyNavigation::Index(), direction);
        } else {
            toControl = prevEnabled(activeSubSec->controls(), activeControl->index(), direction);
            if (!toControl) { // active is first
                IKeyNavigation::Index index = activeControl->index();
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

void KeyNavigationController::doTriggerControl()
{
    LOGI() << "====";
    IKeyNavigationControl* activeCtrl= activeControl();
    if (!activeCtrl) {
        return;
    }

    LOGD() << "triggered control: " << activeCtrl->name();
    activeCtrl->trigger();
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

    sec->setActive(true);
    LOGD() << "activated section: " << sec->name() << ", order: " << sec->index().order();

    IKeyNavigationSubSection* activeSub = findActive(sec->subsections());
    if (activeSub && activeSub != sub) {
        doDeactivateSubSection(activeSub);
    }

    sub->setActive(true);
    LOGD() << "activated subsection: " << sub->name() << ", order: " << sub->index().order();

    IKeyNavigationControl* activeCtrl = findActive(sub->controls());
    if (activeCtrl && activeCtrl != ctrl) {
        activeCtrl->setActive(false);
    }

    ctrl->setActive(true);
    LOGD() << "activated control: " << ctrl->name() << ", row: " << ctrl->index().row << ", column: " << ctrl->index().column;
}
