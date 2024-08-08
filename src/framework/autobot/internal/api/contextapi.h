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
#ifndef MUSE_AUTOBOT_API_CONTEXTAPI_H
#define MUSE_AUTOBOT_API_CONTEXTAPI_H

#include <QJSValue>
#include <QHash>

#include "api/apiobject.h"

#include "modularity/ioc.h"
#include "autobot/iautobot.h"

namespace muse::api {
class ContextApi : public ApiObject
{
    Q_OBJECT

    Inject<autobot::IAutobot> autobot = { this };
public:
    explicit ContextApi(IApiEngine* e);

    Q_INVOKABLE void setGlobalVal(const QString& key, const QJSValue& val);
    Q_INVOKABLE QJSValue globalVal(const QString& key) const;

    // work with current (last) step
    Q_INVOKABLE void setStepVal(const QString& key, const QJSValue& val);

    Q_INVOKABLE QJSValue stepVal(const QString& stepName, const QString& key) const;
    Q_INVOKABLE QJSValue findVal(const QString& key) const;
};
}

#endif // MUSE_AUTOBOT_API_CONTEXTAPI_H
