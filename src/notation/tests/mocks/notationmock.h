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
#include "notation/inotation.h"
#include "notation/inotationinteraction.h"

namespace mu::notation {

class NotationMock : public INotation {
public:
    MOCK_METHOD(INotationInteraction*, interaction, (), (const, override));
    MOCK_METHOD(INotationUndoStack*, undoStack, (), (const, override));
    MOCK_METHOD(INotationElements*, elements, (), (const, override));
    MOCK_METHOD(INotationParts*, parts, (), (const, override));
    MOCK_METHOD(INotationStyle*, style, (), (const, override));
    MOCK_METHOD(INotationPlayback*, playback, (), (const, override));
    MOCK_METHOD(INotationMidiInput*, midiInput, (), (const, override));
    MOCK_METHOD(INotationAccessibility*, accessibility, (), (const, override));
    MOCK_METHOD(INotationPainting*, painting, (), (const, override));
    MOCK_METHOD(INotationViewState*, viewState, (), (const, override));
    MOCK_METHOD(INotationSoloMuteState*, soloMuteState, (), (const, override));
};

} // namespace mu::notation
