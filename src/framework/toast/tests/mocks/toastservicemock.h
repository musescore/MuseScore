/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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

#include "toast/itoastservice.h"

namespace muse::toast {
class ToastServiceMock : public IToastService
{
public:
    MOCK_METHOD(async::Promise<ToastResult>, show, (const std::string&, const std::string&, muse::ui::IconCode::Code, bool,
                                                    const std::vector<ToastAction>&), (override));
    MOCK_METHOD(async::Promise<ToastResult>, showWithTimeout, (const std::string&, const std::string&, std::chrono::seconds,
                                                               muse::ui::IconCode::Code, bool, const std::vector<ToastAction>&),
                (override));
    MOCK_METHOD(void, showSuccess, (const std::string&, const std::string&), (override));
    MOCK_METHOD(void, showError, (const std::string&, const std::string&), (override));
    MOCK_METHOD(void, showInfo, (const std::string&, const std::string&), (override));
    MOCK_METHOD(void, showWarning, (const std::string&, const std::string&), (override));
    MOCK_METHOD(async::Promise<ToastResult>, showWithProgress, (const std::string&, const std::string&, std::shared_ptr<muse::Progress>,
                                                                muse::ui::IconCode::Code, bool, const std::vector<ToastAction>&, bool),
                (override));
};
}
