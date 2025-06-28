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
#ifndef MUSE_ASYNC_PROMISE_H
#define MUSE_ASYNC_PROMISE_H

#include "../thirdparty/kors_async/async/promise.h"

namespace muse::async {
template<typename ... T>
using Promise = kors::async::Promise<T...>;

using PromiseType = kors::async::PromiseType;

template<typename ... T>
auto make_promise = [](typename Promise<T...>::Body f, PromiseType type = PromiseType::AsyncByPromise) {
    return kors::async::make_promise<T...>(f, type);
};
}

#endif // MUSE_ASYNC_PROMISE_H
