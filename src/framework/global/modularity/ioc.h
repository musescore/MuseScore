/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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
#ifndef MU_MODULARITY_IOC_H
#define MU_MODULARITY_IOC_H

#ifndef NO_QT_SUPPORT
#include <QObject>
class QQmlEngine;
#endif

#include "../thirdparty/kors_modularity/modularity/ioc.h" // IWYU pragma: export

namespace muse::modularity {
using ModulesIoC = kors::modularity::ModulesIoC;
using Context = kors::modularity::Context;
using ContextPtr = kors::modularity::ContextPtr;

template<class T>
using Creator = kors::modularity::Creator<T>;

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
template<class I>
using Inject = kors::modularity::Inject<I>;
template<class I>
using GlobalInject = kors::modularity::GlobalInject<I>;

#define INJECT(Interface, getter) muse::Inject<Interface> getter;
#define INJECT_STATIC(Interface, getter) static inline muse::Inject<Interface> getter;

using Injectable = kors::modularity::Injectable;

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

namespace mu {
template<class I>
using Inject = kors::modularity::Inject<I>;
}

#endif // MU_MODULARITY_IOC_H
