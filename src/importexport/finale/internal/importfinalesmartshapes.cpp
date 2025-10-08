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

#include <vector>
#include <exception>

#include "musx/musx.h"

#include "types/string.h"
#include "types/translatablestring.h"

#include "engraving/dom/anchors.h"
#include "engraving/dom/chordrest.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/note.h"
#include "engraving/dom/noteline.h"
#include "engraving/dom/ottava.h"
#include "engraving/dom/hairpin.h"
#include "engraving/dom/score.h"
#include "engraving/dom/slurtie.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/spanner.h"
#include "engraving/dom/textlinebase.h"
#include "engraving/dom/trill.h"
#include "engraving/dom/utils.h"
#include "engraving/dom/vibrato.h"

#include "engraving/types/typesconv.h"

#include "log.h"

using namespace mu::engraving;
using namespace muse;
using namespace musx::dom;

namespace mu::iex::finale {

static const std::map<std::string, ElementType> elementByRegexTable = {
    { R"(\bped(ale?)?\b)",                               ElementType::PEDAL },
    { R"(\b(((de)?cresc)|(dim))\.?\b)",                  ElementType::HAIRPIN },
    { R"(\b((rit(\.|ardando)?)|(rall(\.|entando)?))\b)", ElementType::GRADUAL_TEMPO_CHANGE },
    { R"(\blet ring\b)",                                 ElementType::LET_RING },
    { R"(\b(?:(?:8v)|(?:(?:15|22)m))(a|b)\b)",           ElementType::OTTAVA },
    { R"(\bw(?:\/|(?:hammy ))bar\b)",                    ElementType::WHAMMY_BAR },
    { R"(\brasg(?:ueado)?\b)",                           ElementType::RASGUEADO },
    { R"(\bp(?:\.|ick) ?s(?:\.\B|crape\b))",             ElementType::PICK_SCRAPE },
    { R"(\bp(?:\.|alm) ?m(?:\.\B|ute\b))",               ElementType::PALM_MUTE },
};

ReadableCustomLine::ReadableCustomLine(const FinaleParser& context, const MusxInstance<musx::dom::others::SmartShapeCustomLine>& customLine)
{
    beginText       = context.stringFromEnigmaText(customLine->getLeftStartRawTextCtx(context.currentMusxPartId()));
    continueText    = context.stringFromEnigmaText(customLine->getLeftContRawTextCtx(context.currentMusxPartId()));
    endText         = context.stringFromEnigmaText(customLine->getRightEndRawTextCtx(context.currentMusxPartId()));
    centerLongText  = context.stringFromEnigmaText(customLine->getCenterFullRawTextCtx(context.currentMusxPartId()));
    centerShortText = context.stringFromEnigmaText(customLine->getCenterAbbrRawTextCtx(context.currentMusxPartId()));

    SymId lineSym = SymId::noSym;

    switch (customLine->lineStyle) {
    case others::SmartShapeCustomLine::LineStyle::Char:
        lineSym = FinaleTextConv::symIdFromFinaleChar(customLine->charParams->lineChar, customLine->charParams->font);
        lineVisible = customLine->charParams->lineChar != U' ' && lineSym != SymId::space; /// @todo general space symbols
        break;
    case others::SmartShapeCustomLine::LineStyle::Solid:
        lineStyle   = LineType::SOLID;
        lineVisible = customLine->solidParams->lineWidth != 0;
        lineWidth   = Spatium(doubleFromEfix(customLine->solidParams->lineWidth));
        break;
    case others::SmartShapeCustomLine::LineStyle::Dashed:
        lineStyle   = LineType::DASHED; /// @todo When should we set lineStyle to LineType::DOTTED ?
        lineVisible = customLine->dashedParams->lineWidth != 0;
        lineWidth   = Spatium(doubleFromEfix(customLine->dashedParams->lineWidth)); /// @todo de-apply staff scaling later
        dashLineLen = doubleFromEfix(customLine->dashedParams->dashOn) / lineWidth.val();
        dashGapLen  = doubleFromEfix(customLine->dashedParams->dashOff) / lineWidth.val();
        break;
    }

    elementType = [&]() {
        if (customLine->lineStyle == others::SmartShapeCustomLine::LineStyle::Char && lineVisible) {
            switch (lineSym) {
                // Trills
                case SymId::wiggleTrill:
                    // Also used for glissandos, but those are not possible as custom lines
                    trillType = TrillType::TRILL_LINE;
                    return ElementType::TRILL;
                case SymId::ornamentZigZagLineNoRightEnd:
                case SymId::ornamentZigZagLineWithRightEnd:
                    /// @todo detect prall type
                    trillType = TrillType::UPPRALL_LINE;
                    return ElementType::TRILL;

                // Vibratos
                case SymId::guitarVibratoStroke:
                case SymId::guitarWideVibratoStroke:
                case SymId::wiggleSawtooth:
                case SymId::wiggleSawtoothWide:
                    vibratoType = vibratoTypeFromSymId(lineSym);
                    return ElementType::VIBRATO;

                default: break;
            }
            /// @todo TremoloBar?
        }

        for (auto [regexStr, elementType] : elementByRegexTable) {
            static const std::regex regex(regexStr, std::regex_constants::icase);
            if (std::regex_search(beginText.toStdString(), regex) || std::regex_search(continueText.toStdString(), regex)) {
                return elementType;
            }
        }
        if (endText == u"*" /*maestro symbol for pedal star*/) {
            return ElementType::PEDAL;
        }
        // MusxInstanceList<texts::SmartShapeText> customLineTexts = m_doc->getTexts()->getArray<texts::SmartShapeText>();
        /// @todo lines with up hooks below piano staves as pedal, detect other pedal types (sostenuto)
        /// @todo use symbols to detect line types before resorting to plaintext regex searches
        /// @todo regex search with font style/size/face tags removed
        /// Not detected / needed: VOLTA, SLUR, HAMMER_ON_PULL_OFF, GLISSANDO, NOTELINE, HARMONIC_MARK, GUITAR_BEND
        return ElementType::TEXTLINE;
    }();

    beginHookType = customLine->lineCapStartType == others::SmartShapeCustomLine::LineCapType::Hook ? HookType::HOOK_90 : HookType::NONE;
    endHookType   = customLine->lineCapEndType == others::SmartShapeCustomLine::LineCapType::Hook ? HookType::HOOK_90 : HookType::NONE;
    beginHookHeight = Spatium(doubleFromEfix(customLine->lineCapStartHookLength));
    endHookHeight   = Spatium(doubleFromEfix(customLine->lineCapEndHookLength));
    gapBetweenTextAndLine = Spatium(doubleFromEvpu(customLine->lineStartX)); // Don't use lineEndX or lineContX
    textSizeSpatiumDependent = true; /// ???
    diagonal = !customLine->makeHorz;

    beginTextPlace    = customLine->lineAfterLeftStartText ? TextPlace::LEFT : TextPlace::BELOW;
    continueTextPlace = customLine->lineAfterLeftContText ? TextPlace::LEFT : TextPlace::BELOW;
    // In MuseScore, this value has no effect. End text always uses left placement on layout.
    // endTextPlace   = customLine->lineBeforeRightEndText ? TextPlace::LEFT : TextPlace::BELOW;

    // Finale's vertical line position is set relative to the text baseline.
    // Horizontal alignment affects the (visible) offset, so use left placement and set the offset later.
    beginTextAlign    = Align(AlignH::LEFT, AlignV::BASELINE);
    continueTextAlign = Align(AlignH::LEFT, AlignV::BASELINE);
    endTextAlign      = Align(AlignH::LEFT, AlignV::BASELINE);
    // As the name suggests, this text needs to be centered.
    centerLongTextAlign  = AlignH::HCENTER;
    centerShortTextAlign = AlignH::HCENTER;

    // The following are currently saved directly to the
    // text String and not treated as properties. This will eventually be changed.
    // beginFontFamily;
    // continueFontFamily;
    // endFontFamily;
    // centerLongFontFamily;
    // centerShortFontFamily;
    // beginFontSize;
    // continueFontSize;
    // endFontSize;
    // centerLongFontSize;
    // centerShortFontSize;
    // beginFontStyle;
    // continueFontStyle;
    // endFontStyle;
    // centerLongFontStyle;
    // centerShortFontStyle;

    /// @todo I'm not yet sure how text offset affects the default offset/alignment of lines when added to the score.
    /// This may need to be accounted for in spanner segment positioning.
    beginTextOffset    = evpuToPointF(customLine->leftStartX, customLine->lineStartY - customLine->leftStartY);
    continueTextOffset = evpuToPointF(customLine->leftContX, customLine->lineStartY - customLine->leftContY);
    endTextOffset      = evpuToPointF(customLine->rightEndX, customLine->lineEndY - customLine->rightEndY);
    centerLongTextOffset  = evpuToPointF(customLine->centerFullX, customLine->lineStartY - customLine->centerFullY);
    centerShortTextOffset = evpuToPointF(customLine->centerAbbrX, customLine->lineStartY - customLine->centerAbbrY);
}

static bool elementsValidForSpannerType(const ElementType type, const EngravingItem* startElement, const EngravingItem* endElement)
{
    switch (type) {
    case ElementType::GLISSANDO:
    case ElementType::GUITAR_BEND:
    case ElementType::NOTELINE:
        return startElement && startElement->isNote() && endElement && endElement->isNote();
    case ElementType::SLUR:
        return startElement && startElement->isChordRest() && endElement && endElement->isChordRest();
    case ElementType::OTTAVA:
        return startElement && startElement->isChordRest() && (!endElement || endElement->isChordRest()); // the end may be the end of the piece.
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
    auto tickFromTerminationSeg = [&](ElementType type, const MusxInstance<others::SmartShape>& smartShape, EngravingItem*& e, bool start) -> Fraction {
        logger()->logInfo(String(u"Finding spanner element..."));
        bool findExactEntry = type != ElementType::OTTAVA && type != ElementType::SLUR;
        bool useNextCr = !start && type == ElementType::OTTAVA;
        const MusxInstance<others::SmartShape::TerminationSeg>& termSeg = start ? smartShape->startTermSeg : smartShape->endTermSeg;
        EntryInfoPtr entryInfoPtr = termSeg->endPoint->calcAssociatedEntry(m_currentMusxPartId, findExactEntry);
        if (entryInfoPtr) {
            NoteNumber nn = start ? smartShape->startNoteId : smartShape->endNoteId;
            if (nn) {
                e = toEngravingItem(noteFromEntryInfoAndNumber(entryInfoPtr, nn));
            } else {
                ChordRest* cr = chordRestFromEntryInfoPtr(entryInfoPtr);
                if (useNextCr) {
                    if (Segment* nextSeg = cr->nextSegmentAfterCR(SegmentType::ChordRest)) {
                        cr = nextSeg->nextChordRest(cr->track());
                    }
                }
                e = toEngravingItem(cr);
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
        if (useNextCr && entryInfoPtr) {
            rTick += musxFractionToFraction(entryInfoPtr.calcGlobalActualDuration());
        }
        // logger()->logInfo(String(u"Created TimeTickAnchor"));
        return mTick + rTick; //toEngravingItem(measure->getChordRestOrTimeTickSegment(tick)); // Do we have to create a segment, or will layout handle this for us?
    };

    /// @note Getting the entire array of smart shapes works for SCORE_PARTID, but if we ever need to do it for excerpts it could fail.
    /// This is because `getArray` currently cannot pull a mix of score and partially shared part instances. Adding the ability to do so
    /// would require significant refactoring of musx. -- RGP
    MusxInstanceList<others::SmartShape> smartShapes = m_doc->getOthers()->getArray<others::SmartShape>(m_currentMusxPartId); //, BASE_SYSTEM_ID
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
        ReadableCustomLine* customLine = [&]() -> ReadableCustomLine* {
            if (smartShape->lineStyleId == 0) {
                // Shape does not use custom line
                return nullptr;
            }
            // Search our converted shape library, or if not found add to it
            ReadableCustomLine* line = muse::value(m_customLines, smartShape->lineStyleId, nullptr);
            if (!line) {
                line = new ReadableCustomLine(*this, m_doc->getOthers()->get<others::SmartShapeCustomLine>(m_currentMusxPartId, smartShape->lineStyleId));
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
            logger()->logInfo(String(u"Cannot create spanner of %1 type with given start/end elements. Start: %2, end: %3").arg(TConv::userName(type).translated(), startTick.toString(), endTick.toString()));
            continue;
        }

        // Create spanner
        logger()->logInfo(String(u"Creating spanner of %1 type").arg(TConv::userName(type).translated()));
        Spanner* newSpanner = toSpanner(Factory::createItem(type, m_score->dummy()));
        newSpanner->setScore(m_score);
        newSpanner->styleChanged();

        if (smartShape->entryBased) {
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
        /// @todo guitar bends and their type
        newSpanner->setVisible(!smartShape->hidden);

        if (customLine) {
            if (newSpanner->isTextLineBase()) {
                TextLineBase* textLineBase = toTextLineBase(newSpanner);

                textLineBase->setLineVisible(customLine->lineVisible);
                textLineBase->setBeginHookType(customLine->beginHookType);
                textLineBase->setEndHookType(customLine->endHookType);
                textLineBase->setBeginHookHeight(customLine->beginHookHeight);
                textLineBase->setEndHookHeight(customLine->endHookHeight);
                textLineBase->setGapBetweenTextAndLine(customLine->gapBetweenTextAndLine);
                textLineBase->setTextSizeSpatiumDependent(customLine->textSizeSpatiumDependent);
                textLineBase->setDiagonal(customLine->diagonal);
                textLineBase->setLineWidth(customLine->lineWidth);
                textLineBase->setDashLineLen(customLine->dashLineLen);
                textLineBase->setDashGapLen(customLine->dashGapLen);

                textLineBase->setBeginTextPlace(customLine->beginTextPlace);
                textLineBase->setBeginText(customLine->beginText);
                textLineBase->setBeginTextAlign(customLine->beginTextAlign);
                // textLineBase->setBeginFontFamily(customLine->beginFontFamily);
                // textLineBase->setBeginFontSize(customLine->beginFontSize);
                // textLineBase->setBeginFontStyle(customLine->beginFontStyle);
                textLineBase->setBeginTextOffset(customLine->beginTextOffset);

                textLineBase->setContinueTextPlace(customLine->continueTextPlace);
                textLineBase->setContinueText(customLine->continueText);
                textLineBase->setContinueTextAlign(customLine->continueTextAlign);
                // textLineBase->setContinueFontFamily(customLine->continueFontFamily);
                // textLineBase->setContinueFontSize(customLine->continueFontSize);
                // textLineBase->setContinueFontStyle(customLine->continueFontStyle);
                textLineBase->setContinueTextOffset(customLine->continueTextOffset);

                textLineBase->setEndTextPlace(customLine->endTextPlace);
                textLineBase->setEndText(customLine->endText);
                textLineBase->setEndTextAlign(customLine->endTextAlign);
                // textLineBase->setEndFontFamily(customLine->endFontFamily);
                // textLineBase->setEndFontSize(customLine->endFontSize);
                // textLineBase->setEndFontStyle(customLine->endFontStyle);
                textLineBase->setEndTextOffset(customLine->endTextOffset);
            } else if (newSpanner->isTrill()) {
                toTrill(newSpanner)->setTrillType(customLine->trillType);
            } else if (newSpanner->isVibrato()) {
                toVibrato(newSpanner)->setVibratoType(customLine->vibratoType);
            }
        } else {
            if (type == ElementType::OTTAVA) {
                toOttava(newSpanner)->setOttavaType(ottavaTypeFromShapeType(smartShape->shapeType));
            } else if (type == ElementType::HAIRPIN) {
                toHairpin(newSpanner)->setHairpinType(hairpinTypeFromShapeType(smartShape->shapeType));
            } else if (type == ElementType::SLUR) {
                toSlur(newSpanner)->setStyleType(slurStyleTypeFromShapeType(smartShape->shapeType));
                /// @todo is there a way to read the calculated direction
                toSlur(newSpanner)->setSlurDirection(directionVFromShapeType(smartShape->shapeType));
            } else if (type == ElementType::TEXTLINE) {
                TextLineBase* textLine = toTextLineBase(newSpanner);
                textLine->setLineStyle(lineTypeFromShapeType(smartShape->shapeType));
                /// @todo read more settings from smartshape options, set styles for more elements
                auto [beginHook, endHook] = hookHeightsFromShapeType(smartShape->shapeType);
                if (beginHook) {
                    textLine->setBeginHookType(HookType::HOOK_90);
                    textLine->setBeginHookHeight(Spatium(beginHook * doubleFromEvpu(musxOptions().smartShapeOptions->hookLength)));
                    // continue doesn't have no hook
                }
                if (endHook) {
                    textLine->setEndHookType(HookType::HOOK_90);
                    textLine->setEndHookHeight(Spatium(endHook * doubleFromEvpu(musxOptions().smartShapeOptions->hookLength)));
                }
            }
        }

        /// Not needed? segments don't exist yet
        // for (auto ss : newSpanner->spannerSegments()) {
            // ss->setTrack(newSpanner->track());
        // }

        // if (isMeasureAnchor) {
            // Measure* endMeasure = tick2measureMM(tick2);
            // if (endMeasure->tick() != tick2) {
                // tick2 = endMeasure->endTick();
            // }
        // }
        // if (newSpanner->hasVoiceAssignmentProperties()) {
            // newSpanner->setInitialTrackAndVoiceAssignment(newSpanner->track(), false);
        // }

        if (newSpanner->anchor() == Spanner::Anchor::NOTE) {
            toNote(startElement)->add(newSpanner);
            logger()->logInfo(String(u"Added spanner of %1 type to note at tick %2, end: %3").arg(TConv::userName(type).translated(), startTick.toString(), endTick.toString()));
        } else {
            m_score->addSpanner(newSpanner);
            logger()->logInfo(String(u"Added spanner of %1 type at tick %2, end: %3").arg(TConv::userName(type).translated(), startTick.toString(), endTick.toString()));
        }
    }
    logger()->logInfo(String(u"Import smart shapes: Finished importing smart shapes"));
}

}
