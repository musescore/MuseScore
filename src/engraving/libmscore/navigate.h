/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#ifndef __NAVIGATE_H__
#define __NAVIGATE_H__

namespace mu::engraving {
class ChordRest;
class Lyrics;

extern int pitch2y(int pitch, int enh, int clefOffset, int key, int& prefix, const char* tversatz);
extern ChordRest* nextChordRest(const ChordRest* cr, bool skipGrace = false, bool skipMeasureRepeatRests = true);
extern ChordRest* prevChordRest(const ChordRest* cr, bool skipGrace = false, bool skipMeasureRepeatRests = true);
extern Lyrics* prevLyrics(const Lyrics* lyrics);
} // namespace mu::engraving
#endif
