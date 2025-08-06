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
#ifndef MUSE_UPDATE_UPDATECONFIGURATIONMOCK_H
#define MUSE_UPDATE_UPDATECONFIGURATIONMOCK_H

#include <gmock/gmock.h>

#include "update/iupdateconfiguration.h"

namespace muse::update {
class UpdateConfigurationMock : public IUpdateConfiguration
{
public:
    MOCK_METHOD(bool, isAppUpdatable, (), (const, override));

    MOCK_METHOD(bool, allowUpdateOnPreRelease, (), (const, override));
    MOCK_METHOD(void, setAllowUpdateOnPreRelease, (bool), (override));

    MOCK_METHOD(bool, needCheckForUpdate, (), (const, override));
    MOCK_METHOD(void, setNeedCheckForUpdate, (bool), (override));
    MOCK_METHOD(muse::async::Notification, needCheckForUpdateChanged, (), (const, override));

    MOCK_METHOD(std::string, skippedReleaseVersion, (), (const, override));
    MOCK_METHOD(void, setSkippedReleaseVersion, (const std::string&), (override));

    MOCK_METHOD(bool, checkForUpdateTestMode, (), (const, override));

    MOCK_METHOD(std::string, checkForAppUpdateUrl, (), (const, override));
    MOCK_METHOD(std::string, previousAppReleasesNotesUrl, (), (const, override));

    MOCK_METHOD(muse::network::RequestHeaders, updateHeaders, (), (const, override));

    MOCK_METHOD(std::string, museScoreUrl, (), (const, override));
    MOCK_METHOD(std::string, museScorePrivacyPolicyUrl, (), (const, override));

    MOCK_METHOD(muse::io::path_t, updateDataPath, (), (const, override));
    MOCK_METHOD(muse::io::path_t, updateRequestHistoryJsonPath, (), (const, override));
};
}

#endif // MUSE_UPDATE_UPDATECONFIGURATIONMOCK_H
