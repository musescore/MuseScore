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

#include "audio/common/alignmentbuffer.h"

using namespace muse::audio;

TEST(Audio_AlignmentBufferTests, Process)
{
    const size_t requiredSamplesTotal = 10 * 2;
    const size_t driverSamplesTotal = 9 * 2;

    AlignmentBuffer abuf(requiredSamplesTotal * 2);

    std::vector<float> drv_buf;
    drv_buf.resize(driverSamplesTotal);

    std::vector<float> proc_buf;
    proc_buf.resize(requiredSamplesTotal);

    // audio loop
    for (int i = 0; i < 40; ++i) {
        if (abuf.availableRead() >= requiredSamplesTotal) {
            abuf.read(&drv_buf[0], driverSamplesTotal);
        } else {
            // process
            for (size_t s = 0; s < requiredSamplesTotal; s += 2) {
                proc_buf[s] = 1.0;
                proc_buf[s + 1] = 2.0;
            }

            abuf.write(&proc_buf[0], requiredSamplesTotal);
            abuf.read(&drv_buf[0], driverSamplesTotal);
        }

        // check
        for (size_t s = 0; s < driverSamplesTotal; s += 2) {
            EXPECT_FLOAT_EQ(drv_buf[s], 1.0);
            EXPECT_FLOAT_EQ(drv_buf[s + 1], 2.0);
        }
    }
}
