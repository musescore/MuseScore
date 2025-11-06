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

#include <string>

#include "global/containers.h"

#include "measurehandler.h"
#include "note.h"

using namespace mu::engraving;
namespace mu::iex::tabledit {
//---------------------------------------------------------
//   MeasureHandler
//   Calculate measure-level timing corrections for pickup measures
//   A TablEdit pickup measure contains gaps at either the left or right side,
//   that must be removed. Also the measure's actual size must be calculated.
//---------------------------------------------------------

static int nominalSize(const std::vector<TefMeasure>& tefMeasures, const size_t idx);

// debug support

static void dumpIntVec(const char* name, const std::vector<int>& gaps)
{
    std::string s { name };
    for (const auto gap : gaps) {
        s += ' ';
        s += std::to_string(gap);
    }
    LOGN("%s", s.c_str());
}

void MeasureHandler::dumpActualsAndSumGaps(const std::vector<TefMeasure>& tefMeasures) const
{
    std::vector<int> actuals;
    for (unsigned int i = 0; i < tefMeasures.size(); ++i) {
        actuals.push_back(actualSize(tefMeasures, i));
    }
    dumpIntVec("actuals", actuals);

    std::vector<int> sumGaps;
    for (unsigned int i = 0; i < tefMeasures.size(); ++i) {
        sumGaps.push_back(sumPreviousGaps(i));
    }
    dumpIntVec("sumGaps", sumGaps);
}

static void dumpNominals(const std::vector<TefMeasure>& tefMeasures)
{
    std::vector<int> nominals;
    for (unsigned int i = 0; i < tefMeasures.size(); ++i) {
        nominals.push_back(nominalSize(tefMeasures, i));
    }
    dumpIntVec("nominals", nominals);
}

static void dumpPickups(const std::vector<TefMeasure>& tefMeasures)
{
    std::vector<int> pickups;
    for (unsigned int i = 0; i < tefMeasures.size(); ++i) {
        pickups.push_back(tefMeasures.at(i).isPickup ? 1 : 0);
    }
    dumpIntVec("pickups", pickups);
}

// return the nominal size of measure idx, based on time signature

static int nominalSize(const std::vector<TefMeasure>& tefMeasures, const size_t idx)
{
    return 64 * tefMeasures.at(idx).numerator / tefMeasures.at(idx).denominator;
}

// return the actual size of measure idx
// normal measure: based on time signature
// pickup measure: based on time signature and left and right gaps

int MeasureHandler::actualSize(const std::vector<TefMeasure>& tefMeasures, const size_t idx) const
{
    int size { nominalSize(tefMeasures, idx) };
    if (tefMeasures.at(idx).isPickup) {
        size -= gapsLeft.at(idx) + gapsRight.at(idx);
    }
    LOGN("idx %zu size %d", idx, size);
    return size;
}

void MeasureHandler::initializeMeasureStartsAndGaps(const std::vector<TefMeasure>& tefMeasures)
{
    int measureStart { 0 };
    for (size_t i = 0; i < tefMeasures.size(); ++i) {
        nominalMeasureStarts.push_back(measureStart);
        int measureSize { nominalSize(tefMeasures, i) };
        measureStart += measureSize;
        // also initialize the gaps
        // normal measure: set to 0 to ignore gaps
        // pickup measure: set to the measure size (to be corrected later when a smaller gap is found)
        const int gap { tefMeasures.at(i).isPickup ? measureSize : 0 };
        gapsLeft.push_back(gap);
        gapsRight.push_back(gap);
    }
}

// return the index of the measure containing tstart
// note O2 behaviour in score size

size_t MeasureHandler::measureIndex(int tstart, const std::vector<TefMeasure>& tefMeasures) const
{
    for (size_t i = 0; i < tefMeasures.size(); ++i) {
        auto start { nominalMeasureStarts.at(i) };
        auto size { 64 * tefMeasures.at(i).numerator / tefMeasures.at(i).denominator };
        if (start <= tstart && tstart < start + size) {
            return i;
        }
    }
    return muse::nidx; // not found
}

// return the offset of tstart (distance from its measure's start)

int MeasureHandler::offsetInMeasure(int tstart, const std::vector<TefMeasure>& tefMeasures)
{
    size_t index { measureIndex(tstart, tefMeasures) };
    if (index >= tefMeasures.size()) {
        return -1; // not found
    }
    return tstart - nominalMeasureStarts.at(index);
}

// find the smallest offset of any note in a pickup measure

void MeasureHandler::updateGapLeft(std::vector<int>& gapLeft, const int position, const std::vector<TefMeasure>& tefMeasures)
{
    size_t index { measureIndex(position, tefMeasures) };
    if (index >= tefMeasures.size()) {
        return; // not found
    }
    if (tefMeasures.at(index).isPickup) {
        auto offset { offsetInMeasure(position, tefMeasures) };
        if (0 <= offset) {
            if (offset < gapLeft[index]) {
                gapLeft[index] = offset;
            }
        }
    }
}

// find the largest end time of any note in a pickup measure

void MeasureHandler::updateGapRight(std::vector<int>& gapRight, const TefNote& note, const std::vector<TefMeasure>& tefMeasures)
{
    auto pos { note.position };
    size_t index { measureIndex(pos, tefMeasures) };
    if (index >= tefMeasures.size()) {
        return; // not found
    }
    if (tefMeasures.at(index).isPickup) {
        auto offset { offsetInMeasure(pos, tefMeasures) };
        LOGN("pos %d index %zu offset %d", pos, index, offset);
        if (0 <= offset) {
            auto dur { durationToInt(note.duration) };
            auto end { offset + dur };
            auto size { 64 * tefMeasures.at(index).numerator / tefMeasures.at(index).denominator };
            auto gap { size - end };
            LOGN("dur %d end %d size %d gap %d", dur, end, size, gap);
            if (gap < gapRight[index]) {
                gapRight[index] = gap;
            }
        }
    }
}

// start time correction to be subtracted from note position due to gaps:
// sum of gaps in previous measure(s) plus left gap in current measure
// note only pickup measures count, regular measures always have gaps set to 0

int MeasureHandler::sumPreviousGaps(const size_t idx) const
{
    auto corr { 0 };
    for (unsigned int j = 0; j < idx; ++j) {
        corr += gapsLeft.at(j) + gapsRight.at(j);
    }
    corr += gapsLeft.at(idx);
    LOGN("idx %zu corr %d", idx, corr);
    return corr;
}

void MeasureHandler::updateGaps(const std::vector<TefNote>& tefContents, const std::vector<TefMeasure>& tefMeasures)
{
    for (const TefNote& note : tefContents) {
        updateGapLeft(gapsLeft, note.position, tefMeasures);
        updateGapRight(gapsRight, note, tefMeasures);
    }
}

void MeasureHandler::calculate(const std::vector<TefNote>& tefContents, const std::vector<TefMeasure>& tefMeasures)
{
    initializeMeasureStartsAndGaps(tefMeasures);
    updateGaps(tefContents, tefMeasures);

    // debug: dump result
    dumpIntVec("starts", nominalMeasureStarts);
    dumpIntVec("gapsLeft", gapsLeft);
    dumpIntVec("gapsRight", gapsRight);
    dumpNominals(tefMeasures);
    dumpPickups(tefMeasures);
    dumpActualsAndSumGaps(tefMeasures);
}
} // namespace mu::iex::tabledit
