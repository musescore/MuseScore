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
#ifndef MU_AUTOBOT_AUTOBOTCONFIGURATION_H
#define MU_AUTOBOT_AUTOBOTCONFIGURATION_H

#include "../iautobotconfiguration.h"

namespace mu::autobot {
class AutobotConfiguration : public IAutobotConfiguration
{
public:
    AutobotConfiguration() = default;

    io::path dataPath() const override;

    io::path filesPath() const override;

    io::path drawDataPath() const override;
    io::path fileDrawDataPath(const io::path& filePath) const override;

    io::path reportsPath() const override;
};
}

#endif // MU_AUTOBOT_AUTOBOTCONFIGURATION_H
