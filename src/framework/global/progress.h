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
#ifndef MU_FRAMEWORK_PROGRESS_H
#define MU_FRAMEWORK_PROGRESS_H

#include "async/channel.h"
#include "async/notification.h"

#include "ret.h"

namespace mu::framework {
struct Progress
{
    async::Notification started;
    async::Channel<int64_t /*current*/, int64_t /*total*/, std::string /*title*/> progressChanged;
    async::Channel<Ret> finished;
};
}

#endif // MU_FRAMEWORK_PROGRESS_H
