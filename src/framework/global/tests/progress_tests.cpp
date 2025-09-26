/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include "global/progress.h"
#include "global/async/asyncable.h"

using namespace muse;

class Global_Types_ProgressTests : public ::testing::Test, public async::Asyncable
{
public:
};

TEST_F(Global_Types_ProgressTests, RegularIncrementsAreForwarded)
{
    Progress progress;

    auto notificationCount = 0;
    progress.progressChanged().onReceive(this, [&notificationCount](auto, auto, auto) {
        ++notificationCount;
    });
    progress.start();

    EXPECT_TRUE(progress.progress(0, 10));
    EXPECT_TRUE(progress.progress(1, 10));
    EXPECT_EQ(notificationCount, 2);
}

TEST_F(Global_Types_ProgressTests, DuplicateIncrementsAreDiscarded)
{
    Progress progress;

    auto notificationCount = 0;
    progress.progressChanged().onReceive(this, [&notificationCount](auto, auto, auto) {
        ++notificationCount;
    });
    progress.start();

    EXPECT_TRUE(progress.progress(0, 10));
    EXPECT_FALSE(progress.progress(0, 10));
    EXPECT_EQ(notificationCount, 1);
}

TEST_F(Global_Types_ProgressTests, IncrementsAreResetAfterFinish)
{
    Progress progress;

    auto notificationCount = 0;
    progress.progressChanged().onReceive(this, [&notificationCount](auto, auto, auto) {
        ++notificationCount;
    });
    progress.start();

    EXPECT_TRUE(progress.progress(0, 10));
    progress.finish({});

    progress.start();
    EXPECT_TRUE(progress.progress(0, 10));

    EXPECT_EQ(notificationCount, 2);
}

TEST_F(Global_Types_ProgressTests, IncrementsAreResetAfterCancel)
{
    Progress progress;

    auto notificationCount = 0;
    progress.progressChanged().onReceive(this, [&notificationCount](auto, auto, auto) {
        ++notificationCount;
    });
    progress.start();

    EXPECT_TRUE(progress.progress(0, 10));
    progress.cancel();

    progress.start();
    EXPECT_TRUE(progress.progress(0, 10));

    EXPECT_EQ(notificationCount, 2);
}

TEST_F(Global_Types_ProgressTests, NegativeIncrementsAreForwarded)
{
    Progress progress;

    auto notificationCount = 0;
    progress.progressChanged().onReceive(this, [&notificationCount](auto, auto, auto) {
        ++notificationCount;
    });
    progress.start();

    EXPECT_TRUE(progress.progress(0, 10));
    EXPECT_TRUE(progress.progress(1, 10));
    EXPECT_TRUE(progress.progress(0, 10));

    EXPECT_EQ(notificationCount, 3);
}

TEST_F(Global_Types_ProgressTests, progressReturnsTrueIfAndOnlyIfCallWasForwarded) {
    Progress progress;

    auto notificationCount = 0;
    progress.progressChanged().onReceive(this, [&notificationCount](auto, auto, auto) {
        ++notificationCount;
    });
    progress.start();

    EXPECT_TRUE(progress.progress(0, 10));
    EXPECT_EQ(notificationCount, 1);

    EXPECT_FALSE(progress.progress(0, 10));
    EXPECT_EQ(notificationCount, 1);
}

TEST_F(Global_Types_ProgressTests, IncrementsAreForwardedIfTotalChanged) {
    Progress progress;

    auto notificationCount = 0;
    progress.progressChanged().onReceive(this, [&notificationCount](auto, auto, auto) {
        ++notificationCount;
    });
    progress.start();

    EXPECT_TRUE(progress.progress(0, 10));
    EXPECT_TRUE(progress.progress(0, 11));
    EXPECT_EQ(notificationCount, 2);
}

TEST_F(Global_Types_ProgressTests, maxNumIncrementsIsRespected) {
    Progress progress;

    auto notificationCount = 0;
    progress.progressChanged().onReceive(this, [&notificationCount](auto, auto, auto) {
        ++notificationCount;
    });
    progress.start();
    progress.setMaxNumIncrements(10);
    for (auto i = 0; i < 100; ++i) {
        progress.progress(i, 100);
    }

    EXPECT_EQ(notificationCount, 10);
}
