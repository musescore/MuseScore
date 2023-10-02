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
#include "horizontalspacing.h"

#include "dom/chord.h"
#include "dom/engravingitem.h"
#include "dom/glissando.h"
#include "dom/note.h"
#include "dom/rest.h"
#include "dom/score.h"
#include "dom/stemslash.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::dev;

double HorizontalSpacing::computePadding(const EngravingItem* item1, const EngravingItem* item2)
{
    const PaddingTable& paddingTable = item1->score()->paddingTable();
    ElementType type1 = item1->type();
    ElementType type2 = item2->type();

    double padding = paddingTable.at(type1).at(type2);
    double scaling = (item1->mag() + item2->mag()) / 2;

    if (type1 == ElementType::NOTE && isSpecialNotePaddingType(type2)) {
        computeNotePadding(toNote(item1), item2, padding, scaling);
    } else {
        padding *= scaling;
    }

    if (!item1->isLedgerLine() && item2->isRest()) {
        computeLedgerRestPadding(toRest(item2), padding);
    }

    return padding;
}

bool HorizontalSpacing::isSpecialNotePaddingType(ElementType type)
{
    switch (type) {
    case ElementType::NOTE:
    case ElementType::REST:
    case ElementType::STEM:
        return true;
    default:
        return false;
    }
}

void HorizontalSpacing::computeNotePadding(const Note* note, const EngravingItem* item2, double& padding, double scaling)
{
    const MStyle& style = note->style();

    bool sameVoiceNoteOrStem = (item2->isNote() || item2->isStem()) && note->track() == item2->track();
    if (sameVoiceNoteOrStem) {
        bool intersection = note->shape().translate(note->pos()).intersects(item2->shape().translate(item2->pos()));
        if (intersection) {
            padding = std::max(padding, static_cast<double>(style.styleMM(Sid::minNoteDistance)));
        }
    }

    padding *= scaling;

    if (!(item2->isNote() || item2->isRest())) {
        return;
    }

    if (note->isGrace() && item2->isNote() && toNote(item2)->isGrace()) {
        // Grace-to-grace
        padding = std::max(padding, static_cast<double>(style.styleMM(Sid::graceToGraceNoteDist)));
    } else if (note->isGrace() && (item2->isRest() || (item2->isNote() && !toNote(item2)->isGrace()))) {
        // Grace-to-main
        padding = std::max(padding, static_cast<double>(style.styleMM(Sid::graceToMainNoteDist)));
    } else if (!note->isGrace() && item2->isNote() && toNote(item2)->isGrace()) {
        // Main-to-grace
        padding = std::max(padding, static_cast<double>(style.styleMM(Sid::graceToMainNoteDist)));
    }

    if (!item2->isNote()) {
        return;
    }

    const Note* note2 = toNote(item2);
    if (note->lineAttachPoints().empty() || note2->lineAttachPoints().empty()) {
        return;
    }

    // Allocate space for minTieLenght and minGlissandoLength
    for (LineAttachPoint laPoint1 : note->lineAttachPoints()) {
        for (LineAttachPoint laPoint2 : note2->lineAttachPoints()) {
            if (laPoint1.line() != laPoint2.line()) {
                continue;
            }

            double minEndPointsDistance = 0.0;
            if (laPoint1.line()->isTie()) {
                minEndPointsDistance = style.styleMM(Sid::MinTieLength);
            } else if (laPoint1.line()->isGlissando()) {
                bool straight = toGlissando(laPoint1.line())->glissandoType() == GlissandoType::STRAIGHT;
                double minGlissandoLength = straight
                                            ? style.styleMM(Sid::MinStraightGlissandoLength)
                                            : style.styleMM(Sid::MinWigglyGlissandoLength);
                minEndPointsDistance = minGlissandoLength;
            }

            double lapPadding = (laPoint1.pos().x() - note->headWidth()) + minEndPointsDistance - laPoint2.pos().x();
            lapPadding *= scaling;

            padding = std::max(padding, lapPadding);
        }
    }
}

void HorizontalSpacing::computeLedgerRestPadding(const Rest* rest2, double& padding)
{
    SymId restSym = rest2->ldata()->sym();
    switch (restSym) {
    case SymId::restWholeLegerLine:
    case SymId::restDoubleWholeLegerLine:
    case SymId::restHalfLegerLine:
        padding += rest2->ldata()->bbox().left();
        return;
    default:
        return;
    }
}

KerningType HorizontalSpacing::computeKerning(const EngravingItem* item1, const EngravingItem* item2)
{
    if (isSameVoiceKerningLimited(item1) && isSameVoiceKerningLimited(item2) && item1->track() == item2->track()) {
        return KerningType::NON_KERNING;
    }

    if ((isNeverKernable(item1) || isNeverKernable(item2))
        && !(isAlwaysKernable(item1) || isAlwaysKernable(item2))) {
        return KerningType::NON_KERNING;
    }

    return doComputeKerningType(item1, item2);
}

bool HorizontalSpacing::isSameVoiceKerningLimited(const EngravingItem* item)
{
    ElementType type = item->type();

    switch (type) {
    case ElementType::NOTE:
    case ElementType::REST:
    case ElementType::STEM:
    case ElementType::CHORDLINE:
    case ElementType::BREATH:
        return true;
    default:
        return false;
    }
}

bool HorizontalSpacing::isNeverKernable(const EngravingItem* item)
{
    ElementType type = item->type();

    switch (type) {
    case ElementType::CLEF:
    case ElementType::TIMESIG:
    case ElementType::KEYSIG:
    case ElementType::BAR_LINE:
        return true;
    default:
        return false;
    }
}

bool HorizontalSpacing::isAlwaysKernable(const EngravingItem* item)
{
    return item->isTextBase() || item->isChordLine();
}

KerningType HorizontalSpacing::doComputeKerningType(const EngravingItem* item1, const EngravingItem* item2)
{
    ElementType type1 = item1->type();
    switch (type1) {
    case ElementType::BAR_LINE:
        return KerningType::NON_KERNING;
    case ElementType::CHORDLINE:
        return item2->isBarLine() ? KerningType::ALLOW_COLLISION : KerningType::KERNING;
    case ElementType::HARMONY:
        return item2->isHarmony() ? KerningType::NON_KERNING : KerningType::KERNING;
    case ElementType::LYRICS:
        return (item2->isLyrics() || item2->isBarLine()) ? KerningType::NON_KERNING : KerningType::KERNING;
    case ElementType::NOTE:
        return computeNoteKerningType(toNote(item1), item2);
    case ElementType::STEM_SLASH:
        return computeStemSlashKerningType(toStemSlash(item1), item2);
    default:
        return KerningType::KERNING;
    }
}

KerningType HorizontalSpacing::computeNoteKerningType(const Note* note, const EngravingItem* item2)
{
    EngravingItem* nextParent = item2->parentItem(true);
    if (nextParent && nextParent->isNote() && toNote(nextParent)->isTrillCueNote()) {
        return KerningType::NON_KERNING;
    }

    Chord* c = note->chord();
    if (!c || (c->allowKerningAbove() && c->allowKerningBelow())) {
        return KerningType::KERNING;
    }
    bool kerningAbove = item2->canvasPos().y() < note->canvasPos().y();
    if (kerningAbove && !c->allowKerningAbove()) {
        return KerningType::NON_KERNING;
    }
    if (!kerningAbove && !c->allowKerningBelow()) {
        return KerningType::NON_KERNING;
    }

    return KerningType::KERNING;
}

KerningType HorizontalSpacing::computeStemSlashKerningType(const StemSlash* stemSlash, const EngravingItem* item2)
{
    if (!stemSlash->chord() || !stemSlash->chord()->beam() || !item2->parentItem()) {
        return KerningType::KERNING;
    }

    EngravingItem* nextParent = item2->parentItem();
    Chord* nextChord = nullptr;
    if (nextParent->isChord()) {
        nextChord = toChord(nextParent);
    } else if (nextParent->isNote()) {
        nextChord = toChord(nextParent->parentItem());
    }
    if (!nextChord) {
        return KerningType::KERNING;
    }

    if (nextChord->beam() && nextChord->beam() == stemSlash->chord()->beam()) {
        // Stem slash is allowed to collide with items from the same grace notes group
        return KerningType::ALLOW_COLLISION;
    }

    return KerningType::KERNING;
}
