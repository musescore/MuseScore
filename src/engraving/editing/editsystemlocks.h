/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
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

#pragma once

#include <vector>

namespace mu::engraving {
class Measure;
class MeasureBase;
class Score;
class System;
class Transaction;
class RangeLock;

enum class LayoutBreakType : unsigned char;

class EditSystemLocks
{
public:
    static void undoAddSystemLock(Transaction& tx, Score* score, const RangeLock* lock);
    static void undoRemoveSystemLock(Transaction& tx, Score* score, const RangeLock* lock);
    static void undoRemoveAllLocks(Transaction& tx, Score* score);

    static void toggleSystemLock(Transaction& tx, Score* score, const std::vector<System*>& systems);
    static void toggleScoreLock(Transaction& tx, Score* score);

    static void addRemoveSystemLocks(Transaction& tx, Score* score, int interval, bool lock);

    static void makeIntoSystem(Transaction& tx, Score* score, MeasureBase* first, MeasureBase* last);
    static void moveMeasureToPrevSystem(Transaction& tx, Score* score, MeasureBase* m);
    static void moveMeasureToNextSystem(Transaction& tx, Score* score, MeasureBase* m);

    static void applyLockToSelection(Transaction& tx, Score* score);

    static void removeSystemLocksOnAddLayoutBreak(Transaction& tx, Score* score, LayoutBreakType breakType, const MeasureBase* measure);
    static void removeLayoutBreaksOnAddSystemLock(Transaction& tx, Score* score, const RangeLock* lock);
    static void removeSystemLocksOnRemoveMeasures(Transaction& tx, Score* score, const MeasureBase* m1, const MeasureBase* m2);
    static void removeSystemLocksContainingMMRests(Transaction& tx, Score* score);
    static void updateSystemLocksOnCreateMMRests(Transaction& tx, Score* score, Measure* first, Measure* last);
};
}
