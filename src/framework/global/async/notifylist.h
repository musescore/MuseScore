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
#ifndef MU_ASYNC_NOTIFYLIST_H
#define MU_ASYNC_NOTIFYLIST_H

#include "thirdparty/deto_async/async/notifylist.h"
#include "thirdparty/deto_async/async/changednotify.h"

namespace mu::async {
template<typename T>
using NotifyList = deto::async::NotifyList<T>;

template<typename T>
using ChangedNotifier = deto::async::ChangedNotifier<T>;
}

#endif // MU_ASYNC_NOTIFYLIST_H
