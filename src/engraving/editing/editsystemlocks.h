/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include <vector>

namespace mu::engraving {
class Measure;
class MeasureBase;
class Score;
class System;
class SystemLock;

enum class LayoutBreakType : unsigned char;

class EditSystemLocks
{
public:
    static void undoAddSystemLock(Score* score, const SystemLock* lock);
    static void undoRemoveSystemLock(Score* score, const SystemLock* lock);
    static void undoRemoveAllLocks(Score* score);

    static void toggleSystemLock(Score* score, const std::vector<System*>& systems);
    static void toggleScoreLock(Score* score);

    static void addRemoveSystemLocks(Score* score, int interval, bool lock);

    static void makeIntoSystem(Score* score, MeasureBase* first, MeasureBase* last);
    static void moveMeasureToPrevSystem(Score* score, MeasureBase* m);
    static void moveMeasureToNextSystem(Score* score, MeasureBase* m);

    static void applyLockToSelection(Score* score);

    static void removeSystemLocksOnAddLayoutBreak(Score* score, LayoutBreakType breakType, const MeasureBase* measure);
    static void removeLayoutBreaksOnAddSystemLock(Score* score, const SystemLock* lock);
    static void removeSystemLocksOnRemoveMeasures(Score* score, const MeasureBase* m1, const MeasureBase* m2);
    static void removeSystemLocksContainingMMRests(Score* score);
    static void updateSystemLocksOnCreateMMRests(Score* score, Measure* first, Measure* last);
};
}
