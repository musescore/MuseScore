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

#include "notation/inotation.h"

namespace mu::notation {
class NotationMock : public INotation
{
public:
    MOCK_METHOD(project::INotationProject*, project, (), (const, override));
    MOCK_METHOD(IMasterNotation*, masterNotation, (), (const, override));

    MOCK_METHOD(QString, name, (), (const, override));
    MOCK_METHOD(QString, projectName, (), (const, override));
    MOCK_METHOD(QString, projectNameAndPartName, (), (const, override));
    MOCK_METHOD(QString, workTitle, (), (const, override));
    MOCK_METHOD(QString, projectWorkTitle, (), (const, override));
    MOCK_METHOD(QString, projectWorkTitleAndPartName, (), (const, override));

    MOCK_METHOD(bool, isOpen, (), (const, override));
    MOCK_METHOD(void, setIsOpen, (bool), (override));
    MOCK_METHOD(muse::async::Notification, openChanged, (), (const, override));

    MOCK_METHOD(bool, hasVisibleParts, (), (const, override));
    MOCK_METHOD(bool, isMaster, (), (const, override));

    MOCK_METHOD(ViewMode, viewMode, (), (const, override));
    MOCK_METHOD(void, setViewMode, (const ViewMode&), (override));
    MOCK_METHOD(muse::async::Notification, viewModeChanged, (), (const, override));

    MOCK_METHOD(INotationPaintingPtr, painting, (), (const, override));
    MOCK_METHOD(INotationViewStatePtr, viewState, (), (const, override));
    MOCK_METHOD(INotationSoloMuteStatePtr, soloMuteState, (), (const, override));
    MOCK_METHOD(INotationInteractionPtr, interaction, (), (const, override));
    MOCK_METHOD(INotationMidiInputPtr, midiInput, (), (const, override));
    MOCK_METHOD(INotationUndoStackPtr, undoStack, (), (const, override));
    MOCK_METHOD(INotationStylePtr, style, (), (const, override));
    MOCK_METHOD(INotationElementsPtr, elements, (), (const, override));
    MOCK_METHOD(INotationAccessibilityPtr, accessibility, (), (const, override));
    MOCK_METHOD(INotationPartsPtr, parts, (), (const, override));
    MOCK_METHOD(muse::async::Notification, notationChanged, (), (const, override));
};
}
