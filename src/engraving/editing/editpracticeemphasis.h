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

#include "global/allocator.h"

#include "transaction/undoablecommand.h"

#include "engraving/playback/practiceemphasisdata.h"

namespace mu::engraving {
class MasterScore;

class ChangePracticeEmphasis : public UndoableCommand
{
    OBJECT_ALLOCATOR(engraving, ChangePracticeEmphasis)

public:
    ChangePracticeEmphasis(MasterScore* score, PracticeEmphasisData::Document document);

    UNDO_TYPE(CommandType::ChangePracticeEmphasis)
    UNDO_NAME("ChangePracticeEmphasis")
    UNDO_CHANGED_OBJECTS({ m_score })

private:
    void flip() override;

    MasterScore* m_score = nullptr;
    PracticeEmphasisData::Document m_document;
};
}
