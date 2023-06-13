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
