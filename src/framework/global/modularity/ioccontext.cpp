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

#include "ioc.h"

#include "log.h"

#ifndef NO_QT_SUPPORT
#include <QQmlEngine>
#include <QQmlContext>
#include <QWidget>

muse::Contextable::GetContext muse::iocCtxForQmlObject(const QObject* o)
{
    return [o]() {
        const QObject* p = o;
        QQmlContext* ctx = qmlContext(p);
        while (!ctx && p->parent()) {
            p = p->parent();
            ctx = qmlContext(p);
        }

        IF_ASSERT_FAILED(ctx) {
            LOGW() << "QQmlContext is not set for QML Object: " << o->metaObject()->className();
            return modularity::ContextPtr();
        }

        return iocCtxForQmlContext(ctx);
    };
}

muse::modularity::ContextPtr muse::iocCtxForQmlContext(const QQmlContext* ctx)
{
    TRACEFUNC;

    //! NOTE: Currently, each IoC context (window)
    // has its own instance of the Qml Engine,
    // and thus its own root Qml Context. We can use it directly.
    const QQmlContext* rootContext = ctx->engine()->rootContext();

    QmlIoCContext* qmlIoc = rootContext->contextProperty("ioc_context").value<QmlIoCContext*>();
    IF_ASSERT_FAILED(qmlIoc) {
        LOGW() << "QmlIoCContext is not set for QML Context: " << rootContext->objectName();
        return modularity::ContextPtr();
    }

    return qmlIoc->ctx;
}

muse::Contextable::GetContext muse::iocCtxForQWidget(const QWidget* w)
{
    return [w]() {
        IF_ASSERT_FAILED(w) {
            return modularity::ContextPtr();
        }

        const QObject* obj = w;
        while (obj) {
            bool ok = false;
            int ctxId = obj->property("ioc_context").toInt(&ok);
            if (ok) {
                return std::make_shared<modularity::Context>(ctxId);
            }
            obj = obj->parent();
        }

        UNREACHABLE;
        return modularity::ContextPtr();
    };
}

#endif
