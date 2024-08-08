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
#ifndef MUSE_SHORTCUTS_API_SHORTCUTSAPI_H
#define MUSE_SHORTCUTS_API_SHORTCUTSAPI_H

#include "api/apiobject.h"

#include "modularity/ioc.h"
#include "shortcuts/ishortcutscontroller.h"

namespace muse::api {
class ShortcutsApi : public api::ApiObject
{
    Q_OBJECT

    Inject<shortcuts::IShortcutsController> shortcutsController = { this };

public:
    explicit ShortcutsApi(api::IApiEngine* e);

    Q_INVOKABLE void activate(const QString& sequence);
};
}

#endif // MUSE_SHORTCUTS_API_SHORTCUTSAPI_H
