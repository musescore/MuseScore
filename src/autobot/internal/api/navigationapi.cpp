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
#include "navigationapi.h"

#include "log.h"

using namespace mu::api;

NavigationApi::NavigationApi(IApiEngine* e)
    : ApiObject(e)
{
}

void NavigationApi::nextPanel()
{
    dispatcher()->dispatch("nav-next-panel");
}

void NavigationApi::prevPanel()
{
    dispatcher()->dispatch("nav-prev-panel");
}

void NavigationApi::right()
{
    dispatcher()->dispatch("nav-right");
}

void NavigationApi::left()
{
    dispatcher()->dispatch("nav-left");
}

void NavigationApi::up()
{
    dispatcher()->dispatch("nav-up");
}

void NavigationApi::down()
{
    dispatcher()->dispatch("nav-down");
}

bool NavigationApi::goToControlByName(const QString& section, const QString& panel, const QString& contol)
{
    bool ok = navigation()->requestActivateByName(section.toStdString(), panel.toStdString(), contol.toStdString());
    return ok;
}

void NavigationApi::triggerCurrentControl()
{
    dispatcher()->dispatch("nav-trigger-control");
}

bool NavigationApi::triggerControlByName(const QString& section, const QString& panel, const QString& contol)
{
    bool ok = goToControlByName(section, panel, contol);
    if (ok) {
        triggerCurrentControl();
    }
    return ok;
}
