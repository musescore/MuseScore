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

#include "multiwindows/imultiwindowsprovider.h"

namespace muse::mi {
class MultiWindowsProviderMock : public IMultiWindowsProvider
{
public:
    MOCK_METHOD(size_t, windowCount, (), (const, override));
    MOCK_METHOD(bool, isFirstWindow, (), (const, override));
    MOCK_METHOD(bool, isProjectAlreadyOpened, (const io::path_t&), (const, override));
    MOCK_METHOD(void, activateWindowWithProject, (const io::path_t&), (override));
    MOCK_METHOD(bool, isHasWindowWithoutProject, (), (const, override));
    MOCK_METHOD(void, activateWindowWithoutProject, (const QStringList&), (override));
    MOCK_METHOD(bool, openNewWindow, (const QStringList&), (override));
    MOCK_METHOD(bool, isPreferencesAlreadyOpened, (), (const, override));
    MOCK_METHOD(void, activateWindowWithOpenedPreferences, (), (const, override));
    MOCK_METHOD(void, settingsBeginTransaction, (), (override));
    MOCK_METHOD(void, settingsCommitTransaction, (), (override));
    MOCK_METHOD(void, settingsRollbackTransaction, (), (override));
    MOCK_METHOD(void, settingsReset, (), (override));
    MOCK_METHOD(void, settingsSetValue, (const std::string&, const Val&), (override));
    MOCK_METHOD(bool, lockResource, (const std::string&), (override));
    MOCK_METHOD(bool, unlockResource, (const std::string&), (override));
    MOCK_METHOD(void, notifyAboutResourceChanged, (const std::string&), (override));
    MOCK_METHOD(async::Channel<std::string>, resourceChanged, (), (override));
    MOCK_METHOD(void, notifyAboutWindowWasQuited, (), (override));
    MOCK_METHOD(void, quitForAll, (), (override));
    MOCK_METHOD(void, quitWindow, (const modularity::ContextPtr&), (override));
    MOCK_METHOD(void, quitAllAndRestartLast, (), (override));
    MOCK_METHOD(void, quitAllAndRunInstallation, (const io::path_t&), (override));
};
}
