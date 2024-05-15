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
using namespace mu::engraving::rendering::dev;

void AccidentalsLayout::layoutAccidentals(const std::vector<Chord*>& chords, LayoutContext& ctx)
{
    std::vector<Accidental*> allAccidentals = collectAccidentals(chords);

    if (allAccidentals.empty()) {
        return;
    }

    for (Accidental* acc : allAccidentals) {
        acc->computeMag();
        TLayout::layoutAccidental(acc, acc->mutldata(), ctx.conf());
    }

    AccidentalPlacementEngine accidentalPlacementEngine(allAccidentals, chords);
    accidentalPlacementEngine.doAccidentalPlacement();
}

std::vector<Accidental*> AccidentalsLayout::collectAccidentals(const std::vector<Chord*> chords)
{
    std::vector<Accidental*> accidentals;

    for (Chord* chord : chords) {
        for (Note* note : chord->notes()) {
            Accidental* acc = note->accidental();
            if (!acc) {
                continue;
            }
            acc->setPos(0.0, 0.0);
            bool isRedundant = false;
            for (Accidental* otherAcc : accidentals) {
                if (otherAcc->accidentalType() == acc->accidentalType() && otherAcc->line() == acc->line()) {
                    isRedundant = true;
                    break;
                }
            }
            if (isRedundant) {
                acc->setbbox(RectF());
                continue;
            }
            accidentals.push_back(acc);
        }
    }

    return accidentals;
}

AccidentalsLayout::AccidentalPlacementEngine::AccidentalPlacementEngine(const std::vector<Accidental*>& allAccidentals,
                                                                        const std::vector<Chord*>& chords)
    : m_allAccidentals(allAccidentals), m_chords(chords)
{
    sortTopDown(m_allAccidentals);

    Accidental* accidental = allAccidentals.front();
    const MStyle& style = accidental->style();

    m_spatium = accidental->spatium();

    m_verticalAccidentalToAccidentalClearance = 0.15 * m_spatium;
    m_verticalSharpToSharpClearance = 0.05 * m_spatium;
    m_verticalAccidentalToChordClearance = 0.10 * m_spatium;
    m_additionalPaddingForVerticals = 0.1 * m_spatium;
    m_accidentalAccidentalDistance = style.styleMM(Sid::accidentalDistance);

    m_orderFollowNoteDisplacement = style.styleB(Sid::accidentalOrderFollowsNoteDisplacement);
    m_alignOctavesAcrossSubChords = style.styleB(Sid::alignAccidentalOctavesAcrossSubChords);
    m_keepSecondsTogether = style.styleB(Sid::keepAccidentalSecondsTogether);
    m_alignOffsetOctaves = style.styleB(Sid::alignOffsetOctaveAccidentals);

    m_stackedAccidentals.reserve(m_allAccidentals.size());
    m_stackedAccidentalsShape.elements().reserve(4 * m_allAccidentals.size());
}

void AccidentalsLayout::AccidentalPlacementEngine::doAccidentalPlacement()
{
    createChordsShape();

    findOctaves();
    findSeconds();
    splitIntoSubChords();

    for (std::vector<Accidental*>& accidentalSubChord : m_accidentalSubChords) {
        layoutSubChord(accidentalSubChord);
    }

    verticallyAlignAccidentals();
}

void AccidentalsLayout::AccidentalPlacementEngine::findOctaves()
{
    for (Accidental* acc1 : m_allAccidentals) {
        std::vector<Accidental*>& octaves = acc1->mutldata()->octaves.mut_value();
        octaves.clear();
        for (Accidental* acc2 : m_allAccidentals) {
            if (acc2 == acc1) {
                continue;
            }
            if (isOctave(acc1, acc2)) {
                octaves.push_back(acc2);
            }
        }
    }
}

void AccidentalsLayout::AccidentalPlacementEngine::findSeconds()
{
    for (Accidental* acc1 : m_allAccidentals) {
        std::vector<Accidental*>& seconds = acc1->mutldata()->seconds.mut_value();
        seconds.clear();
        for (Accidental* acc2 : m_allAccidentals) {
            if (acc2 == acc1) {
                continue;
            }
            if (abs(acc1->line() - acc2->line()) == 1) {
                seconds.push_back(acc2);
            }
        }
    }
}

void AccidentalsLayout::AccidentalPlacementEngine::splitIntoSubChords()
{
    m_accidentalSubChords.reserve(2);

    std::vector<Accidental*> firstGroup;
    firstGroup.reserve(m_allAccidentals.size());
    firstGroup.push_back(m_allAccidentals.front());
    m_accidentalSubChords.push_back(firstGroup);

    for (int i = 1; i < m_allAccidentals.size(); ++i) {
        Accidental* prevAcc = m_allAccidentals[i - 1];
        Accidental* curAcc = m_allAccidentals[i];
        bool startNewGroup = curAcc->line() - prevAcc->line() >= 6;
        if (startNewGroup) {
            m_accidentalSubChords.push_back(std::vector<Accidental*> { curAcc });
        } else {
            m_accidentalSubChords.back().push_back(curAcc);
        }
    }

    mergeAdjacentSubGroupsIfTooSmall(m_accidentalSubChords);

    m_subChordsCountBeforeOctaveMerge = m_accidentalSubChords.size();
    if (m_alignOctavesAcrossSubChords) {
        mergeSubGroupsWithOctavesAcross(m_accidentalSubChords);
    }
}

void AccidentalsLayout::AccidentalPlacementEngine::mergeAdjacentSubGroupsIfTooSmall(std::vector<std::vector<Accidental*> >& subGroups)
{
    bool groupingChanged = false;
    do {
        groupingChanged = false;
        for (int i = 0; i < subGroups.size() - 1; ++i) {
            std::vector<Accidental*>& thisGroup = subGroups[i];
            std::vector<Accidental*>& nextGroup = subGroups[i + 1];
            bool groupIsSmall = thisGroup.size() + nextGroup.size() <= m_smallGroupLimit;
            bool nextStartsWithOffsetNote = nextGroup.front()->note()->x() > 0.5 * m_spatium;
            if (groupIsSmall && !nextStartsWithOffsetNote) {
                groupingChanged = true;
                thisGroup.insert(thisGroup.end(), nextGroup.begin(), nextGroup.end());
                subGroups.erase(m_accidentalSubChords.begin() + i + 1);
            }
        }
    } while (groupingChanged);
}

void AccidentalsLayout::AccidentalPlacementEngine::mergeSubGroupsWithOctavesAcross(
    std::vector<std::vector<Accidental*> >& subGroups)
{
    bool foundOctave = false;
    do {
        foundOctave = false;
        for (int i = 0; i < subGroups.size() - 1; ++i) {
            std::vector<Accidental*>& thisGroup = subGroups[i];
            for (int j = i + 1; j < subGroups.size(); ++j) {
                std::vector<Accidental*>& nextGroup = subGroups[j];
                for (Accidental* acc : thisGroup) {
                    for (Accidental* octaveAcc : acc->ldata()->octaves.value()) {
                        if (muse::contains(nextGroup, octaveAcc)) {
                            foundOctave = true;
                            break;
                        }
                    }
                    if (foundOctave) {
                        break;
                    }
                }
                if (foundOctave) {
                    thisGroup.insert(thisGroup.end(), nextGroup.begin(), nextGroup.end());
                    subGroups.erase(subGroups.begin() + j);
                    break;
                }
            }
            if (foundOctave) {
                break;
            }
        }
    } while (foundOctave);
}

void AccidentalsLayout::AccidentalPlacementEngine::createChordsShape()
{
    m_chordsShape.clear();

    for (Chord* chord : m_chords) {
        LedgerLine* ledger = chord->ledgerLines();
        while (ledger) {
            m_chordsShape.add(ledger->shape().translate(ledger->pos() + chord->pos()));
            ledger = ledger->next();
        }
        for (Note* note : chord->notes()) {
            SymId noteSym = note->ldata()->cachedNoteheadSym;
            if (noteSym == SymId::noSym) {
                noteSym = note->noteHead();
            }
            m_chordsShape.add(note->symShapeWithCutouts(noteSym).translate(note->pos() + chord->pos()));
        }
        Stem* stem = chord->stem();
        if (stem) {
            m_chordsShape.add(stem->shape().translate(stem->pos() + chord->pos()));
        }
    }
}

void AccidentalsLayout::AccidentalPlacementEngine::layoutSubChord(std::vector<Accidental*>& accidentals)
{
    computeOrdering(accidentals);

    for (Accidental* acc : accidentals) {
        stackAccidental(acc);
    }
}

void AccidentalsLayout::AccidentalPlacementEngine::stackAccidental(Accidental* acc)
{
    Shape accShape = acc->shape();
    accShape.translateY(acc->note()->y());

    double x = minAccidentalToChordDistance(acc, accShape);
    accShape.translateX(-x);

    double minDistToAccidGroup = minAccidentalToAccidentalGroupDistance(acc, accShape, m_stackedAccidentalsShape);
    accShape.translateX(-minDistToAccidGroup);
    x += minDistToAccidGroup;

    setXposRelativeToSegment(acc, -x);

    checkZeroColumn(acc);

    m_stackedAccidentalsShape.add(accShape);
    m_stackedAccidentals.push_back(acc);
}

void AccidentalsLayout::AccidentalPlacementEngine::checkZeroColumn(Accidental* acc)
{
    int column = acc->ldata()->column;
    if (column != 0) {
        return;
    }

    RectF thisBBox = acc->ldata()->bbox().translated(xPosRelativeToSegment(acc), 0.0);
    for (Accidental* stackedAcc : m_stackedAccidentals) {
        RectF placedBBox = stackedAcc->ldata()->bbox().translated(xPosRelativeToSegment(stackedAcc), 0.0);
        if (thisBBox.right() < placedBBox.left()) {
            column = std::max(column, stackedAcc->ldata()->column.value() + 1);
        }
    }
    acc->mutldata()->column = column;
}

void AccidentalsLayout::AccidentalPlacementEngine::computeOrdering(std::vector<Accidental*>& accidentals)
{
    std::vector<std::vector<Accidental*> > priorityGroups = splitIntoPriorityGroups(accidentals);

    accidentals.clear();
    for (std::vector<Accidental*>& group : priorityGroups) {
        determineStackingOrder(group);
        accidentals.insert(accidentals.end(), group.begin(), group.end());
    }

    applyOrderingOffsets(accidentals);
}

std::vector<std::vector<Accidental*> > AccidentalsLayout::AccidentalPlacementEngine::splitIntoPriorityGroups
    (std::vector<Accidental*>& accidentals)
{
    std::vector<std::vector<Accidental*> > groups;

    if (m_orderFollowNoteDisplacement) {
        std::vector<std::vector<Accidental*> > noteSplitGroups = splitAccordingToNoteDisplacement(accidentals);
        for (auto noteSplitGroup : noteSplitGroups) {
            std::vector<std::vector<Accidental*> > accidSplitGroups = splitAccordingToAccidDisplacement(noteSplitGroup);
            groups.insert(groups.end(), accidSplitGroups.begin(), accidSplitGroups.end());
        }
    } else {
        groups = splitAccordingToAccidDisplacement(accidentals);
    }

    size_t curGroupsCount = groups.size();
    if (m_alignOctavesAcrossSubChords) {
        mergeSubGroupsWithOctavesAcross(groups);
        if (groups.size() != curGroupsCount) {
            for (auto& subGroup : groups) {
                sortTopDown(subGroup);
            }
        }
    }

    return groups;
}

std::vector<std::vector<Accidental*> > AccidentalsLayout::AccidentalPlacementEngine::splitAccordingToAccidDisplacement
    (std::vector<Accidental*>& accidentals)
{
    std::vector<std::vector<Accidental*> > subGroups = groupAccidentalsByXPos(accidentals);

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

std::vector<std::vector<Accidental*> > AccidentalsLayout::AccidentalPlacementEngine::groupAccidentalsByXPos
    (std::vector<Accidental*>& accidentals)
{
    std::map<double, std::vector<Accidental*> > groupsOfEqualX;

    for (Accidental* acc : accidentals) {
        Shape accRoughShape = Shape(acc->symBbox(acc->symId()), acc);
        accRoughShape.translateY(acc->note()->y());
        double x = -minAccidentalToChordDistance(acc, accRoughShape) + acc->symWidth(acc->symId());
        groupsOfEqualX[std::round(x)].push_back(acc);
    }

    double splitPoint = DBL_MAX;
    double prevX = (*groupsOfEqualX.begin()).first;
    for (auto& pair : groupsOfEqualX) {
        double curX = pair.first;
        if (curX - prevX > 0.5 * m_spatium) {
            splitPoint = curX;
            break;
        }
        prevX = curX;
    }

    std::vector<std::vector<Accidental*> > subGroups(2);
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

void AccidentalsLayout::AccidentalPlacementEngine::moveOctavesToSecondGroup(std::vector<std::vector<Accidental*> >& subGroups)
{
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

std::vector<std::vector<Accidental*> > AccidentalsLayout::AccidentalPlacementEngine::splitAccordingToNoteDisplacement
    (std::vector<Accidental*>& accidentals)
{
    std::vector<std::vector<Accidental*> > subGroups = groupAccidentalsByNoteXPos(accidentals);

    if (m_keepSecondsTogether) {
        moveSecondsInSameGroup(subGroups);
    }

    for (int i = 0; i < subGroups.size(); ++i) {
        for (Accidental* acc : subGroups[i]) {
            acc->mutldata()->verticalSubgroup = i;
        }
    }

    return subGroups;
}

std::vector<std::vector<Accidental*> > AccidentalsLayout::AccidentalPlacementEngine::groupAccidentalsByNoteXPos
    (std::vector<Accidental*>& accidentals)
{
    std::vector<std::vector<Accidental*> > subGroups;

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

void AccidentalsLayout::AccidentalPlacementEngine::moveSecondsInSameGroup(std::vector<std::vector<Accidental*> >& subGroups)
{
    bool secondMoved = false;
    do {
        secondMoved = false;
        for (int i = 0; i < subGroups.size() - 1; ++i) {
            std::vector<Accidental*>& thisGroup = subGroups[i];
            std::vector<Accidental*>& nextGroup = subGroups[i + 1];
            for (int j = 0; j < thisGroup.size(); ++j) {
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

void AccidentalsLayout::AccidentalPlacementEngine::determineStackingOrder(std::vector<Accidental*>& accidentals)
{
    size_t groupSize = accidentals.size();

    if (groupSize <= m_smallGroupLimit) {
        computeStandardOrdering(accidentals);
    } else if (groupSize > m_largeGroupLimit || (m_alignOctavesAcrossSubChords && m_subChordsCountBeforeOctaveMerge > 1)) {
        computeCompactOrdering(accidentals);
    } else {
        computeOrderingWithLeastColumns(accidentals);
    }
}

void AccidentalsLayout::AccidentalPlacementEngine::computeOrderingWithLeastColumns(std::vector<Accidental*>& accidentals)
{
    std::vector<Accidental*> standardOrdered = accidentals;
    computeStandardOrdering(standardOrdered);

    std::vector<Accidental*> compactOrdered = accidentals;
    computeCompactOrdering(compactOrdered);

    auto totalColumns = [&](std::vector<Accidental*>& accGroup) {
        std::map<Accidental*, int> accidentalColumn;
        accidentalColumn[accGroup.front()] = 0;
        int maxColumn = 0;
        for (int i = 1; i < accGroup.size(); ++i) {
            Accidental* acc1 = accGroup[i];
            accidentalColumn[acc1] = maxColumn;
            for (int j = i - 1; j >= 0; --j) {
                Accidental* acc2 = accGroup[j];
                if (accidentalColumn[acc2] < maxColumn) {
                    continue;
                }
                if (!canFitInSameColumn(acc1, acc2)) {
                    maxColumn += 1;
                    accidentalColumn[acc1] = maxColumn;
                }
            }
        }
        return maxColumn;
    };

    accidentals = totalColumns(compactOrdered) < totalColumns(standardOrdered) ? compactOrdered : standardOrdered;
}

void AccidentalsLayout::AccidentalPlacementEngine::computeStandardOrdering(std::vector<Accidental*>& accidentals)
{
    std::list<Accidental*> accidentalsToPlace { accidentals.begin(), accidentals.end() };

    std::vector<Accidental*> accidentalsPlaced;
    accidentalsPlaced.reserve(accidentalsToPlace.size());

    bool pickFromTop = true;
    while (accidentalsToPlace.size() > 0) {
        Accidental* acc = pickFromTop ? accidentalsToPlace.front() : accidentalsToPlace.back();
        pickFromTop ? accidentalsToPlace.pop_front() : accidentalsToPlace.pop_back();

        accidentalsPlaced.push_back(acc);

        if (m_keepSecondsTogether) {
            findAndInsertSecond(acc, accidentalsPlaced, accidentalsToPlace);
        }

        bool foundOctave = findAndInsertOctave(acc, accidentalsPlaced, accidentalsToPlace);

        if (foundOctave) {
            pickFromTop = true;
        } else {
            pickFromTop = !pickFromTop;
        }
    }

    accidentals = accidentalsPlaced;
}

void AccidentalsLayout::AccidentalPlacementEngine::computeCompactOrdering(std::vector<Accidental*>& accidentals)
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

        if (findAndInsertOctave(acc, accidentalsPlaced, accidentalsToPlace, !pickFromTop, pickFromTop)) {
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
            if (canFitInSameColumn(acc, acc2)) {
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

        if (m_keepSecondsTogether) {
            for (Accidental* alreadyPlacedAcc : accidentalsPlaced) {
                findAndInsertSecond(alreadyPlacedAcc, accidentalsPlaced, accidentalsToPlace);
            }
        }

        if (hitBottom || (foundOneThatFits && totAccidNumber <= m_largeGroupLimit)) {
            pickFromTop = true;
        } else {
            pickFromTop = !pickFromTop;
        }
    }

    accidentals = accidentalsPlaced;
}

void AccidentalsLayout::AccidentalPlacementEngine::findAndInsertSecond(Accidental* acc, std::vector<Accidental*>& accidentalsPlaced,
                                                                       std::list<Accidental*>& accidentalsToPlace)
{
    const std::vector<Accidental*> seconds = acc->ldata()->seconds.value();
    for (Accidental* secondAcc : seconds) {
        if (muse::remove(accidentalsToPlace, secondAcc)) {
            auto position = std::find(accidentalsPlaced.begin(), accidentalsPlaced.end(), acc);
            if (m_orderFollowNoteDisplacement && secondAcc->note()->x() > acc->note()->x()) {
                accidentalsPlaced.insert(position, secondAcc);
            } else {
                accidentalsPlaced.push_back(secondAcc);
            }
        }
    }
}

bool AccidentalsLayout::AccidentalPlacementEngine::findAndInsertOctave(Accidental* acc, std::vector<Accidental*>& accidentalsPlaced,
                                                                       std::list<Accidental*>& accidentalsToPlace, bool acceptAbove,
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
                if (m_keepSecondsTogether) {
                    findAndInsertSecond(octaveAcc, accidentalsPlaced, accidentalsToPlace);
                }
            }
        }
    }

    return foundOctave;
}

void AccidentalsLayout::AccidentalPlacementEngine::applyOrderingOffsets(std::vector<Accidental*>& accidentals)
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
        return acc1->stackingOrder() <= acc2->stackingOrder();
    });
}

double AccidentalsLayout::AccidentalPlacementEngine::minAccidentalToChordDistance(Accidental* acc, const Shape& accShape)
{
    double dist = -DBL_MAX;
    for (const ShapeElement& accidentalElement : accShape.elements()) {
        double accTop = accidentalElement.top();
        double accBottom = accidentalElement.bottom();
        for (const ShapeElement& chordElement : m_chordsShape.elements()) {
            double chordElementTop = chordElement.top();
            double chordElementBottom = chordElement.bottom();
            if (mu::engraving::intersects(accTop, accBottom, chordElementTop, chordElementBottom, m_verticalAccidentalToChordClearance)) {
                double padding = computePadding(acc, chordElement.item());
                padding += additionalPaddingForVerticals(acc, chordElement.item());
                dist = std::max(dist, accidentalElement.right() - chordElement.left() + padding);
            }
            dist = std::max(dist, kerningLimitationsIntoChord(acc, accShape, chordElement));
        }
    }

    return dist;
}

double AccidentalsLayout::AccidentalPlacementEngine::kerningLimitationsIntoChord(Accidental* acc, const Shape& accShape,
                                                                                 const ShapeElement& chordElement)
{
    const EngravingItem* chordItem = chordElement.item();
    if (!chordItem) {
        return 0.0;
    }

    AccidentalType accType = acc->accidentalType();
    if (chordItem->isLedgerLine() && (accType == AccidentalType::NATURAL || accType == AccidentalType::SHARP)) {
        if (chordItem->y() > accShape.top() && chordItem->y() < accShape.bottom()) {
            return accShape.right() - chordElement.left() + 0.1 * m_spatium;
        }
    }

    return 0.0;
}

double AccidentalsLayout::AccidentalPlacementEngine::computePadding(Accidental* acc, const EngravingItem* chordElement)
{
    AccidentalType accType = acc->accidentalType();
    bool isFlat = accType == AccidentalType::FLAT || accType == AccidentalType::FLAT2;

    bool kernFlatIntoLedger = isFlat && chordElement->isLedgerLine() && chordElement->y() > acc->note()->y();
    if (kernFlatIntoLedger) {
        return 0.0;
    }

    bool kernSharpIntoLedger = accType == AccidentalType::SHARP && chordElement->isLedgerLine()
                               && chordElement->y() > acc->note()->y() + 1.1 * m_spatium;
    if (kernSharpIntoLedger) {
        return 0.0;
    }

    bool kernFlatIntoNote = isFlat && chordElement->isNote() && chordElement->y() > acc->note()->y();
    if (kernFlatIntoNote) {
        return 0.15 * m_spatium;
    }

    const PaddingTable& paddingTable = acc->score()->paddingTable();

    return paddingTable.at(ElementType::ACCIDENTAL).at(chordElement->type()) * 0.5 * (acc->mag() + chordElement->mag());
}

double AccidentalsLayout::AccidentalPlacementEngine::minAccidentalToAccidentalGroupDistance(Accidental* acc, Shape accShape,
                                                                                            const Shape& accidentalsShape)
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
                double distFourth = kerningOfFourth(accShape, groupElement);
                if (!muse::RealIsNull(distFourth) && distFourth > curDist) {
                    collisionFound = true;
                    curDist = distFourth;
                }
                column = std::max(column, acc2->ldata()->column + 1);
                continue;
            }

            double vertPadding = verticalPadding(acc, acc2);
            double horPadding = horizontalPadding(acc, acc2);
            if (accShape.intersects(groupElement, horPadding, vertPadding)) {
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
                    minDist += additionalPaddingForVerticals(acc, acc2);
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

bool AccidentalsLayout::AccidentalPlacementEngine::isExceptionOfFourth(const Accidental* acc1, const Accidental* acc2)
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

bool AccidentalsLayout::AccidentalPlacementEngine::isExceptionOfNaturalsSixth(const Accidental* acc1, const Accidental* acc2)
{
    return acc1->accidentalType() == AccidentalType::NATURAL
           && acc2->accidentalType() == AccidentalType::NATURAL
           && abs(acc2->line() - acc1->line()) == 5;
}

bool AccidentalsLayout::AccidentalPlacementEngine::canFitInSameColumn(const Accidental* acc1, const Accidental* acc2)
{
    if (isExceptionOfNaturalsSixth(acc1, acc2)) {
        return true;
    }

    RectF acc1Box = acc1->ldata()->bbox().translated(PointF(0.0, acc1->note()->y()));
    RectF acc2Box = acc2->ldata()->bbox().translated(PointF(0.0, acc2->note()->y()));

    return !mu::engraving::intersects(acc1Box.top(), acc1Box.bottom(), acc2Box.top(), acc2Box.bottom(), verticalPadding(acc1, acc2));
}

bool AccidentalsLayout::AccidentalPlacementEngine::isOctave(const Accidental* acc1, const Accidental* acc2)
{
    return acc1->accidentalType() == acc2->accidentalType() && std::abs(acc2->line() - acc1->line()) % 7 == 0;
}

double AccidentalsLayout::AccidentalPlacementEngine::kerningOfFourth(const Shape& accShape, const ShapeElement& accGroupShapeElement)
{
    AccidentalType accType2 = toAccidental(accGroupShapeElement.item())->accidentalType();

    const double flatKerning = 0.15 * m_spatium;
    const double naturalKerning = 0.05 * m_spatium;

    return accShape.right() - accGroupShapeElement.left() - (accType2 == AccidentalType::NATURAL ? naturalKerning : flatKerning);
}

double AccidentalsLayout::AccidentalPlacementEngine::additionalPaddingForVerticals(const Accidental* acc, const EngravingItem* item)
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

    return m_additionalPaddingForVerticals;
}

void AccidentalsLayout::AccidentalPlacementEngine::verticallyAlignAccidentals()
{
    std::set<Accidental*> accidentalsAlreadyGrouped;
    std::map<Accidental*, std::vector<Accidental*> > verticalSets;

    if (m_alignOffsetOctaves) {
        collectVerticalSetsOfOffsetOctaves(accidentalsAlreadyGrouped, verticalSets);
    }

    collectVerticalSets(accidentalsAlreadyGrouped, verticalSets);

    if (verticalSets.empty()) {
        return;
    }

    // Turn map into vector
    std::vector<std::vector<Accidental*> > vertSets;
    vertSets.reserve(verticalSets.size());
    for (auto& pair : verticalSets) {
        vertSets.push_back(pair.second);
    }

    // Sort vertical sets by column
    std::sort(vertSets.begin(), vertSets.end(), [&](auto& set1, auto& set2) {
        return set1.front()->ldata()->column < set2.front()->ldata()->column;
    });

    alignVerticalSets(vertSets);
}

void AccidentalsLayout::AccidentalPlacementEngine::collectVerticalSetsOfOffsetOctaves(
    std::set<Accidental*>& accidentalsAlreadyGrouped, std::map<Accidental*, std::vector<Accidental*> >& verticalSets)
{
    for (std::vector<Accidental*>& accidentalGroup : m_accidentalSubChords) {
        for (int i = 0; i < accidentalGroup.size(); ++i) {
            Accidental* acc1 = accidentalGroup[i];
            acc1->ldata()->column.value();
            if (muse::contains(accidentalsAlreadyGrouped, acc1)) {
                continue;
            }
            double x1 = xPosRelativeToSegment(acc1);
            for (int j = i + 1; j < accidentalGroup.size(); ++j) {
                Accidental* acc2 = accidentalGroup[j];
                if (!isOctave(acc1, acc2) || muse::contains(accidentalsAlreadyGrouped, acc2)) {
                    continue;
                }
                double x2 = xPosRelativeToSegment(acc2);
                bool alignOctave = abs(x1 - x2) < 2.0 * m_spatium;
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

void AccidentalsLayout::AccidentalPlacementEngine::collectVerticalSets(
    std::set<Accidental*>& accidentalsAlreadyGrouped, std::map<Accidental*, std::vector<Accidental*> >& verticalSets)
{
    for (int i = 0; i < m_allAccidentals.size(); ++i) {
        Accidental* acc1 = m_allAccidentals[i];
        if (muse::contains(accidentalsAlreadyGrouped, acc1)) {
            continue;
        }
        int column1 = acc1->ldata()->column;
        int verticalSub1 = acc1->ldata()->verticalSubgroup;
        double x1 = xPosRelativeToSegment(acc1);
        for (int j = i + 1; j < m_allAccidentals.size(); ++j) {
            Accidental* acc2 = m_allAccidentals[j];
            if (acc2->accidentalType() != acc1->accidentalType() || muse::contains(accidentalsAlreadyGrouped, acc2)) {
                continue;
            }
            int column2 = acc2->ldata()->column;
            int verticalSub2 = acc2->ldata()->verticalSubgroup;
            double x2 = xPosRelativeToSegment(acc2);
            bool alignAccidentals = column1 == column2 && verticalSub1 == verticalSub2 && abs(x1 - x2) < 0.75 * m_spatium;
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

void AccidentalsLayout::AccidentalPlacementEngine::alignVerticalSets(std::vector<std::vector<Accidental*> >& vertSets)
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
        for (std::vector<Accidental*>& group : m_accidentalSubChords) {
            for (Accidental* acc : group) {
                double curXPos = xPosRelativeToSegment(acc);
                Shape accShape = acc->shape().translate(PointF(curXPos, acc->note()->y()));
                int accColumn = acc->ldata()->column;
                if (accColumn >= curColumn && !muse::contains(vertSet, acc)) {
                    double minDistToAccidGroup = minAccidentalToAccidentalGroupDistance(acc, accShape, accidentalGroupShape);
                    accShape.translateX(-minDistToAccidGroup);
                    setXposRelativeToSegment(acc, curXPos - minDistToAccidGroup);
                }
                accidentalGroupShape.add(accShape);
            }
        }
    }
}

void AccidentalsLayout::AccidentalPlacementEngine::setXposRelativeToSegment(Accidental* accidental, double x)
{
    Note* note = accidental->note();
    Chord* chord = note ? note->chord() : nullptr;
    if (note) {
        x -= note->pos().x();
    }
    if (chord) {
        x -= chord->pos().x();
    }
    accidental->mutldata()->setPosX(x);
}

double AccidentalsLayout::AccidentalPlacementEngine::xPosRelativeToSegment(const Accidental* accidental)
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

void AccidentalsLayout::AccidentalPlacementEngine::sortTopDown(std::vector<Accidental*>& accidentals)
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

double AccidentalsLayout::AccidentalPlacementEngine::verticalPadding(const Accidental* acc1, const Accidental* acc2)
{
    double verticalClearance = acc1->accidentalType() == AccidentalType::SHARP && acc2->accidentalType() == AccidentalType::SHARP
                               ? m_verticalSharpToSharpClearance : m_verticalAccidentalToAccidentalClearance;
    return verticalClearance *= 0.5 * (acc1->mag() + acc2->mag());
}

double AccidentalsLayout::AccidentalPlacementEngine::horizontalPadding(const Accidental* acc1, const Accidental* acc2)
{
    return m_accidentalAccidentalDistance * (0.5 * (acc1->mag() + acc2->mag()));
}
