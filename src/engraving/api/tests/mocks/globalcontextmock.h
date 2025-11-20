/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "context/iglobalcontext.h"

namespace mu::context {
class GlobalContextMock : public IGlobalContext
{
public:
    MOCK_METHOD(void, setCurrentProject, (const project::INotationProjectPtr&), (override));
    MOCK_METHOD(project::INotationProjectPtr, currentProject, (), (const, override));
    MOCK_METHOD(muse::async::Notification, currentProjectChanged, (), (const, override));

    MOCK_METHOD(notation::IMasterNotationPtr, currentMasterNotation, (), (const, override));
    MOCK_METHOD(muse::async::Notification, currentMasterNotationChanged, (), (const, override));

    MOCK_METHOD(void, setCurrentNotation, (const notation::INotationPtr&), (override));
    MOCK_METHOD(notation::INotationPtr, currentNotation, (), (const, override));
    MOCK_METHOD(muse::async::Notification, currentNotationChanged, (), (const, override));

    MOCK_METHOD(void, setCurrentPlayer, (const muse::audio::IPlayerPtr&), (override));
    MOCK_METHOD(IPlaybackStatePtr, playbackState, (), (const, override));
};
}
