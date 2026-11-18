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

#include <gtest/gtest.h>

#include <QVariantMap>

#include "notationscene/utilities/percussionutilities.h"

using namespace muse;
using namespace mu::engraving;
using namespace mu::notation;

namespace mu::notation {
class PercussionUtilitiesTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        addDrum(SNARE, u"Snare", u"S");
        addDrum(KICK, u"Kick", u"K");
    }

    void addDrum(int pitch, const String& name, const String& shortcut)
    {
        DrumInstrument drum;
        drum.name = name;
        drum.shortcut = shortcut;
        m_drumset.setDrum(pitch, drum);
    }

    static Val dialogResult(const QString& newShortcut, int conflictDrumPitch)
    {
        return Val::fromQVariant(QVariantMap {
                { "newShortcut", newShortcut },
                { "conflictDrumPitch", conflictDrumPitch },
            });
    }

    static constexpr int SNARE = 38;
    static constexpr int KICK = 36;

    Drumset m_drumset;
};
}

TEST_F(PercussionUtilitiesTests, ApplyPercussionShortcut_NoConflict_SetsTheShortcut)
{
    //! [WHEN] The user saves a free shortcut
    PercussionUtilities::applyPercussionShortcut(m_drumset, SNARE, dialogResult("D", -1));

    //! [THEN] The drum gets it, the other drums keep theirs
    EXPECT_EQ(m_drumset.shortcut(SNARE), u"D");
    EXPECT_EQ(m_drumset.shortcut(KICK), u"K");
}

TEST_F(PercussionUtilitiesTests, ApplyPercussionShortcut_Conflict_TakesTheShortcutFromTheOtherDrum)
{
    //! [WHEN] The user saves the shortcut of another drum
    PercussionUtilities::applyPercussionShortcut(m_drumset, SNARE, dialogResult("K", KICK));

    //! [THEN] The drum gets it, and the other drum loses it
    EXPECT_EQ(m_drumset.shortcut(SNARE), u"K");
    EXPECT_TRUE(m_drumset.shortcut(KICK).isEmpty());
}

TEST_F(PercussionUtilitiesTests, ApplyPercussionShortcut_Cleared_RemovesTheShortcut)
{
    //! [WHEN] The user clears the shortcut
    PercussionUtilities::applyPercussionShortcut(m_drumset, SNARE, dialogResult("", -1));

    //! [THEN] The drum has no shortcut anymore
    EXPECT_TRUE(m_drumset.shortcut(SNARE).isEmpty());
    EXPECT_EQ(m_drumset.shortcut(KICK), u"K");
}
