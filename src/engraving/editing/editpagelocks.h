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

#include <vector>

namespace mu::engraving {
class Measure;
class MeasureBase;
class Page;
class Score;
class System;
class Transaction;
class RangeLock;

enum class LayoutBreakType : unsigned char;

class EditPageLocks
{
public:
    static void undoAddPageLock(Transaction& tx, Score* score, const RangeLock* lock);
    static void undoRemovePageLock(Transaction& tx, Score* score, const RangeLock* lock);
    static void undoRemoveAllLocks(Transaction& tx, Score* score);

    static void togglePageLock(Transaction& tx, Score* score, const std::vector<Page*>& pages);
    static void toggleScoreLock(Transaction& tx, Score* score);

    static void addRemovePageLocks(Transaction& tx, Score* score, int interval, bool lock);

    static void makeIntoPage(Transaction& tx, Score* score, MeasureBase* first, MeasureBase* last);
    static void moveMeasureToPrevPage(Transaction& tx, Score* score, MeasureBase* m);
    static void moveMeasureToNextPage(Transaction& tx, Score* score, MeasureBase* m);

    static void applyLockToSelection(Transaction& tx, Score* score);

    static void removeLayoutBreaksOnAddPageLock(Transaction& tx, const RangeLock* lock);
    static void removePageLocksOnAddLayoutBreak(Transaction& tx, Score* score, LayoutBreakType breakType, const MeasureBase* measure);
    static void removePageLocksOnRemoveMeasures(Transaction& tx, Score* score, const MeasureBase* m1, const MeasureBase* m2);
    static void removePageLocksContainingMMRests(Transaction& tx, Score* score);
    static void updatePageLocksOnCreateMMRests(Transaction& tx, Score* score, Measure* first, Measure* last);
};
}
