/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include "importmidi_meter.h"

#include <QQueue>

#include "importmidi_fraction.h"
#include "engraving/dom/durationtype.h"
#include "engraving/dom/mscore.h"
#include "importmidi_tuplet.h"
#include "importmidi_chord.h"
#include "importmidi_inner.h"

namespace mu::iex::midi {
namespace Meter {
bool isSimple(const ReducedFraction& barFraction)       // 2/2, 3/4, 4/4, ...
{
    return barFraction.numerator() < 5;
}

bool isCompound(const ReducedFraction& barFraction)     // 6/8, 12/4, ...
{
    return barFraction.numerator() % 3 == 0 && barFraction.numerator() > 3;
}

bool isComplex(const ReducedFraction& barFraction)      // 5/4, 7/8, ...
{
    return barFraction.numerator() == 5 || barFraction.numerator() == 7;
}

bool isDuple(const ReducedFraction& barFraction)        // 2/2, 6/8, ...
{
    return barFraction.numerator() == 2 || barFraction.numerator() == 6;
}

bool isTriple(const ReducedFraction& barFraction)       // 3/4, 9/4, ...
{
    return barFraction.numerator() == 3 || barFraction.numerator() == 9;
}

bool isQuadruple(const ReducedFraction& barFraction)    // 4/4, 12/8, ...
{
    return barFraction.numerator() % 4 == 0;
}

bool isQuintuple(const ReducedFraction& barFraction)    // 5/4, 15/8, ...
{
    return barFraction.numerator() % 5 == 0;
}

bool isSeptuple(const ReducedFraction& barFraction)    // 7/8, 21/8, ...
{
    return barFraction.numerator() % 7 == 0;
}

// list of bar division lengths in ticks (whole bar len, half bar len, ...)
// and its corresponding levels
// tuplets are not taken into account here

DivisionInfo metricDivisionsOfBar(const ReducedFraction& barFraction)
{
    DivisionInfo barDivInfo;
    barDivInfo.len = barFraction;
    // first value of each element in list is a length (in ticks) of every part of bar
    // on which bar is subdivided on each level
    // the level value is a second value of each element
    auto& divLengths = barDivInfo.divLengths;
    int level = 0;
    divLengths.push_back({ barFraction, level });
    // pulse-level division
    if (isDuple(barFraction)) {
        divLengths.push_back({ barFraction / 2, --level });
    } else if (isTriple(barFraction)) {
        divLengths.push_back({ barFraction / 3, --level });
    } else if (isQuadruple(barFraction)) {
        divLengths.push_back({ barFraction / 2, --level });      // additional central accent
        divLengths.push_back({ barFraction / 4, --level });
    } else if (isQuintuple(barFraction)) {
        divLengths.push_back({ barFraction / 5, --level });
    } else if (isSeptuple(barFraction)) {
        divLengths.push_back({ barFraction / 7, --level });
    } else {            // if other - complex meter
        divLengths.push_back({ barFraction / barFraction.numerator(), --level });
    }

    if (isCompound(barFraction)) {
        --level;        // additional min level for pulse divisions
                        // subdivide pulse of compound meter into 3 parts
        divLengths.push_back({ divLengths.back().len / 3, --level });
    }

    while (divLengths.back().len >= MChord::minAllowedDuration() * 2) {
        divLengths.push_back({ divLengths.back().len / 2, --level });
    }

    return barDivInfo;
}

DivisionInfo metricDivisionsOfTuplet(const MidiTuplet::TupletData& tuplet,
                                     int tupletStartLevel)
{
    DivisionInfo tupletDivInfo;
    tupletDivInfo.onTime = tuplet.onTime;
    tupletDivInfo.len = tuplet.len;
    tupletDivInfo.isTuplet = true;
    tupletDivInfo.divLengths.push_back({ tuplet.len, TUPLET_BOUNDARY_LEVEL });
    const auto divLen = tuplet.len / tuplet.tupletNumber;
    tupletDivInfo.divLengths.push_back({ divLen, tupletStartLevel-- });
    while (tupletDivInfo.divLengths.back().len >= MChord::minAllowedDuration() * 2) {
        tupletDivInfo.divLengths.push_back({
                    tupletDivInfo.divLengths.back().len / 2, tupletStartLevel-- });
    }
    return tupletDivInfo;
}

ReducedFraction beatLength(const ReducedFraction& barFraction)
{
    auto beatLen = barFraction / 4;
    if (isDuple(barFraction)) {
        beatLen = barFraction / 2;
    } else if (isTriple(barFraction)) {
        beatLen = barFraction / 3;
    } else if (isQuadruple(barFraction)) {
        beatLen = barFraction / 4;
    } else if (isComplex(barFraction)) {
        beatLen = barFraction / barFraction.numerator();
    }
    return beatLen;
}

std::vector<ReducedFraction> divisionsOfBarForTuplets(const ReducedFraction& barFraction)
{
    const DivisionInfo info = metricDivisionsOfBar(barFraction);
    std::vector<ReducedFraction> divLengths;
    const auto beatLen = beatLength(barFraction);
    for (const auto& i: info.divLengths) {
        // in compound or complex meter
        // tuplet starts from beat level, not the whole bar
        if ((isCompound(barFraction) || isComplex(barFraction)) && i.len > beatLen) {
            continue;
        }
        divLengths.push_back(i.len);
    }
    return divLengths;
}

// result in vector: first elements - all tuplets info, one at the end - bar division info

std::vector<DivisionInfo> divisionInfo(const ReducedFraction& barFraction,
                                       const std::vector<MidiTuplet::TupletData>& tupletsInBar)
{
    std::vector<DivisionInfo> divsInfo;

    const auto barDivisionInfo = metricDivisionsOfBar(barFraction);
    for (const auto& tuplet: tupletsInBar) {
        int tupletStartLevel = 0;
        for (const auto& divLenInfo: barDivisionInfo.divLengths) {
            if (divLenInfo.len == tuplet.len) {
                tupletStartLevel = divLenInfo.level;
                break;
            }
        }
        divsInfo.push_back(metricDivisionsOfTuplet(tuplet, --tupletStartLevel));
    }
    divsInfo.push_back(barDivisionInfo);

    return divsInfo;
}

// tick is counted from the beginning of bar

int levelOfTick(const ReducedFraction& tick, const std::vector<DivisionInfo>& divsInfo)
{
    for (const auto& divInfo: divsInfo) {
        if (tick < divInfo.onTime || tick > divInfo.onTime + divInfo.len) {
            continue;
        }
        for (const auto& divLenInfo: divInfo.divLengths) {
            const auto ratio = (tick - divInfo.onTime) / divLenInfo.len;
            if (ratio.numerator() % ratio.denominator() == 0) {
                return divLenInfo.level;
            }
        }
    }
    return 0;
}

std::vector<int> metricLevelsOfBar(const ReducedFraction& barFraction,
                                   const std::vector<DivisionInfo>& divsInfo,
                                   const ReducedFraction& minDuration)
{
    std::vector<int> levels;
    for (ReducedFraction tick(0, 1); tick < barFraction; tick += minDuration) {
        levels.push_back(levelOfTick(tick, divsInfo));
    }
    return levels;
}

// return level with pos == Fraction(-1, 1) if undefined - see MaxLevel class

Meter::MaxLevel maxLevelBetween(const ReducedFraction& startTickInBar,
                                const ReducedFraction& endTickInBar,
                                const DivisionInfo& divInfo)
{
    MaxLevel level;
    const auto startTickInDiv = startTickInBar - divInfo.onTime;
    const auto endTickInDiv = endTickInBar - divInfo.onTime;
    if (startTickInDiv >= endTickInDiv || startTickInDiv < ReducedFraction(0, 1)
        || endTickInDiv > divInfo.len) {
        return level;
    }

    for (const auto& divLengthInfo: divInfo.divLengths) {
        const auto& divLen = divLengthInfo.len;
        const auto ratio = endTickInDiv / divLen;
        auto maxEndRaster = divLen * (ratio.numerator() / ratio.denominator());
        if (maxEndRaster == endTickInDiv) {
            maxEndRaster -= divLen;
        }
        if (startTickInDiv < maxEndRaster) {
            // max level is found
            const auto sratio = startTickInDiv / divLen;
            const auto maxStartRaster = divLen * (sratio.numerator() / sratio.denominator());
            const auto count = (maxEndRaster - maxStartRaster) / divLen;
            level.pos = maxStartRaster + divLen + divInfo.onTime;
            level.levelCount = count.numerator() / count.denominator();
            level.level = divLengthInfo.level;
            break;
        }
    }
    return level;
}

// vector<DivisionInfo>:
//    first elements - tuplet division info, if there are any tuplets
//    last element - always the whole bar division info

// here we use levelCount = 1 always for simplicity
// because TUPLET_BOUNDARY_LEVEL is 'max enough'

Meter::MaxLevel findMaxLevelBetween(const ReducedFraction& startTickInBar,
                                    const ReducedFraction& endTickInBar,
                                    const std::vector<DivisionInfo>& divsInfo)
{
    MaxLevel level;

    for (const auto& divInfo: divsInfo) {
        if (divInfo.isTuplet) {
            if (startTickInBar < divInfo.onTime + divInfo.len
                && endTickInBar > divInfo.onTime + divInfo.len) {
                level.level = TUPLET_BOUNDARY_LEVEL;
                level.levelCount = 1;
                level.pos = divInfo.onTime + divInfo.len;
                break;
            }
            if (startTickInBar < divInfo.onTime
                && endTickInBar > divInfo.onTime
                && endTickInBar <= divInfo.onTime + divInfo.len) {
                level.level = TUPLET_BOUNDARY_LEVEL;
                level.levelCount = 1;
                level.pos = divInfo.onTime;
                break;
            }
            if (startTickInBar >= divInfo.onTime
                && endTickInBar <= divInfo.onTime + divInfo.len) {
                level = maxLevelBetween(startTickInBar, endTickInBar, divInfo);
                break;
            }
        } else {
            level = maxLevelBetween(startTickInBar, endTickInBar, divInfo);
            break;
        }
    }
    return level;
}

int tupletNumberForDuration(const ReducedFraction& startTick,
                            const ReducedFraction& endTick,
                            const std::vector<MidiTuplet::TupletData>& tupletsInBar)
{
    for (const auto& tupletData: tupletsInBar) {
        if (startTick >= tupletData.onTime
            && endTick <= tupletData.onTime + tupletData.len) {
            return tupletData.tupletNumber;
        }
    }
    return -1;    // this duration is not inside any tuplet
}

bool isPowerOfTwo(unsigned int x)
{
    return x && !(x & (x - 1));
}

bool isSimpleNoteDuration(const ReducedFraction& duration)
{
    const auto division = ReducedFraction::fromTicks(engraving::Constants::DIVISION);
    auto div = (duration > division) ? duration / division : division / duration;
    if (div > ReducedFraction(0, 1)) {
        div.reduce();
        int minVal = qMin(div.numerator(), div.denominator());
        int maxVal = qMax(div.numerator(), div.denominator());
        return minVal == 1 && isPowerOfTwo((unsigned int)maxVal);
    }
    return false;
}

bool isQuarterDuration(const ReducedFraction& ticks)
{
    return ticks.numerator() == 1 && ticks.denominator() == 4;
}

// If last 2/3 of beat in compound meter is rest,
// it should be split into 2 rests

bool is23EndOfBeatInCompoundMeter(const ReducedFraction& startTickInBar,
                                  const ReducedFraction& endTickInBar,
                                  const ReducedFraction& barFraction)
{
    if (endTickInBar <= startTickInBar) {
        return false;
    }
    if (!isCompound(barFraction)) {
        return false;
    }

    const auto beatLen = beatLength(barFraction);
    const auto divLen = beatLen / 3;
    const auto ratio1 = startTickInBar / beatLen;
    const auto ratio2 = endTickInBar / beatLen;
    if ((startTickInBar - beatLen * (ratio1.numerator() / ratio1.denominator()) == divLen)
        && (ratio2.numerator() % ratio2.denominator() == 0)) {
        return true;
    }
    return false;
}

bool is2of3RestInTripleMeter(const ReducedFraction& startTickInBar,
                             const ReducedFraction& endTickInBar,
                             const ReducedFraction& barFraction)
{
    if (endTickInBar - startTickInBar <= ReducedFraction(0, 1)) {
        return false;
    }
    if (isTriple(barFraction)
        && (startTickInBar == ReducedFraction(0, 1)
            || endTickInBar == barFraction)
        && endTickInBar - startTickInBar == (barFraction * 2) / 3) {
        return true;
    }
    return false;
}

struct Node
{
    Node(int edgeLevel, int midLevel)
        : edgeLevel(edgeLevel)
        , midLevel(midLevel)
    {}

    int edgeLevel;
    int midLevel;
};

// all durations inside tuplets are smaller/larger than their regular versions
// this difference is represented by tuplet ratio: 3/2 for triplets, etc.

// if node duration is completely inside some tuplet
// then assign to the node tuplet-to-regular-duration conversion coefficient

ReducedFraction findTupletRatio(const ReducedFraction& startPos,
                                const ReducedFraction& endPos,
                                const std::vector<MidiTuplet::TupletData>& tupletsInBar)
{
    ReducedFraction tupletRatio = { 2, 2 };
    int tupletNumber = tupletNumberForDuration(startPos, endPos, tupletsInBar);
    if (tupletNumber != -1) {
        tupletRatio = MidiTuplet::tupletLimits(tupletNumber).ratio;
    }

    return tupletRatio;
}

QList<std::pair<ReducedFraction, engraving::TDuration> >
collectDurations(const std::map<ReducedFraction, Node>& nodes,
                 const std::vector<MidiTuplet::TupletData>& tupletsInBar,
                 bool useDots,
                 bool printRestRemains)
{
    QList<std::pair<ReducedFraction, engraving::TDuration> > resultDurations;

    for (auto it1 = nodes.begin(); it1 != nodes.end(); ++it1) {
        auto it2 = it1;
        ++it2;
        if (it2 == nodes.end()) {
            break;
        }
        const auto tupletRatio = findTupletRatio(it1->first, it2->first, tupletsInBar);
        const auto duration = tupletRatio * (it2->first - it1->first);
        auto list = mu::engraving::toDurationList(duration.fraction(), useDots, 1, printRestRemains);
        for (const auto& dur : list) {
            resultDurations.push_back({ tupletRatio, dur });
        }
    }

    return resultDurations;
}

bool badLevelCondition(int startLevelDiff, int endLevelDiff, int tol)
{
    return startLevelDiff > tol || endLevelDiff > tol;
}

int noteCount(const ReducedFraction& duration,
              bool useDots)
{
    return int(mu::engraving::toDurationList(duration.fraction(), useDots, 1, false).size());
}

bool isLessNoteCount(const ReducedFraction& t1,
                     const ReducedFraction& t2,
                     const ReducedFraction& t3,
                     bool useDots)
{
    return noteCount(t3 - t1, useDots)
           < noteCount(t2 - t1, useDots) + noteCount(t3 - t2, useDots);
}

void excludeNodes(std::map<ReducedFraction, Node>& nodes,
                  int tol,
                  bool useDots)
{
    if (tol == 0) {     // no nodes can be excluded
        return;
    }

    auto p1 = nodes.begin();
    if (p1 == nodes.end()) {
        return;
    }
    auto p2 = p1;
    ++p2;
    if (p2 == nodes.end()) {
        return;
    }
    auto p3 = p2;
    ++p3;
    if (p3 == nodes.end()) {
        return;
    }

    while (p3 != nodes.end()) {
        if (!badLevelCondition(p2->second.midLevel - p1->second.edgeLevel,
                               p2->second.midLevel - p3->second.edgeLevel, tol)
            && isLessNoteCount(p1->first, p2->first, p3->first, useDots)) {
            p2 = nodes.erase(p2);
            ++p3;
            continue;
        }
        ++p1;
        ++p2;
        ++p3;
    }
}

// set tuplet boundary level to regular, non-tuplet bar division level
// because there is no more need in tuplet boundary level after split
// and such big level may confuse the estimation algorithm

int adjustEdgeLevelIfTuplet(const Meter::MaxLevel& splitPoint,
                            const std::vector<DivisionInfo>& divInfo)
{
    int tupletLevel = splitPoint.level;
    if (splitPoint.level == TUPLET_BOUNDARY_LEVEL) {
        std::vector<DivisionInfo> nonTupletDivs({ divInfo.back() });
        tupletLevel = levelOfTick(splitPoint.pos, nonTupletDivs);
    }

    return tupletLevel;
}

// duration start/end should be quantisized, quantum >= 1/128 note
// pair here represents the tuplet ratio of duration and the duration itself
// for regular (non-tuplet) durations fraction.numerator == fraction.denominator

// tol - max allowed difference between start/end level of duration and split point level
//    1 for notes, 0 for rests

QList<std::pair<ReducedFraction, engraving::TDuration> >
toDurationList(const ReducedFraction& startTickInBar,
               const ReducedFraction& endTickInBar,
               const ReducedFraction& barFraction,
               const std::vector<MidiTuplet::TupletData>& tupletsInBar,
               DurationType durationType,
               bool useDots,
               bool printRestRemains)
{
    if (startTickInBar < ReducedFraction(0, 1)
        || endTickInBar <= startTickInBar || endTickInBar > barFraction) {
        return QList<std::pair<ReducedFraction, engraving::TDuration> >();
    }

    const auto divInfo = divisionInfo(barFraction, tupletsInBar);    // metric structure of bar
    const auto minDuration = MChord::minAllowedDuration() * 2;    // >= minAllowedDuration() after subdivision

    std::map<ReducedFraction, Node> nodes;      // <onTime, Node>
    {
        int level = levelOfTick(startTickInBar, divInfo);
        nodes.insert({ startTickInBar, Node(level, level) });
        level = levelOfTick(endTickInBar, divInfo);
        nodes.insert({ endTickInBar, Node(level, level) });
    }

    QQueue<std::pair<ReducedFraction, ReducedFraction> > gapsToProcess;
    gapsToProcess.enqueue({ startTickInBar, endTickInBar });

    while (!gapsToProcess.isEmpty()) {
        const auto gap = gapsToProcess.dequeue();
        // don't split gap if its duration is less than minDuration
        if (gap.second - gap.first < minDuration) {
            continue;
        }
        const auto splitPoint = findMaxLevelBetween(gap.first, gap.second, divInfo);
        // sum levels if there are several positions (beats) with max level value
        // for example, 8th + half duration + 8th in 3/4, and half is over two beats
        if (splitPoint.pos == ReducedFraction(-1, 1)) {       // undefined
            continue;
        }
        const int effectiveLevel = splitPoint.level + splitPoint.levelCount - 1;
        const Node& startNode = nodes.find(gap.first)->second;
        const Node& endNode = nodes.find(gap.second)->second;

        if (badLevelCondition(effectiveLevel - startNode.edgeLevel,
                              effectiveLevel - endNode.edgeLevel, 0)
            || is2of3RestInTripleMeter(gap.first, gap.second, barFraction)
            || (durationType == DurationType::REST
                && is23EndOfBeatInCompoundMeter(gap.first, gap.second, barFraction))) {
            const int edgeLevel = adjustEdgeLevelIfTuplet(splitPoint, divInfo);
            // split gap in splitPoint position
            nodes.insert({ splitPoint.pos, Node(edgeLevel, effectiveLevel) });
            gapsToProcess.enqueue({ gap.first, splitPoint.pos });
            gapsToProcess.enqueue({ splitPoint.pos, gap.second });
        }
    }

    const int tol = (durationType == DurationType::NOTE) ? 1 : 0;
    excludeNodes(nodes, tol, useDots);

    return collectDurations(nodes, tupletsInBar, useDots, printRestRemains);
}
} // namespace Meter
} // namespace mu::iex::midi
