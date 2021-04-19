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
#ifndef MU_UI_IKEYNAVIGATION_H
#define MU_UI_IKEYNAVIGATION_H

#include <tuple>
#include <memory>
#include <QString>
#include <QList>
#include "async/channel.h"
#include "async/notification.h"

namespace mu::ui {
class IKeyNavigationSection;
class IKeyNavigationSubSection;
class IKeyNavigationControl;

using SubSectionControl = std::tuple<IKeyNavigationSubSection*, IKeyNavigationControl*>;
using SectionSubSectionControl = std::tuple<IKeyNavigationSection*, IKeyNavigationSubSection*, IKeyNavigationControl*>;

class IKeyNavigation
{
public:
    virtual ~IKeyNavigation() = default;

    struct Event
    {
        //! NOTE Please sync with view/KeyNavigationEvent::Type
        enum Type {
            Undefined = 0,
            Left,
            Right,
            Up,
            Down,
            Escape
        };

        Type type = Undefined;
        bool accepted = false;

        Event(Type t)
            : type(t) {}

        static std::shared_ptr<Event> make(Type t) { return std::make_shared<Event>(t); }
    };
    using EventPtr = std::shared_ptr<Event>;

    struct Index
    {
        int column = -1;
        int row = -1;

        void setOrder(int n) { column = n; }
        int order() const { return column; }
    };

    virtual QString name() const = 0;

    virtual const Index& index() const = 0;
    virtual async::Channel<Index> indexChanged() const = 0;

    virtual bool enabled() const = 0;
    virtual async::Channel<bool> enabledChanged() const = 0;

    virtual bool active() const = 0;
    virtual void setActive(bool arg) = 0;
    virtual async::Channel<bool> activeChanged() const = 0;

    virtual void onEvent(EventPtr e) = 0;
};

class IKeyNavigationControl : public IKeyNavigation
{
public:
    virtual ~IKeyNavigationControl() = default;

    virtual void trigger() = 0;
    virtual async::Channel<IKeyNavigationControl*> forceActiveRequested() const = 0;
};

class IKeyNavigationSubSection : public IKeyNavigation
{
public:
    virtual ~IKeyNavigationSubSection() = default;

    //! NOTE Please sync with view/KeyNavigationSubSection::Direction
    enum class Direction {
        Horizontal = 0,
        Vertical,
        Both
    };

    virtual Direction direction() const = 0;
    virtual const std::set<IKeyNavigationControl*>& controls() const = 0;
    virtual async::Notification controlsListChanged() const = 0;
    virtual async::Channel<SubSectionControl> forceActiveRequested() const = 0;
};

class IKeyNavigationSection : public IKeyNavigation
{
public:
    virtual ~IKeyNavigationSection() = default;

    virtual const std::set<IKeyNavigationSubSection*>& subsections() const = 0;
    virtual async::Notification subsectionsListChanged() const = 0;
    virtual async::Channel<SectionSubSectionControl> forceActiveRequested() const = 0;
};
}

#endif // MU_UI_IKEYNAVIGATION_H
