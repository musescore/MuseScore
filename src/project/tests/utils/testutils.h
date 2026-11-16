/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include <functional>

#include "async/processevents.h"
#include "async/promise.h"

namespace mu::project {
//! NOTE: a processed message may queue a new one, so process several rounds
inline void pumpEvents(int iterations = 10)
{
    for (int i = 0; i < iterations; ++i) {
        muse::async::processMessages();
    }
}

//! NOTE: keep the body queued, don't run it eagerly
//! otherwise resolve() can fire before onResolve/onReject are attached
template<typename T>
muse::async::Promise<T> resolvedPromise(const T& val)
{
    return muse::async::make_promise<T>([val](auto resolve, auto reject) {
        (void)reject;
        return resolve(val);
    });
}

//! NOTE: a promise that never resolves, for tests that don't care about the outcome
template<typename T>
muse::async::Promise<T> pendingPromise()
{
    return muse::async::make_promise<T>([](auto resolve, auto reject) {
        (void)resolve;
        (void)reject;
        return muse::async::Promise<T>::dummy_result();
    });
}

//! NOTE: resolved manually by calling "resolver", once set (after a pumpEvents())
template<typename T>
muse::async::Promise<T> deferredPromise(std::function<void(const T&)>& resolver)
{
    return muse::async::make_promise<T>([&resolver](auto resolve, auto reject) {
        (void)reject;
        resolver = [resolve](const T& val) { (void)resolve(val); };
        return muse::async::Promise<T>::dummy_result();
    });
}
}
