/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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

#include "stemlayout.h"
#include "dom/chord.h"
#include "dom/hook.h"
#include "dom/note.h"
#include "dom/staff.h"
#include "dom/tremolosinglechord.h"
#include "dom/tremolotwochord.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::score;

//-----------------------------------------------------------------------------
//   defaultStemLength
///   Get the default stem length for this chord
///   all internal calculation is done in quarter spaces
///   using integers to eliminate all possibilities for rounding errors
//-----------------------------------------------------------------------------
double StemLayout::calcDefaultStemLength(Chord* item, const LayoutContext& ctx)
{
    // returns default length even if the chord doesn't have a stem
    const MStyle& style = ctx.conf().style();
    const Chord::LayoutData* ldata = item->ldata();
    const Staff* staff = item->staff();

    double spatium = item->spatium();
    double lineDistance = (staff ? staff->lineDistance(item->tick()) : 1.0);

    const StaffType* staffType = staff ? staff->staffTypeForElement(item) : nullptr;
    const StaffType* tab = (staffType && staffType->isTabStaff()) ? staffType : nullptr;

    bool isBesideTabStaff = tab && !tab->stemless() && !tab->stemThrough();
    if (isBesideTabStaff) {
        return tab->chordStemLength(item) * spatium;
    }

    int defaultStemLength = style.styleD(Sid::stemLength) * 4;
    defaultStemLength += stemLengthBeamAddition(item, ctx);
    if (tab) {
        defaultStemLength *= 1.5;
    }
    // extraHeight represents the extra vertical distance between notehead and stem start
    // eg. slashed noteheads etc
    double extraHeight = (ldata->up ? item->upNote()->stemUpSE().y() : item->downNote()->stemDownNW().y()) / item->intrinsicMag()
                         / spatium;
    int shortestStem = style.styleB(Sid::useWideBeams) ? 12 : (style.styleD(Sid::shortestStem) + std::abs(extraHeight)) * 4;
    int quarterSpacesPerLine = std::floor(lineDistance * 2);
    int chordHeight = (item->downLine() - item->upLine()) * quarterSpacesPerLine; // convert to quarter spaces
    int stemLength = defaultStemLength;

    int minStemLengthQuarterSpaces = calcMinStemLength(item, ctx);

    int staffLineCount = staff ? staff->lines(item->tick()) : 5;
    int shortStemStart = style.styleI(Sid::shortStemStartLocation) * quarterSpacesPerLine + 1;
    bool useWideBeams = style.styleB(Sid::useWideBeams);
    int beamCount = calcBeamCount(item);
    int middleLine = minStaffOverlap(ldata->up, staffLineCount,
                                     beamCount, !!item->hook(), useWideBeams ? 4 : 3,
                                     useWideBeams, !(item->isGrace() || item->isSmall()));
    int idealStemLength = defaultStemLength;

    if (ldata->up) {
        int stemEndPosition = item->upLine() * quarterSpacesPerLine - defaultStemLength;
        double stemEndPositionMag = (double)item->upLine() * quarterSpacesPerLine - (defaultStemLength * item->intrinsicMag());

        if (stemEndPositionMag <= -shortStemStart) {
            int reduction = maxReduction(item, ctx, std::abs((int)floor(stemEndPositionMag) + shortStemStart));
            idealStemLength = std::max(idealStemLength - reduction, shortestStem);
        } else if (stemEndPosition > middleLine) {
            // this case will be taken care of below; even if we were to adjust here we'd have
            // to adjust again later if the line spacing != 1.0 or if _relativeMag != 1.0
        } else {
            idealStemLength -= stemOpticalAdjustment(item, stemEndPosition);
            idealStemLength = std::max(idealStemLength, shortestStem);
        }
        stemLength = std::max(idealStemLength, minStemLengthQuarterSpaces);
    } else {
        int stemEndPosition = item->downLine() * quarterSpacesPerLine + defaultStemLength;
        double stemEndPositionMag = (double)item->downLine() * quarterSpacesPerLine + (defaultStemLength * item->intrinsicMag());

        int downShortStemStart = (staffLineCount - 1) * (2 * quarterSpacesPerLine) + shortStemStart;
        if (stemEndPositionMag >= downShortStemStart) {
            int reduction = maxReduction(item, ctx, std::abs((int)ceil(stemEndPositionMag) - downShortStemStart));
            idealStemLength = std::max(idealStemLength - reduction, shortestStem);
        } else if (stemEndPosition < middleLine) {
            // this case will be taken care of below; even if we were to adjust here we'd have
            // to adjust again later if the line spacing != 1.0 or if _relativeMag != 1.0
        } else {
            idealStemLength -= stemOpticalAdjustment(item, stemEndPosition);
            idealStemLength = std::max(idealStemLength, shortestStem);
        }

        stemLength = std::max(idealStemLength, minStemLengthQuarterSpaces);
    }
    if (beamCount == 4 && !item->hook()) {
        stemLength = calc4BeamsException(item, stemLength);
    }

    double finalStemLength = (chordHeight / 4.0 * spatium) + ((stemLength / 4.0 * spatium) * item->intrinsicMag());
    double extraLength = 0.;
    Note* startNote = ldata->up ? item->downNote() : item->upNote();
    if (!startNote->fixed()) {
        // when the chord's magnitude is < 1, the stem length with mag can find itself below the middle line.
        // in those cases, we have to add the extra amount to it to bring it to a minimum.
        double upValue = ldata->up ? -1. : 1.;
        double stemStart = startNote->ldata()->pos().y();
        double stemEndMag = stemStart + (finalStemLength * upValue);
        double topLine = 0.0;
        lineDistance *= spatium;
        double bottomLine = lineDistance * (staffLineCount - 1.0);
        double target = 0.0;
        double midLine = middleLine / 4.0 * lineDistance;
        if (muse::RealIsEqualOrMore(lineDistance / spatium, 1.0)) {
            // need to extend to middle line, or to opposite line if staff is < 2sp tall
            if (bottomLine < 2 * spatium) {
                target = ldata->up ? topLine : bottomLine;
            } else {
                double twoSpIn = ldata->up ? bottomLine - (2 * spatium) : topLine + (2 * spatium);
                target = muse::RealIsEqual(lineDistance / spatium, 1.0) ? midLine : twoSpIn;
            }
        } else {
            // need to extend to second line in staff, or to opposite line if staff has < 3 lines
            if (staffLineCount < 3) {
                target = ldata->up ? topLine : bottomLine;
            } else {
                target = ldata->up ? bottomLine - (2 * lineDistance) : topLine + (2 * lineDistance);
            }
        }
        extraLength = 0.0;
        if (ldata->up && stemEndMag > target) {
            extraLength = stemEndMag - target;
        } else if (!ldata->up && stemEndMag < target) {
            extraLength = target - stemEndMag;
        }
    }
    return finalStemLength + extraLength;
}

int StemLayout::minStaffOverlap(bool up, int staffLines, int beamCount, bool hasHook, double beamSpacing, bool useWideBeams,
                                bool isFullSize)
{
    int beamOverlap = 8;
    if (isFullSize) {
        if (beamCount == 3 && !hasHook) {
            beamOverlap = 12;
        } else if (beamCount >= 4 && !hasHook) {
            beamOverlap = (beamCount - 4) * beamSpacing + (useWideBeams ? 16 : 14);
        }
    }

    int staffLineOffset = isFullSize ? 1 : 4;
    int staffOverlap = std::min(beamOverlap, (staffLines - staffLineOffset) * 4);
    if (!up) {
        return staffOverlap;
    }
    return (staffLines - 1) * 4 - staffOverlap;
}

int StemLayout::stemLengthBeamAddition(const Chord* item, const LayoutContext& ctx)
{
    if (item->hook()) {
        return 0;
    }
    const MStyle& style = ctx.conf().style();
    int beamCount = calcBeamCount(item);
    switch (beamCount) {
    case 0:
    case 1:
    case 2:
        return 0;
    case 3:
        return 2;
    default:
        return (beamCount - 3) * (style.styleB(Sid::useWideBeams) ? 4 : 3);
    }
}

int StemLayout::maxReduction(const Chord* item, const LayoutContext& ctx, int extensionOutsideStaff)
{
    const MStyle& style = ctx.conf().style();
    if (!style.styleB(Sid::shortenStem)) {
        return 0;
    }
    // [extensionOutsideStaff][beamCount]
    static const int maxReductions[4][5] = {
        //1sp 1.5sp 2sp 2.5sp >=3sp -- extensionOutsideStaff
        { 1, 2, 3, 4, 4 }, // 0 beams
        { 0, 1, 2, 3, 3 }, // 1 beam
        { 0, 1, 1, 1, 1 }, // 2 beams
        { 0, 0, 0, 1, 1 }, // 3 beams
    };
    int beamCount = 0;
    if (!item->hook()) {
        beamCount = calcBeamCount(item);
    }
    bool hasTradHook = item->hook() && !style.styleB(Sid::useStraightNoteFlags);
    if (item->hook() && !hasTradHook) {
        beamCount = std::min(beamCount, 2); // the straight glyphs extend outwards after 2 beams
    }
    if (beamCount >= 4) {
        return 0;
    }
    int extensionHalfSpaces = floor(extensionOutsideStaff / 2.0);
    extensionHalfSpaces = std::min(extensionHalfSpaces, 4);
    int reduction = maxReductions[beamCount][extensionHalfSpaces];
    if (item->intrinsicMag() < 1) {
        // there is an exception for grace-sized stems with hooks.
        // reducing by the full amount puts the hooks too low. Limit reduction to 0.5sp
        if (hasTradHook) {
            reduction = std::min(reduction, 1);
        }
    } else {
        // there are a few exceptions for normal-sized (non-grace) beams
        if (beamCount == 1 && extensionHalfSpaces < 2) {
            // 1) if the extension is less than 1sp above or below the staff, they've been adjusted
            //    already to play nicely with staff lines. Reduce by 1sp.
            reduction = 2;
        } else if (beamCount == 3 && extensionHalfSpaces == 3) {
            // 2) if there are three beams and it extends 1.5sp above or below the staff, we need to
            //    *extend* the stem rather than reduce it.
            reduction = 0;
        }
        if (hasTradHook) {
            reduction = std::min(reduction, 1);
        } else if (item->hook() && item->beams() > 2) {
            reduction += 1;
        }
    }
    return reduction;
}

int StemLayout::stemOpticalAdjustment(const Chord* item, int stemEndPosition)
{
    if (item->hook() && !item->beam()) {
        return 0;
    }
    int beamCount = calcBeamCount(item);
    if (beamCount == 0 || beamCount > 2) {
        return 0;
    }
    bool isOnEvenLine = fmod(stemEndPosition + 4, 4) == 2;
    if (isOnEvenLine) {
        return 1;
    }
    return 0;
}

int StemLayout::calcMinStemLength(Chord* item, const LayoutContext& ctx)
{
    int minStemLength = 0; // in quarter spaces
    double spatium = item->spatium();
    const MStyle& style = ctx.conf().style();
    const Chord::LayoutData* ldata = item->ldata();
    const Staff* staff = item->staff();

    if (item->tremoloSingleChord()) {
        // buzz roll's height is actually half of the visual height,
        // so we need to multiply it by 2 to get the actual height
        int buzzRollMultiplier = item->tremoloSingleChord()->isBuzzRoll() ? 2 : 1;
        minStemLength += ceil(item->tremoloSingleChord()->minHeight() / item->intrinsicMag() * 4.0 * buzzRollMultiplier);
        int outSidePadding = style.styleMM(Sid::tremoloOutSidePadding).val() / spatium * 4.0;
        int noteSidePadding = style.styleMM(Sid::tremoloNoteSidePadding).val() / spatium * 4.0;

        int outsideStaffOffset = 0;
        if (!staff->isTabStaff(item->tick())) {
            Note* lineNote = ldata->up ? item->upNote() : item->downNote();
            if (lineNote->line() == INVALID_LINE) {
                lineNote->updateLine();
            }

            int line = lineNote->line();
            line *= 2; // convert to quarter spaces

            if (!ldata->up && line < -2) {
                outsideStaffOffset = -line;
            } else if (ldata->up && line > staff->lines(item->tick()) * 4) {
                outsideStaffOffset = line - (staff->lines(item->tick()) * 4) + 4;
            }
        }
        minStemLength += (outSidePadding + std::max(noteSidePadding, outsideStaffOffset));

        if (item->hook()) {
            bool straightFlags = style.styleB(Sid::useStraightNoteFlags);
            double smuflAnchor = item->hook()->smuflAnchor().y() * (ldata->up ? 1 : -1);
            int hookOffset = floor((item->hook()->height() / item->intrinsicMag() + smuflAnchor) / spatium * 4) - (straightFlags ? 0 : 2);
            // some fonts have hooks that extend very far down (making the height of the hook very large)
            // so we constrain to a reasonable maximum for hook length
            hookOffset = std::min(hookOffset, 11);
            // TODO: when the SMuFL metadata includes a cutout for flags, replace this with that metadata
            // https://github.com/w3c/smufl/issues/203
            int cutout = ldata->up ? 5 : 7;
            if (straightFlags) {
                // don't need cutout for straight flags (they are similar to beams)
                cutout = 0;
            } else if (item->beams() >= 2) {
                // beams greater than two extend outwards and thus don't factor into the cutout
                cutout -= 2;
            }

            minStemLength += hookOffset - cutout;

            // hooks with trems inside them no longer ceil (snap) to nearest 0.5sp.
            // if we want to add that back in, here is the place to do it:
            // minStemLength = ceil(minStemLength / 2.0) * 2;
        }
    }
    if (item->beam() || item->tremoloTwoChord()) {
        int beamCount = calcBeamCount(item);
        static const int minInnerStemLengths[4] = { 10, 9, 8, 7 };
        int innerStemLength = minInnerStemLengths[std::min(beamCount, 3)];
        int beamsHeight = beamCount * (style.styleB(Sid::useWideBeams) ? 4 : 3) - 1;
        int newMinStemLength = std::max(minStemLength, innerStemLength);
        newMinStemLength += beamsHeight;
        // for 4+ beams, there are a few situations where we need to lengthen the stem by 1
        int noteLine = item->line();
        int staffLines = staff->lines(item->tick());
        bool noteInStaff = (ldata->up && noteLine > 0) || (!ldata->up && noteLine < (staffLines - 1) * 2);
        if (beamCount >= 4 && noteInStaff) {
            newMinStemLength++;
        }
        minStemLength = std::max(minStemLength, newMinStemLength);
    }
    return minStemLength;
}

int StemLayout::calc4BeamsException(const Chord* item, int stemLength)
{
    const Chord::LayoutData* ldata = item->ldata();
    int difference = 0;
    int staffLines = (item->staff()->lines(item->tick()) - 1) * 2;
    if (ldata->up && item->upNote()->line() > staffLines) {
        difference = item->upNote()->line() - staffLines;
    } else if (!ldata->up && item->downNote()->line() < 0) {
        difference = std::abs(item->downNote()->line());
    }
    switch (difference) {
    case 2:
        return std::max(stemLength, 21);
    case 3:
    case 4:
        return std::max(stemLength, 23);
    default:
        return stemLength;
    }
}

int StemLayout::calcBeamCount(const Chord* item)
{
    const TremoloTwoChord* trem = item->tremoloTwoChord();
    return (trem ? trem->lines() : 0) + (item->beam() ? item->beams() : 0);
}
