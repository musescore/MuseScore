/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited and others
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
#pragma once

#include <QJSValue>
#include <QJSEngine>
#include <QObject>
#include "modularity/ioc.h"

namespace muse::api {
class IApiEngine
{
public:
    virtual ~IApiEngine() = default;

    virtual const modularity::ContextPtr& iocContext() const = 0;

    virtual int apiversion() const = 0;

    virtual QJSValue newQObject(QObject* o) = 0;
    virtual QJSValue newObject() = 0;
    virtual QJSValue newArray(size_t length = 0) = 0;
    virtual QJSValue freeze(const QJSValue& val) = 0;
};

class JsApiEngine : public muse::api::IApiEngine
{
public:
    JsApiEngine(QJSEngine* e, const modularity::ContextPtr& iocContext)
        : m_engine(e), m_iocContext(iocContext)
    {
        bool ok = false;
        m_apiversion = m_engine->property("apiversion").toInt(&ok);
        if (!ok) {
            m_apiversion = 2;
        }
        m_freezeFn = m_engine->evaluate("Object.freeze");
    }

    const modularity::ContextPtr& iocContext() const override
    {
        return m_iocContext;
    }

    int apiversion() const override
    {
        return m_apiversion;
    }

    QJSValue newQObject(QObject* o) override
    {
        if (!o->parent()) {
            o->setParent(m_engine);
        }
        return m_engine->newQObject(o);
    }

    QJSValue newObject() override
    {
        return m_engine->newObject();
    }

    QJSValue newArray(size_t length = 0) override
    {
        return m_engine->newArray(uint(length));
    }

    QJSValue freeze(const QJSValue& val) override
    {
        return m_freezeFn.call({ val });
    }

private:
    QJSEngine* m_engine = nullptr;
    const modularity::ContextPtr& m_iocContext;
    mutable int m_apiversion = -1;
    QJSValue m_freezeFn;
};
}
