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
#ifndef MU_UI_INAVIGATION_H
#define MU_UI_INAVIGATION_H

#include <tuple>
#include <memory>
#include <QString>
#include <QList>
#include "async/channel.h"
#include "async/notification.h"

namespace mu::ui {
class INavigationSection;
class INavigationPanel;
class INavigationControl;

using PanelControl = std::tuple<INavigationPanel*, INavigationControl*>;
using SectionPanelControl = std::tuple<INavigationSection*, INavigationPanel*, INavigationControl*>;

class INavigation
{
public:
    virtual ~INavigation() = default;

    struct Event
    {
        //! NOTE Please sync with view/NavigationEvent::Type
        enum Type {
            Undefined = 0,
            Left,
            Right,
            Up,
            Down,
            Trigger,
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

class INavigationControl : public INavigation
{
public:
    virtual ~INavigationControl() = default;

    virtual void trigger() = 0;
    virtual async::Channel<INavigationControl*> forceActiveRequested() const = 0;
};

class INavigationPanel : public INavigation
{
public:
    virtual ~INavigationPanel() = default;

    //! NOTE Please sync with view/NavigationPanel::Direction
    enum class Direction {
        Horizontal = 0,
        Vertical,
        Both
    };

    virtual Direction direction() const = 0;
    virtual const std::set<INavigationControl*>& controls() const = 0;
    virtual async::Notification controlsListChanged() const = 0;
    virtual async::Channel<PanelControl> forceActiveRequested() const = 0;
};

class INavigationSection : public INavigation
{
public:
    virtual ~INavigationSection() = default;

    virtual const std::set<INavigationPanel*>& panels() const = 0;
    virtual async::Notification panelsListChanged() const = 0;
    virtual async::Channel<SectionPanelControl> forceActiveRequested() const = 0;
};
}

#endif // MU_UI_INAVIGATION_H
