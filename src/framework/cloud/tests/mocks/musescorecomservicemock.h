/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
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

#include "cloud/musescorecom/imusescorecomservice.h"

namespace muse::cloud {
class MuseScoreComServiceMock : public IMuseScoreComService
{
public:
    MOCK_METHOD(IAuthorizationServicePtr, authorization, (), (override));

    MOCK_METHOD(IMuseScoreComConvertServicePtr, convert, (), (override));

    MOCK_METHOD(QUrl, scoreManagerUrl, (), (const, override));

    MOCK_METHOD(ProgressPtr, uploadScore, (DevicePtr, const QString&, cloud::Visibility, const QUrl&, int), (override));
    MOCK_METHOD(ProgressPtr, uploadAudio, (DevicePtr, const QString&, const QUrl&), (override));

    MOCK_METHOD(RetVal<ScoreInfo>, downloadScoreInfo, (const QUrl&), (override));
    MOCK_METHOD(RetVal<ScoreInfo>, downloadScoreInfo, (int), (override));

    MOCK_METHOD(async::Promise<ScoresList>, downloadScoresList, (int, int), (override));

    MOCK_METHOD(ProgressPtr, downloadScore, (int, DevicePtr, const QString&, const QString&), (override));
};
}
