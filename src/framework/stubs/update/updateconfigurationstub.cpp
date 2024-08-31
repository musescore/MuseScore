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

using namespace muse::update;

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

void UpdateConfigurationStub::setSkippedReleaseVersion(const std::string&)
{
}

std::string UpdateConfigurationStub::lastShownMuseSoundsReleaseVersion() const
{
    return "";
}

void UpdateConfigurationStub::setLastShownMuseSoundsReleaseVersion(const std::string&)
{
}

std::string UpdateConfigurationStub::checkForAppUpdateUrl() const
{
    return "";
}

std::string UpdateConfigurationStub::previousAppReleasesNotesUrl() const
{
    return "";
}

std::string UpdateConfigurationStub::checkForMuseSamplerUpdateUrl() const
{
    return "";
}

muse::network::RequestHeaders UpdateConfigurationStub::updateHeaders() const
{
    return muse::network::RequestHeaders();
}

std::string UpdateConfigurationStub::museScoreUrl() const
{
    return "";
}

std::string UpdateConfigurationStub::museScorePrivacyPolicyUrl() const
{
    return "";
}

muse::io::path_t UpdateConfigurationStub::updateDataPath() const
{
    return "";
}

muse::io::path_t UpdateConfigurationStub::updateRequestHistoryJsonPath() const
{
    return "";
}
