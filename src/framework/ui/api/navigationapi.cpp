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

using namespace muse::api;
using namespace muse::ui;

NavigationApi::NavigationApi(api::IApiEngine* e)
    : api::ApiObject(e)
{
}

NavigationApi::~NavigationApi()
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

void NavigationApi::escape()
{
    dispatcher()->dispatch("nav-escape");
}

bool NavigationApi::goToControl(const QString& section, const QString& panel, const QJSValue& controlNameOrIndex)
{
    bool ok = false;
    if (controlNameOrIndex.isString()) {
        ok = navigation()->requestActivateByName(section.toStdString(), panel.toStdString(), controlNameOrIndex.toString().toStdString());
    } else if (controlNameOrIndex.isArray()) {
        if (controlNameOrIndex.property("length").toInt() == 2) {
            INavigation::Index idx;
            idx.row = controlNameOrIndex.property(0).toInt();
            idx.column = controlNameOrIndex.property(1).toInt();
            ok = navigation()->requestActivateByIndex(section.toStdString(), panel.toStdString(), idx);
        } else {
            LOGE() << "bad argument `control`: " << controlNameOrIndex.toString();
            ok = false;
        }
    } else {
        LOGE() << "bad argument `control`: " << controlNameOrIndex.toString();
        ok = false;
    }

    return ok;
}

void NavigationApi::trigger()
{
    dispatcher()->dispatch("nav-trigger-control");
}

bool NavigationApi::triggerControl(const QString& section, const QString& panel, const QJSValue& controlNameOrIndex)
{
    bool ok = goToControl(section, panel, controlNameOrIndex);
    if (ok) {
        trigger();
    }
    return ok;
}

QString NavigationApi::activeSection() const
{
    INavigationSection* sec = navigation()->activeSection();
    return sec ? sec->name() : QString();
}

QString NavigationApi::activePanel() const
{
    INavigationPanel* p = navigation()->activePanel();
    return p ? p->name() : QString();
}

QString NavigationApi::activeControl() const
{
    INavigationControl* c = navigation()->activeControl();
    return c ? c->name() : QString();
}

static QJSValue toQJSValue(const INavigation* n, IApiEngine* e)
{
    QJSValue o = e->newObject();
    o.setProperty("name", n->name());

    QJSValue io = e->newObject();
    io.setProperty("column", n->index().column);
    io.setProperty("row", n->index().row);
    o.setProperty("index", io);

    o.setProperty("enabled", n->enabled());
    o.setProperty("active", n->active());

    return o;
}

QJSValue NavigationApi::sections() const
{
    const std::set<INavigationSection*>& sects = navigation()->sections();
    QJSValue arr = engine()->newArray(sects.size());
    quint32 i = 0;
    for (const INavigationSection* s : sects) {
        arr.setProperty(i, toQJSValue(s, engine()));
        ++i;
    }

    return arr;
}

QJSValue NavigationApi::panels(const QString& sectionName) const
{
    const INavigationSection* s = navigation()->findSection(sectionName.toStdString());
    QJSValue arr = engine()->newArray(s->panels().size());
    quint32 i = 0;
    for (const INavigationPanel* p : s->panels()) {
        arr.setProperty(i, toQJSValue(p, engine()));
        ++i;
    }

    return arr;
}

QJSValue NavigationApi::controls(const QString& sectionName, const QString& panelName) const
{
    const INavigationPanel* p = navigation()->findPanel(sectionName.toStdString(), panelName.toStdString());
    QJSValue arr = engine()->newArray(p->controls().size());
    quint32 i = 0;
    for (const INavigationControl* c : p->controls()) {
        arr.setProperty(i, toQJSValue(c, engine()));
        ++i;
    }

    return arr;
}

void NavigationApi::dump() const
{
    navigation()->dump();
}
