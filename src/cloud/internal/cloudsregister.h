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

#ifndef MU_CLOUD_CLOUDSREGISTER_H
#define MU_CLOUD_CLOUDSREGISTER_H

#include <unordered_map>

#include "../icloudsregister.h"

namespace mu::cloud {
class CloudsRegister : public ICloudsRegister
{
public:
    void reg(const QString& cloudCode, IAuthorizationServicePtr cloud) override;

    IAuthorizationServicePtr cloud(const QString& cloudCode) const override;
    std::vector<IAuthorizationServicePtr> clouds() const override;

private:
    std::unordered_map<QString, IAuthorizationServicePtr> m_clouds;
};
}

#endif // MU_CLOUD_CLOUDSREGISTER_H
