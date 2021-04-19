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
#ifndef MU_UI_KEYNAVIGATIONEVENT_H
#define MU_UI_KEYNAVIGATIONEVENT_H

#include <QObject>
#include "ui/ikeynavigation.h"

namespace mu::ui {
class KeyNavigationEvent
{
    Q_GADGET
    Q_PROPERTY(Type type READ type CONSTANT)
    Q_PROPERTY(bool accepted READ accepted WRITE setAccepted)

public:
    KeyNavigationEvent(IKeyNavigation::EventPtr event = nullptr);

    //! NOTE Please sync with IKeyNavigation::Event::Type
    enum Type {
        Undefined = 0,
        Left,
        Right,
        Up,
        Down,
        Escape
    };
    Q_ENUM(Type)

    void setEvent(IKeyNavigation::EventPtr event);

    Type type() const;
    bool accepted() const;
    void setAccepted(bool accepted);

private:
    IKeyNavigation::EventPtr m_event;
};
}
Q_DECLARE_METATYPE(mu::ui::KeyNavigationEvent)

#endif // MU_UI_KEYNAVIGATIONEVENT_H
