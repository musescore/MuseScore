/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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

#include <QQmlEngine>

#include "plugins/view/pluginview.h"
#include "plugins/api/qmlplugin.h"

using namespace mu;
using namespace mu::plugins;

class Plugins_ApiTests : public ::testing::Test
{
public:
};

TEST_F(Plugins_ApiTests, Enums)
{
    PluginView view;

    Ret ret = view.load(QUrl::fromLocalFile(plugins_tests_DATA_ROOT "/api_data/enums/enums.qml"));
    ASSERT_TRUE(ret);

    view.run();
    // Should not trigger assertion failures

    EXPECT_EQ(view.qmlPlugin()->property("errorCount").toInt(), 0);
}
