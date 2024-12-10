/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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
#include <cfloat>

#include "accidentalslayout.h"
#include "layoutcontext.h"
#include "../paddingtable.h"
#include "shape.h"
#include "tlayout.h"

#include "dom/accidental.h"
#include "dom/chord.h"
#include "dom/ledgerline.h"
#include "dom/note.h"
#include "dom/score.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::score;

AccidentalsLayout::AccidentalsLayoutContext::AccidentalsLayoutContext(std::vector<Accidental*> acc,
                                                                      std::vector<Chord*> ch)
    : allAccidentals(std::move(acc)), chords(std::move(ch))
{
    sortTopDown(allAccidentals);
    stackedAccidentals.reserve(allAccidentals.size());
    stackedAccidentalsShape.elements().reserve(4 * allAccidentals.size());

    initConstants();
}

void AccidentalsLayout::AccidentalsLayoutContext::initConstants()
{
    m_spatium = allAccidentals.front()->spatium();

    // Hardcoded engraving parameters that don't have a style (some of these may get a style in future)
    m_verticalAccidentalToAccidentalClearance = 0.15 * m_spatium;
    m_verticalSharpToSharpClearance = 0.05 * m_spatium;
    m_verticalAccidentalToChordClearance = 0.10 * m_spatium;
    m_additionalPaddingForVerticals = 0.1 * m_spatium;
    m_xPosSplitThreshold = 0.5 * m_spatium;
    m_xVerticalAlignmentThreshold = 0.75 * m_spatium;
    m_sharpAndNaturalLedgerLinePadding = 0.1 * m_spatium;
    m_reducedFlatToNotePadding = 0.15 * m_spatium;
    m_flatKerningOfFourth = -0.15 * m_spatium;
    m_naturalKerningOfFourth = -0.05 * m_spatium;

    const MStyle& style = allAccidentals.front()->style();
    m_accidentalAccidentalDistance = style.styleMM(Sid::accidentalDistance);
    m_orderFollowNoteDisplacement = style.styleB(Sid::accidentalOrderFollowsNoteDisplacement);
    m_alignOctavesAcrossSubChords = style.styleB(Sid::alignAccidentalOctavesAcrossSubChords);
    m_keepSecondsTogether = style.styleB(Sid::keepAccidentalSecondsTogether);
    m_alignOffsetOctaves = style.styleB(Sid::alignOffsetOctaveAccidentals);
}

void AccidentalsLayout::layoutAccidentals(const std::vector<Chord*>& chords, LayoutContext& ctx)
{
    std::vector<Accidental*> allAccidentals;
    std::vector<Accidental*> redundantAccidentals;
    std::vector<Accidental*> invisibleAccidentals;

    collectAccidentals(chords, allAccidentals, redundantAccidentals, invisibleAccidentals);

    for (Accidental* acc : invisibleAccidentals) {
        acc->computeMag();
        TLayout::layoutAccidental(acc, acc->mutldata(), ctx.conf());
        double x = -acc->ldata()->bbox().width();
        x -= acc->note() ? acc->note()->pos().x() : 0.0;
        acc->setPos(x, 0.0);
    }

    if (allAccidentals.empty()) {
        return;
    }

    for (Accidental* acc : redundantAccidentals) {
        acc->setPos(0.0, 0.0);
        acc->setbbox(RectF());
    }

    for (Accidental* acc : allAccidentals) {
        acc->setPos(0.0, 0.0);
        acc->computeMag();
        TLayout::layoutAccidental(acc, acc->mutldata(), ctx.conf());
    }

    AccidentalsLayoutContext accidentalsLayoutContext(std::move(allAccidentals), chords);
    doAccidentalPlacement(accidentalsLayoutContext);
}

void AccidentalsLayout::collectAccidentals(const std::vector<Chord*> chords, std::vector<Accidental*>& allAccidentals,
                                           std::vector<Accidental*>& redundantAccidentals, std::vector<Accidental*>& invisibleAccidentals)
{
    for (const Chord* chord : chords) {
        for (const Note* note : chord->notes()) {
            Accidental* acc = note->accidental();
            if (!acc) {
                continue;
            }
            if (accidentalIsRedundant(acc, allAccidentals)) {
                redundantAccidentals.push_back(acc);
            } else if (!acc->addToSkyline()) {
                invisibleAccidentals.push_back(acc);
            } else {
                allAccidentals.push_back(acc);
            }
        }
    }
}

bool AccidentalsLayout::accidentalIsRedundant(const Accidental* acc, const std::vector<Accidental*>& allAccidentals)
{
    for (const Accidental* otherAcc : allAccidentals) {
        if (otherAcc->accidentalType() == acc->accidentalType() && otherAcc->line() == acc->line()) {
            return true;
        }
    }

    return false;
}

void AccidentalsLayout::doAccidentalPlacement(AccidentalsLayoutContext& ctx)
{
    createChordsShape(ctx);

    findOctavesAndSeconds(ctx);
    splitIntoSubChords(ctx);

    for (std::vector<Accidental*>& accidentalSubChord : ctx.accidentalSubChords) {
        layoutSubChord(accidentalSubChord, ctx);
    }

    verticallyAlignAccidentals(ctx);
}

void AccidentalsLayout::findOctavesAndSeconds(const AccidentalsLayoutContext& ctx)
{
    for (Accidental* acc1 : ctx.allAccidentals) {
        std::vector<Accidental*>& octaves = acc1->mutldata()->octaves.mut_value();
        octaves.clear();
        std::vector<Accidental*>& seconds = acc1->mutldata()->seconds.mut_value();
        seconds.clear();
        for (Accidental* acc2 : ctx.allAccidentals) {
            if (acc2 == acc1) {
                continue;
            }
            if (isOctave(acc1, acc2)) {
                octaves.push_back(acc2);
            } else if (abs(acc1->line() - acc2->line()) == 1) {
                seconds.push_back(acc2);
            }
        }
    }
}

void AccidentalsLayout::splitIntoSubChords(AccidentalsLayoutContext& ctx)
{
    ctx.accidentalSubChords.reserve(2);

    std::vector<Accidental*> firstGroup;
    firstGroup.reserve(ctx.allAccidentals.size());
    firstGroup.push_back(ctx.allAccidentals.front());
    ctx.accidentalSubChords.push_back(firstGroup);

    static constexpr int LINE_DIFF_OF_SEVENTH = 6;

    for (size_t i = 1; i < ctx.allAccidentals.size(); ++i) {
        Accidental* prevAcc = ctx.allAccidentals[i - 1];
        Accidental* curAcc = ctx.allAccidentals[i];
        bool startNewGroup = curAcc->line() - prevAcc->line() >= LINE_DIFF_OF_SEVENTH;
        if (startNewGroup) {
            ctx.accidentalSubChords.push_back(std::vector<Accidental*> { curAcc });
        } else {
            ctx.accidentalSubChords.back().push_back(curAcc);
        }
    }

    mergeAdjacentSubGroupsIfTooSmall(ctx);

    ctx.subChordsCountBeforeOctaveMerge = ctx.accidentalSubChords.size();
    if (ctx.alignOctavesAcrossSubChords()) {
        mergeSubGroupsWithOctavesAcross(ctx);
    }
}

void AccidentalsLayout::mergeAdjacentSubGroupsIfTooSmall(AccidentalsLayoutContext& ctx)
{
    const double approxOffsetNoteThreshold = 0.5 * ctx.spatium();
    bool groupingChanged = false;
    do {
        groupingChanged = false;
        for (size_t i = 0; i < ctx.accidentalSubChords.size() - 1; ++i) {
            std::vector<Accidental*>& thisGroup = ctx.accidentalSubChords[i];
            std::vector<Accidental*>& nextGroup = ctx.accidentalSubChords[i + 1];
            bool groupIsSmall = thisGroup.size() + nextGroup.size() <= ctx.smallGroupLimit();
            bool nextStartsWithOffsetNote = nextGroup.front()->note()->x() > approxOffsetNoteThreshold;
            if (groupIsSmall && !nextStartsWithOffsetNote) {
                groupingChanged = true;
                thisGroup.insert(thisGroup.end(), nextGroup.begin(), nextGroup.end());
                ctx.accidentalSubChords.erase(ctx.accidentalSubChords.begin() + i + 1);
            }
        }
    } while (groupingChanged);
}

void AccidentalsLayout::mergeSubGroupsWithOctavesAcross(AccidentalsLayoutContext& ctx)
{
    auto isOctaveAcrossGroups = [](const std::vector<Accidental*>& group1, const std::vector<Accidental*>& group2) {
        for (const Accidental* acc1 : group1) {
            for (Accidental* octaveAcc : acc1->ldata()->octaves.value()) {
                if (muse::contains(group2, octaveAcc)) {
                    return true;
                }
            }
        }
        return false;
    };

    bool foundOctave = false;
    do {
        foundOctave = false;
        for (size_t i = 0; i < ctx.accidentalSubChords.size() - 1; ++i) {
            std::vector<Accidental*>& thisGroup = ctx.accidentalSubChords[i];
            for (size_t j = i + 1; j < ctx.accidentalSubChords.size(); ++j) {
                std::vector<Accidental*>& nextGroup = ctx.accidentalSubChords[j];
                foundOctave = isOctaveAcrossGroups(thisGroup, nextGroup);
                if (foundOctave) {
                    thisGroup.insert(thisGroup.end(), nextGroup.begin(), nextGroup.end());
                    ctx.accidentalSubChords.erase(ctx.accidentalSubChords.begin() + j);
                    break;
                }
            }
            if (foundOctave) {
                break;
            }
        }
    } while (foundOctave);
}

void AccidentalsLayout::createChordsShape(AccidentalsLayoutContext& ctx)
{
    ctx.chordsShape.clear();

    for (const Chord* chord : ctx.chords) {
        PointF chordPos = keepAccidentalsCloseToChord(chord) ? PointF() : chord->pos();
        for (const LedgerLine* ledger : chord->ledgerLines()) {
            ctx.chordsShape.add(ledger->shape().translate(ledger->pos() + chordPos));
        }
        for (const Note* note : chord->notes()) {
            SymId noteSym = note->ldata()->cachedNoteheadSym;
            if (noteSym == SymId::noSym) {
                noteSym = note->noteHead();
            }
            ctx.chordsShape.add(note->symShapeWithCutouts(noteSym).translate(note->pos() + chordPos));
        }
        const Stem* stem = chord->stem();
        if (stem && stem->addToSkyline()) {
            ctx.chordsShape.add(stem->shape().translate(stem->pos() + chordPos));
        }
    }
}

void AccidentalsLayout::layoutSubChord(std::vector<Accidental*>& accidentals, AccidentalsLayoutContext& ctx)
{
    computeOrdering(accidentals, ctx);

    for (Accidental* acc : accidentals) {
        stackAccidental(acc, ctx);
    }
}

void AccidentalsLayout::stackAccidental(Accidental* acc, AccidentalsLayoutContext& ctx)
{
    Shape accShape = acc->shape();
    accShape.translateY(acc->note()->y());

    double x = minAccidentalToChordDistance(acc, accShape, ctx);
    accShape.translateX(-x);

    double minDistToAccidGroup = minAccidentalToAccidentalGroupDistance(acc, accShape, ctx.stackedAccidentalsShape, ctx);
    accShape.translateX(-minDistToAccidGroup);
    x += minDistToAccidGroup;

    setXposRelativeToSegment(acc, -x);

    checkZeroColumn(acc, ctx);
    ctx.stackedAccidentalsShape.add(accShape);
    ctx.stackedAccidentals.push_back(acc);
}

void AccidentalsLayout::checkZeroColumn(Accidental* acc, const AccidentalsLayoutContext& ctx)
{
    int column = acc->ldata()->column;
    if (column != 0) {
        return;
    }

    RectF thisBBox = acc->ldata()->bbox().translated(xPosRelativeToSegment(acc), 0.0);
    for (Accidental* stackedAcc : ctx.stackedAccidentals) {
        RectF placedBBox = stackedAcc->ldata()->bbox().translated(xPosRelativeToSegment(stackedAcc), 0.0);
        if (thisBBox.right() < placedBBox.left()) {
            column = std::max(column, stackedAcc->ldata()->column.value() + 1);
        }
    }
    acc->mutldata()->column = column;
}

void AccidentalsLayout::computeOrdering(std::vector<Accidental*>& accidentals, AccidentalsLayoutContext& ctx)
{
    AccidentalGroups priorityGroups = splitIntoPriorityGroups(accidentals, ctx);

    accidentals.clear();
    for (std::vector<Accidental*>& group : priorityGroups) {
        determineStackingOrder(group, ctx);
        accidentals.insert(accidentals.end(), group.begin(), group.end());
    }

    applyOrderingOffsets(accidentals);
}

AccidentalGroups AccidentalsLayout::splitIntoPriorityGroups(std::vector<Accidental*>& accidentals, AccidentalsLayoutContext& ctx)
{
    AccidentalGroups groups;

    if (ctx.orderFollowNoteDisplacement()) {
        AccidentalGroups noteSplitGroups = splitAccordingToNoteDisplacement(accidentals, ctx);
        for (auto noteSplitGroup : noteSplitGroups) {
            AccidentalGroups accidSplitGroups = splitAccordingToAccidDisplacement(noteSplitGroup, ctx);
            groups.insert(groups.end(), accidSplitGroups.begin(), accidSplitGroups.end());
        }
    } else {
        groups = splitAccordingToAccidDisplacement(accidentals, ctx);
    }

    size_t curGroupsCount = groups.size();
    if (ctx.alignOctavesAcrossSubChords()) {
        mergeSubGroupsWithOctavesAcross(ctx);
        if (groups.size() != curGroupsCount) {
            for (auto& subGroup : groups) {
                sortTopDown(subGroup);
            }
        }
    }

    return groups;
}

AccidentalGroups AccidentalsLayout::splitAccordingToAccidDisplacement(std::vector<Accidental*>& accidentals,
                                                                      const AccidentalsLayoutContext& ctx)
{
    AccidentalGroups subGroups = groupAccidentalsByXPos(accidentals, ctx);

    muse::remove_if(subGroups, [](auto& subGroup) {
        return subGroup.size() == 0;
    });

    if (subGroups.size() > 1 && subGroups[0].size() > 1) {
        moveOctavesToSecondGroup(subGroups);
    }

    for (std::vector<Accidental*>& subGroup : subGroups) {
        sortTopDown(subGroup);
    }

    return subGroups;
}

AccidentalGroups AccidentalsLayout::groupAccidentalsByXPos(std::vector<Accidental*>& accidentals, const AccidentalsLayoutContext& ctx)
{
    std::map<double, std::vector<Accidental*> > groupsOfEqualX;

    for (Accidental* acc : accidentals) {
        Shape accRoughShape = Shape(acc->symBbox(acc->symId()), acc);
        accRoughShape.translateY(acc->note()->y());
        double x = -minAccidentalToChordDistance(acc, accRoughShape, ctx) + acc->symWidth(acc->symId());
        groupsOfEqualX[std::round(x)].push_back(acc);
    }

    double splitPoint = DBL_MAX;
    double prevX = (*groupsOfEqualX.begin()).first;
    for (auto& pair : groupsOfEqualX) {
        double curX = pair.first;
        if (curX - prevX > ctx.xPosSplitThreshold()) {
            splitPoint = curX;
            break;
        }
        prevX = curX;
    }

    AccidentalGroups subGroups(2);
    for (auto& pair : groupsOfEqualX) {
        if (pair.first >= splitPoint) {
            for (Accidental* acc : pair.second) {
                subGroups[0].push_back(acc);
                acc->mutldata()->verticalSubgroup = 0;
            }
        } else {
            for (Accidental* acc : pair.second) {
                subGroups[1].push_back(acc);
                acc->mutldata()->verticalSubgroup = 1;
            }
        }
    }

    return subGroups;
}

void AccidentalsLayout::moveOctavesToSecondGroup(AccidentalGroups& subGroups)
{
    IF_ASSERT_FAILED(subGroups.size() == 2) {
        return;
    }

    std::vector<Accidental*>& firstGroup = subGroups[0];
    std::vector<Accidental*>& secondGroup = subGroups[1];
    auto iter = firstGroup.begin();
    while (iter != firstGroup.end()) {
        Accidental* acc = *iter;
        bool removeOne = false;
        for (Accidental* octaveAcc : acc->ldata()->octaves.value()) {
            if (muse::contains(secondGroup, octaveAcc)) {
                removeOne = true;
                secondGroup.push_back(acc);
                acc->mutldata()->verticalSubgroup = 1;
                break;
            }
        }
        if (removeOne) {
            iter = firstGroup.erase(iter);
        } else {
            ++iter;
        }
    }
}

AccidentalGroups AccidentalsLayout::splitAccordingToNoteDisplacement(std::vector<Accidental*>& accidentals,
                                                                     const AccidentalsLayoutContext& ctx)
{
    AccidentalGroups subGroups = groupAccidentalsByNoteXPos(accidentals);

    if (ctx.keepSecondsTogether()) {
        moveSecondsInSameGroup(subGroups);
    }

    for (size_t i = 0; i < subGroups.size(); ++i) {
        for (Accidental* acc : subGroups[i]) {
            acc->mutldata()->verticalSubgroup = static_cast<int>(i);
        }
    }

    return subGroups;
}

AccidentalGroups AccidentalsLayout::groupAccidentalsByNoteXPos(const std::vector<Accidental*>& accidentals)
{
    AccidentalGroups subGroups;

    std::map<double, std::vector<Accidental*> > groupsOfEqualNoteX;
    for (Accidental* accidental : accidentals) {
        Note* note = accidental->note();
        double x = std::round(note->pos().x() + note->chord()->pos().x());
        groupsOfEqualNoteX[x].push_back(accidental);
    }

    for (auto iter = groupsOfEqualNoteX.rbegin(); iter != groupsOfEqualNoteX.rend(); ++iter) {
        subGroups.push_back((*iter).second);
    }

    return subGroups;
}

void AccidentalsLayout::moveSecondsInSameGroup(AccidentalGroups& subGroups)
{
    bool secondMoved = false;
    do {
        secondMoved = false;
        for (size_t i = 0; i < subGroups.size() - 1; ++i) {
            std::vector<Accidental*>& thisGroup = subGroups[i];
            std::vector<Accidental*>& nextGroup = subGroups[i + 1];
            for (size_t j = 0; j < thisGroup.size(); ++j) {
                Accidental* acc1 = thisGroup[j];
                for (Accidental* secondAcc : acc1->ldata()->seconds.value()) {
                    if (muse::remove(nextGroup, secondAcc)) {
                        secondMoved = true;
                        auto position = thisGroup.begin() + j;
                        if (secondAcc->line() < acc1->line()) {
                            thisGroup.insert(position, secondAcc);
                        } else {
                            thisGroup.insert(position + 1, secondAcc);
                        }
                        break;
                    }
                }
                if (secondMoved) {
                    break;
                }
            }
            if (nextGroup.size() == 0) {
                subGroups.erase(subGroups.begin() + i + 1);
            }
            if (secondMoved) {
                break;
            }
        }
    } while (secondMoved);
}

void AccidentalsLayout::determineStackingOrder(std::vector<Accidental*>& accidentals, AccidentalsLayoutContext& ctx)
{
    size_t groupSize = accidentals.size();

    if (groupSize <= ctx.smallGroupLimit()) {
        computeStandardOrdering(accidentals, ctx);
    } else if (groupSize > ctx.largeGroupLimit()
               || (ctx.alignOctavesAcrossSubChords() && ctx.subChordsCountBeforeOctaveMerge > 1)) {
        computeCompactOrdering(accidentals, ctx);
    } else {
        computeOrderingWithLeastColumns(accidentals, ctx);
    }
}

void AccidentalsLayout::computeOrderingWithLeastColumns(std::vector<Accidental*>& accidentals, AccidentalsLayoutContext& ctx)
{
    std::vector<Accidental*> standardOrdered = accidentals;
    computeStandardOrdering(standardOrdered, ctx);

    std::vector<Accidental*> compactOrdered = accidentals;
    computeCompactOrdering(compactOrdered, ctx);

    auto totalColumns = [&](std::vector<Accidental*>& accGroup) {
        std::map<Accidental*, int> accidentalColumn;
        accidentalColumn[accGroup.front()] = 0;
        int maxColumn = 0;
        for (size_t i = 1; i < accGroup.size(); ++i) {
            Accidental* acc1 = accGroup[i];
            accidentalColumn[acc1] = maxColumn;
            for (int j = static_cast<int>(i - 1); j >= 0; --j) {
                Accidental* acc2 = accGroup[j];
                if (accidentalColumn[acc2] < maxColumn) {
                    continue;
                }
                if (!canFitInSameColumn(acc1, acc2, ctx)) {
                    maxColumn += 1;
                    accidentalColumn[acc1] = maxColumn;
                }
            }
        }
        return maxColumn;
    };

    accidentals = totalColumns(compactOrdered) < totalColumns(standardOrdered) ? compactOrdered : standardOrdered;
}

void AccidentalsLayout::computeStandardOrdering(std::vector<Accidental*>& accidentals, AccidentalsLayoutContext& ctx)
{
    std::list<Accidental*> accidentalsToPlace { accidentals.begin(), accidentals.end() };

    std::vector<Accidental*> accidentalsPlaced;
    accidentalsPlaced.reserve(accidentalsToPlace.size());

    bool pickFromTop = true;
    while (accidentalsToPlace.size() > 0) {
        Accidental* acc = pickFromTop ? accidentalsToPlace.front() : accidentalsToPlace.back();
        pickFromTop ? accidentalsToPlace.pop_front() : accidentalsToPlace.pop_back();

        accidentalsPlaced.push_back(acc);

        if (ctx.keepSecondsTogether()) {
            findAndInsertSecond(acc, accidentalsPlaced, accidentalsToPlace, ctx);
        }

        bool foundOctave = findAndInsertOctave(acc, accidentalsPlaced, accidentalsToPlace, ctx);

        if (foundOctave) {
            pickFromTop = true;
        } else {
            pickFromTop = !pickFromTop;
        }
    }

    accidentals = accidentalsPlaced;
}

void AccidentalsLayout::computeCompactOrdering(std::vector<Accidental*>& accidentals, AccidentalsLayoutContext& ctx)
{
    std::list<Accidental*> accidentalsToPlace { accidentals.begin(), accidentals.end() };
    size_t totAccidNumber = accidentalsToPlace.size();

    std::vector<Accidental*> accidentalsPlaced;
    accidentalsPlaced.reserve(totAccidNumber);

    Accidental* bottomAcc = accidentalsToPlace.back();

    bool pickFromTop = true;
    while (accidentalsToPlace.size() > 0) {
        Accidental* acc = pickFromTop ? accidentalsToPlace.front() : accidentalsToPlace.back();
        pickFromTop ? accidentalsToPlace.pop_front() : accidentalsToPlace.pop_back();

        accidentalsPlaced.push_back(acc);

        bool foundOneThatFits = false;
        bool hitBottom = false;

        if (findAndInsertOctave(acc, accidentalsPlaced, accidentalsToPlace, ctx, !pickFromTop, pickFromTop)) {
            foundOneThatFits = true;
            acc = accidentalsPlaced.back();
            if (acc == bottomAcc) {
                hitBottom = true;
            }
        }

        auto startIter = pickFromTop ? accidentalsToPlace.begin() : --accidentalsToPlace.end();
        auto endIter = pickFromTop ? accidentalsToPlace.end() : --accidentalsToPlace.begin();
        auto moveIter = [pickFromTop](auto& iter) {
            pickFromTop ? ++iter : --iter;
        };

        auto iter = startIter;
        while (iter != endIter) {
            Accidental* acc2 = *iter;
            if ((pickFromTop && acc2->line() < acc->line()) || (!pickFromTop && acc2->line() > acc->line())) {
                moveIter(iter);
                continue;
            }
            if (canFitInSameColumn(acc, acc2, ctx)) {
                if (acc2->ldata()->octaves.value().size() > 0) {
                    moveIter(iter);
                    continue;
                }
                foundOneThatFits = true;
                accidentalsPlaced.push_back(acc2);
                acc = acc2;
                if (acc2 == bottomAcc) {
                    hitBottom = true;
                }
                moveIter(iter);
                accidentalsToPlace.remove(acc2);
                continue;
            }
            moveIter(iter);
        }

        if (ctx.keepSecondsTogether()) {
            for (Accidental* alreadyPlacedAcc : accidentalsPlaced) {
                findAndInsertSecond(alreadyPlacedAcc, accidentalsPlaced, accidentalsToPlace, ctx);
            }
        }

        if (hitBottom || (foundOneThatFits && totAccidNumber <= ctx.largeGroupLimit())) {
            pickFromTop = true;
        } else {
            pickFromTop = !pickFromTop;
        }
    }

    accidentals = accidentalsPlaced;
}

void AccidentalsLayout::findAndInsertSecond(Accidental* acc, std::vector<Accidental*>& accidentalsPlaced,
                                            std::list<Accidental*>& accidentalsToPlace, AccidentalsLayoutContext& ctx)
{
    const std::vector<Accidental*> seconds = acc->ldata()->seconds.value();
    for (Accidental* secondAcc : seconds) {
        if (muse::remove(accidentalsToPlace, secondAcc)) {
            auto position = std::find(accidentalsPlaced.begin(), accidentalsPlaced.end(), acc);
            if (ctx.orderFollowNoteDisplacement() && secondAcc->note()->x() > acc->note()->x()) {
                accidentalsPlaced.insert(position, secondAcc);
            } else {
                accidentalsPlaced.push_back(secondAcc);
            }
        }
    }
}

bool AccidentalsLayout::findAndInsertOctave(Accidental* acc, std::vector<Accidental*>& accidentalsPlaced,
                                            std::list<Accidental*>& accidentalsToPlace, AccidentalsLayoutContext& ctx, bool acceptAbove,
                                            bool acceptBelow)
{
    bool foundOctave = false;
    int thisLine = acc->line();
    const std::vector<Accidental*>& octaves = acc->ldata()->octaves.value();
    for (Accidental* octaveAcc : octaves) {
        if (muse::contains(accidentalsToPlace, octaveAcc)) {
            int octaveLine = octaveAcc->line();
            bool acceptOctave = (octaveLine < thisLine && acceptAbove) || (octaveLine > thisLine && acceptBelow);
            if (acceptOctave) {
                foundOctave = true;
                accidentalsPlaced.push_back(octaveAcc);
                accidentalsToPlace.remove(octaveAcc);
                if (ctx.keepSecondsTogether()) {
                    findAndInsertSecond(octaveAcc, accidentalsPlaced, accidentalsToPlace, ctx);
                }
            }
        }
    }

    return foundOctave;
}

void AccidentalsLayout::applyOrderingOffsets(std::vector<Accidental*>& accidentals)
{
    // Set default stacking number
    int stackingNumber = 0;
    for (Accidental* acc : accidentals) {
        acc->mutldata()->stackingNumber = stackingNumber;
        ++stackingNumber;
    }

    // Limit stacking offsets within boundaries
    for (Accidental* acc : accidentals) {
        int actualStackingOrder = acc->stackingOrder();
        if (actualStackingOrder < 0) {
            acc->setStackingOrderOffset(acc->stackingOrderOffset() - actualStackingOrder);
        } else if (actualStackingOrder >= stackingNumber) {
            int diff = actualStackingOrder - stackingNumber;
            acc->setStackingOrderOffset(acc->stackingOrderOffset() - diff - 1);
        }
    }

    // Sort vector
    std::sort(accidentals.begin(), accidentals.end(), [](const Accidental* acc1, const Accidental* acc2) {
        return acc1->stackingOrder() < acc2->stackingOrder();
    });
}

double AccidentalsLayout::minAccidentalToChordDistance(Accidental* acc, const Shape& accShape, const AccidentalsLayoutContext& ctx)
{
    double dist = -DBL_MAX;
    for (const ShapeElement& accidentalElement : accShape.elements()) {
        double accTop = accidentalElement.top();
        double accBottom = accidentalElement.bottom();
        for (const ShapeElement& chordElement : ctx.chordsShape.elements()) {
            double chordElementTop = chordElement.top();
            double chordElementBottom = chordElement.bottom();
            if (mu::engraving::intersects(accTop, accBottom, chordElementTop, chordElementBottom,
                                          ctx.verticalAccidentalToChordClearance())) {
                double padding = computePadding(acc, chordElement.item(), ctx);
                padding += additionalPaddingForVerticals(acc, chordElement.item(), ctx);
                dist = std::max(dist, accidentalElement.right() - chordElement.left() + padding);
            }
            dist = std::max(dist, kerningLimitationsIntoChord(acc, accShape, chordElement, ctx));
        }
    }

    return dist;
}

double AccidentalsLayout::kerningLimitationsIntoChord(Accidental* acc, const Shape& accShape, const ShapeElement& chordElement,
                                                      const AccidentalsLayoutContext& ctx)
{
    const EngravingItem* chordItem = chordElement.item();
    if (!chordItem) {
        return 0.0;
    }

    AccidentalType accType = acc->accidentalType();
    if (chordItem->isLedgerLine() && (accType == AccidentalType::NATURAL || accType == AccidentalType::SHARP)) {
        if (chordItem->y() > accShape.top() && chordItem->y() < accShape.bottom()) {
            return accShape.right() - chordElement.left() + ctx.sharpAndNaturalLedgerLinePadding();
        }
    }

    return 0.0;
}

double AccidentalsLayout::computePadding(Accidental* acc, const EngravingItem* chordElement, const AccidentalsLayoutContext& ctx)
{
    AccidentalType accType = acc->accidentalType();
    bool isFlat = accType == AccidentalType::FLAT || accType == AccidentalType::FLAT2;

    bool kernFlatIntoLedger = isFlat && chordElement->isLedgerLine() && chordElement->y() > acc->note()->y();
    if (kernFlatIntoLedger) {
        return 0.0;
    }

    const double yApproxThresholdForSharpLedgerKerning = 1.1 * ctx.spatium();
    bool kernSharpIntoLedger = accType == AccidentalType::SHARP && chordElement->isLedgerLine()
                               && chordElement->y() > acc->note()->y() + yApproxThresholdForSharpLedgerKerning;
    if (kernSharpIntoLedger) {
        return 0.0;
    }

    bool kernFlatIntoNote = isFlat && chordElement->isNote() && chordElement->y() > acc->note()->y();
    if (kernFlatIntoNote) {
        return ctx.reducedFlatToNotePadding();
    }

    const PaddingTable& paddingTable = acc->score()->paddingTable();

    return paddingTable.at(ElementType::ACCIDENTAL).at(chordElement->type()) * 0.5 * (acc->mag() + chordElement->mag());
}

double AccidentalsLayout::minAccidentalToAccidentalGroupDistance(Accidental* acc, Shape accShape, const Shape& accidentalsShape,
                                                                 const AccidentalsLayoutContext& ctx)
{
    double dist = 0.0;
    int column = acc->ldata()->column;

    bool collisionFound;
    int iter = 0;
    static constexpr int safetyNet = 50;
    do {
        ++iter;
        double curDist = 0.0;
        collisionFound = false;
        for (const ShapeElement& groupElement : accidentalsShape.elements()) {
            const Accidental* acc2 = toAccidental(groupElement.item());

            if (isExceptionOfFourth(acc, acc2)) {
                double distFourth = kerningOfFourth(accShape, groupElement, ctx);
                if (!muse::RealIsNull(distFourth) && distFourth > curDist) {
                    collisionFound = true;
                    curDist = distFourth;
                }
                column = std::max(column, acc2->ldata()->column + 1);
                continue;
            }

            double vertPadding = verticalPadding(acc, acc2, ctx);
            double horPadding = horizontalPadding(acc, acc2, ctx);
            if (accShape.intersects(groupElement.adjusted(-horPadding, -vertPadding, horPadding, vertPadding))) {
                collisionFound = true;
            } else {
                continue;
            }

            if (!isExceptionOfNaturalsSixth(acc, acc2)) {
                column = std::max(column, acc2->ldata()->column + 1);
            }

            double groupTop = groupElement.top();
            double groupBottom = groupElement.bottom();
            for (ShapeElement& accElement : accShape.elements()) {
                if (mu::engraving::intersects(accElement.top(), accElement.bottom(), groupTop, groupBottom, vertPadding)) {
                    double minDist = accElement.right() - groupElement.left() + horPadding;
                    minDist += additionalPaddingForVerticals(acc, acc2, ctx);
                    curDist = std::max(curDist, minDist);
                }
            }
        }

        accShape.translateX(-curDist);
        dist += curDist;
    } while (collisionFound && iter < safetyNet);

    IF_ASSERT_FAILED(iter < safetyNet) {
        LOGE() << "Accidental layout error.";
    }

    acc->mutldata()->column = column;

    return dist;
}

bool AccidentalsLayout::isExceptionOfFourth(const Accidental* acc1, const Accidental* acc2)
{
    if (!acc1 || !acc2) {
        return false;
    }

    if ((acc1->bracket() == AccidentalBracket::BRACKET || acc2->bracket() == AccidentalBracket::BRACKET)
        && acc1->mag() == 1 && acc2->mag() == 1) {
        return false;
    }

    AccidentalType accType1 = acc1->accidentalType();
    AccidentalType accType2 = acc2->accidentalType();

    return (accType1 == AccidentalType::FLAT || accType1 == AccidentalType::FLAT2)
           && (accType2 == AccidentalType::FLAT || accType2 == AccidentalType::FLAT2 || accType2 == AccidentalType::NATURAL)
           && acc1->line() - acc2->line() == 3;
}

bool AccidentalsLayout::isExceptionOfNaturalsSixth(const Accidental* acc1, const Accidental* acc2)
{
    return acc1->accidentalType() == AccidentalType::NATURAL
           && acc2->accidentalType() == AccidentalType::NATURAL
           && abs(acc2->line() - acc1->line()) == 5;
}

bool AccidentalsLayout::canFitInSameColumn(const Accidental* acc1, const Accidental* acc2, AccidentalsLayoutContext& ctx)
{
    if (isExceptionOfNaturalsSixth(acc1, acc2)) {
        return true;
    }

    RectF acc1Box = acc1->ldata()->bbox().translated(PointF(0.0, acc1->note()->y()));
    RectF acc2Box = acc2->ldata()->bbox().translated(PointF(0.0, acc2->note()->y()));

    return !mu::engraving::intersects(acc1Box.top(), acc1Box.bottom(), acc2Box.top(), acc2Box.bottom(),
                                      verticalPadding(acc1, acc2, ctx));
}

bool AccidentalsLayout::isOctave(const Accidental* acc1, const Accidental* acc2)
{
    return acc1->accidentalType() == acc2->accidentalType() && std::abs(acc2->line() - acc1->line()) % 7 == 0;
}

double AccidentalsLayout::kerningOfFourth(const Shape& accShape, const ShapeElement& accGroupShapeElement,
                                          const AccidentalsLayoutContext& ctx)
{
    AccidentalType accType2 = toAccidental(accGroupShapeElement.item())->accidentalType();

    return accShape.right() - accGroupShapeElement.left()
           + (accType2 == AccidentalType::NATURAL ? ctx.naturalKerningOfFourth() : ctx.flatKerningOfFourth());
}

double AccidentalsLayout::additionalPaddingForVerticals(const Accidental* acc, const EngravingItem* item,
                                                        const AccidentalsLayoutContext& ctx)
{
    if (acc->accidentalType() != AccidentalType::NATURAL || acc->bracket() != AccidentalBracket::NONE
        || (!item->isAccidental() && !item->isStem())) {
        return 0.0;
    }

    if (item->isAccidental()) {
        const Accidental* acc2 = toAccidental(item);
        AccidentalType accType2 = acc2->accidentalType();
        if (accType2 != AccidentalType::NATURAL && accType2 != AccidentalType::FLAT && accType2 != AccidentalType::FLAT2) {
            return 0.0;
        }
        int lineDiff = acc2->line() - acc->line();
        if (lineDiff < -2 || lineDiff > 5) {
            return 0.0;
        }
    }

    return ctx.additionalPaddingForVerticals();
}

void AccidentalsLayout::verticallyAlignAccidentals(AccidentalsLayoutContext& ctx)
{
    std::set<Accidental*> accidentalsAlreadyGrouped;
    std::map<Accidental*, std::vector<Accidental*> > verticalSets;

    if (ctx.alignOffsetOctaves()) {
        collectVerticalSetsOfOffsetOctaves(accidentalsAlreadyGrouped, verticalSets, ctx);
    }

    collectVerticalSets(accidentalsAlreadyGrouped, verticalSets, ctx);

    if (verticalSets.empty()) {
        return;
    }

    // Turn map into vector
    AccidentalGroups vertSets;
    vertSets.reserve(verticalSets.size());
    for (auto& pair : verticalSets) {
        vertSets.push_back(pair.second);
    }

    // Sort vertical sets by column
    std::sort(vertSets.begin(), vertSets.end(), [&](auto& set1, auto& set2) {
        return set1.front()->ldata()->column < set2.front()->ldata()->column;
    });

    alignVerticalSets(vertSets, ctx);
}

void AccidentalsLayout::collectVerticalSetsOfOffsetOctaves
    (std::set<Accidental*>& accidentalsAlreadyGrouped, std::map<Accidental*, std::vector<Accidental*> >& verticalSets,
    AccidentalsLayoutContext& ctx)
{
    for (std::vector<Accidental*>& accidentalGroup : ctx.accidentalSubChords) {
        for (size_t i = 0; i < accidentalGroup.size(); ++i) {
            Accidental* acc1 = accidentalGroup[i];
            acc1->ldata()->column.value();
            if (muse::contains(accidentalsAlreadyGrouped, acc1)) {
                continue;
            }
            double x1 = xPosRelativeToSegment(acc1);
            for (size_t j = i + 1; j < accidentalGroup.size(); ++j) {
                Accidental* acc2 = accidentalGroup[j];
                if (!isOctave(acc1, acc2) || muse::contains(accidentalsAlreadyGrouped, acc2)) {
                    continue;
                }
                double x2 = xPosRelativeToSegment(acc2);
                bool alignOctave = abs(x1 - x2) < 2.0 * ctx.spatium();
                acc2->ldata()->column.value();
                if (alignOctave) {
                    std::vector<Accidental*>& verticalSet = verticalSets[acc1];
                    if (verticalSet.empty()) {
                        verticalSet.push_back(acc1);
                    }
                    verticalSet.push_back(acc2);
                    accidentalsAlreadyGrouped.insert(acc1);
                    accidentalsAlreadyGrouped.insert(acc2);
                    int maxColumn = 0;
                    for (Accidental* acc : verticalSet) {
                        maxColumn = std::max(maxColumn, acc->ldata()->column.value());
                    }
                    for (Accidental* acc : verticalSet) {
                        acc->mutldata()->column = maxColumn;
                    }
                }
            }
        }
    }
}

void AccidentalsLayout::collectVerticalSets(
    std::set<Accidental*>& accidentalsAlreadyGrouped, std::map<Accidental*, std::vector<Accidental*> >& verticalSets,
    AccidentalsLayoutContext& ctx)
{
    for (size_t i = 0; i < ctx.allAccidentals.size(); ++i) {
        Accidental* acc1 = ctx.allAccidentals[i];
        if (muse::contains(accidentalsAlreadyGrouped, acc1)) {
            continue;
        }
        int column1 = acc1->ldata()->column;
        int verticalSub1 = acc1->ldata()->verticalSubgroup;
        double x1 = xPosRelativeToSegment(acc1);
        for (size_t j = i + 1; j < ctx.allAccidentals.size(); ++j) {
            Accidental* acc2 = ctx.allAccidentals[j];
            if (acc2->accidentalType() != acc1->accidentalType() || muse::contains(accidentalsAlreadyGrouped, acc2)) {
                continue;
            }
            int column2 = acc2->ldata()->column;
            int verticalSub2 = acc2->ldata()->verticalSubgroup;
            double x2 = xPosRelativeToSegment(acc2);
            bool alignAccidentals = column1 == column2 && verticalSub1 == verticalSub2 && abs(x1 - x2) < ctx.xVerticalAlignmentThreshold();
            if (alignAccidentals) {
                std::vector<Accidental*>& verticalSet = verticalSets[acc1];
                if (verticalSet.empty()) {
                    verticalSet.push_back(acc1);
                }
                verticalSet.push_back(acc2);
                accidentalsAlreadyGrouped.insert(acc1);
                accidentalsAlreadyGrouped.insert(acc2);
            }
        }
    }
}

void AccidentalsLayout::alignVerticalSets(AccidentalGroups& vertSets, AccidentalsLayoutContext& ctx)
{
    for (std::vector<Accidental*>& vertSet : vertSets) {
        // Align the set
        double x = DBL_MAX;
        for (Accidental* acc : vertSet) {
            x = std::min(x, xPosRelativeToSegment(acc));
        }
        for (Accidental* acc : vertSet) {
            setXposRelativeToSegment(acc, x);
        }

        // Re-check the outer ones for collisions
        int curColumn = vertSet.front()->ldata()->column.value();
        Shape accidentalGroupShape;
        for (std::vector<Accidental*>& group : ctx.accidentalSubChords) {
            for (Accidental* acc : group) {
                double curXPos = xPosRelativeToSegment(acc);
                Shape accShape = acc->shape().translate(PointF(curXPos, acc->note()->y()));
                int accColumn = acc->ldata()->column;
                if (accColumn >= curColumn && !muse::contains(vertSet, acc)) {
                    double minDistToAccidGroup = minAccidentalToAccidentalGroupDistance(acc, accShape, accidentalGroupShape, ctx);
                    accShape.translateX(-minDistToAccidGroup);
                    setXposRelativeToSegment(acc, curXPos - minDistToAccidGroup);
                }
                accidentalGroupShape.add(accShape);
            }
        }
    }
}

void AccidentalsLayout::setXposRelativeToSegment(Accidental* accidental, double x)
{
    Note* note = accidental->note();
    Chord* chord = note ? note->chord() : nullptr;
    if (note) {
        x -= note->pos().x();
    }
    if (chord && !keepAccidentalsCloseToChord(chord)) {
        x -= chord->pos().x();
    }
    accidental->mutldata()->setPosX(x);
}

double AccidentalsLayout::xPosRelativeToSegment(const Accidental* accidental)
{
    Note* note = accidental->note();
    Chord* chord = note ? note->chord() : nullptr;
    double x = accidental->ldata()->pos().x();
    if (note) {
        x += note->pos().x();
    }
    if (chord) {
        x += chord->pos().x();
    }
    return x;
}

void AccidentalsLayout::sortTopDown(std::vector<Accidental*>& accidentals)
{
    std::sort(accidentals.begin(), accidentals.end(), [](const Accidental* acc1, const Accidental* acc2){
        int line1 = acc1->line();
        int line2 = acc2->line();
        if (line1 == line2) {
            Note* note1 = acc1->note();
            Note* note2 = acc2->note();
            return note1->x() + note1->chord()->x() > note2->x() + note2->chord()->x();
        }
        return line1 < line2;
    });
}

bool AccidentalsLayout::keepAccidentalsCloseToChord(const Chord* chord)
{
    return chord->isTrillCueNote() || chord->isGraceAfter();
}

double AccidentalsLayout::verticalPadding(const Accidental* acc1, const Accidental* acc2, const AccidentalsLayoutContext& ctx)
{
    double verticalClearance = acc1->accidentalType() == AccidentalType::SHARP && acc2->accidentalType() == AccidentalType::SHARP
                               ? ctx.verticalSharpToSharpClearance() : ctx.verticalAccidentalToAccidentalClearance();
    return verticalClearance *= 0.5 * (acc1->mag() + acc2->mag());
}

double AccidentalsLayout::horizontalPadding(const Accidental* acc1, const Accidental* acc2, const AccidentalsLayoutContext& ctx)
{
    return ctx.accidentalAccidentalDistance() * (0.5 * (acc1->mag() + acc2->mag()));
}
