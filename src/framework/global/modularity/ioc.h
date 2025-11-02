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

#ifndef NO_QT_SUPPORT
#include <QObject>
class QQmlEngine;
#endif

#include "../thirdparty/kors_modularity/modularity/ioc.h" // IWYU pragma: export

namespace muse::modularity {
using kors::modularity::ModulesIoC;
using kors::modularity::Context;
using kors::modularity::ContextPtr;

using kors::modularity::Creator;

inline ModulesIoC* _ioc(const ContextPtr& ctx = nullptr)
{
    return kors::modularity::_ioc(ctx);
}

inline ModulesIoC* globalIoc()
{
    return kors::modularity::_ioc(nullptr);
}

inline muse::modularity::ContextPtr globalCtx()
{
    static muse::modularity::ContextPtr ctx = std::make_shared<kors::modularity::Context>();
    return ctx;
}

inline ModulesIoC* fixmeIoc()
{
    return kors::modularity::_ioc(nullptr);
}

inline void removeIoC(const ContextPtr& ctx = nullptr)
{
    kors::modularity::removeIoC(ctx);
}
}

namespace muse {
using kors::modularity::Inject;
using kors::modularity::GlobalInject;
using kors::modularity::ThreadSafeInject;

#define INJECT(Interface, getter) muse::Inject<Interface> getter;
#define INJECT_STATIC(Interface, getter) static inline muse::Inject<Interface> getter;

using kors::modularity::Injectable;

#ifndef NO_QT_SUPPORT
struct QmlIoCContext : public QObject
{
    Q_OBJECT
public:
    QmlIoCContext(QObject* p)
        : QObject(p) {}

    modularity::ContextPtr ctx;
};

Injectable::GetContext iocCtxForQmlObject(const QObject* o);
modularity::ContextPtr iocCtxForQmlEngine(const QQmlEngine* e);
modularity::ContextPtr iocCtxForQWidget(const QWidget* o);
#endif
}
