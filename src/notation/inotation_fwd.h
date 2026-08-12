/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include <memory>

namespace mu::notation {
class INotation;
using INotationPtr = std::shared_ptr<INotation>;
using INotationWeakPtr = std::weak_ptr<INotation>;
using INotationPtrList = std::vector<INotationPtr>;

class IMasterNotation;
using IMasterNotationPtr = std::shared_ptr<IMasterNotation>;

class IExcerptNotation;
using IExcerptNotationPtr = std::shared_ptr<IExcerptNotation>;
using ExcerptNotationList = std::vector<IExcerptNotationPtr>;

class INotationPainting;
using INotationPaintingPtr = std::shared_ptr<INotationPainting>;

class INotationViewState;
using INotationViewStatePtr = std::shared_ptr<INotationViewState>;

class INotationSoloMuteState;
using INotationSoloMuteStatePtr = std::shared_ptr<INotationSoloMuteState>;

class INotationInteraction;
using INotationInteractionPtr = std::shared_ptr<INotationInteraction>;

class INotationSelection;
using INotationSelectionPtr = std::shared_ptr<INotationSelection>;

class INotationSelectionRange;
using INotationSelectionRangePtr = std::shared_ptr<INotationSelectionRange>;

class INotationSelectionFilter;
using INotationSelectionFilterPtr = std::shared_ptr<INotationSelectionFilter>;

class INotationNoteInput;
using INotationNoteInputPtr = std::shared_ptr<INotationNoteInput>;

class INotationMidiInput;
using INotationMidiInputPtr = std::shared_ptr<INotationMidiInput>;

class INotationUndoStack;
using INotationUndoStackPtr = std::shared_ptr<INotationUndoStack>;

class INotationStyle;
using INotationStylePtr = std::shared_ptr<INotationStyle>;

class INotationElements;
using INotationElementsPtr = std::shared_ptr<INotationElements>;

class INotationAccessibility;
using INotationAccessibilityPtr = std::shared_ptr<INotationAccessibility>;

class INotationParts;
using INotationPartsPtr = std::shared_ptr<INotationParts>;

class INotationPlayback;
using INotationPlaybackPtr = std::shared_ptr<INotationPlayback>;

class INotationAutomation;
using INotationAutomationPtr = std::shared_ptr<INotationAutomation>;
}
