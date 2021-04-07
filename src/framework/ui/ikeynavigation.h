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
#ifndef MU_UI_IKEYNAVIGATION_H
#define MU_UI_IKEYNAVIGATION_H

#include <tuple>
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

    virtual QString name() const = 0;
    virtual int order() const = 0;
    virtual bool enabled() const = 0;
    virtual bool active() const = 0;
    virtual void setActive(bool arg) = 0;
    virtual async::Channel<bool> activeChanged() const = 0;
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

    virtual const QSet<IKeyNavigationControl*>& controls() const = 0;
    virtual async::Channel<SubSectionControl> forceActiveRequested() const = 0;
};

class IKeyNavigationSection : public IKeyNavigation
{
public:
    virtual ~IKeyNavigationSection() = default;

    virtual const QSet<IKeyNavigationSubSection*>& subsections() const = 0;
    virtual async::Channel<SectionSubSectionControl> forceActiveRequested() const = 0;
};
}

#endif // MU_UI_IKEYNAVIGATION_H
