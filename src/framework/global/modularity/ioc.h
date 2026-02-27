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
class QQmlContext;
#endif

#include "../thirdparty/kors_modularity/modularity/ioc.h" // IWYU pragma: export

namespace muse::modularity {
#ifdef IOC_CHECK_INTERFACE_TYPE
using ModulesGlobalIoC = kors::modularity::ModulesGlobalIoC;
using ModulesContextIoC = kors::modularity::ModulesContextIoC;
#else
using ModulesGlobalIoC = kors::modularity::ModulesIoCBase;
using ModulesContextIoC = kors::modularity::ModulesIoCBase;
#endif
using kors::modularity::IoCID;
using kors::modularity::Context;
using kors::modularity::ContextPtr;

inline muse::modularity::ContextPtr globalCtx()
{
    return kors::modularity::globalCtx;
}

inline ModulesGlobalIoC* globalIoc()
{
    return kors::modularity::globalIoc();
}

inline ModulesContextIoC* ioc(const ContextPtr& ctx)
{
    return kors::modularity::ioc(ctx);
}

inline void removeIoC(const ContextPtr& ctx = nullptr)
{
    kors::modularity::removeIoC(ctx);
}

inline void resetAll()
{
    kors::modularity::resetAll();
}
}

namespace muse {
using kors::modularity::GlobalInject;
using kors::modularity::GlobalThreadSafeInject;
using kors::modularity::ContextInject;
using kors::modularity::ContextThreadSafeInject;
using kors::modularity::Contextable;

//! NOTE Temporary for compatibility
using kors::modularity::Inject;
using kors::modularity::ThreadSafeInject;
using kors::modularity::Injectable;
//! ----

#ifndef NO_QT_SUPPORT
struct QmlIoCContext : public QObject
{
    Q_OBJECT
public:
    QmlIoCContext(QObject* p)
        : QObject(p) {}

    modularity::ContextPtr ctx;
};

Contextable::GetContext iocCtxForQmlObject(const QObject* o);
modularity::ContextPtr iocCtxForQmlContext(const QQmlContext* c);
modularity::ContextPtr iocCtxForQWidget(const QWidget* o);
#endif
}
