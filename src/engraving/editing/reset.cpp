/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "reset.h"

#include <vector>

#include "global/containers.h"

#include "../dom/engravingitem.h"
#include "../dom/layoutbreak.h"
#include "../dom/measure.h"
#include "../dom/score.h"
#include "../dom/segment.h"
#include "../dom/select.h"
#include "../dom/spacer.h"

#include "editsystemlocks.h"

#include "log.h"

using namespace mu;
using namespace muse;
using namespace mu::engraving;

//---------------------------------------------------------
//    For use with Score::scanElements.
//    Reset positions and autoplacement for the given
//    element.
//---------------------------------------------------------

static void resetElementPosition(EngravingItem* e)
{
    if (e->generated()) {
        return;
    }

    e->undoResetProperty(Pid::AUTOPLACE);
    e->undoResetProperty(Pid::OFFSET);
    e->undoResetProperty(Pid::LEADING_SPACE);
    e->setOffsetChanged(false);
    if (e->isSpanner()) {
        e->undoResetProperty(Pid::OFFSET2);
    }
}

static void resetTextProperties(EngravingItem* e)
{
    if (e->generated() || !e->isTextBase()) {
        return;
    }

    static const std::vector<Pid> TEXT_STYLE_TO_RESET {
        Pid::FONT_FACE,
        Pid::FONT_SIZE,
        Pid::FONT_STYLE,
        Pid::SIZE_SPATIUM_DEPENDENT,
        Pid::FRAME_TYPE,
        Pid::TEXT_LINE_SPACING,
        Pid::FRAME_FG_COLOR,
        Pid::FRAME_BG_COLOR,
        Pid::FRAME_WIDTH,
        Pid::FRAME_PADDING,
        Pid::FRAME_ROUND,
        Pid::ALIGN
    };

    for (Pid pid : TEXT_STYLE_TO_RESET) {
        // TODO: use undoResetProperty: https://github.com/musescore/MuseScore/issues/16516
        // But for now, we'll use resetPropety since undoResetProperty leads to various problems
        e->resetProperty(pid);
    }
}

void Reset::resetToDefaultLayout(Transaction& tx, Score* score)
{
    TRACEFUNC;

    StyleIdSet dontResetTheseStyles {
        Sid::lyricsPlacement,
        Sid::repeatBarTips,
        Sid::startBarlineSingle,
        Sid::startBarlineMultiple,
        Sid::dividerLeft,
        Sid::dividerRightY,
        Sid::useStraightNoteFlags,
        Sid::mrNumberSeries,
        Sid::mrNumberEveryXMeasures,
        Sid::mrNumberSeriesWithParentheses,
        Sid::oneMeasureRepeatShow1,
        Sid::fourMeasureRepeatShowExtenders,
        Sid::useWideBeams,
        Sid::hairpinPlacement,
        Sid::pedalPlacement,
        Sid::trillPlacement,
        Sid::vibratoPlacement,
        Sid::harmonyPlacement,
        Sid::romanNumeralPlacement,
        Sid::nashvilleNumberPlacement,
        Sid::harmonyVoiceLiteral,
        Sid::harmonyVoicing,
        Sid::harmonyDuration,
        Sid::capoPosition,
        Sid::fretNumPos,
        Sid::fretPlacement,
        Sid::fretStrings,
        Sid::fretFrets,
        Sid::fretNut,
        Sid::fretOrientation,
        Sid::showPageNumber,
        Sid::showPageNumberOne,
        Sid::pageNumberOddEven,
        Sid::showMeasureNumber,
        Sid::showMeasureNumberOne,
        Sid::measureNumberInterval,
        Sid::measureNumberSystem,
        Sid::measureNumberPlacementMode,
        Sid::genClef,
        Sid::hideTabClefAfterFirst,
        Sid::genKeysig,
        Sid::genCourtesyTimesig,
        Sid::genCourtesyKeysig,
        Sid::genCourtesyClef,
        Sid::swingRatio,
        Sid::swingUnit,
        Sid::chordSymbolSpelling,
        Sid::automaticCapitalization,
        Sid::lowerCaseMinorChords,
        Sid::lowerCaseBassNotes,
        Sid::allCapsNoteNames,
        Sid::chordStyle,
        Sid::chordsXmlFile,
        Sid::chordDescriptionFile,
        Sid::concertPitch,
        Sid::createMultiMeasureRests,
        Sid::minEmptyMeasures,
        Sid::hideEmptyStaves,
        Sid::dontHideStavesInFirstSystem,
        Sid::enableIndentationOnFirstSystem,
        Sid::firstSystemIndentationValue,
        Sid::hideInstrumentNameIfOneInstrument,
        Sid::gateTime,
        Sid::tenutoGateTime,
        Sid::staccatoGateTime,
        Sid::slurGateTime,
        Sid::sectionPause,
        Sid::showHeader,
        Sid::headerFirstPage,
        Sid::headerOddEven,
        Sid::evenHeaderL,
        Sid::evenHeaderC,
        Sid::evenHeaderR,
        Sid::oddHeaderL,
        Sid::oddHeaderC,
        Sid::oddHeaderR,
        Sid::showFooter,
        Sid::footerFirstPage,
        Sid::footerOddEven,
        Sid::evenFooterL,
        Sid::evenFooterC,
        Sid::evenFooterR,
        Sid::oddFooterL,
        Sid::oddFooterC,
        Sid::oddFooterR,
        Sid::tupletOutOfStaff,
        Sid::tupletDirection,
        Sid::tupletBracketType,
        Sid::dynamicsPlacement,
        Sid::textLinePlacement,
        Sid::harpPedalDiagramPlacement,
        Sid::harpPedalTextDiagramPlacement,
        Sid::expressionPlacement,
        Sid::tupletNumberType,
        Sid::mmRestShowMeasureNumberRange,
        Sid::mmRestRangeBracketType,
        Sid::mmRestRangeVPlacement,
        Sid::staffTextPlacement,
        Sid::repeatLeftPlacement,
        Sid::repeatRightPlacement,
        Sid::instrumentChangePlacement,
        Sid::stickingPlacement,
        Sid::letRingPlacement,
        Sid::palmMutePlacement,
        Sid::pageWidth,
        Sid::pageHeight,
        Sid::pagePrintableWidth,
        Sid::pageEvenTopMargin,
        Sid::pageEvenBottomMargin,
        Sid::pageEvenLeftMargin,
        Sid::pageOddTopMargin,
        Sid::pageOddBottomMargin,
        Sid::pageOddLeftMargin,
        Sid::pageTwosided,
        Sid::spatium,
        Sid::concertPitch,
        Sid::createMultiMeasureRests
    };

    auto resetPositionAndTextProperties = [](EngravingItem* e) {
        resetElementPosition(e);
        resetTextProperties(e);
    };

    resetMeasuresLayout(tx, score);
    score->scanElements(resetPositionAndTextProperties);
    resetAllStyles(tx, score, dontResetTheseStyles);
    EditSystemLocks::undoRemoveAllLocks(tx, score);
}

void Reset::resetTextStyleOverrides(Score* score)
{
    TRACEFUNC;

    score->scanElements(resetTextProperties);
}

void Reset::resetAllStyles(Transaction&, Score* score, const StyleIdSet& exceptTheseOnes)
{
    TRACEFUNC;

    int beginIdx = int(Sid::NOSTYLE) + 1;
    int endIdx = int(Sid::STYLES);

    StyleIdSet stylesToReset;

    for (int idx = beginIdx; idx < endIdx; idx++) {
        Sid styleId = Sid(idx);
        if (!muse::contains(exceptTheseOnes, styleId)) {
            stylesToReset.insert(styleId);
        }
    }

    score->resetStyleValues(stylesToReset);
}

//---------------------------------------------------------
//   resetMeasuresLayout
//   Removes system/page breaks and spacers
//---------------------------------------------------------

void Reset::resetMeasuresLayout(Transaction&, Score* score)
{
    TRACEFUNC;

    std::vector<EngravingItem*> itemsToRemove;

    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (mb->isMeasure()) {
            mb->undoResetProperty(Pid::USER_STRETCH);

            for (const MStaff* staff : toMeasure(mb)->mstaves()) {
                if (Spacer* spacer = staff->vspacerDown()) {
                    itemsToRemove.push_back(spacer);
                }

                if (Spacer* spacer = staff->vspacerUp()) {
                    itemsToRemove.push_back(spacer);
                }
            }
        }

        if (!mb->lineBreak() && !mb->pageBreak()) {
            continue;
        }

        for (EngravingItem* item : mb->el()) {
            if (item->isLayoutBreak() && !toLayoutBreak(item)->isSectionBreak()) {
                itemsToRemove.push_back(item);
            }
        }
    }

    for (EngravingItem* item : itemsToRemove) {
        score->undoRemoveElement(item);
    }
}

void Reset::resetAllPositions(Transaction&, Score* score)
{
    TRACEFUNC;

    score->resetAutoplace();
}
