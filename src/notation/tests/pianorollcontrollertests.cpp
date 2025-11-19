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

#include "gtest/gtest.h"
#include "notation/view/pianoroll/pianorollcontroller.h"
#include "mocks/notationmock.h"
#include "mocks/notationinteractionmock.h"
#include "mocks/notationnoteinputmock.h"
#include "engraving/dom/drumset.h"

using namespace mu::notation;
using namespace testing;

TEST(PianoRollControllerTests, TestDrumNames)
{
    PianoRollController controller;

    auto notation = std::make_shared<NiceMock<NotationMock>>();
    auto interaction = std::make_shared<NiceMock<NotationInteractionMock>>();
    auto noteInput = std::make_shared<NiceMock<NotationNoteInputMock>>();
    mu::engraving::Drumset drumset;
    drumset.drum(35).name = "Bass Drum";
    drumset.drum(38).name = "Snare";
    NoteInputState state;
    state.setDrumset(&drumset);

    EXPECT_CALL(*notation, interaction()).WillRepeatedly(Return(interaction.get()));
    EXPECT_CALL(*interaction, noteInput()).WillRepeatedly(Return(noteInput.get()));
    EXPECT_CALL(*noteInput, state()).WillRepeatedly(ReturnRef(state));

    controller.setNotation(notation);

    QStringList drumNames = controller.drumNames();
    ASSERT_EQ(drumNames.size(), 2);
    ASSERT_EQ(drumNames[0], "Bass Drum");
    ASSERT_EQ(drumNames[1], "Snare");
}
