/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include <gtest/gtest.h>

#include "dom/engravingitem.h"
#include "dom/factory.h"
#include "dom/masterscore.h"

#include "utils/scorerw.h"
#include "engraving/compat/scoreaccess.h"

using namespace mu::engraving;

class Engraving_ElementTests : public ::testing::Test
{
};

TEST_F(Engraving_ElementTests, DISABLED_testIds)
{
    ElementType ids[] = {
        ElementType::VOLTA,
        ElementType::OTTAVA,
        ElementType::TEXTLINE,
        ElementType::TRILL,
        ElementType::PEDAL,
        ElementType::HAIRPIN,
        ElementType::CLEF,
        ElementType::KEYSIG,
        ElementType::TIMESIG,
        ElementType::BAR_LINE,
        ElementType::ARPEGGIO,
        ElementType::BREATH,
        ElementType::GLISSANDO,
//            ElementType::BRACKET,
        ElementType::ARTICULATION,
        ElementType::CHORDLINE,
        ElementType::ACCIDENTAL,
        ElementType::DYNAMIC,
        ElementType::TEXT,
        ElementType::INSTRUMENT_NAME,
        ElementType::STAFF_TEXT,
        ElementType::PLAYTECH_ANNOTATION,
        ElementType::REHEARSAL_MARK,
        ElementType::INSTRUMENT_CHANGE,
        ElementType::NOTEHEAD,
        ElementType::NOTEDOT,
        ElementType::LAYOUT_BREAK,
        ElementType::MARKER,
        ElementType::JUMP,
        ElementType::MEASURE_REPEAT,
        ElementType::ACTION_ICON,
        ElementType::NOTE,
        ElementType::SYMBOL,
        ElementType::FSYMBOL,
        ElementType::CHORD,
        ElementType::REST,
        ElementType::SPACER,
        ElementType::STAFF_STATE,
        ElementType::TEMPO_TEXT,
        ElementType::HARMONY,
        ElementType::FRET_DIAGRAM,
        ElementType::BEND,
        ElementType::TREMOLOBAR,
        ElementType::LYRICS,
        ElementType::FIGURED_BASS,
        ElementType::STEM,
        ElementType::SLUR,
        ElementType::FINGERING,
        ElementType::HBOX,
        ElementType::VBOX,
        ElementType::TBOX,
        ElementType::FBOX,
        ElementType::MEASURE,
        ElementType::TAB_DURATION_SYMBOL,
    };

    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);
    for (ElementType t : ids) {
        EngravingItem* e = Factory::createItem(t, score->dummy());
        EngravingItem* ee = ScoreRW::writeReadElement(e);
        EXPECT_EQ(e->type(), ee->type());
        delete e;
        delete ee;
    }
}
