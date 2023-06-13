#include "horizontalspacing.h"

#include "libmscore/chord.h"
#include "libmscore/engravingitem.h"
#include "libmscore/glissando.h"
#include "libmscore/note.h"
#include "libmscore/rest.h"
#include "libmscore/score.h"
#include "libmscore/segment.h"
#include "libmscore/stemslash.h"

using namespace mu::engraving;
using namespace mu::engraving::layout::v0;

void HorizontalSpacing::createPaddingTable(PaddingTable& paddingTable, Score* score)
{
    const double spatium = score->spatium();
    const double minimumPaddingUnit = 0.1 * spatium;
    const double ledgerPad = 0.25 * spatium;
    const double ledgerLength = score->styleMM(Sid::ledgerLineLength);

    for (size_t i=0; i < TOT_ELEMENT_TYPES; ++i) {
        for (size_t j=0; j < TOT_ELEMENT_TYPES; ++j) {
            paddingTable[i][j] = minimumPaddingUnit;
        }
    }

    /* NOTE: the padding value for note->note is NOT minNoteDistance, because minNoteDistance
     * should only apply to notes of the same voice. Notes from different voices should be
     * allowed to get much closer. So we set the general padding at minimumPaddingUnit,
     * but we introduce an appropriate exception for same-voice cases in Shape::minHorizontalDistance().
     */
    paddingTable[ElementType::NOTE][ElementType::NOTE] = minimumPaddingUnit;
    paddingTable[ElementType::NOTE][ElementType::LEDGER_LINE] = 0.35 * spatium;
    paddingTable[ElementType::NOTE][ElementType::ACCIDENTAL]
        = std::max(static_cast<double>(score->styleMM(Sid::accidentalNoteDistance)), 0.35 * spatium);
    paddingTable[ElementType::NOTE][ElementType::REST] = score->styleMM(Sid::minNoteDistance);
    paddingTable[ElementType::NOTE][ElementType::CLEF] = 1.0 * spatium;
    paddingTable[ElementType::NOTE][ElementType::ARPEGGIO] = 0.6 * spatium;
    paddingTable[ElementType::NOTE][ElementType::BAR_LINE] = score->styleMM(Sid::noteBarDistance);
    paddingTable[ElementType::NOTE][ElementType::KEYSIG] = 0.75 * spatium;
    paddingTable[ElementType::NOTE][ElementType::TIMESIG] = 0.75 * spatium;

    paddingTable[ElementType::LEDGER_LINE][ElementType::NOTE] = paddingTable[ElementType::NOTE][ElementType::LEDGER_LINE];
    paddingTable[ElementType::LEDGER_LINE][ElementType::LEDGER_LINE] = ledgerPad;
    paddingTable[ElementType::LEDGER_LINE][ElementType::ACCIDENTAL]
        = std::max(static_cast<double>(score->styleMM(
                                           Sid::accidentalNoteDistance)),
                   paddingTable[ElementType::NOTE][ElementType::ACCIDENTAL] - ledgerLength / 2);
    paddingTable[ElementType::LEDGER_LINE][ElementType::REST] = paddingTable[ElementType::LEDGER_LINE][ElementType::NOTE];
    paddingTable[ElementType::LEDGER_LINE][ElementType::CLEF]
        = std::max(paddingTable[ElementType::NOTE][ElementType::CLEF] - ledgerLength / 2, ledgerPad);
    paddingTable[ElementType::LEDGER_LINE][ElementType::ARPEGGIO] = 0.5 * spatium;
    paddingTable[ElementType::LEDGER_LINE][ElementType::BAR_LINE]
        = std::max(paddingTable[ElementType::NOTE][ElementType::BAR_LINE] - ledgerLength, ledgerPad);
    paddingTable[ElementType::LEDGER_LINE][ElementType::KEYSIG]
        = std::max(paddingTable[ElementType::NOTE][ElementType::KEYSIG] - ledgerLength / 2, ledgerPad);
    paddingTable[ElementType::LEDGER_LINE][ElementType::TIMESIG]
        = std::max(paddingTable[ElementType::NOTE][ElementType::TIMESIG] - ledgerLength / 2, ledgerPad);

    paddingTable[ElementType::HOOK][ElementType::NOTE] = 0.5 * spatium;
    paddingTable[ElementType::HOOK][ElementType::LEDGER_LINE]
        = std::max(paddingTable[ElementType::HOOK][ElementType::NOTE] - ledgerLength, ledgerPad);
    paddingTable[ElementType::HOOK][ElementType::ACCIDENTAL] = 0.35 * spatium;
    paddingTable[ElementType::HOOK][ElementType::REST] = paddingTable[ElementType::HOOK][ElementType::NOTE];
    paddingTable[ElementType::HOOK][ElementType::CLEF] = 0.5 * spatium;
    paddingTable[ElementType::HOOK][ElementType::ARPEGGIO] = 0.35 * spatium;
    paddingTable[ElementType::HOOK][ElementType::BAR_LINE] = 1 * spatium;
    paddingTable[ElementType::HOOK][ElementType::KEYSIG] = 1.15 * spatium;
    paddingTable[ElementType::HOOK][ElementType::TIMESIG] = 1.15 * spatium;

    paddingTable[ElementType::NOTEDOT][ElementType::NOTE]
        = std::max(score->styleMM(Sid::dotNoteDistance), score->styleMM(Sid::dotDotDistance));
    paddingTable[ElementType::NOTEDOT][ElementType::LEDGER_LINE]
        = std::max(paddingTable[ElementType::NOTEDOT][ElementType::NOTE] - ledgerLength, ledgerPad);
    paddingTable[ElementType::NOTEDOT][ElementType::ACCIDENTAL] = paddingTable[ElementType::NOTEDOT][ElementType::NOTE];
    paddingTable[ElementType::NOTEDOT][ElementType::REST] = paddingTable[ElementType::NOTEDOT][ElementType::NOTE];
    paddingTable[ElementType::NOTEDOT][ElementType::CLEF] = 1.0 * spatium;
    paddingTable[ElementType::NOTEDOT][ElementType::ARPEGGIO] = 0.5 * spatium;
    paddingTable[ElementType::NOTEDOT][ElementType::BAR_LINE] = 0.8 * spatium;
    paddingTable[ElementType::NOTEDOT][ElementType::KEYSIG] = 1.35 * spatium;
    paddingTable[ElementType::NOTEDOT][ElementType::TIMESIG] = 1.35 * spatium;

    paddingTable[ElementType::REST][ElementType::NOTE] = paddingTable[ElementType::NOTE][ElementType::REST];
    paddingTable[ElementType::REST][ElementType::LEDGER_LINE]
        = std::max(paddingTable[ElementType::REST][ElementType::NOTE] - ledgerLength / 2, ledgerPad);
    paddingTable[ElementType::REST][ElementType::ACCIDENTAL] = 0.45 * spatium;
    paddingTable[ElementType::REST][ElementType::REST] = paddingTable[ElementType::REST][ElementType::NOTE];
    paddingTable[ElementType::REST][ElementType::CLEF] = paddingTable[ElementType::NOTE][ElementType::CLEF];
    paddingTable[ElementType::REST][ElementType::BAR_LINE] = 1.65 * spatium;
    paddingTable[ElementType::REST][ElementType::KEYSIG] = 1.5 * spatium;
    paddingTable[ElementType::REST][ElementType::TIMESIG] = 1.5 * spatium;

    paddingTable[ElementType::CLEF][ElementType::NOTE] = score->styleMM(Sid::clefKeyRightMargin);
    paddingTable[ElementType::CLEF][ElementType::LEDGER_LINE]
        = std::max(paddingTable[ElementType::CLEF][ElementType::NOTE] - ledgerLength / 2, ledgerPad);
    paddingTable[ElementType::CLEF][ElementType::ACCIDENTAL] = 0.75 * spatium;
    paddingTable[ElementType::CLEF][ElementType::REST] = 1.35 * spatium;
    paddingTable[ElementType::CLEF][ElementType::CLEF] = 0.75 * spatium;
    paddingTable[ElementType::CLEF][ElementType::ARPEGGIO] = 1.15 * spatium;
    paddingTable[ElementType::CLEF][ElementType::BAR_LINE] = score->styleMM(Sid::clefBarlineDistance);
    paddingTable[ElementType::CLEF][ElementType::KEYSIG] = score->styleMM(Sid::clefKeyDistance);
    paddingTable[ElementType::CLEF][ElementType::TIMESIG] = score->styleMM(Sid::clefTimesigDistance);

    paddingTable[ElementType::BAR_LINE][ElementType::NOTE] = score->styleMM(Sid::barNoteDistance);
    paddingTable[ElementType::BAR_LINE][ElementType::LEDGER_LINE]
        = std::max(paddingTable[ElementType::BAR_LINE][ElementType::NOTE] - ledgerLength, ledgerPad);
    paddingTable[ElementType::BAR_LINE][ElementType::ACCIDENTAL] = score->styleMM(Sid::barAccidentalDistance);
    paddingTable[ElementType::BAR_LINE][ElementType::REST] = score->styleMM(Sid::barNoteDistance);
    paddingTable[ElementType::BAR_LINE][ElementType::CLEF] = score->styleMM(Sid::clefLeftMargin);
    paddingTable[ElementType::BAR_LINE][ElementType::ARPEGGIO] = 0.65 * spatium;
    paddingTable[ElementType::BAR_LINE][ElementType::BAR_LINE] = 1.35 * spatium;
    paddingTable[ElementType::BAR_LINE][ElementType::KEYSIG] = score->styleMM(Sid::keysigLeftMargin);
    paddingTable[ElementType::BAR_LINE][ElementType::TIMESIG] = score->styleMM(Sid::timesigLeftMargin);

    paddingTable[ElementType::KEYSIG][ElementType::NOTE] = 1.75 * spatium;
    paddingTable[ElementType::KEYSIG][ElementType::LEDGER_LINE]
        = std::max(paddingTable[ElementType::KEYSIG][ElementType::NOTE] - ledgerLength, ledgerPad);
    paddingTable[ElementType::KEYSIG][ElementType::ACCIDENTAL] = 1.6 * spatium;
    paddingTable[ElementType::KEYSIG][ElementType::REST] = paddingTable[ElementType::KEYSIG][ElementType::NOTE];
    paddingTable[ElementType::KEYSIG][ElementType::CLEF] = 1.0 * spatium;
    paddingTable[ElementType::KEYSIG][ElementType::ARPEGGIO] = 1.35 * spatium;
    paddingTable[ElementType::KEYSIG][ElementType::BAR_LINE] = score->styleMM(Sid::keyBarlineDistance);
    paddingTable[ElementType::KEYSIG][ElementType::KEYSIG] = 1 * spatium;
    paddingTable[ElementType::KEYSIG][ElementType::TIMESIG] = score->styleMM(Sid::keyTimesigDistance);

    paddingTable[ElementType::TIMESIG][ElementType::NOTE] = 1.35 * spatium;
    paddingTable[ElementType::TIMESIG][ElementType::LEDGER_LINE]
        = std::max(paddingTable[ElementType::TIMESIG][ElementType::NOTE] - ledgerLength, ledgerPad);
    paddingTable[ElementType::TIMESIG][ElementType::ACCIDENTAL] = 0.8 * spatium;
    paddingTable[ElementType::TIMESIG][ElementType::REST] = paddingTable[ElementType::TIMESIG][ElementType::NOTE];
    paddingTable[ElementType::TIMESIG][ElementType::CLEF] = 1.0 * spatium;
    paddingTable[ElementType::TIMESIG][ElementType::ARPEGGIO] = 1.35 * spatium;
    paddingTable[ElementType::TIMESIG][ElementType::BAR_LINE] = score->styleMM(Sid::timesigBarlineDistance);
    paddingTable[ElementType::TIMESIG][ElementType::KEYSIG] = score->styleMM(Sid::keyTimesigDistance);
    paddingTable[ElementType::TIMESIG][ElementType::TIMESIG] = 1.0 * spatium;

    // Obtain the Stem -> * and * -> Stem values from the note equivalents
    paddingTable[ElementType::STEM] = paddingTable[ElementType::NOTE];
    for (auto& elem: paddingTable) {
        elem[ElementType::STEM] = elem[ElementType::NOTE];
    }

    paddingTable[ElementType::STEM][ElementType::NOTE] = score->styleMM(Sid::minNoteDistance);
    paddingTable[ElementType::STEM][ElementType::STEM] = 0.85 * spatium;
    paddingTable[ElementType::STEM][ElementType::ACCIDENTAL] = 0.35 * spatium;
    paddingTable[ElementType::STEM][ElementType::LEDGER_LINE] = 0.35 * spatium;
    paddingTable[ElementType::LEDGER_LINE][ElementType::STEM] = 0.35 * spatium;

    // Ambitus
    paddingTable[ElementType::AMBITUS].fill(score->styleMM(Sid::ambitusMargin));
    for (auto& elem: paddingTable) {
        elem[ElementType::AMBITUS] = score->styleMM(Sid::ambitusMargin);
    }

    // Breath
    paddingTable[ElementType::BREATH].fill(1.0 * spatium);
    for (auto& elem: paddingTable) {
        elem[ElementType::BREATH] = 1.0 * spatium;
    }

    // Temporary hack, because some padding is already constructed inside the lyrics themselves.
    paddingTable[ElementType::BAR_LINE][ElementType::LYRICS] = 0.0 * spatium;

    // Harmony
    paddingTable[ElementType::BAR_LINE][ElementType::HARMONY] = 0.5 * score->styleMM(Sid::minHarmonyDistance);
    paddingTable[ElementType::HARMONY][ElementType::HARMONY] = score->styleMM(Sid::minHarmonyDistance);

    // Chordlines
    paddingTable[ElementType::CHORDLINE].fill(0.35 * spatium);
    for (auto& elem: paddingTable) {
        elem[ElementType::CHORDLINE] = 0.35 * spatium;
    }
    paddingTable[ElementType::BAR_LINE][ElementType::CHORDLINE] = 0.65 * spatium;
    paddingTable[ElementType::CHORDLINE][ElementType::BAR_LINE] = 0.65 * spatium;

    // For the x -> fingering padding use the same values as x -> accidental
    for (auto& elem : paddingTable) {
        elem[ElementType::FINGERING] = elem[ElementType::ACCIDENTAL];
    }

    // This is needed for beamlets, not beams themselves
    paddingTable[ElementType::BEAM][ElementType::BEAM] = 0.4 * spatium;

    // Symbols (semi-hack: the only symbol for which
    // this is relevant is noteHead parenthesis)
    paddingTable[ElementType::SYMBOL] = paddingTable[ElementType::NOTE];
    paddingTable[ElementType::SYMBOL][ElementType::NOTE] = 0.35 * spatium;
    for (auto& elem : paddingTable) {
        elem[ElementType::SYMBOL] = elem[ElementType::ACCIDENTAL];
    }
    paddingTable[ElementType::NOTEDOT][ElementType::SYMBOL] = 0.2 * spatium;
}

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
    Score* score = note->score();

    bool sameVoiceNoteOrStem = (item2->isNote() || item2->isStem()) && note->track() == item2->track();
    if (sameVoiceNoteOrStem) {
        bool intersection = note->shape().translate(note->pos()).intersects(item2->shape().translate(item2->pos()));
        if (intersection) {
            padding = std::max(padding, static_cast<double>(score->styleMM(Sid::minNoteDistance)));
        }
    }

    padding *= scaling;

    if (!(item2->isNote() || item2->isRest())) {
        return;
    }

    if (note->isGrace() && item2->isNote() && toNote(item2)->isGrace()) {
        // Grace-to-grace
        padding = std::max(padding, static_cast<double>(score->styleMM(Sid::graceToGraceNoteDist)));
    } else if (note->isGrace() && (item2->isRest() || (item2->isNote() && !toNote(item2)->isGrace()))) {
        // Grace-to-main
        padding = std::max(padding, static_cast<double>(score->styleMM(Sid::graceToMainNoteDist)));
    } else if (!note->isGrace() && item2->isNote() && toNote(item2)->isGrace()) {
        // Main-to-grace
        padding = std::max(padding, static_cast<double>(score->styleMM(Sid::graceToMainNoteDist)));
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
                minEndPointsDistance = score->styleMM(Sid::MinTieLength);
            } else if (laPoint1.line()->isGlissando()) {
                bool straight = toGlissando(laPoint1.line())->glissandoType() == GlissandoType::STRAIGHT;
                double minGlissandoLength = straight ? score->styleMM(Sid::MinStraightGlissandoLength)
                                            : score->styleMM(Sid::MinWigglyGlissandoLength);
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
    SymId restSym = rest2->sym();
    switch (restSym) {
    case SymId::restWholeLegerLine:
    case SymId::restDoubleWholeLegerLine:
    case SymId::restHalfLegerLine:
        padding += rest2->bbox().left();
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

double HorizontalSpacing::segmentMinHorizontalDistance(const Segment* segment1, const Segment* segment2, bool systemHeaderGap)
{
    if (!segment1 || !segment2) {
        return 0.0;
    }

    if (segment1->isBeginBarLineType() && segment2->isStartRepeatBarLineType()) {
        return 0.0;
    }

    double result = 0.0;
    double distance = 0.0;
    for (unsigned staffIdx = 0; staffIdx < segment1->shapes().size(); ++staffIdx) {
        Shape shape1 = segment1->staffShape(staffIdx);
        Shape shape2 = segment2->staffShape(staffIdx);
        distance = shape1.minHorizontalDistance(shape2);

        if (systemHeaderGap) {
            distance = std::max(distance, shape1.right());
        }

        result = std::max(result, distance);
    }

    double spatium = segment1->spatium();
    double minRight = segment1->minRight();
    Score* score = segment1->score();

    // Header exceptions that need additional space (more than the padding)
    double absoluteMinHeaderDist = 1.5 * spatium;
    if (systemHeaderGap) {
        if (segment1->isTimeSigType()) {
            result = std::max(result, minRight + score->styleMM(Sid::systemHeaderTimeSigDistance));
        } else {
            result = std::max(result, minRight + score->styleMM(Sid::systemHeaderDistance));
        }
        if (segment2 && segment2->isStartRepeatBarLineType()) {
            // Align the thin barline of the start repeat to the header
            result -= score->styleMM(Sid::endBarWidth) + score->styleMM(Sid::endBarDistance);
        }
        double diff = result - minRight - segment2->minLeft();
        if (diff < absoluteMinHeaderDist) {
            result += absoluteMinHeaderDist - diff;
        }
    }

    // Multimeasure rest exceptions that need special handling
    Measure* measure = segment1->measure();
    if (measure && measure->isMMRest()) {
        if (segment2->isChordRestType()) {
            double minDist = minRight;
            if (segment1->isClefType()) {
                minDist += score->paddingTable().at(ElementType::CLEF).at(ElementType::REST);
            } else if (segment1->isKeySigType()) {
                minDist += score->paddingTable().at(ElementType::KEYSIG).at(ElementType::REST);
            } else if (segment1->isTimeSigType()) {
                minDist += score->paddingTable().at(ElementType::TIMESIG).at(ElementType::REST);
            }
            result = std::max(result, minDist);
        } else if (segment1->isChordRestType()) {
            double minWidth = score->styleMM(Sid::minMMRestWidth).val();
            if (!score->styleB(Sid::oldStyleMultiMeasureRests)) {
                minWidth += score->styleMM(Sid::multiMeasureRestMargin).val();
            }
            result = std::max(result, minWidth);
        }
    }

    // Allocate space to ensure minimum length of "dangling" ties or gliss at start of system
    if (systemHeaderGap && segment2 && segment2->isChordRestType()) {
        for (EngravingItem* e : segment2->elist()) {
            if (!e || !e->isChord()) {
                continue;
            }
            double headerTieMargin = score->styleMM(Sid::HeaderToLineStartDistance);
            for (Note* note : toChord(e)->notes()) {
                bool tieOrGlissBack = note->spannerBack().size() || note->tieBack();
                if (!tieOrGlissBack || note->lineAttachPoints().empty()) {
                    continue;
                }
                const EngravingItem* attachedLine = note->lineAttachPoints().front().line();
                double minLength = 0.0;
                if (attachedLine->isTie()) {
                    minLength = score->styleMM(Sid::MinTieLength);
                } else if (attachedLine->isGlissando()) {
                    bool straight = toGlissando(attachedLine)->glissandoType() == GlissandoType::STRAIGHT;
                    minLength = straight ? score->styleMM(Sid::MinStraightGlissandoLength)
                                : score->styleMM(Sid::MinWigglyGlissandoLength);
                }
                double tieStartPointX = minRight + headerTieMargin;
                double notePosX = result + note->pos().x() + toChord(e)->pos().x() + note->headWidth() / 2;
                double tieEndPointX = notePosX + note->lineAttachPoints().at(0).pos().x();
                double tieLength = tieEndPointX - tieStartPointX;
                if (tieLength < minLength) {
                    result += minLength - tieLength;
                }
            }
        }
    }

    return result;
}
