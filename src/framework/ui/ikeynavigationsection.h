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
#ifndef MU_UI_IKEYNAVIGATIONSECTION_H
#define MU_UI_IKEYNAVIGATIONSECTION_H

#include <QString>
#include <QList>

namespace mu::ui {
class IKeyNavigationSubSection
{
public:
    virtual ~IKeyNavigationSubSection() = default;

    virtual QString name() const = 0;
    virtual int order() const = 0;
    virtual bool enabled() const = 0;
    virtual bool active() const = 0;
    virtual void setActive(bool arg) = 0;
};

class IKeyNavigationSection
{
public:
    virtual ~IKeyNavigationSection() = default;

    virtual QString name() const = 0;
    virtual int order() const = 0;
    virtual bool enabled() const = 0;
    virtual bool active() const = 0;
    virtual void setActive(bool arg) = 0;

    virtual const QList<IKeyNavigationSubSection*>& subsections() const = 0;
};
}

#endif // MU_UI_IKEYNAVIGATIONSECTION_H
