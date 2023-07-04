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
#include "updateconfigurationstub.h"

using namespace mu::update;

bool UpdateConfigurationStub::isAppUpdatable() const
{
    return false;
}

bool UpdateConfigurationStub::allowUpdateOnPreRelease() const
{
    return false;
}

void UpdateConfigurationStub::setAllowUpdateOnPreRelease(bool)
{
}

bool UpdateConfigurationStub::needCheckForUpdate() const
{
    return false;
}

void UpdateConfigurationStub::setNeedCheckForUpdate(bool)
{
}

std::string UpdateConfigurationStub::skippedReleaseVersion() const
{
    return "";
}

void UpdateConfigurationStub::setSkippedReleaseVersion(const std::string&) const
{
}

std::string UpdateConfigurationStub::checkForUpdateUrl() const
{
    return "";
}

mu::network::RequestHeaders UpdateConfigurationStub::checkForUpdateHeaders() const
{
    return network::RequestHeaders();
}

std::string UpdateConfigurationStub::museScoreUrl() const
{
    return "";
}

std::string UpdateConfigurationStub::museScorePrivacyPolicyUrl() const
{
    return "";
}

mu::io::path_t UpdateConfigurationStub::updateDataPath() const
{
    return "";
}
