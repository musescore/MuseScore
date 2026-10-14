/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#pragma once

#include <gmock/gmock.h>

#include "musesounds/imusesoundsconfiguration.h"

namespace mu::musesounds {
class MuseSoundsConfigurationMock : public IMuseSoundsConfiguration
{
public:
    MOCK_METHOD(muse::network::RequestHeaders, headers, (), (const, override));

    MOCK_METHOD(muse::UriQuery, soundsUri, (), (const, override));
    MOCK_METHOD(muse::UriQuery, soundPageUri, (const muse::String&), (const, override));

    MOCK_METHOD(bool, needCheckForMuseSoundsUpdate, (), (const, override));

    MOCK_METHOD(QUrl, checkForMuseSoundsUpdateUrl, (), (const, override));
    MOCK_METHOD(QUrl, checkForMuseSamplerUpdateUrl, (), (const, override));
    MOCK_METHOD(QUrl, museSoundsTroubleshootUrl, (), (const, override));

    MOCK_METHOD(QString, getMuseSamplerVersionQuery, (), (const, override));

    MOCK_METHOD(std::string, lastShownMuseSoundsReleaseVersion, (), (const, override));
    MOCK_METHOD(void, setLastShownMuseSoundsReleaseVersion, (const std::string&), (override));

    MOCK_METHOD(bool, showLoadingFailedWarning, (), (const, override));
    MOCK_METHOD(void, setShowLoadingFailedWarning, (bool), (override));

    MOCK_METHOD(bool, museSoundsCheckForUpdateTestMode, (), (const, override));

    MOCK_METHOD(bool, getSoundsTestMode, (), (const, override));
    MOCK_METHOD(void, setSoundsTestMode, (bool), (override));
};
}
