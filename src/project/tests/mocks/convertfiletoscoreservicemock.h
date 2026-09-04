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

#include "project/iconvertfiletoscoreservice.h"

namespace mu::project {
class ConvertFileToScoreServiceMock : public IConvertFileToScoreService
{
public:
    MOCK_METHOD(const ConvertConfig&, config, (), (const, override));

    MOCK_METHOD(bool, isFileSupported, (const muse::io::path_t&), (const, override));
    MOCK_METHOD(muse::RetVal<ConvertFilesValidation>, validateFiles, (const muse::io::paths_t&), (const, override));
    MOCK_METHOD(muse::Ret, validateLink, (const QUrl&), (const, override));

    MOCK_METHOD(muse::Ret, startConvert, (const ConvertInput&, const muse::String&), (override));
    MOCK_METHOD((muse::async::Channel<muse::Ret, muse::io::path_t>), convertFinished, (), (const, override));

    MOCK_METHOD(muse::StringList, fileNamesBeingConverted, (), (const, override));
    MOCK_METHOD(muse::async::Notification, fileNamesBeingConvertedChanged, (), (const, override));

    MOCK_METHOD((muse::async::Channel<ConvertType, int>), reviewRequested, (), (const, override));
    MOCK_METHOD(void, submitReview, (ConvertType, int, ReviewRating, const QString&), (override));
    MOCK_METHOD(void, submitReviewComment, (ConvertType, int, const QString&), (override));
};
}
