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
    std::vector<Accidental*> allAccidentals;
    collectAccidentals(allAccidentals, chords);

    if (allAccidentals.empty()) {
        return;
    }

    std::vector<std::vector<Accidental*> > accidentalGroups;
    createGroups(accidentalGroups, allAccidentals);

    Shape chordsShape = createChordsShape(chords);

    for (std::vector<Accidental*>& accidentalGroup : accidentalGroups) {
        if (!accidentalGroup.empty()) {
            layoutAccidentalGroup(accidentalGroup, chordsShape, ctx);
        }
    }

    verticallyAlignAccidentalsWhereNeeded(allAccidentals, accidentalGroups);
}

void AccidentalsLayout::collectAccidentals(std::vector<Accidental*>& allAccidentals, const std::vector<Chord*> chords)
{
    for (auto chordIt = chords.rbegin(); chordIt != chords.rend(); ++chordIt) {
        std::vector<Note*> notes = (*chordIt)->notes();
        for (auto noteIt = notes.rbegin(); noteIt != notes.rend(); ++noteIt) {
            Accidental* acc = (*noteIt)->accidental();
            if (!acc) {
                continue;
            }
            acc->setPos(0.0, 0.0);
            bool isRedundant = false;
            for (Accidental* otherAcc : allAccidentals) {
                if (otherAcc->accidentalType() == acc->accidentalType() && otherAcc->note()->line() == acc->note()->line()) {
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
            return acc1->note()->line() < acc2->note()->line();
        });
    }
}

void AccidentalsLayout::createGroups(std::vector<std::vector<Accidental*> >& accidentalGroups, std::vector<Accidental*> allAccidentals)
{
    accidentalGroups.reserve(2);

    std::vector<Accidental*> firstGroup;
    firstGroup.reserve(allAccidentals.size());
    firstGroup.push_back(allAccidentals.front());
    accidentalGroups.push_back(firstGroup);

    for (int i = 1; i < allAccidentals.size(); ++i) {
        Accidental* prevAcc = allAccidentals[i - 1];
        Accidental* curAcc = allAccidentals[i];
        bool startNewGroup = curAcc->note()->line() - prevAcc->note()->line() >= 6;
        if (startNewGroup) {
            accidentalGroups.push_back(std::vector<Accidental*> { curAcc });
        } else {
            accidentalGroups.back().push_back(curAcc);
        }
    }
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
            chordsShape.add(note->symBbox(noteSym).translated(note->pos() + chord->pos()), note);
        }
        Stem* stem = chord->stem();
        if (stem) {
            chordsShape.add(stem->shape().translate(stem->pos() + chord->pos()));
        }
    }

    return chordsShape;
}

void AccidentalsLayout::layoutAccidentalGroup(std::vector<Accidental*>& accidentals, const Shape& chordsShape, LayoutContext& ctx)
{
    determineAccidentalsOrder(accidentals);

    Shape accidGroupShape;
    accidGroupShape.elements().reserve(accidentals.size());

    for (int i = 0; i < accidentals.size(); ++i) {
        Accidental* acc = accidentals[i];
        acc->computeMag();
        TLayout::layoutAccidental(acc, acc->mutldata(), ctx.conf());
        Shape accShape = acc->shape();
        accShape.translateY(acc->note()->y());
        double x = minAccidentalToChordDistance(acc, accShape, chordsShape);
        accShape.translateX(-x);
        if (collidesWithPreviousAccidentals(acc, accShape, accidGroupShape)) {
            double minDistToAccidGroup = minAccidentalToAccidentalGroupDistance(acc, accShape, accidGroupShape);
            accShape.translateX(-minDistToAccidGroup);
            x += minDistToAccidGroup;
        }
        setXposRelativeToSegment(acc, -x);
        accidGroupShape.add(accShape);
    }
}

void AccidentalsLayout::determineAccidentalsOrder(std::vector<Accidental*>& accidentals)
{
    std::map<int, std::vector<Accidental*> > groupsOfEqualNoteXPos;
    for (Accidental* acc : accidentals) {
        Note* note = acc->note();
        int roundedNoteXpos = round(note->x() + note->chord()->x());
        groupsOfEqualNoteXPos[roundedNoteXpos].push_back(acc);
    }

    std::vector<std::vector<Accidental*> > groups;
    groups.reserve(groupsOfEqualNoteXPos.size());
    for (auto it = groupsOfEqualNoteXPos.rbegin(); it != groupsOfEqualNoteXPos.rend(); ++it) {
        groups.push_back((*it).second);
    }

    accidentals.clear();

    for (int idx = 0; idx < groups.size(); ++idx) {
        std::vector<Accidental*> group = groups[idx];
        bool startFromTopNote = true;
        if (group.size() > 1 && idx > 0) {
            std::vector<Accidental*> prevGroup = groups[idx - 1];
            if (prevGroup.size() == 1) {
                int prevGroupNoteLine = prevGroup.front()->note()->line();
                int thisGroupTopNoteLine = group.front()->note()->line();
                int thisGroupBottomNoteLine = group.back()->note()->line();
                if (abs(thisGroupTopNoteLine - prevGroupNoteLine) < abs(thisGroupBottomNoteLine - prevGroupNoteLine)) {
                    startFromTopNote = false;
                }
            }
        }

        static constexpr int largeGroupLimit = 4;
        if (group.size() <= largeGroupLimit) {
            bool pickFromTop = startFromTopNote;
            int i = 0;
            int j = group.size() - 1;
            while (i < group.size() && i <= j) {
                accidentals.push_back(pickFromTop ? group[i] : group[j]);
                pickFromTop ? ++i : --j;
                pickFromTop = !pickFromTop;
            }
        } else {
            static constexpr int subgroupSize = 3;
            int i = 0;
            while (i < subgroupSize) {
                int j = i + subgroupSize;
                accidentals.push_back(group[i]);
                while (j < group.size()) {
                    accidentals.push_back(group[j]);
                    j += subgroupSize;
                }
                ++i;
            }
        }
    }
}

double AccidentalsLayout::minAccidentalToChordDistance(Accidental* acc, const Shape& accShape, const Shape& chordsShape)
{
    const double verticalClearance = 0.1 * acc->spatium();

    double dist = -DBL_MAX;
    for (const ShapeElement& accidentalElement : accShape.elements()) {
        double accTop = accidentalElement.top();
        double accBottom = accidentalElement.bottom();
        for (const ShapeElement& chordElement : chordsShape.elements()) {
            double chordElementTop = chordElement.top();
            double chordElementBottom = chordElement.bottom();
            if (mu::engraving::intersects(accTop, accBottom, chordElementTop, chordElementBottom, verticalClearance)) {
                double padding = computePadding(acc, chordElement.item());
                dist = std::max(dist, accidentalElement.right() - chordElement.left() + padding);
            }
        }
    }

    return dist;
}

double AccidentalsLayout::computePadding(Accidental* acc, const EngravingItem* chordElement)
{
    AccidentalType accType = acc->accidentalType();
    bool isFlat = accType == AccidentalType::FLAT || accType == AccidentalType::FLAT2;

    bool kernFlatIntoLedger = isFlat && chordElement->isLedgerLine() && chordElement->y() > acc->note()->y();
    if (kernFlatIntoLedger) {
        return 0.0;
    }

    bool kernFlatIntoNote = isFlat && chordElement->isNote() && chordElement->y() > acc->note()->y();
    if (kernFlatIntoNote) {
        return 0.15 * acc->spatium();
    }

    const PaddingTable& paddingTable = acc->score()->paddingTable();

    return paddingTable.at(ElementType::ACCIDENTAL).at(chordElement->type()) * 0.5 * (acc->mag() + chordElement->mag());
}

bool AccidentalsLayout::collidesWithPreviousAccidentals(const Accidental* acc, const Shape& accShape, const Shape& accidentalsShape)
{
    const double accidentalToAccidentalPadding = acc->style().styleMM(Sid::accidentalDistance);
    const double verticalClearance = 0.15 * acc->spatium();
    return accShape.intersects(accidentalsShape, accidentalToAccidentalPadding, verticalClearance);
}

double AccidentalsLayout::minAccidentalToAccidentalGroupDistance(const Accidental* acc, const Shape& accShape,
                                                                 const Shape& accidentalsShape)
{
    const double accidentalToAccidentalPadding = acc->style().styleMM(Sid::accidentalDistance);
    const double verticalClearance = 0.15 * acc->spatium();

    double dist = -DBL_MAX;
    for (const ShapeElement& accidentalElement : accShape.elements()) {
        double accTop = accidentalElement.top();
        double accBottom = accidentalElement.bottom();
        for (const ShapeElement& groupElement : accidentalsShape.elements()) {
            double groupElementTop = groupElement.top();
            double groupElementBottom = groupElement.bottom();
            if (mu::engraving::intersects(accTop, accBottom, groupElementTop, groupElementBottom, verticalClearance)) {
                dist = std::max(dist, accidentalElement.right() - groupElement.left() + accidentalToAccidentalPadding);
            }
        }
    }

    return dist;
}

void AccidentalsLayout::verticallyAlignAccidentalsWhereNeeded(std::vector<Accidental*> allAccidentals,
                                                              std::vector<std::vector<Accidental*> > accidentalGroups)
{
    // Detect sets of accidentals that should be vertically aligned
    std::set<Accidental*> alreadyInAVerticalSet;
    std::map<Accidental*, std::vector<Accidental*> > verticalSets;
    for (int i = 0; i < allAccidentals.size(); ++i) {
        Accidental* acc1 = allAccidentals[i];
        if (mu::contains(alreadyInAVerticalSet, acc1)) {
            continue;
        }
        for (int j = i + 1; j < allAccidentals.size(); ++j) {
            Accidental* acc2 = allAccidentals[j];
            if (mu::contains(alreadyInAVerticalSet, acc2) || acc2->accidentalType() != acc1->accidentalType()) {
                continue;
            }
            double x1 = xPosRelativeToSegment(acc1);
            double x2 = xPosRelativeToSegment(acc2);
            if (abs(x1 - x2) < 0.35 * acc1->spatium()) {
                std::vector<Accidental*>& verticalSet = verticalSets[acc1];
                if (verticalSet.empty()) {
                    verticalSet.push_back(acc1);
                }
                verticalSet.push_back(acc2);
            }
        }
    }

    if (verticalSets.empty()) {
        return;
    }

    // Align the set
    for (auto pair : verticalSets) {
        std::vector<Accidental*>& verticalSet = pair.second;
        double x = DBL_MAX;
        for (Accidental* acc : verticalSet) {
            x = std::min(x, xPosRelativeToSegment(acc));
        }
        for (Accidental* acc : verticalSet) {
            setXposRelativeToSegment(acc, x);
        }
    }

    // Re-check for accidental collisions
    for (std::vector<Accidental*> accGroup : accidentalGroups) {
        Shape accGroupShape;
        for (Accidental* acc : accGroup) {
            double currentXpos = xPosRelativeToSegment(acc);
            Shape accShape = acc->shape();
            accShape.translateY(acc->note()->y());
            accShape.translateX(currentXpos);
            if (collidesWithPreviousAccidentals(acc, accShape, accGroupShape)) {
                double minDistToAccidGroup = minAccidentalToAccidentalGroupDistance(acc, accShape, accGroupShape);
                accShape.translateX(-minDistToAccidGroup);
                setXposRelativeToSegment(acc, currentXpos - minDistToAccidGroup);
            }
            accGroupShape.add(accShape);
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
