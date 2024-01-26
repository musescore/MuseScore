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

    computeCommonConstants(allAccidentals.front());

    std::vector<std::vector<Accidental*> > accidentalGroups = splitVertically(allAccidentals);

    Shape chordsShape = createChordsShape(chords);

    for (std::vector<Accidental*>& group : accidentalGroups) {
        layoutAccidentalGroup(group, chordsShape, ctx);
    }

    verticallyAlignAccidentalsWhereNeeded(allAccidentals, accidentalGroups);
}

std::vector<Accidental*> AccidentalsLayout::collectAccidentals(const std::vector<Chord*> chords)
{
    std::vector<Accidental*> allAccidentals;

    for (auto chordIt = chords.rbegin(); chordIt != chords.rend(); ++chordIt) {
        std::vector<Note*>& notes = (*chordIt)->notes();
        for (auto noteIt = notes.rbegin(); noteIt != notes.rend(); ++noteIt) {
            Accidental* acc = (*noteIt)->accidental();
            if (!acc) {
                continue;
            }
            acc->setPos(0.0, 0.0);
            bool isRedundant = false;
            for (Accidental* otherAcc : allAccidentals) {
                if (otherAcc->accidentalType() == acc->accidentalType() && otherAcc->line() == acc->line()) {
                    isRedundant = true;
                    break;
                }
            }
            if (isRedundant) {
                acc->setbbox(RectF());
            } else {
                allAccidentals.push_back(acc);
            }
        }
    }

    if (!allAccidentals.empty()) {
        std::sort(allAccidentals.begin(), allAccidentals.end(), [](const Accidental* acc1, const Accidental* acc2){
            return acc1->line() < acc2->line();
        });
    }

    return allAccidentals;
}

std::vector<std::vector<Accidental*> > AccidentalsLayout::splitVertically(std::vector<Accidental*> allAccidentals)
{
    std::vector<std::vector<Accidental*> > accidentalGroups;
    accidentalGroups.reserve(2);

    std::vector<Accidental*> firstGroup;
    firstGroup.reserve(allAccidentals.size());
    firstGroup.push_back(allAccidentals.front());
    accidentalGroups.push_back(firstGroup);

    for (int i = 1; i < allAccidentals.size(); ++i) {
        Accidental* prevAcc = allAccidentals[i - 1];
        Accidental* curAcc = allAccidentals[i];
        bool startNewGroup = curAcc->line() - prevAcc->line() >= 6;
        if (startNewGroup) {
            accidentalGroups.push_back(std::vector<Accidental*> { curAcc });
        } else {
            accidentalGroups.back().push_back(curAcc);
        }
    }

    return accidentalGroups;
}

Shape AccidentalsLayout::createChordsShape(const std::vector<Chord*> chords)
{
    Shape chordsShape;
    for (Chord* chord : chords) {
        LedgerLine* ledger = chord->ledgerLines();
        while (ledger) {
            chordsShape.add(ledger->shape().translate(ledger->pos() + chord->pos()));
            ledger = ledger->next();
        }
        for (Note* note : chord->notes()) {
            SymId noteSym = note->ldata()->cachedNoteheadSym;
            if (noteSym == SymId::noSym) {
                noteSym = note->noteHead();
            }
            chordsShape.add(note->symShapeWithCutouts(noteSym).translate(note->pos() + chord->pos()));
        }
        Stem* stem = chord->stem();
        if (stem) {
            chordsShape.add(stem->shape().translate(stem->pos() + chord->pos()));
        }
    }

    return chordsShape;
}

void AccidentalsLayout::layoutAccidentalGroup(std::vector<Accidental*>& group, const Shape& chordsShape, LayoutContext& ctx)
{
    for (Accidental* acc : group) {
        acc->computeMag();
        TLayout::layoutAccidental(acc, acc->mutldata(), ctx.conf());
    }

    Shape accidGroupShape;
    accidGroupShape.elements().reserve(4 * group.size());

    std::vector<std::vector<Accidental*> > subGroups = splitHorizontally(group, chordsShape);

    group.clear();
    for (std::vector<Accidental*>& subGroup : subGroups) {
        determineStackingOrder(subGroup);
        group.insert(group.end(), subGroup.begin(), subGroup.end());
        for (Accidental* acc : subGroup) {
            doLayoutAccidental(acc, chordsShape, accidGroupShape);
        }
    }
}

void AccidentalsLayout::doLayoutAccidental(Accidental* acc, const Shape& chordsShape, Shape& accidGroupShape)
{
    Shape accShape = acc->shape();
    accShape.translateY(acc->note()->y());

    double x = minAccidentalToChordDistance(acc, accShape, chordsShape);
    accShape.translateX(-x);

    double minDistToAccidGroup = minAccidentalToAccidentalGroupDistance(acc, accShape, accidGroupShape);
    accShape.translateX(-minDistToAccidGroup);
    x += minDistToAccidGroup;

    setXposRelativeToSegment(acc, -x);

    accidGroupShape.add(accShape);
}

std::vector<std::vector<Accidental*> > AccidentalsLayout::splitHorizontally(std::vector<Accidental*>& accidentals, const Shape& chordsShape)
{
    std::vector<std::vector<Accidental*> > subGroups(2);

    std::map<double, std::vector<Accidental*> > groupsOfEqualX;
    for (Accidental* acc : accidentals) {
        Shape accRoughShape = Shape(acc->symBbox(acc->symId()), acc);
        accRoughShape.translateY(acc->note()->y());
        double x = -minAccidentalToChordDistance(acc, accRoughShape, chordsShape) + acc->symWidth(acc->symId());
        groupsOfEqualX[round(x)].push_back(acc);
    }

    double splitX = DBL_MAX;
    double prevX = (*groupsOfEqualX.begin()).first;
    for (auto& pair : groupsOfEqualX) {
        double curX = pair.first;
        if (curX - prevX > 0.5 * m_spatium) {
            splitX = curX;
            break;
        }
        prevX = curX;
    }

    for (auto& pair : groupsOfEqualX) {
        if (pair.first >= splitX) {
            for (Accidental* acc : pair.second) {
                subGroups[0].push_back(acc);
                m_verticalSubgroup[acc] = 0;
            }
        } else {
            for (Accidental* acc : pair.second) {
                subGroups[1].push_back(acc);
                m_verticalSubgroup[acc] = 1;
            }
        }
    }

    mu::remove_if(subGroups, [](auto& subGroup) {
        return subGroup.size() == 0;
    });

    for (std::vector<Accidental*>& subGroup : subGroups) {
        std::sort(subGroup.begin(), subGroup.end(), [](const Accidental* acc1, const Accidental* acc2){
            return acc1->line() < acc2->line();
        });
    }

    return subGroups;
}

void AccidentalsLayout::determineStackingOrder(std::vector<Accidental*>& accidentals)
{
    static constexpr int largeGroupLimit = 4;
    if (accidentals.size() > largeGroupLimit) {
        computeCompactOrdering(accidentals);
    } else {
        computeStandardOrdering(accidentals);
    }
}

void AccidentalsLayout::computeStandardOrdering(std::vector<Accidental*>& accidentals)
{
    std::list<Accidental*> accidentalList { accidentals.begin(), accidentals.end() };
    accidentals.clear();

    bool pickFromTop = true;
    while (accidentalList.size() > 0) {
        Accidental* acc = pickFromTop ? accidentalList.front() : accidentalList.back();
        pickFromTop ? accidentalList.pop_front() : accidentalList.pop_back();

        accidentals.push_back(acc);

        bool hitOctave = false;
        auto iter = accidentalList.begin();
        while (iter != accidentalList.end()) {
            Accidental* acc2 = *iter;
            if (acc2->accidentalType() == acc->accidentalType()
                && (acc2->line() - acc->line()) % 7 == 0) {
                hitOctave = true;
                accidentals.push_back(acc2);
                iter = accidentalList.erase(iter);
                continue;
            }
            ++iter;
        }

        if (hitOctave) {
            pickFromTop = true;
        } else {
            pickFromTop = !pickFromTop;
        }
    }
}

void AccidentalsLayout::computeCompactOrdering(std::vector<Accidental*>& accidentals)
{
    std::list<Accidental*> accidentalList { accidentals.begin(), accidentals.end() };
    accidentals.clear();

    Accidental* bottomAcc = accidentalList.back();

    bool pickFromTop = true;
    while (accidentalList.size() > 0) {
        Accidental* acc = pickFromTop ? accidentalList.front() : accidentalList.back();
        pickFromTop ? accidentalList.pop_front() : accidentalList.pop_back();

        accidentals.push_back(acc);

        // TODO: find a more elegant solution with reverse iterators
        auto startIter = pickFromTop ? accidentalList.begin() : --accidentalList.end();
        auto endIter = pickFromTop ? accidentalList.end() : --accidentalList.begin();

        bool hitBottom = false;
        auto iter = startIter;
        while (iter != endIter) {
            RectF accBox = acc->ldata()->bbox().translated(PointF(0.0, acc->note()->y()));
            Accidental* acc2 = *iter;
            RectF acc2Box = acc2->ldata()->bbox().translated(PointF(0.0, acc2->note()->y()));
            bool fits = isExceptionOfNaturalsSixth(acc, acc2) || !mu::engraving::intersects(
                accBox.top(), accBox.bottom(), acc2Box.top(), acc2Box.bottom(), verticalClearance(acc, acc2));
            if (fits) {
                accidentals.push_back(acc2);
                acc = acc2;
                if (acc2 == bottomAcc) {
                    hitBottom = true;
                }
                pickFromTop ? ++iter : --iter;
                accidentalList.remove(acc2);
                continue;
            }
            pickFromTop ? ++iter : --iter;
        }

        if (hitBottom) {
            pickFromTop = true;
        } else {
            pickFromTop = !pickFromTop;
        }
    }
}

double AccidentalsLayout::minAccidentalToChordDistance(Accidental* acc, const Shape& accShape, const Shape& chordsShape)
{
    double dist = -DBL_MAX;
    for (const ShapeElement& accidentalElement : accShape.elements()) {
        double accTop = accidentalElement.top();
        double accBottom = accidentalElement.bottom();
        for (const ShapeElement& chordElement : chordsShape.elements()) {
            double chordElementTop = chordElement.top();
            double chordElementBottom = chordElement.bottom();
            if (mu::engraving::intersects(accTop, accBottom, chordElementTop, chordElementBottom, m_verticalAccidentalToChordClearance)) {
                double padding = computePadding(acc, chordElement.item());
                dist = std::max(dist, accidentalElement.right() - chordElement.left() + padding);
            }
            dist = std::max(dist, kerningLimitationsIntoChord(acc, accShape, chordElement));
        }
    }

    return dist;
}

double AccidentalsLayout::kerningLimitationsIntoChord(Accidental* acc, const Shape& accShape, const ShapeElement& chordElement)
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

double AccidentalsLayout::computePadding(Accidental* acc, const EngravingItem* chordElement)
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

double AccidentalsLayout::minAccidentalToAccidentalGroupDistance(const Accidental* acc, Shape accShape,
                                                                 const Shape& accidentalsShape)
{
    double dist = 0.0;
    int column = 0;

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
                if (!RealIsNull(distFourth) && distFourth > curDist) {
                    collisionFound = true;
                    curDist = distFourth;
                }
                column = std::max(column, m_column[acc2] + 1);
                continue;
            }

            double vertClearance = verticalClearance(acc, acc2);
            if (accShape.intersects(groupElement, m_accidentalAccidentalDistance, vertClearance)) {
                collisionFound = true;
            } else {
                continue;
            }

            if (!isExceptionOfNaturalsSixth(acc, acc2)) {
                column = std::max(column, m_column[acc2] + 1);
            }

            double groupTop = groupElement.top();
            double groupBottom = groupElement.bottom();
            for (ShapeElement& accElement : accShape.elements()) {
                if (mu::engraving::intersects(accElement.top(), accElement.bottom(), groupTop, groupBottom, vertClearance)) {
                    curDist = std::max(curDist, accElement.right() - groupElement.left() + m_accidentalAccidentalDistance);
                }
            }
        }

        accShape.translateX(-curDist);
        dist += curDist;
    } while (collisionFound && iter < safetyNet);

    IF_ASSERT_FAILED(iter < safetyNet) {
        LOGE() << "Accidental layout error.";
    }

    m_column[acc] = column;

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

double AccidentalsLayout::kerningOfFourth(const Shape& accShape,
                                          const ShapeElement& accGroupShapeElement)
{
    AccidentalType accType2 = toAccidental(accGroupShapeElement.item())->accidentalType();

    const double flatKerning = 0.15 * m_spatium;
    const double naturalKerning = 0.05 * m_spatium;

    return accShape.right() - accGroupShapeElement.left() - (accType2 == AccidentalType::NATURAL ? naturalKerning : flatKerning);
}

void AccidentalsLayout::verticallyAlignAccidentalsWhereNeeded(std::vector<Accidental*> allAccidentals,
                                                              std::vector<std::vector<Accidental*> > accidentalGroups)
{
    // Detect sets of accidentals that should be vertically aligned
    std::set<Accidental*> alreadyGrouped;
    std::map<Accidental*, std::vector<Accidental*> > verticalSets;
    for (int i = 0; i < allAccidentals.size(); ++i) {
        Accidental* acc1 = allAccidentals[i];
        if (mu::contains(alreadyGrouped, acc1)) {
            continue;
        }
        int column1 = m_column[acc1];
        int verticalSub1 = m_verticalSubgroup[acc1];
        double x1 = xPosRelativeToSegment(acc1);
        for (int j = i + 1; j < allAccidentals.size(); ++j) {
            Accidental* acc2 = allAccidentals[j];
            if (acc2->accidentalType() != acc1->accidentalType() || mu::contains(alreadyGrouped, acc2)) {
                continue;
            }
            int column2 = m_column[acc2];
            int verticalSub2 = m_verticalSubgroup[acc2];
            double x2 = xPosRelativeToSegment(acc2);
            bool alignAccidentals = column1 == column2 && verticalSub1 == verticalSub2 && abs(x1 - x2) < 0.75 * m_spatium;
            if (alignAccidentals) {
                std::vector<Accidental*>& verticalSet = verticalSets[acc1];
                if (verticalSet.empty()) {
                    verticalSet.push_back(acc1);
                }
                verticalSet.push_back(acc2);
                alreadyGrouped.insert(acc1);
                alreadyGrouped.insert(acc2);
            }
        }
    }

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
        return m_column[set1.front()] < m_column[set2.front()];
    });

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
        int curColumn = m_column[vertSet.front()];
        Shape accidentalGroupShape;
        for (std::vector<Accidental*>& group : accidentalGroups) {
            for (Accidental* acc : group) {
                double curXPos = xPosRelativeToSegment(acc);
                Shape accShape = acc->shape().translate(PointF(curXPos, acc->note()->y()));
                int accColumn = m_column[acc];
                if (accColumn > curColumn) {
                    double minDistToAccidGroup = minAccidentalToAccidentalGroupDistance(acc, accShape, accidentalGroupShape);
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
    if (chord) {
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

double AccidentalsLayout::verticalClearance(const Accidental* acc1, const Accidental* acc2)
{
    double verticalClearance = acc1->accidentalType() == AccidentalType::SHARP && acc2->accidentalType() == AccidentalType::SHARP
                               ? m_verticalSharpToSharpClearance : m_verticalAccidentalToAccidentalClearance;
    return verticalClearance *= 0.5 * (acc1->mag() + acc2->mag());
}

void AccidentalsLayout::computeCommonConstants(const Accidental* accidental)
{
    m_spatium = accidental->spatium();

    m_verticalAccidentalToAccidentalClearance = 0.15 * m_spatium;
    m_verticalSharpToSharpClearance = 0.05 * m_spatium;
    m_verticalAccidentalToChordClearance = 0.10 * m_spatium;

    m_accidentalAccidentalDistance = accidental->style().styleMM(Sid::accidentalDistance);

    m_verticalSubgroup.clear();
    m_column.clear();
}
