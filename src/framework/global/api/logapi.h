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
#ifndef MUSE_GLOBAL_LOGAPI_H
#define MUSE_GLOBAL_LOGAPI_H

#include "api/apiobject.h"

namespace muse::api {
class LogApi : public api::ApiObject
{
    Q_OBJECT
public:
    explicit LogApi(api::IApiEngine* e);

    Q_INVOKABLE void error(const QString& message);
    Q_INVOKABLE void error(const QString& tag, const QString& message);

    Q_INVOKABLE void warn(const QString& message);
    Q_INVOKABLE void warn(const QString& tag, const QString& message);

    Q_INVOKABLE void info(const QString& message);
    Q_INVOKABLE void info(const QString& tag, const QString& message);

    Q_INVOKABLE void debug(const QString& message);
    Q_INVOKABLE void debug(const QString& tag, const QString& message);
};
}

#endif // MUSE_GLOBAL_LOGAPI_H
