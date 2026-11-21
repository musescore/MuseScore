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

#include "editpracticeemphasis.h"

using namespace mu::engraving;

ChangePracticeEmphasis::ChangePracticeEmphasis(MasterScore* score, PracticeEmphasisData::Document document)
    : m_score(score), m_document(std::move(document))
{
}

void ChangePracticeEmphasis::flip()
{
    if (!m_score) {
        return;
    }

    PracticeEmphasisData& data = m_score->practiceEmphasis();
    PracticeEmphasisData::Document current = data.document();
    data.applyUndoableSnapshot(std::move(m_document));
    m_document = std::move(current);
}
