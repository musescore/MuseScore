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

#ifndef MU_CLOUD_ICLOUDSSREGISTER_H
#define MU_CLOUD_ICLOUDSSREGISTER_H

#include "modularity/imoduleexport.h"
#include "iauthorizationservice.h"

namespace mu::cloud {
class ICloudsRegister : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(ICloudsRegister)

public:
    virtual ~ICloudsRegister() = default;

    virtual void reg(const QString& cloudCode, IAuthorizationServicePtr cloud) = 0;
    virtual IAuthorizationServicePtr cloud(const QString& cloudCode) const = 0;

    virtual std::vector<IAuthorizationServicePtr> clouds() const = 0;
};
}

#endif // MU_CLOUD_ICLOUDSSREGISTER_H
