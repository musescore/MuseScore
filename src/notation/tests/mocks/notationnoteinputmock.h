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

#include "gmock/gmock.h"
#include "notation/inotationnoteinput.h"

namespace mu::notation {

class NotationNoteInputMock : public INotationNoteInput {
public:
    MOCK_METHOD(bool, isNoteInputMode, (), (const, override));
    MOCK_METHOD(const NoteInputState&, state, (), (const, override));
    MOCK_METHOD(void, setDrumNote, (int), (override));
    MOCK_METHOD(void, setCurrentTrack, (int), (override));
    MOCK_METHOD(muse::async::INotifier*, stateChanged, (), (const, override));
};

} // namespace mu::notation
