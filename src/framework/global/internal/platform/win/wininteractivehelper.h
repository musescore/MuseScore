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
#ifndef MU_FRAMEWORK_WININTERACTIVEHELPER_H
#define MU_FRAMEWORK_WININTERACTIVEHELPER_H

#include "io/path.h"
#include "types/ret.h"
#include "types/uri.h"

#include "async/asyncable.h"
#include "async/promise.h"

namespace muse {
class WinInteractiveHelper : public async::Asyncable
{
public:
    static async::Promise<Ret> openApp(const Uri& uri);
};
}

#endif // MU_FRAMEWORK_WININTERACTIVEHELPER_H
