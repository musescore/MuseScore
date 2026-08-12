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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <optional>
#include <vector>

#include "engraving/types/types.h"

namespace mu::engraving {
class EngravingItem;
class EngravingObject;

enum class CommandType : signed char {
    Unknown = -1,

    // Parts
    InsertPart,
    RemovePart,
    AddPartToExcerpt,
    SetSoloist,
    ChangePart,
    ConnectSharedPart,
    DisconnectSharedPart,

    // Staves
    InsertStaff,
    RemoveStaff,
    AddSystemObjectStaff,
    RemoveSystemObjectStaff,
    SortStaves,
    ChangeStaff,
    ChangeStaffType,

    // MStaves
    InsertMStaff,
    RemoveMStaff,
    InsertStaves,
    RemoveStaves,
    ChangeMStaffProperties,
    ChangeMStaffHideIfEmpty,

    // Instruments
    ChangeInstrumentShort,
    ChangeInstrumentLong,
    ChangeInstrumentGroupOptions,
    ChangeInstrumentNumber,
    ChangeInstrument,
    ChangeDrumset,

    // Measures
    RemoveMeasures,
    InsertMeasures,
    ChangeMeasureLen,
    ChangeMMRest,
    ChangeMeasureRepeatCount,

    // Elements
    AddElement,
    RemoveElement,
    Unlink,
    Link,
    ChangeElement,
    ChangeParent,

    // Notes
    ChangePitch,
    ChangeFretting,
    ChangeVelocity,

    // ChordRest
    ChangeChordStaffMove,
    SwapCR,
    AddNoteParenthesesInfo,
    RemoveNoteParenthesesInfo,
    RemoveSingleNoteParentheses,

    // Brackets
    RemoveBracket,
    AddBracket,

    // Fret
    FretDataChange,
    FretDot,
    FretMarker,
    FretBarre,
    FretClear,
    AddFretDiagramToFretBox,
    RemoveFretDiagramFromFretBox,

    // Harmony
    TransposeHarmony,

    // KeySig
    ChangeKeySig,

    // Clef
    ChangeClefType,

    // Tremolo
    MoveTremolo,

    // Spanners
    ChangeSpannerElements,
    InsertTimeUnmanagedSpanner,
    ChangeStartEndSpanner,

    // Ties
    ChangeTieEndPointActive,

    // Style
    ChangeStyle,
    ChangeStyleValues,

    // Property
    ChangeProperty,

    // Voices
    ExchangeVoice,

    // Excerpts
    AddExcerpt,
    RemoveExcerpt,
    SwapExcerpt,
    ChangeExcerptTitle,

    // Meta info
    ChangeMetaInfo,

    // Text
    TextEdit,

    // Automation
    EditAutomationPoints,

    // Other
    InsertTime,
    ChangeScoreOrder,
};

enum class UndoableCommandFilter : unsigned char {
    TextEdit,
    AddElement,
    AddElementLinked,
    Link,
    RemoveElement,
    RemoveElementLinked,
    ChangePropertyLinked,
};

#define UNDO_TYPE(t) CommandType type() const override { return t; }
#define UNDO_NAME(a) const char* name() const override { return a; }
#define UNDO_CHANGED_OBJECTS(...) std::vector<EngravingObject*> objectItems() const override { return __VA_ARGS__; }

struct ChangedRange {
    Fraction tickFrom;
    Fraction tickTo;
    staff_idx_t staffIdxFrom = muse::nidx;
    staff_idx_t staffIdxTo = muse::nidx;
};

class UndoableCommand
{
public:
    virtual ~UndoableCommand() = default;

    virtual void undo();
    virtual void redo();

    virtual void cleanup(bool /*wasDone*/) {}

    virtual std::vector<EngravingObject*> objectItems() const { return {}; }
    virtual const char* name() const { return "UndoableCommand"; }
    virtual CommandType type() const { return CommandType::Unknown; }

    virtual std::optional<ChangedRange> changedRange() const { return std::nullopt; }

    virtual bool matchesFilter(UndoableCommandFilter, const EngravingItem* /* target */) const { return false; }

protected:
    virtual void flip() {}
};

std::vector<EngravingObject*> compoundObjects(EngravingObject* object);
}
