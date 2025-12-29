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
#include "internal/importfinaleparser.h"
#include "internal/importfinalelogger.h"
#include "internal/finaletypesconv.h"
#include "internal/text/finaletextconv.h"

#include <vector>
#include <exception>

#include "musx/musx.h"

#include "types/string.h"
#include "types/translatablestring.h"

#include "draw/fontmetrics.h"

#include "engraving/dom/anchors.h"
#include "engraving/dom/chordrest.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/glissando.h"
#include "engraving/dom/hairpin.h"
#include "engraving/dom/line.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/navigate.h"
#include "engraving/dom/note.h"
#include "engraving/dom/noteline.h"
#include "engraving/dom/ottava.h"
#include "engraving/dom/score.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/slurtie.h"
#include "engraving/dom/spanner.h"
#include "engraving/dom/system.h"
#include "engraving/dom/textlinebase.h"
#include "engraving/dom/trill.h"
#include "engraving/dom/utils.h"
#include "engraving/dom/vibrato.h"
#include "engraving/dom/volta.h"

#include "engraving/types/typesconv.h"

#include "log.h"

using namespace mu::engraving;
using namespace muse;
using namespace musx::dom;

namespace mu::iex::finale {
static const std::map<std::wstring, ElementType> elementByRegexTable = {
    { LR"(\bped(ale?)?\b)",                                         ElementType::PEDAL },
    { LR"(<sym>keyboardPedal[^>]*?</sym>)",                         ElementType::PEDAL },
    { LR"(\b(((de)?cresc)|(dim))\.?\b)",                            ElementType::HAIRPIN },
    { LR"(\b((rit(\.|ardando)?)|(rall(\.|entando)?))\b)",           ElementType::GRADUAL_TEMPO_CHANGE },
    { LR"(\blet ring\b)",                                           ElementType::LET_RING },
    { LR"(\b(?:(?:8v)|(?:(?:15|22)m))(a|b)\b)",                     ElementType::OTTAVA },
    { LR"(<sym>((ottava|quindicesima)|ventiduesima)[^>]*?<sym>)",   ElementType::OTTAVA },
    { LR"(\bw(?:\/|(?:hammy ))bar\b)",                              ElementType::WHAMMY_BAR },
    { LR"(\brasg(?:ueado)?\b)",                                     ElementType::RASGUEADO },
    { LR"(\bp(?:\.|ick) ?s(?:\.\B|crape\b))",                       ElementType::PICK_SCRAPE },
    { LR"(\bp(?:\.|alm) ?m(?:\.\B|ute\b))",                         ElementType::PALM_MUTE },
    { LR"(<sym>ornamentTrill[^>]*?</sym>)",                         ElementType::TRILL },
};

ReadableCustomLine::ReadableCustomLine(const FinaleParser& context, const MusxInstance<others::SmartShapeCustomLine>& customLine)
{
    // The following properties will be saved directly to the text String later
    // and are usually read from there. This will eventually be changed.
    EnigmaParsingOptions options;
    options.plainText = true; // Easier regex detection
    FontTracker firstFontInfo;
    beginText = context.stringFromEnigmaText(customLine->getLeftStartRawTextCtx(
                                                 context.currentMusxPartId()), options, &firstFontInfo);
    beginFontFamily = firstFontInfo.fontName;
    beginFontSize   = std::max(firstFontInfo.fontSize, 0.1);
    beginFontStyle  = firstFontInfo.fontStyle;

    continueText       = context.stringFromEnigmaText(customLine->getLeftContRawTextCtx(
                                                          context.currentMusxPartId()), options, &firstFontInfo);
    continueFontFamily = firstFontInfo.fontName;
    continueFontSize   = std::max(firstFontInfo.fontSize, 0.1);
    continueFontStyle  = firstFontInfo.fontStyle;

    endText       = context.stringFromEnigmaText(customLine->getRightEndRawTextCtx(
                                                     context.currentMusxPartId()), options, &firstFontInfo);
    endFontFamily = firstFontInfo.fontName;
    endFontSize   = std::max(firstFontInfo.fontSize, 0.1);
    endFontStyle  = firstFontInfo.fontStyle;

    centerLongText       = context.stringFromEnigmaText(customLine->getCenterFullRawTextCtx(
                                                            context.currentMusxPartId()), options, &firstFontInfo);
    centerLongFontFamily = firstFontInfo.fontName;
    centerLongFontSize   = std::max(firstFontInfo.fontSize, 0.1);
    centerLongFontStyle  = firstFontInfo.fontStyle;

    centerShortText       = context.stringFromEnigmaText(customLine->getCenterAbbrRawTextCtx(
                                                             context.currentMusxPartId()), options, &firstFontInfo);
    centerShortFontFamily = firstFontInfo.fontName;
    centerShortFontSize   = std::max(firstFontInfo.fontSize, 0.1);
    centerShortFontStyle  = firstFontInfo.fontStyle;

    if (endText == u"*" /*maestro symbol for pedal star*/) {
        elementType = ElementType::PEDAL;
    }
    for (auto [regexStr, type] : elementByRegexTable) {
        const std::wregex regex(regexStr, std::regex_constants::icase);
        if (beginText.contains(regex) || continueText.contains(regex)) {
            elementType = type;
            break;
        }
    }
    /// @todo lines with up hooks below piano staves as pedal, detect other pedal types (sostenuto)
    /// Not detected / needed: VOLTA, SLUR, HAMMER_ON_PULL_OFF, NOTELINE, HARMONIC_MARK, (GLISSANDO, GUITAR_BEND)

    switch (customLine->lineStyle) {
    case others::SmartShapeCustomLine::LineStyle::Char: {
        SymId lineSym = FinaleTextConv::symIdFromFinaleChar(customLine->charParams->lineChar, customLine->charParams->font);
        lineVisible = customLine->charParams->lineChar != U' ' && lineSym != SymId::space; /// @todo general space symbols
        lineStyle   = LineType::SOLID; // fallback value
        if (lineVisible) {
            glissandoType = GlissandoType::WAVY;
            switch (lineSym) {
            // Trills
            case SymId::wiggleTrill:
                // Also used for glissandos (in MuseScore), but those don't read element type from custom line
                // Tab slide and guitar bend also use custom lines but don't read element type.
                trillType = TrillType::TRILL_LINE;
                elementType = ElementType::TRILL;
                break;
            case SymId::ornamentZigZagLineNoRightEnd:
            case SymId::ornamentZigZagLineWithRightEnd:
                /// @todo detect prall type
                trillType = TrillType::UPPRALL_LINE;
                elementType = ElementType::TRILL;
                break;

            // Vibratos
            case SymId::guitarVibratoStroke:
            case SymId::guitarWideVibratoStroke:
            case SymId::wiggleSawtooth:
            case SymId::wiggleSawtoothWide:
                vibratoType = vibratoTypeFromSymId(lineSym);
                elementType = ElementType::VIBRATO;
                break;

            default: break;
            }
            /// @todo TremoloBar?
        }
        break;
    }
    case others::SmartShapeCustomLine::LineStyle::Solid:
        lineStyle   = LineType::SOLID;
        lineVisible = customLine->solidParams->lineWidth != 0;
        lineWidth   = Spatium(doubleFromEfix(customLine->solidParams->lineWidth));
        break;
    case others::SmartShapeCustomLine::LineStyle::Dashed:
        lineStyle   = LineType::DASHED; /// @todo When should we set lineStyle to LineType::DOTTED ?
        lineVisible = customLine->dashedParams->lineWidth != 0;
        lineWidth   = Spatium(doubleFromEfix(customLine->dashedParams->lineWidth));
        dashLineLen = doubleFromEfix(customLine->dashedParams->dashOn) / lineWidth.val();
        dashGapLen  = doubleFromEfix(customLine->dashedParams->dashOff) / lineWidth.val();
        break;
    }

    context.logger()->logInfo(String(u"Adding spanner of %1 type to custom library").arg(TConv::userName(elementType).translated()));

    if (elementType == ElementType::HAIRPIN) {
        if (beginText.contains(u"decresc", CaseSensitivity::CaseInsensitive)
            || beginText.contains(u"decresc", CaseSensitivity::CaseInsensitive)) {
            hairpinType = HairpinType::DIM_LINE;
        }
    }

    beginHookType = customLine->lineCapStartType == others::SmartShapeCustomLine::LineCapType::Hook ? HookType::HOOK_90 : HookType::NONE;
    endHookType   = customLine->lineCapEndType == others::SmartShapeCustomLine::LineCapType::Hook ? HookType::HOOK_90 : HookType::NONE;
    beginHookHeight = Spatium(doubleFromEfix(customLine->lineCapStartHookLength));
    endHookHeight   = Spatium(doubleFromEfix(customLine->lineCapEndHookLength));
    gapBetweenTextAndLine = Spatium(doubleFromEvpu(customLine->lineStartX)); // Don't use lineEndX or lineContX
    textSizeSpatiumDependent = firstFontInfo.spatiumDependent; /// @todo account for differences between text types
    diagonal = !customLine->makeHorz;

    beginTextPlace    = customLine->lineAfterLeftStartText ? TextPlace::LEFT : TextPlace::BELOW;
    continueTextPlace = customLine->lineAfterLeftContText ? TextPlace::LEFT : TextPlace::BELOW;
    // In MuseScore, this value has no effect. End text always uses left placement on layout.
    // endTextPlace   = customLine->lineBeforeRightEndText ? TextPlace::LEFT : TextPlace::BELOW;

    // Finale's vertical line position is set relative to the text baseline.
    // Horizontal alignment affects the (visible) offset, so use left placement and set the offset later.
    /// @todo distinguish alignment / position
    beginTextAlign    = Align(AlignH::LEFT, AlignV::BASELINE);
    continueTextAlign = Align(AlignH::LEFT, AlignV::BASELINE);
    endTextAlign      = Align(AlignH::RIGHT, AlignV::BASELINE);
    // As the name suggests, this text needs to be centered.
    centerLongTextAlign  = AlignH::HCENTER;
    centerShortTextAlign = AlignH::HCENTER;

    options.plainText = false;
    beginText = context.stringFromEnigmaText(customLine->getLeftStartRawTextCtx(
                                                 context.currentMusxPartId()), options, &firstFontInfo);
    continueText       = context.stringFromEnigmaText(customLine->getLeftContRawTextCtx(
                                                          context.currentMusxPartId()), options, &firstFontInfo);
    endText       = context.stringFromEnigmaText(customLine->getRightEndRawTextCtx(
                                                     context.currentMusxPartId()), options, &firstFontInfo);
    // These are currently only used by glissandos, as plain text.
    // centerLongText = context.stringFromEnigmaText(customLine->getCenterFullRawTextCtx(context.currentMusxPartId()));
    // centerShortText = context.stringFromEnigmaText(customLine->getCenterAbbrRawTextCtx(context.currentMusxPartId()));

    /// @todo I'm not yet sure how text offset affects the default offset/alignment of lines when added to the score.
    /// This may need to be accounted for in spanner segment positioning.
    beginTextOffset    = evpuToPointF(customLine->leftStartX, customLine->lineStartY - customLine->leftStartY);
    continueTextOffset = evpuToPointF(customLine->leftContX, customLine->lineStartY - customLine->leftContY);
    endTextOffset      = evpuToPointF(customLine->rightEndX, customLine->lineEndY - customLine->rightEndY);
    centerLongTextOffset  = evpuToPointF(customLine->centerFullX, customLine->lineStartY - customLine->centerFullY);
    centerShortTextOffset = evpuToPointF(customLine->centerAbbrX, customLine->lineStartY - customLine->centerAbbrY);
}

DirectionV FinaleParser::calculateSlurDirection(Slur* slur)
{
    // Cross-staff or cross-layer chords
    if (slur->track() != slur->track2() || slur->startCR()->vStaffIdx() != slur->endCR()->vStaffIdx()) {
        for (ChordRest* cr : { slur->startCR(), slur->endCR() }) {
            if (!cr->isChord()) {
                continue;
            }
            Chord* c = toChord(cr);
            if (muse::contains(m_fixedChords, c)) {
                return c->stemDirection();
            }
        }
        return DirectionV::UP;
    }
    // Exception for grace notes
    if (slur->startCR()->isGrace() && toChord(slur->startCR())->stemDirection() == DirectionV::UP
        && (!slur->endCR()->isChord()
            || (!muse::contains(m_fixedChords, toChord(slur->endCR())) && toChord(slur->endCR())->notes().size() == 1))) {
        return DirectionV::DOWN;
    }
    for (ChordRest* cr = slur->startCR(); cr; cr = nextChordRest(cr)) {
        if (!cr->isChord()) {
            continue;
        }
        Chord* c = toChord(cr);
        if (!c->isGrace()) {
            if (muse::contains(m_fixedChords, c)) {
                return c->stemDirection();
            }
        }
        if (c->stemDirection() != DirectionV::UP) {
            return DirectionV::UP;
        }
        if (cr == slur->endCR() || cr->tick() >= slur->endCR()->endTick()) {
            break;
        }
    }
    return DirectionV::DOWN;
}

static bool elementsValidForSpannerType(const ElementType type, EngravingItem*& startElement, EngravingItem*& endElement)
{
    switch (type) {
    case ElementType::GLISSANDO:
    case ElementType::GUITAR_BEND:
    case ElementType::NOTELINE:
        if (!startElement || !endElement) {
            return false;
        }
        // bendHat guitar bend is chord-anchored, re-anchor here
        if (startElement->isChord()) {
            startElement = toChord(startElement)->upNote();
        }
        if (endElement->isChord()) {
            endElement = toChord(endElement)->upNote();
        }
        return startElement->isNote() && endElement->isNote() && startElement->parentItem() != endElement->parentItem();
    case ElementType::SLUR:
        return startElement && startElement->isChordRest() && endElement && endElement->isChordRest() && startElement != endElement;
    default:
        break;
    }
    return true;
}

static ElementType spannerTypeFromElements(EngravingItem* startElement, EngravingItem* endElement)
{
    if (startElement && startElement->isNote() && endElement && endElement->isNote()) {
        return ElementType::NOTELINE;
    }
    return ElementType::TEXTLINE;
}

void FinaleParser::importSmartShapes()
{
    const MusxInstanceList<others::SmartShape> smartShapes = m_doc->getOthers()->getArray<others::SmartShape>(m_currentMusxPartId);
    logger()->logInfo(String(u"Import smart shapes: Found %1 smart shapes").arg(smartShapes.size()));
    for (const MusxInstance<others::SmartShape>& smartShape : smartShapes) {
        if (smartShape->shapeType == others::SmartShape::ShapeType::WordExtension
            || smartShape->shapeType == others::SmartShape::ShapeType::Hyphen) {
            // Will be handled elsewhere
            continue;
        }
        if (!smartShape->startTermSeg->endPoint->calcIsAssigned() || !smartShape->endTermSeg->endPoint->calcIsAssigned()) {
            // Unassigned shape, no need to import
            continue;
        }

        bool endsOnBarline = false;
        bool startsOnBarline = false;
        auto tickFromTerminationSeg
            = [&](ElementType type, const MusxInstance<others::SmartShape>& smartShape, EngravingItem*& e, bool start) -> Fraction {
            logger()->logInfo(String(u"Finding spanner element..."));
            const MusxInstance<others::SmartShape::TerminationSeg>& termSeg = start ? smartShape->startTermSeg : smartShape->endTermSeg;
            // Slurs must anchor to a specific entry in MuseScore
            EntryInfoPtr entryInfoPtr = termSeg->endPoint->calcAssociatedEntry(m_currentMusxPartId, type != ElementType::SLUR);
            if (entryInfoPtr) {
                NoteNumber nn = start ? smartShape->startNoteId : smartShape->endNoteId;
                if (nn) {
                    e = toEngravingItem(noteFromEntryInfoAndNumber(entryInfoPtr, nn));
                } else {
                    e = toEngravingItem(chordRestFromEntryInfoPtr(entryInfoPtr));
                }
                if (e) {
                    logger()->logInfo(String(u"Found %1 to anchor to").arg(TConv::userName(e->type()).translated()));
                    return e->tick();
                } else {
                    logger()->logInfo(String(u"Can't anchor to note/CR!"));
                    return Fraction(-1, 1); // dbg
                }
            }
            logger()->logInfo(String(u"No anchor found"));
            staff_idx_t staffIdx = muse::value(m_inst2Staff, termSeg->endPoint->staffId, muse::nidx);
            Fraction mTick = muse::value(m_meas2Tick, termSeg->endPoint->measId, Fraction(-1, 1));
            Measure* measure = !mTick.negative() ? m_score->tick2measure(mTick) : nullptr;
            if (!measure || staffIdx == muse::nidx) {
                return Fraction(-1, 1);
            }
            Fraction rTick = musxFractionToFraction(termSeg->endPoint->calcGlobalPosition());
            if (start) {
                startsOnBarline = rTick >= measure->ticks();
            } else {
                endsOnBarline = rTick >= measure->ticks();
            }
            return mTick + rTick;
        };

        ReadableCustomLine* customLine = [&]() -> ReadableCustomLine* {
            if (smartShape->lineStyleId == 0) {
                // Shape does not use custom line
                return nullptr;
            }
            // Search our converted shape library, or if not found add to it
            ReadableCustomLine* line = muse::value(m_customLines, smartShape->lineStyleId, nullptr);
            if (!line) {
                line
                    = new ReadableCustomLine(*this, m_doc->getOthers()->get<others::SmartShapeCustomLine>(m_currentMusxPartId,
                                                                                                          smartShape->lineStyleId));
                m_customLines.emplace(smartShape->lineStyleId, line);
            }
            return line;
        }();

        ElementType type = elementTypeFromShapeType(smartShape->shapeType);
        if (type == ElementType::INVALID) {
            if (!customLine) {
                logger()->logWarning(String(u"Invalid spanner type"));
                continue;
            }
            type = customLine->elementType;
        }

        // Find start and end elements, and change element type if needed
        EngravingItem* startElement = nullptr;
        EngravingItem* endElement = nullptr;
        Fraction startTick = tickFromTerminationSeg(type, smartShape, startElement, true);
        Fraction endTick = tickFromTerminationSeg(type, smartShape, endElement, false);
        if (customLine && type == ElementType::TEXTLINE) {
            /// @todo create notelines instead of textlines also for non-custom?
            type = spannerTypeFromElements(startElement, endElement);
        }
        if (startTick.negative() || endTick.negative() || !elementsValidForSpannerType(type, startElement, endElement)) {
            logger()->logInfo(String(u"Cannot create spanner of %1 type with given start/end elements. Start: %2, end: %3").arg(
                                  TConv::userName(type).translated(), startTick.toString(), endTick.toString()));
            continue;
        }

        // Don't create backwards spanners (should never happen, but just to be safe)
        IF_ASSERT_FAILED(endTick >= startTick) {
            std::swap(startTick, endTick);
            std::swap(startElement, endElement);
        }

        // Create spanner
        logger()->logInfo(String(u"Creating spanner of %1 type").arg(TConv::userName(type).translated()));
        Spanner* newSpanner = toSpanner(Factory::createItem(type, m_score->dummy()));
        newSpanner->setScore(m_score);

        if (smartShape->entryBased || newSpanner->isSlur()) {
            if (!startElement || !endElement) {
                // should never happen
                logger()->logInfo(String(u"No start/end element for spanner of %1 type").arg(TConv::userName(type).translated()));
                delete newSpanner;
                continue;
            }
            if (smartShape->startNoteId && smartShape->endNoteId) {
                newSpanner->setAnchor(Spanner::Anchor::NOTE);
            } else {
                newSpanner->setAnchor(Spanner::Anchor::CHORD);
            }
            newSpanner->setTrack(startElement->track());
            newSpanner->setTrack2(endElement->track());
            newSpanner->setStartElement(startElement);
            newSpanner->setEndElement(endElement);
        } else {
            newSpanner->setAnchor(Spanner::Anchor::SEGMENT);
            staff_idx_t staffIdx1 = muse::value(m_inst2Staff, smartShape->startTermSeg->endPoint->staffId, muse::nidx);
            staff_idx_t staffIdx2 = muse::value(m_inst2Staff, smartShape->endTermSeg->endPoint->staffId, muse::nidx);
            if (staffIdx1 == muse::nidx || staffIdx2 == muse::nidx) {
                logger()->logInfo(String(u"No start/end staff for spanner of %1 type").arg(TConv::userName(type).translated()));
                delete newSpanner;
                continue;
            }
            newSpanner->setTrack(staff2track(staffIdx1));
            newSpanner->setTrack2(staff2track(staffIdx2));
            // don't set end elements, instead a computed start/end segment is called
        }
        newSpanner->setTick(startTick);
        newSpanner->setTick2(endTick);

        // Set properties
        newSpanner->setVisible(!smartShape->hidden);

        if (customLine) {
            if (!newSpanner->isSLine()) {
                throw std::invalid_argument("Spanner is not line-based.");
            }

            SLine* line = (SLine*)newSpanner; /// @todo add to engraving/dom

            setAndStyleProperty(line, Pid::DIAGONAL, customLine->diagonal);
            setAndStyleProperty(line, Pid::LINE_STYLE, customLine->lineStyle);
            setAndStyleProperty(line, Pid::LINE_WIDTH, customLine->lineWidth);
            setAndStyleProperty(line, Pid::DASH_LINE_LEN, customLine->dashLineLen);
            setAndStyleProperty(line, Pid::DASH_GAP_LEN, customLine->dashGapLen);

            if (newSpanner->isTextLineBase()) {
                TextLineBase* textLineBase = toTextLineBase(newSpanner);

                setAndStyleProperty(textLineBase, Pid::LINE_VISIBLE, customLine->lineVisible);
                setAndStyleProperty(textLineBase, Pid::BEGIN_HOOK_TYPE, customLine->beginHookType);
                setAndStyleProperty(textLineBase, Pid::END_HOOK_TYPE, customLine->endHookType);
                setAndStyleProperty(textLineBase, Pid::BEGIN_HOOK_HEIGHT, customLine->beginHookHeight);
                setAndStyleProperty(textLineBase, Pid::END_HOOK_HEIGHT, customLine->endHookHeight);
                setAndStyleProperty(textLineBase, Pid::GAP_BETWEEN_TEXT_AND_LINE, customLine->gapBetweenTextAndLine);
                setAndStyleProperty(textLineBase, Pid::TEXT_SIZE_SPATIUM_DEPENDENT, customLine->textSizeSpatiumDependent);

                setAndStyleProperty(textLineBase, Pid::BEGIN_TEXT_PLACE, customLine->continueTextPlace);
                setAndStyleProperty(textLineBase, Pid::BEGIN_TEXT, customLine->beginText);
                setAndStyleProperty(textLineBase, Pid::BEGIN_TEXT_ALIGN, customLine->beginTextAlign);
                setAndStyleProperty(textLineBase, Pid::BEGIN_FONT_FACE, customLine->beginFontFamily);
                setAndStyleProperty(textLineBase, Pid::BEGIN_FONT_SIZE, customLine->beginFontSize);
                setAndStyleProperty(textLineBase, Pid::BEGIN_FONT_STYLE, int(customLine->beginFontStyle));
                setAndStyleProperty(textLineBase, Pid::BEGIN_TEXT_OFFSET, customLine->beginTextOffset);

                setAndStyleProperty(textLineBase, Pid::CONTINUE_TEXT_PLACE, customLine->continueTextPlace);
                setAndStyleProperty(textLineBase, Pid::CONTINUE_TEXT, customLine->continueText);
                setAndStyleProperty(textLineBase, Pid::CONTINUE_TEXT_ALIGN, customLine->continueTextAlign);
                setAndStyleProperty(textLineBase, Pid::CONTINUE_FONT_FACE, customLine->continueFontFamily);
                setAndStyleProperty(textLineBase, Pid::CONTINUE_FONT_SIZE, customLine->continueFontSize);
                setAndStyleProperty(textLineBase, Pid::CONTINUE_FONT_STYLE, int(customLine->continueFontStyle));
                setAndStyleProperty(textLineBase, Pid::CONTINUE_TEXT_OFFSET, customLine->continueTextOffset);

                // setAndStyleProperty(textLineBase, Pid::END_TEXT_PLACE, customLine->endTextPlace);
                setAndStyleProperty(textLineBase, Pid::END_TEXT, customLine->endText);
                setAndStyleProperty(textLineBase, Pid::END_TEXT_ALIGN, customLine->endTextAlign);
                setAndStyleProperty(textLineBase, Pid::END_FONT_FACE, customLine->endFontFamily);
                setAndStyleProperty(textLineBase, Pid::END_FONT_SIZE, customLine->endFontSize);
                setAndStyleProperty(textLineBase, Pid::END_FONT_STYLE, int(customLine->endFontStyle));
                setAndStyleProperty(textLineBase, Pid::END_TEXT_OFFSET, customLine->endTextOffset);

                /// @todo which custom lines have playback?
                if (newSpanner->isOttava()) {
                    setAndStyleProperty(newSpanner, Pid::PLAY, false);
                } else if (newSpanner->isHairpin()) {
                    setAndStyleProperty(newSpanner, Pid::PLAY, false);
                    setAndStyleProperty(newSpanner, Pid::HAIRPIN_TYPE, int(customLine->hairpinType));
                }
            } else if (newSpanner->isTrill()) {
                toTrill(newSpanner)->setTrillType(customLine->trillType);
            } else if (newSpanner->isVibrato()) {
                toVibrato(newSpanner)->setVibratoType(customLine->vibratoType);
            } else if (newSpanner->isGlissando()) {
                Glissando* glissando = toGlissando(newSpanner);
                setAndStyleProperty(glissando, Pid::GLISS_TYPE, int(customLine->glissandoType));
                if (!customLine->centerShortText.empty()) {
                    setAndStyleProperty(glissando, Pid::GLISS_SHOW_TEXT, true);
                    setAndStyleProperty(glissando, Pid::GLISS_TEXT, customLine->centerShortText);
                    setAndStyleProperty(glissando, Pid::FONT_STYLE, int(customLine->centerShortFontStyle));
                    setAndStyleProperty(glissando, Pid::FONT_FACE, customLine->centerShortFontFamily);
                    setAndStyleProperty(glissando, Pid::FONT_SIZE, customLine->centerShortFontSize);
                } else if (!customLine->centerLongText.empty()) {
                    setAndStyleProperty(glissando, Pid::GLISS_SHOW_TEXT, true);
                    setAndStyleProperty(glissando, Pid::GLISS_TEXT, customLine->centerShortText);
                    setAndStyleProperty(glissando, Pid::FONT_STYLE, int(customLine->centerLongFontStyle));
                    setAndStyleProperty(glissando, Pid::FONT_FACE, customLine->centerLongFontFamily);
                    setAndStyleProperty(glissando, Pid::FONT_SIZE, customLine->centerLongFontSize);
                } else {
                    setAndStyleProperty(glissando, Pid::GLISS_SHOW_TEXT, false);
                }
            } else if (newSpanner->isGuitarBend()) {
                // Assume custom line is for tab bends (with arrow)
                collectGlobalProperty(Sid::guitarBendLineWidthTab, customLine->lineWidth);
            }
        } else {
            if (type == ElementType::OTTAVA) {
                toOttava(newSpanner)->setOttavaType(ottavaTypeFromShapeType(smartShape->shapeType));
                if (endElement && !endsOnBarline) {
                    newSpanner->setEndElement(endElement);
                    newSpanner->setTick2(toChordRest(endElement)->endTick());
                }
                // Account for odd text offset
                if (importAllPositions()) {
                    muse::draw::Font f(score()->engravingFont()->family(), muse::draw::Font::Type::MusicSymbol);
                    f.setPointSizeF(2.0 * m_score->style().styleD(Sid::ottavaFontSize) * newSpanner->magS()); // This has been tested and is scaled correctly
                    muse::draw::FontMetrics fm(f);
                    PointF textoffset(0.0, absoluteDouble(0.75, newSpanner));
                    textoffset.ry() += fm.tightBoundingRect(score()->engravingFont()->symCode(SymId::ottavaAlta)).bottom();
                    if (newSpanner->placeAbove()) {
                        textoffset.ry() -= fm.boundingRect(score()->engravingFont()->symCode(SymId::ottavaAlta)).height();
                    }
                    toOttava(newSpanner)->setBeginTextOffset(textoffset);
                    toOttava(newSpanner)->setContinueTextOffset(textoffset);
                    toOttava(newSpanner)->setEndTextOffset(textoffset);
                }
            } else if (type == ElementType::HAIRPIN) {
                HairpinType ht = hairpinTypeFromShapeType(smartShape->shapeType);
                toHairpin(newSpanner)->setHairpinType(ht);
                // Hairpin height: A per-system setting in Finale; We just read the first or last one.
                if (importCustomPositions()) {
                    const auto& termSeg = ht == HairpinType::DIM_HAIRPIN ? smartShape->startTermSeg : smartShape->endTermSeg;
                    if (termSeg->ctlPtAdj->active) {
                        setAndStyleProperty(newSpanner, Pid::HAIRPIN_HEIGHT,
                                            spatiumFromEvpu(termSeg->ctlPtAdj->startCtlPtY, newSpanner));
                    }
                }
            } else if (type == ElementType::SLUR) {
                Slur* slur = toSlur(newSpanner);
                setAndStyleProperty(slur, Pid::SLUR_STYLE_TYPE, slurStyleTypeFromShapeType(smartShape->shapeType));
                if (importCustomPositions()) {
                    setAndStyleProperty(slur, Pid::SLUR_DIRECTION, directionVFromShapeType(smartShape->shapeType));
                }
                if (importAllPositions()) {
                    if (slur->slurDirection() == DirectionV::AUTO) {
                        setAndStyleProperty(slur, Pid::SLUR_DIRECTION, calculateSlurDirection(slur));
                    }
                    if (slur->track() != slur->track2() || slur->startCR()->vStaffIdx() != slur->endCR()->vStaffIdx()) {
                        slur->setAutoplace(false);
                    } else if (smartShape->engraverSlurState == others::SmartShape::EngraverSlurState::Auto) {
                        slur->setAutoplace(musxOptions().smartShapeOptions->useEngraverSlurs);
                    } else {
                        slur->setAutoplace(smartShape->engraverSlurState == others::SmartShape::EngraverSlurState::On);
                    }
                }
            } else if (type == ElementType::GLISSANDO) {
                setAndStyleProperty(newSpanner, Pid::GLISS_TYPE, int(glissandoTypeFromShapeType(smartShape->shapeType)));
                setAndStyleProperty(newSpanner, Pid::GLISS_SHOW_TEXT, false); /// @todo Is this the correct default?
            } else if (type == ElementType::TRILL) {
                toTrill(newSpanner)->setTrillType(TrillType::TRILL_LINE);
            } else if (type == ElementType::TEXTLINE) {
                TextLineBase* textLine = toTextLineBase(newSpanner);
                setAndStyleProperty(textLine, Pid::LINE_STYLE, lineTypeFromShapeType(smartShape->shapeType));
                auto [beginHook, endHook] = hookHeightsFromShapeType(smartShape->shapeType);
                if (beginHook) {
                    setAndStyleProperty(textLine, Pid::BEGIN_HOOK_TYPE, HookType::HOOK_90);
                    Spatium s = spatiumFromEvpu(beginHook * musxOptions().smartShapeOptions->hookLength, textLine);
                    setAndStyleProperty(textLine, Pid::BEGIN_HOOK_HEIGHT, s, true);
                } else {
                    setAndStyleProperty(textLine, Pid::BEGIN_HOOK_TYPE, HookType::NONE);
                }
                if (endHook) {
                    setAndStyleProperty(textLine, Pid::END_HOOK_TYPE, HookType::HOOK_90);
                    Spatium s = spatiumFromEvpu(endHook * musxOptions().smartShapeOptions->hookLength, textLine);
                    setAndStyleProperty(textLine, Pid::END_HOOK_HEIGHT, s, true);
                } else {
                    setAndStyleProperty(textLine, Pid::END_HOOK_TYPE, HookType::NONE);
                }
            }
        }

        // Some guitar bends are both pre-built and have a custom line component
        if (type == ElementType::GUITAR_BEND) {
            if (newSpanner->startElement() == newSpanner->endElement()) {
                toGuitarBend(newSpanner)->setType(GuitarBendType::SLIGHT_BEND);
            } else if (toNote(newSpanner->startElement())->chord()->isGrace()) {
                toGuitarBend(newSpanner)->setType(GuitarBendType::GRACE_NOTE_BEND);
                toGuitarBend(newSpanner)->setEndTimeFactor(GuitarBend::GRACE_NOTE_BEND_DEFAULT_END_TIME_FACTOR);
            } else {
                toGuitarBend(newSpanner)->setType(GuitarBendType::BEND);
            }
        }

        if (newSpanner->anchor() == Spanner::Anchor::NOTE) {
            newSpanner->setParent(startElement);
            toNote(startElement)->add(newSpanner);
            logger()->logInfo(String(u"Added spanner of %1 type to note at tick %2, end: %3").arg(
                                  TConv::userName(type).translated(), startTick.toString(), endTick.toString()));
        } else {
            m_score->addElement(newSpanner);
            logger()->logInfo(String(u"Added spanner of %1 type at tick %2, end: %3").arg(TConv::userName(type).translated(),
                                                                                          startTick.toString(), endTick.toString()));
        }

        if (newSpanner->systemFlag()) {
            m_systemObjectStaves.insert(newSpanner->staffIdx());
        }

        // Layout is currently only supported for segment-based lines
        if (!importCustomPositions() || !newSpanner->isSLine() || newSpanner->anchor() == Spanner::Anchor::NOTE) {
            continue;
        }

        // Calculate position in score

        // Hack: Finale distinguishes between barline and CR anchoring, account for that here
        Measure* startMeasure = m_score->tick2measureMM(startTick);
        const bool startsOnSystemStart = startMeasure->isFirstInSystem() && startMeasure->prevMeasure();
        if (startsOnBarline && startsOnSystemStart) {
            newSpanner->setTick(newSpanner->tick() - Fraction::eps());
        }
        Measure* endMeasure = m_score->tick2measureMM(endTick);
        const bool endsOnSystemEnd = !endMeasure || endMeasure->isFirstInSystem();
        if (!endsOnBarline && endsOnSystemEnd) {
            newSpanner->setTick2(newSpanner->tick2() + Fraction::eps());
        }

        m_score->renderer()->layoutItem(newSpanner);
        logger()->logInfo(String(u"Repositioning %1 spanner segments...").arg(newSpanner->spannerSegments().size()));

        setAndStyleProperty(newSpanner, Pid::PLACEMENT, PlacementV::ABOVE, true); // for now
        const bool diagonal = newSpanner->isSLine() && ((SLine*)newSpanner)->diagonal();
        bool canPlaceBelow = !diagonal;
        bool isEntirelyInStaff = !diagonal;
        const bool isStandardOttava = !customLine && type == ElementType::OTTAVA;
        // Current layout code only uses staff height at start tick
        const double staffHeight = newSpanner->staff()->staffHeight(newSpanner->tick());

        for (SpannerSegment* ss : newSpanner->spannerSegments()) {
            ss->setAutoplace(false);

            auto positionSegmentFromEndPoints = [&](const std::shared_ptr<smartshape::EndPointAdjustment>& leftPoint,
                                                    const std::shared_ptr<smartshape::EndPointAdjustment>& rightPoint) {
                if (leftPoint->active) {
                    ss->setOffset(evpuToPointF(leftPoint->horzOffset, -leftPoint->vertOffset) * ss->spatium());
                    if (leftPoint->contextDir == smartshape::DirectionType::Under) {
                        ss->ryoffset() += staffHeight;
                    }
                }
                if (rightPoint->active) {
                    ss->setUserOff2(evpuToPointF(rightPoint->horzOffset, -rightPoint->vertOffset) * ss->spatium());
                    // For non-diagonal line segments, MS resets userOff2's Y component.
                    // If the left point doesn't set the value, get it from the right point instead.
                    // Points can be active but still not specify a value.
                    if (leftPoint->vertOffset == 0 || (!leftPoint->active && !diagonal)) {
                        ss->ryoffset() = ss->userOff2().y();
                    }
                }
            };

            if (ss->isSingleType()) {
                positionSegmentFromEndPoints(smartShape->startTermSeg->endPointAdj, smartShape->endTermSeg->endPointAdj);
            } else if (ss->isBeginType()) {
                positionSegmentFromEndPoints(smartShape->startTermSeg->endPointAdj, smartShape->startTermSeg->breakAdj);
            } else if (ss->isEndType()) {
                positionSegmentFromEndPoints(smartShape->endTermSeg->breakAdj, smartShape->endTermSeg->endPointAdj);
            } else if (ss->isMiddleType()) {
                MeasCmper measId = muse::value(m_tick2Meas, ss->system()->firstMeasure()->tick(), MeasCmper());
                if (auto measure = m_doc->getOthers()->get<others::Measure>(m_currentMusxPartId, measId)) {
                    if (!measure->hasSmartShape) {
                        continue;
                    }
                    auto assigns = m_doc->getOthers()->getArray<others::SmartShapeMeasureAssign>(m_currentMusxPartId, measId);
                    for (const auto& assign : assigns) {
                        if (assign->shapeNum != smartShape->getCmper()) {
                            continue;
                        }
                        if (const auto centerShape = m_doc->getDetails()->get<details::CenterShape>(m_currentMusxPartId,
                                                                                                    assign->shapeNum,
                                                                                                    assign->centerShapeNum)) {
                            positionSegmentFromEndPoints(centerShape->startBreakAdj, centerShape->endBreakAdj);
                        }
                        break;
                    }
                }
            }

            // Adjust start pos
            if (ss->isSingleBeginType()) {
                Segment* startSeg = ss->spanner()->startSegment();
                if (startsOnBarline && startsOnSystemStart) {
                    Segment* bls = startSeg->next(SegmentType::EndBarLine);
                    if (bls && bls->tick() == ss->spanner()->tick() + Fraction::eps()) {
                        startSeg = bls;
                    }
                } else if (startsOnBarline && !startsOnSystemStart) {
                    Segment* bls = startSeg->prev1(SegmentType::EndBarLine);
                    if (bls && bls->tick() == ss->spanner()->tick()) {
                        startSeg = bls;
                    }
                }
                System* s;
                ss->rxoffset() += startSeg->x() + startSeg->measure()->x() - ((SLine*)newSpanner)->linePos(Grip::START, &s).x();
            } else {
                Segment* firstCRseg = ss->system()->firstMeasure()->first(SegmentType::ChordRest);
                for (Segment* s = firstCRseg->prevActive(); s;
                     s = s->prev(SegmentType::HeaderClef | SegmentType::KeySig | SegmentType::TimeSigType)) {
                    if (!s->isActive() || s->allElementsInvisible() || s->hasTimeSigAboveStaves()) {
                        continue;
                    }
                    ss->rxoffset() += firstCRseg->x() + firstCRseg->measure()->x() - ss->system()->firstNoteRestSegmentX(true);
                    if (s->isHeaderClefType()) {
                        ss->rxoffset() -= absoluteDoubleFromEvpu(musxOptions().clefOptions->clefBackSepar, ss);
                    } else if (s->isKeySigType()) {
                        ss->rxoffset() -= absoluteDoubleFromEvpu(musxOptions().keyOptions->keyBack, ss);
                    } else if (s->isTimeSigType()) {
                        ss->rxoffset() -= absoluteDoubleFromEvpu(partScore() ? musxOptions().timeOptions->timeBackParts
                                                                 : musxOptions().timeOptions->timeBack, ss);
                    }
                    break;
                }
            }

            // In MuseScore, userOff2 is relative/added to offset
            ss->setUserOff2(ss->userOff2() - ss->offset());

            // Adjust end pos
            if (ss->isSingleEndType()) {
                Segment* endSeg = ss->spanner()->endSegment();
                if (!endsOnBarline && endsOnSystemEnd) {
                    Segment* systemStartSeg = endSeg->prev(SegmentType::ChordRest);
                    if (systemStartSeg && systemStartSeg->tick() + Fraction::eps() == endSeg->tick()) {
                        endSeg = systemStartSeg;
                    }
                } else if (endsOnBarline) {
                    Segment* bls = endSeg->prev1(SegmentType::EndBarLine);
                    if (bls && bls->tick() == ss->spanner()->tick2()) {
                        endSeg = bls;
                    }
                } else if (isStandardOttava) {
                    if (ss->spanner()->endElement() && ss->spanner()->endElement()->isChordRest()) {
                        endSeg = toChordRest(ss->spanner()->endElement())->segment();
                    }
                }
                System* s;
                ss->rUserXoffset2() += endSeg->x() + endSeg->measure()->x() - ((SLine*)newSpanner)->linePos(Grip::END, &s).x();
                /// @todo account for presence of text
            } else {
                ss->rUserXoffset2() += ss->style().styleMM(Sid::lineEndToBarlineDistance);
            }

            // Adjust ottava positioning
            if (isStandardOttava) {
                ss->ryoffset() -= absoluteDouble(0.75, ss);
            }

            canPlaceBelow = canPlaceBelow && ss->offset().y() > 0;
            isEntirelyInStaff = isEntirelyInStaff && canPlaceBelow && (ss->offset().y() < staffHeight);
            setAndStyleProperty(ss, Pid::OFFSET, PropertyValue(), true);
        }

        const bool shouldPlaceBelow = canPlaceBelow
                                      && (!isEntirelyInStaff || newSpanner->propertyDefault(Pid::PLACEMENT) == PlacementV::BELOW);
        if (customLine && type == ElementType::OTTAVA) {
            int below = 0;
            static const std::wregex belowRegex(LR"((?:v|m)b)", std::regex_constants::icase);
            static const std::wregex aboveRegex(LR"((?:v|m)(?:a|e))", std::regex_constants::icase);
            if (customLine->beginText.contains(belowRegex)) {
                below = 1;
            } else if (!customLine->beginText.contains(aboveRegex)) {
                below = shouldPlaceBelow;
            }
            if (customLine->beginText.contains(u"15") || customLine->beginText.contains(u"16")) {
                toOttava(newSpanner)->setOttavaType(OttavaType(int(OttavaType::OTTAVA_15MA) + below));
            } else if (customLine->beginText.contains(u"22")) {
                toOttava(newSpanner)->setOttavaType(OttavaType(int(OttavaType::OTTAVA_22MA) + below));
            } else {
                toOttava(newSpanner)->setOttavaType(OttavaType(int(OttavaType::OTTAVA_8VA) + below));
            }
        }

        // Apply placement changes after setting ottava type, to inherit style if possible
        if (shouldPlaceBelow) {
            setAndStyleProperty(newSpanner, Pid::PLACEMENT, PlacementV::BELOW, true);
            for (SpannerSegment* ss : newSpanner->spannerSegments()) {
                ss->ryoffset() -= staffHeight;
                setAndStyleProperty(ss, Pid::OFFSET, PropertyValue(), true);
            }
            // MuseScore inverts hook lengths on elements placed below the staff, so invert for custom set hook heights
            if (!customLine && type == ElementType::TEXTLINE) {
                TextLineBase* textLine = toTextLineBase(newSpanner);
                setAndStyleProperty(textLine, Pid::BEGIN_HOOK_HEIGHT, -textLine->beginHookHeight(), true);
                setAndStyleProperty(textLine, Pid::END_HOOK_HEIGHT, -textLine->endHookHeight(), true);
            }
        }

        if (newSpanner->hasVoiceAssignmentProperties()) {
            setAndStyleProperty(newSpanner, Pid::DIRECTION, newSpanner->placeAbove() ? DirectionV::UP : DirectionV::DOWN);
        }

        if (type == ElementType::HAIRPIN) {
            // todo: declare hairpin placement in hairpin elementStyle?
            /// @todo make hairpins account for anchored dynamics

            // If not otherwise set, determine hairpin height by length
            if (toHairpin(newSpanner)->isLineType() && newSpanner->isStyled(Pid::HAIRPIN_HEIGHT)) {
                SpannerSegment* ss = toHairpin(newSpanner)->hairpinType()
                                     == HairpinType::DIM_HAIRPIN ? newSpanner->frontSegment() : newSpanner->backSegment();
                if (ss->ipos2().x() > (absoluteDoubleFromEvpu(musxOptions().smartShapeOptions->shortHairpinOpeningWidth, ss))) {
                    setAndStyleProperty(newSpanner, Pid::HAIRPIN_HEIGHT,
                                        spatiumFromEvpu(musxOptions().smartShapeOptions->crescHeight, newSpanner), true);
                } else {
                    setAndStyleProperty(newSpanner, Pid::HAIRPIN_HEIGHT,
                                        spatiumFromEvpu(musxOptions().smartShapeOptions->shortHairpinOpeningWidth, newSpanner),
                                        true);
                }
            }
        }
    }

    // Voltas
    const auto endingBegins = m_doc->getOthers()->getArray<others::RepeatEndingStart>(m_currentMusxPartId);
    const auto endingEnds = m_doc->getOthers()->getArray<others::RepeatBack>(m_currentMusxPartId);
    const Evpu leftInset = musxOptions().repeatOptions->bracketStartInset;
    const Evpu rightInset = musxOptions().repeatOptions->bracketEndInset;
    const Evpu beginHookLen = musxOptions().repeatOptions->bracketHookLen;
    const Evpu endHookLen = musxOptions().repeatOptions->bracketEndHookLen;
    const Evpu startY = -musxOptions().repeatOptions->bracketHeight;
    const Evpu textPosX = musxOptions().repeatOptions->bracketTextHPos;
    // const Evpu textPosY = -musxOptions().repeatOptions->bracketTextVPos;
    std::vector<Volta*> openings;
    openings.reserve(endingBegins.size());

    // Read the start endings
    for (const MusxInstance<others::RepeatEndingStart>& endingBegin : endingBegins) {
        // Find staff
        std::vector<std::pair<staff_idx_t, StaffCmper> > links;
        staff_idx_t curStaffIdx = staffIdxForRepeats(endingBegin->topStaffOnly, endingBegin->staffList, endingBegin->getCmper(), links);
        if (curStaffIdx == muse::nidx) {
            logger()->logWarning(String(u"Add voltas: Musx inst value not found."));
            continue;
        }

        // Find location in measure
        Fraction mTick = muse::value(m_meas2Tick, endingBegin->getCmper(), Fraction(-1, 1));
        Measure* measure = !mTick.negative() ? m_score->tick2measure(mTick) : nullptr;
        if (!measure) {
            continue;
        }

        track_idx_t curTrackIdx = staff2track(curStaffIdx);
        logger()->logInfo(String(u"Creating a volta at tick %1 on track %2.").arg(measure->tick().toString(), String::number(curTrackIdx)));

        Volta* volta = Factory::createVolta(m_score->dummy());
        volta->setTrack(curTrackIdx);
        volta->setTick(measure->tick());
        volta->setTick2(measure->endTick());
        volta->setVisible(!endingBegin->hidden);
        volta->setText(String::fromStdString(endingBegin->createEndingText()));
        if (const auto& passList = m_doc->getOthers()->get<others::RepeatPassList>(m_currentMusxPartId, endingBegin->getCmper())) {
            volta->setEndings(passList->values);
        }
        m_score->addElement(volta);
        m_systemObjectStaves.insert(curStaffIdx);

        if (importCustomPositions()) {
            volta->fixupSegments(1, [volta](System* parent) { return volta->createLineSegment(parent); });
            VoltaSegment* vs = toVoltaSegment(volta->frontSegment());
            vs->setSystem(measure->system());

            double startHook = doubleFromEvpu(beginHookLen - endingBegin->leftVPos + endingBegin->rightVPos);
            double endHook = doubleFromEvpu(endHookLen - endingBegin->endLineVPos + endingBegin->rightVPos);
            PointF startP = evpuToPointF(leftInset + endingBegin->leftHPos, startY - endingBegin->rightVPos) * volta->spatium();
            PointF endP = evpuToPointF(-rightInset + endingBegin->rightHPos - startP.x(), 0.0) * volta->spatium();
            PointF textP = evpuToPointF(textPosX - 24 + endingBegin->textHPos, -endingBegin->textVPos) * volta->spatium();
            textP.ry() += startHook * volta->spatium();

            volta->setBeginHookHeight(Spatium(startHook));
            // For open voltas, inherit the starting height (but don't display it)
            if (muse::RealIsEqual(endHook, 0.0)) {
                volta->setVoltaType(Volta::Type::OPEN);
                volta->setEndHookHeight(Spatium(startHook));
            } else {
                volta->setVoltaType(Volta::Type::CLOSED);
                volta->setEndHookHeight(Spatium(endHook));
            }
            volta->setAutoplace(false);
            setAndStyleProperty(volta, Pid::BEGIN_TEXT_OFFSET, textP, true);
            setAndStyleProperty(volta, Pid::CONTINUE_TEXT_OFFSET, textP, true);
            setAndStyleProperty(vs, Pid::OFFSET, startP, true);
            setAndStyleProperty(vs, Pid::OFFSET2, endP, true);
        }

        for (auto [linkedStaffIdx, linkedMusxStaffId] : links) {
            /// @todo improved handling for bottom system objects
            Volta* copy = toVolta(volta->clone());
            copy->setStaffIdx(linkedStaffIdx);

            if (importCustomPositions()) {
                copy->fixupSegments(1, [copy](System* parent) { return copy->createLineSegment(parent); });
                VoltaSegment* linkedVs = toVoltaSegment(volta->frontSegment());
                linkedVs->setSystem(measure->system());

                const auto indiv = endingBegin->getIndividualPositioning(linkedMusxStaffId);
                const auto textindiv = endingBegin->getTextIndividualPositioning(linkedMusxStaffId);
                if (endingBegin->individualPlacement && indiv && textindiv) {
                    copy->setVisible(!indiv->hidden);
                    double linkedStartHook = doubleFromEvpu(beginHookLen - indiv->y1add + indiv->y2add);
                    // MuseScore doesn't (yet?) allow for independent staff hook heights
                    // double linkedEndHook = doubleFromEvpu(endHookLen - textindiv->y2add + indiv->y2add);
                    PointF linkedStartP = evpuToPointF(leftInset + indiv->x1add, startY - indiv->y2add) * copy->spatium();
                    PointF linkedEndP = evpuToPointF(-rightInset + indiv->x2add - linkedStartP.x(), 0.0) * copy->spatium();
                    PointF linkedTextP = evpuToPointF(textPosX - 24 + textindiv->x1add, -textindiv->y1add) * copy->spatium();
                    linkedTextP.ry() += linkedStartHook * copy->spatium();

                    // copy->setEndHookHeight(Spatium(linkedEndHook));
                    setAndStyleProperty(linkedVs, Pid::OFFSET, linkedStartP, true);
                    setAndStyleProperty(copy, Pid::BEGIN_TEXT_OFFSET, linkedTextP, true);
                    setAndStyleProperty(copy, Pid::CONTINUE_TEXT_OFFSET, linkedTextP, true);
                    setAndStyleProperty(linkedVs, Pid::OFFSET2, linkedEndP, true);
                }
            }
            copy->linkTo(volta);
            measure->add(copy);
            m_systemObjectStaves.insert(linkedStaffIdx);
        }
        openings.push_back(volta);
    }

    // Read the back endings and inherit existing volta if possible
    for (const MusxInstance<others::RepeatBack>& endingEnd : endingEnds) {
        const auto musxMeasure = m_doc->getOthers()->get<others::Measure>(m_currentMusxPartId, endingEnd->getCmper());
        if (!musxMeasure || !musxMeasure->hasEnding) {
            continue;
        }
        // Find staff
        std::vector<std::pair<staff_idx_t, StaffCmper> > links;
        staff_idx_t curStaffIdx = staffIdxForRepeats(endingEnd->topStaffOnly, endingEnd->staffList, endingEnd->getCmper(), links);

        if (curStaffIdx == muse::nidx) {
            logger()->logWarning(String(u"Add voltas: Musx inst value not found."));
            continue;
        }

        // Find location in measure
        Fraction mTick = muse::value(m_meas2Tick, endingEnd->getCmper(), Fraction(-1, 1));
        Measure* measure = !mTick.negative() ? m_score->tick2measure(mTick) : nullptr;
        if (!measure) {
            continue;
        }

        Volta* prev = nullptr;
        Volta* cur = nullptr;
        while (!openings.empty() && openings.front()->tick() <= measure->tick()) {
            prev = muse::takeFirst(openings);
        }
        // Don't use inheriting behaviour if voltas cross repeats
        if (prev) {
            for (Measure* m = m_score->tick2measure(prev->tick()); m->tick() < mTick; m = m->nextMeasure()) {
                if ((m->repeatStart() && m->tick() != prev->tick()) || m->repeatEnd()) {
                    prev = nullptr;
                    break;
                }
            }
        }
        if (prev && prev->tick() == measure->tick()) {
            // Borrow existing volta
            if (prev->staffIdx() == curStaffIdx) {
                cur = prev;
            } else if ((cur = toVolta(prev->findLinkedInStaff(m_score->staff(curStaffIdx))))) {
            } else {
                cur = toVolta(prev->clone());
                cur->setStaffIdx(curStaffIdx);
                cur->linkTo(prev);
                measure->add(cur);
                m_systemObjectStaves.insert(curStaffIdx);
            }
            if (cur->voltaType() == Volta::Type::OPEN) {
                cur->setEndHookHeight(0.0_sp);
            }
        } else {
            /// @todo merge adjacent where possible
            track_idx_t curTrackIdx = staff2track(curStaffIdx);
            logger()->logInfo(String(u"Creating a volta at tick %1 on track %2.").arg(measure->tick().toString(),
                                                                                      String::number(curTrackIdx)));

            cur = Factory::createVolta(m_score->dummy());
            cur->setTrack(curTrackIdx);
            cur->setTick(measure->tick());
            cur->setTick2(measure->endTick());
            cur->setVisible(!endingEnd->hidden);
            cur->setBeginHookHeight(0.0_sp);
            cur->setEndHookHeight(0.0_sp);
            cur->setAutoplace(false);
            cur->setText(String());
            m_score->addElement(cur);
            m_systemObjectStaves.insert(curStaffIdx);
        }

        auto voltaCompare = [endingEnd](double current, double possible) {
            // Inherit hook/position values if they are more extreme than
            // existing ones, and only if this ending is visible
            return !endingEnd->hidden && (std::abs(possible) > std::abs(current));
        };

        if (importCustomPositions()) {
            cur->fixupSegments(1, [cur](System* parent) { return cur->createLineSegment(parent); });
            VoltaSegment* vs = toVoltaSegment(cur->frontSegment());
            vs->setSystem(measure->system());

            // There is no start hook or text for repeat back
            /// @todo verify these calculations
            double endHook = doubleFromEvpu(beginHookLen - endingEnd->leftVPos + endingEnd->rightVPos);
            PointF startP = evpuToPointF(leftInset + endingEnd->leftHPos, startY - endingEnd->rightVPos) * cur->spatium();
            PointF endP = evpuToPointF(-rightInset + endingEnd->rightHPos - startP.x(), 0.0) * cur->spatium();

            if (voltaCompare(cur->endHookHeight().val(), endHook * cur->spatium())) {
                cur->setEndHookHeight(Spatium(endHook));
            }
            /// @todo rebase text offset
            if (!voltaCompare(vs->offset().x(), startP.x())) {
                endP.rx() += startP.x() - vs->offset().x();
                startP.rx() = vs->offset().x();
            }
            if (!voltaCompare(vs->offset().y(), startP.y())) {
                startP.ry() = vs->offset().y();
            }
            if (!voltaCompare(vs->userOff2().x(), endP.x())) {
                endP.rx() = vs->userOff2().x();
            }
            if (muse::RealIsEqual(cur->endHookHeight().val(), 0.0)) {
                cur->setEndHookHeight(cur->beginHookHeight());
                cur->setVoltaType(Volta::Type::OPEN);
            } else {
                cur->setVoltaType(Volta::Type::CLOSED);
            }

            setAndStyleProperty(vs, Pid::OFFSET, startP, true);
            setAndStyleProperty(vs, Pid::OFFSET2, endP, true);
        }

        // Simulate playback (for most regular use cases)
        if (prev) {
            cur->setEndings(prev->endings());
            if (prev->tick2() < cur->tick()) {
                Volta* span = toVolta(prev->clone());
                span->setTick(prev->tick2());
                span->setTick2(cur->tick());
                span->setVisible(false);
                m_score->addElement(span);
            }
        }

        for (auto [linkedStaffIdx, linkedMusxStaffId] : links) {
            /// @todo improved handling for bottom system objects
            Volta* copy = toVolta(cur->findLinkedInStaff(m_score->staff(linkedStaffIdx)));
            if (!copy) {
                copy = toVolta(cur->clone());
                copy->setStaffIdx(linkedStaffIdx);
                copy->linkTo(cur);
                measure->add(copy);
                m_systemObjectStaves.insert(linkedStaffIdx);
            }

            if (!importCustomPositions()) {
                continue;
            }

            copy->fixupSegments(1, [copy](System* parent) { return copy->createLineSegment(parent); });
            VoltaSegment* linkedVs = toVoltaSegment(cur->frontSegment());
            linkedVs->setSystem(measure->system());

            const MusxInstance<others::RepeatIndividualPositioning> indiv = endingEnd->getIndividualPositioning(linkedMusxStaffId);
            if (endingEnd->individualPlacement && indiv) {
                copy->setVisible(!indiv->hidden);
                // MuseScore doesn't (yet?) allow for independent staff hook heights
                // double linkedEndHook = doubleFromEvpu(endHookLen - indiv->y1add + indiv->y2add);
                PointF linkedStartP = evpuToPointF(leftInset + indiv->x1add, startY - indiv->y2add) * copy->spatium();
                PointF linkedEndP = evpuToPointF(-rightInset + indiv->x2add - linkedStartP.x(), 0.0) * copy->spatium();

                // if (voltaCompare(copy->endHookHeight(), linkedEndHook * copy->spatium())) {
                //     copy->setEndHookHeight(Spatium(linkedEndHook));
                // }
                if (!voltaCompare(linkedVs->offset().x(), linkedStartP.x())) {
                    linkedEndP.rx() += linkedStartP.x() - linkedVs->offset().x();
                    linkedStartP.rx() = linkedVs->offset().x();
                }
                if (!voltaCompare(linkedVs->offset().y(), linkedStartP.y())) {
                    linkedStartP.ry() = linkedVs->offset().y();
                }
                if (!voltaCompare(linkedVs->userOff2().x(), linkedEndP.x())) {
                    linkedEndP.rx() = linkedVs->userOff2().x();
                }
                setAndStyleProperty(linkedVs, Pid::OFFSET, linkedStartP, true);
                setAndStyleProperty(linkedVs, Pid::OFFSET2, linkedEndP, true);
            }
        }
    }

    logger()->logInfo(String(u"Import smart shapes: Finished importing smart shapes"));
}
}
