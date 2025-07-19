/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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

#include "audioplugins/internal/audiopluginsutils.h"
#include "audioplugins/audiopluginstypes.h"

using namespace muse::audioplugins;

namespace muse::audioplugins {
class AudioPlugins_AudioUtilsTest : public ::testing::Test
{
public:
};
}

TEST_F(AudioPlugins_AudioUtilsTest, AudioPluginTypeFromCategoriesString)
{
    EXPECT_EQ(AudioPluginType::Fx, audioPluginTypeFromCategoriesString(u"Fx|Delay"));
    EXPECT_EQ(AudioPluginType::Fx, audioPluginTypeFromCategoriesString(u"Test|Fx"));

    EXPECT_EQ(AudioPluginType::Instrument, audioPluginTypeFromCategoriesString(u"Instrument|Test"));
    EXPECT_EQ(AudioPluginType::Instrument, audioPluginTypeFromCategoriesString(u"Test|Instrument"));

    //! NOTE: "Instrument" has the highest priority for compatibility reasons
    EXPECT_EQ(AudioPluginType::Instrument, audioPluginTypeFromCategoriesString(u"Instrument|Fx|Test"));
    EXPECT_EQ(AudioPluginType::Instrument, audioPluginTypeFromCategoriesString(u"Fx|Instrument|Test"));

    EXPECT_EQ(AudioPluginType::Undefined, audioPluginTypeFromCategoriesString(u"Test"));
    EXPECT_EQ(AudioPluginType::Undefined, audioPluginTypeFromCategoriesString(u"FX|Test"));
    EXPECT_EQ(AudioPluginType::Undefined, audioPluginTypeFromCategoriesString(u"INSTRUMENT|Test"));
}
