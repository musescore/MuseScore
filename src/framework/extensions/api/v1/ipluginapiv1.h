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
#ifndef MUSE_EXTENSIONS_APIV1_IPLUGINAPIV1_H
#define MUSE_EXTENSIONS_APIV1_IPLUGINAPIV1_H

#include "global/async/notification.h"

class QQmlEngine;
namespace muse::extensions::apiv1 {
class IPluginApiV1
{
public:

    virtual ~IPluginApiV1() = default;

    virtual void setup(QQmlEngine* e) = 0;

    virtual void runPlugin() = 0;

    virtual async::Notification closeRequest() const = 0;
};
}

#endif // MUSE_EXTENSIONS_APIV1_IPLUGINAPIV1_H
