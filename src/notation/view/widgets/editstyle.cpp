//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "editstyle.h"

#include <QButtonGroup>
#include <QSignalMapper>

#include "alignSelect.h"
#include "colorlabel.h"
#include "fontStyleSelect.h"
#include "framework/global/widgetstatestore.h"
#include "libmscore/figuredbass.h"
#include "libmscore/layout.h"
#include "libmscore/sym.h"
#include "log.h"
#include "offsetSelect.h"
#include "translation.h"
#include "ui/view/iconcodes.h"

using namespace mu::notation;
using namespace mu::ui;

static const QChar GO_NEXT_ICON = iconCodeToChar(IconCode::Code::ARROW_RIGHT);
static const QChar GO_PREV_ICON = iconCodeToChar(IconCode::Code::ARROW_LEFT);
static const QChar OPEN_FILE_ICON = iconCodeToChar(IconCode::Code::OPEN_FILE);
static const QChar RESET_ICON = iconCodeToChar(IconCode::Code::REDO);

static const char* lineStyles[] = {
    QT_TRANSLATE_NOOP("notation", "Continuous"),
    QT_TRANSLATE_NOOP("notation", "Dashed"),
    QT_TRANSLATE_NOOP("notation", "Dotted"),
    QT_TRANSLATE_NOOP("notation", "Dash-dotted"),
    QT_TRANSLATE_NOOP("notation", "Dash-dot-dotted")
};

//---------------------------------------------------------
//   EditStyle
//---------------------------------------------------------

EditStyle::EditStyle(QWidget* parent)
    : QDialog(parent)
{
    setObjectName("EditStyle");
    setupUi(this);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setModal(true);

    buttonApplyToAllParts = buttonBox->addButton(tr("Apply to all Parts"), QDialogButtonBox::ApplyRole);
    buttonTogglePagelist->setText(GO_NEXT_ICON);

    // create button groups for every set of radio button widgets
    // use this group widgets in list styleWidgets
    // This works for groups which represent an int enumeration.

    QButtonGroup* fretNumGroup = new QButtonGroup(this);
    fretNumGroup->addButton(radioFretNumLeft, 0);
    fretNumGroup->addButton(radioFretNumRight, 1);

    QButtonGroup* ksng = new QButtonGroup(this);
    ksng->addButton(radioKeySigNatNone, int(Ms::KeySigNatural::NONE));
    ksng->addButton(radioKeySigNatBefore, int(Ms::KeySigNatural::BEFORE));
    ksng->addButton(radioKeySigNatAfter, int(Ms::KeySigNatural::AFTER));

    QButtonGroup* ctg = new QButtonGroup(this);
    ctg->addButton(clefTab1, int(Ms::ClefType::TAB));
    ctg->addButton(clefTab2, int(Ms::ClefType::TAB_SERIF));

    QButtonGroup* fbAlign = new QButtonGroup(this);
    fbAlign->addButton(radioFBTop, 0);
    fbAlign->addButton(radioFBBottom, 1);

    QButtonGroup* fbStyle = new QButtonGroup(this);
    fbStyle->addButton(radioFBModern, 0);
    fbStyle->addButton(radioFBHistoric, 1);

    styleWidgets = {
        //   idx                --- showPercent      --- widget          --- resetButton
        { StyleId::figuredBassAlignment,    false, fbAlign,                 0 },
        { StyleId::figuredBassStyle,        false, fbStyle,                 0 },
        { StyleId::figuredBassFontSize,     false, doubleSpinFBSize,        0 },
        { StyleId::figuredBassYOffset,      false, doubleSpinFBVertPos,     0 },
        { StyleId::figuredBassLineHeight,   true,  spinFBLineHeight,        0 },
        { StyleId::tabClef,                 false, ctg,                     0 },
        { StyleId::keySigNaturals,          false, ksng,                    0 },
        { StyleId::voltaLineStyle,          false, voltaLineStyle,          resetVoltaLineStyle },
        { StyleId::ottavaLineStyle,         false, ottavaLineStyle,         resetOttavaLineStyle },
        { StyleId::pedalLineStyle,          false, pedalLineStyle,          resetPedalLineStyle },

        { StyleId::staffUpperBorder,        false, staffUpperBorder,        resetStaffUpperBorder },
        { StyleId::staffLowerBorder,        false, staffLowerBorder,        resetStaffLowerBorder },
        { StyleId::staffDistance,           false, staffDistance,           resetStaffDistance },
        { StyleId::akkoladeDistance,        false, akkoladeDistance,        resetAkkoladeDistance },
        { StyleId::enableVerticalSpread,    false, enableVerticalSpread,    0 },
        { StyleId::minSystemDistance,       false, minSystemDistance,       resetMinSystemDistance },
        { StyleId::maxSystemDistance,       false, maxSystemDistance,       resetMaxSystemDistance },
        { StyleId::spreadSystem,            false, spreadSystem,            resetSpreadSystem },
        { StyleId::spreadSquareBracket,     false, spreadSquareBracket,     resetSpreadSquareBracket },
        { StyleId::spreadCurlyBracket,      false, spreadCurlyBracket,      resetSpreadCurlyBracket },
        { StyleId::minSystemSpread,         false, minSystemSpread,         resetMinSystemSpread },
        { StyleId::maxSystemSpread,         false, maxSystemSpread,         resetMaxSystemSpread },
        { StyleId::minStaffSpread,          false, minStaffSpread,          resetMinStaffSpread },
        { StyleId::maxStaffSpread,          false, maxStaffSpread,          resetMaxStaffSpread },
        { StyleId::maxAkkoladeDistance,     false, maxAkkoladeDistance,     resetMaxAkkoladeDistance },
        { StyleId::maxPageFillSpread,       false, maxPageFillSpread,       resetMaxPageFillSpread },

        { StyleId::lyricsPlacement,         false, lyricsPlacement,         resetLyricsPlacement },
        { StyleId::lyricsPosAbove,          false, lyricsPosAbove,          resetLyricsPosAbove },
        { StyleId::lyricsPosBelow,          false, lyricsPosBelow,          resetLyricsPosBelow },
        { StyleId::lyricsMinTopDistance,    false, lyricsMinTopDistance,    resetLyricsMinTopDistance },
        { StyleId::lyricsMinBottomDistance, false, lyricsMinBottomDistance, resetLyricsMinBottomDistance },
        { StyleId::lyricsMinDistance,       false, lyricsMinDistance,       resetLyricsMinDistance },
        { StyleId::lyricsLineHeight,        true,  lyricsLineHeight,        resetLyricsLineHeight },
        { StyleId::lyricsDashMinLength,     false, lyricsDashMinLength,     resetLyricsDashMinLength },
        { StyleId::lyricsDashMaxLength,     false, lyricsDashMaxLength,     resetLyricsDashMaxLength },
        { StyleId::lyricsDashMaxDistance,   false, lyricsDashMaxDistance,   resetLyricsDashMaxDistance },
        { StyleId::lyricsDashForce,         false, lyricsDashForce,         resetLyricsDashForce },
        { StyleId::lyricsAlignVerseNumber,  false, lyricsAlignVerseNumber,  resetLyricsAlignVerseNumber },
        { StyleId::lyricsLineThickness,     false, lyricsLineThickness,     resetLyricsLineThickness },
        { StyleId::lyricsMelismaPad,        false, lyricsMelismaPad,        resetLyricsMelismaPad },
        { StyleId::lyricsMelismaAlign,      false, lyricsMelismaAlign,      resetLyricsMelismaAlign },
        { StyleId::lyricsDashPad,           false, lyricsDashPad,           resetLyricsDashPad },
        { StyleId::lyricsDashLineThickness, false, lyricsDashLineThickness, resetLyricsDashLineThickness },
        { StyleId::lyricsDashYposRatio,     false, lyricsDashYposRatio,     resetLyricsDashYposRatio },

        { StyleId::systemFrameDistance,     false, systemFrameDistance,     resetSystemFrameDistance },
        { StyleId::frameSystemDistance,     false, frameSystemDistance,     resetFrameSystemDistance },
        { StyleId::minMeasureWidth,         false, minMeasureWidth_2,       resetMinMeasureWidth },
        { StyleId::measureSpacing,          false, measureSpacing,          resetMeasureSpacing },
        { StyleId::measureRepeatNumberPos,  false, measureRepeatNumberPos,  resetMeasureRepeatNumberPos },
        { StyleId::mrNumberSeries,          false, mrNumberSeries,          0 },
        { StyleId::mrNumberEveryXMeasures,  false, mrNumberEveryXMeasures,  resetMRNumberEveryXMeasures },
        { StyleId::mrNumberSeriesWithParentheses, false, mrNumberSeriesWithParentheses, resetMRNumberSeriesWithParentheses },
        { StyleId::oneMeasureRepeatShow1,   false, oneMeasureRepeatShow1,   resetOneMeasureRepeatShow1 },
        { StyleId::fourMeasureRepeatShowExtenders, false, fourMeasureRepeatShowExtenders, resetFourMeasureRepeatShowExtenders },

        { StyleId::barWidth,                false, barWidth,                resetBarWidth },
        { StyleId::endBarWidth,             false, endBarWidth,             resetEndBarWidth },
        { StyleId::endBarDistance,          false, endBarDistance,          resetEndBarDistance },
        { StyleId::doubleBarWidth,          false, doubleBarWidth,          resetDoubleBarWidth },
        { StyleId::doubleBarDistance,       false, doubleBarDistance,       resetDoubleBarDistance },
        { StyleId::repeatBarlineDotSeparation, false, repeatBarlineDotSeparation, resetRepeatBarlineDotSeparation },

        { StyleId::barGraceDistance,        false, barGraceDistance,        resetBarGraceDistance },
        { StyleId::chordExtensionMag,       false, extensionMag,            resetExtensionMag },
        { StyleId::chordExtensionAdjust,    false, extensionAdjust,         resetExtensionAdjust },
        { StyleId::chordModifierMag,        false, modifierMag,             resetModifierMag },
        { StyleId::chordModifierAdjust,     false, modifierAdjust,          resetModifierAdjust },
        { StyleId::useStandardNoteNames,    false, useStandardNoteNames,    0 },
        { StyleId::useGermanNoteNames,      false, useGermanNoteNames,      0 },
        { StyleId::useFullGermanNoteNames,  false, useFullGermanNoteNames,  0 },
        { StyleId::useSolfeggioNoteNames,   false, useSolfeggioNoteNames,   0 },
        { StyleId::useFrenchNoteNames,      false, useFrenchNoteNames,      0 },
        { StyleId::automaticCapitalization, false, automaticCapitalization, 0 },

        { StyleId::lowerCaseMinorChords,    false, lowerCaseMinorChords,    0 },

        { StyleId::lowerCaseBassNotes,      false, lowerCaseBassNotes,      0 },
        { StyleId::allCapsNoteNames,        false, allCapsNoteNames,        0 },
        { StyleId::concertPitch,            false, concertPitch,            0 },
        { StyleId::createMultiMeasureRests, false, multiMeasureRests,       0 },
        { StyleId::minEmptyMeasures,        false, minEmptyMeasures,        0 },
        { StyleId::minMMRestWidth,          false, minMeasureWidth,         resetMinMMRestWidth },
        { StyleId::mmRestNumberPos,         false, mmRestNumberPos,         resetMMRestNumberPos },
        { StyleId::mmRestNumberMaskHBar,    false, mmRestNumberMaskHBar,    resetMMRestNumberMaskHBar },
        { StyleId::mmRestHBarThickness,     false, mmRestHBarThickness,     resetMMRestHBarThickness },
        { StyleId::multiMeasureRestMargin,  false, multiMeasureRestMargin,  resetMultiMeasureRestMargin },
        { StyleId::mmRestHBarVStrokeThickness, false, mmRestHBarVStrokeThickness, resetMMRestHBarVStrokeThickness },
        { StyleId::mmRestHBarVStrokeHeight, false, mmRestHBarVStrokeHeight, resetMMRestHBarVStrokeHeight },
        { StyleId::oldStyleMultiMeasureRests, false, oldStyleMultiMeasureRests, 0 },
        { StyleId::mmRestOldStyleMaxMeasures, false, mmRestOldStyleMaxMeasures, resetMMRestOldStyleMaxMeasures },
        { StyleId::mmRestOldStyleSpacing,   false, mmRestOldStyleSpacing,   resetMMRestOldStyleSpacing },
        { StyleId::hideEmptyStaves,         false, hideEmptyStaves,         0 },
        { StyleId::dontHideStavesInFirstSystem, false, dontHideStavesInFirstSystem, 0 },
        { StyleId::enableIndentationOnFirstSystem, false, enableIndentationOnFirstSystem, 0 },
        { StyleId::firstSystemIndentationValue, false, indentationValue, resetFirstSystemIndentation },
        { StyleId::alwaysShowBracketsWhenEmptyStavesAreHidden, false, alwaysShowBrackets, 0 },
        { StyleId::hideInstrumentNameIfOneInstrument, false, hideInstrumentNameIfOneInstrument, 0 },
        { StyleId::accidentalNoteDistance,  false, accidentalNoteDistance,  0 },
        { StyleId::accidentalDistance,      false, accidentalDistance,      0 },
        { StyleId::bracketedAccidentalPadding, false, accidentalsBracketsBadding, resetAccidentalsBracketPadding },
        { StyleId::alignAccidentalsLeft,    false, accidentalsOctaveColumnsAlignLeft, resetAccidentalsOctaveColumnsAlignLeft },

        { StyleId::minNoteDistance,         false, minNoteDistance,         resetMinNoteDistance },
        { StyleId::barNoteDistance,         false, barNoteDistance,         resetBarNoteDistance },
        { StyleId::barAccidentalDistance,   false, barAccidentalDistance,   resetBarAccidentalDistance },
        { StyleId::noteBarDistance,         false, noteBarDistance,         resetNoteBarDistance },
        { StyleId::clefLeftMargin,          false, clefLeftMargin,          resetClefLeftMargin },
        { StyleId::keysigLeftMargin,        false, keysigLeftMargin,        resetKeysigLeftMargin },
        { StyleId::timesigLeftMargin,       false, timesigLeftMargin,       resetTimesigLeftMargin },
        { StyleId::midClefKeyRightMargin,   false, clefKeyRightMargin,      resetClefKeyRightMargin },
        { StyleId::clefKeyDistance,         false, clefKeyDistance,         resetClefKeyDistance },
        { StyleId::clefTimesigDistance,     false, clefTimesigDistance,     resetClefTimesigDistance },
        { StyleId::keyTimesigDistance,      false, keyTimesigDistance,      resetKeyTimesigDistance },
        { StyleId::keyBarlineDistance,      false, keyBarlineDistance,      resetKeyBarlineDistance },
        { StyleId::systemHeaderDistance,    false, systemHeaderDistance,    resetSystemHeaderDistance },
        { StyleId::systemHeaderTimeSigDistance, false, systemHeaderTimeSigDistance, resetSystemHeaderTimeSigDistance },

        { StyleId::clefBarlineDistance,     false, clefBarlineDistance,     resetClefBarlineDistance },
        { StyleId::timesigBarlineDistance,  false, timesigBarlineDistance,  resetTimesigBarlineDistance },
        { StyleId::staffLineWidth,          false, staffLineWidth,          resetStaffLineWidth },
        { StyleId::beamWidth,               false, beamWidth,               0 },
        { StyleId::beamMinLen,              false, beamMinLen,              0 },

        { StyleId::hairpinPlacement,        false, hairpinPlacement,        resetHairpinPlacement },
        { StyleId::hairpinPosAbove,         false, hairpinPosAbove,         resetHairpinPosAbove },
        { StyleId::hairpinPosBelow,         false, hairpinPosBelow,         resetHairpinPosBelow },
        { StyleId::hairpinLineWidth,        false, hairpinLineWidth,        resetHairpinLineWidth },
        { StyleId::hairpinHeight,           false, hairpinHeight,           resetHairpinHeight },
        { StyleId::hairpinContHeight,       false, hairpinContinueHeight,   resetHairpinContinueHeight },

        { StyleId::dotNoteDistance,         false, noteDotDistance,         0 },
        { StyleId::dotDotDistance,          false, dotDotDistance,          0 },
        { StyleId::stemWidth,               false, stemWidth,               0 },
        { StyleId::ledgerLineWidth,         false, ledgerLineWidth,         0 },
        { StyleId::ledgerLineLength,        false, ledgerLineLength,        0 },
        { StyleId::shortStemProgression,    false, shortStemProgression,    0 },
        { StyleId::shortestStem,            false, shortestStem,            0 },
        { StyleId::ArpeggioNoteDistance,    false, arpeggioNoteDistance,    0 },
        { StyleId::ArpeggioLineWidth,       false, arpeggioLineWidth,       0 },
        { StyleId::ArpeggioHookLen,         false, arpeggioHookLen,         0 },
        { StyleId::ArpeggioHiddenInStdIfTab,false, arpeggioHiddenInStdIfTab,0 },
        { StyleId::SlurEndWidth,            false, slurEndLineWidth,        resetSlurEndLineWidth },
        { StyleId::SlurMidWidth,            false, slurMidLineWidth,        resetSlurMidLineWidth },
        { StyleId::SlurDottedWidth,         false, slurDottedLineWidth,     resetSlurDottedLineWidth },
        { StyleId::SlurMinDistance,         false, slurMinDistance,         resetSlurMinDistance },
        { StyleId::MinTieLength,            false, minTieLength,            resetMinTieLength },
        { StyleId::bracketWidth,            false, bracketWidth,            0 },
        { StyleId::bracketDistance,         false, bracketDistance,         0 },
        { StyleId::akkoladeWidth,           false, akkoladeWidth,           0 },
        { StyleId::akkoladeBarDistance,     false, akkoladeBarDistance,     0 },
        { StyleId::dividerLeft,             false, dividerLeft,             0 },
        { StyleId::dividerLeftX,            false, dividerLeftX,            0 },
        { StyleId::dividerLeftY,            false, dividerLeftY,            0 },
        { StyleId::dividerRight,            false, dividerRight,            0 },
        { StyleId::dividerRightX,           false, dividerRightX,           0 },
        { StyleId::dividerRightY,           false, dividerRightY,           0 },
        { StyleId::propertyDistanceHead,    false, propertyDistanceHead,    resetPropertyDistanceHead },
        { StyleId::propertyDistanceStem,    false, propertyDistanceStem,    resetPropertyDistanceStem },
        { StyleId::propertyDistance,        false, propertyDistance,        resetPropertyDistance },
        { StyleId::voltaPosAbove,           false, voltaPosAbove,           resetVoltaPosAbove },
        { StyleId::voltaHook,               false, voltaHook,               resetVoltaHook },
        { StyleId::voltaLineWidth,          false, voltaLineWidth,          resetVoltaLineWidth },

        { StyleId::ottavaPosAbove,          false, ottavaPosAbove,          resetOttavaPosAbove },
        { StyleId::ottavaPosBelow,          false, ottavaPosBelow,          resetOttavaPosBelow },
        { StyleId::ottavaHookAbove,         false, ottavaHookAbove,         resetOttavaHookAbove },
        { StyleId::ottavaHookBelow,         false, ottavaHookBelow,         resetOttavaHookBelow },
        { StyleId::ottavaLineWidth,         false, ottavaLineWidth,         resetOttavaLineWidth },

        { StyleId::pedalPlacement,          false, pedalLinePlacement,      resetPedalLinePlacement },
        { StyleId::pedalPosAbove,           false, pedalLinePosAbove,       resetPedalLinePosAbove },
        { StyleId::pedalPosBelow,           false, pedalLinePosBelow,       resetPedalLinePosBelow },
        { StyleId::pedalLineWidth,          false, pedalLineWidth,          resetPedalLineWidth },

        { StyleId::trillPlacement,          false, trillLinePlacement,      resetTrillLinePlacement },
        { StyleId::trillPosAbove,           false, trillLinePosAbove,       resetTrillLinePosAbove },
        { StyleId::trillPosBelow,           false, trillLinePosBelow,       resetTrillLinePosBelow },

        { StyleId::vibratoPlacement,        false, vibratoLinePlacement,    resetVibratoLinePlacement },
        { StyleId::vibratoPosAbove,         false, vibratoLinePosAbove,     resetVibratoLinePosAbove },
        { StyleId::vibratoPosBelow,         false, vibratoLinePosBelow,     resetVibratoLinePosBelow },

        { StyleId::harmonyFretDist,         false, harmonyFretDist,         0 },
        { StyleId::minHarmonyDistance,      false, minHarmonyDistance,      0 },
        { StyleId::maxHarmonyBarDistance,   false, maxHarmonyBarDistance,   0 },
        { StyleId::maxChordShiftAbove,      false, maxChordShiftAbove,      resetMaxChordShiftAbove },
        { StyleId::maxChordShiftBelow,      false, maxChordShiftBelow,      resetMaxChordShiftBelow },
        { StyleId::harmonyPlay,             false, harmonyPlay,             0 },
        { StyleId::harmonyVoiceLiteral,     false, voicingSelectWidget->interpretBox, 0 },
        { StyleId::harmonyVoicing,          false, voicingSelectWidget->voicingBox, 0 },
        { StyleId::harmonyDuration,         false, voicingSelectWidget->durationBox, 0 },

        { StyleId::tupletVHeadDistance,     false, tupletVHeadDistance,     resetTupletVHeadDistance },
        { StyleId::tupletVStemDistance,     false, tupletVStemDistance,     resetTupletVStemDistance },
        { StyleId::tupletStemLeftDistance,  false, tupletStemLeftDistance,  resetTupletStemLeftDistance },
        { StyleId::tupletStemRightDistance, false, tupletStemRightDistance, resetTupletStemRightDistance },
        { StyleId::tupletNoteLeftDistance,  false, tupletNoteLeftDistance,  resetTupletNoteLeftDistance },
        { StyleId::tupletNoteRightDistance, false, tupletNoteRightDistance, resetTupletNoteRightDistance },
        { StyleId::tupletBracketWidth,      false, tupletBracketWidth,      resetTupletBracketWidth },
        { StyleId::tupletBracketHookHeight, false, tupletBracketHookHeight, resetTupletBracketHookHeight },
        { StyleId::tupletDirection,         false, tupletDirection,         resetTupletDirection },
        { StyleId::tupletNumberType,        false, tupletNumberType,        resetTupletNumberType },
        { StyleId::tupletBracketType,       false, tupletBracketType,       resetTupletBracketType },
        { StyleId::tupletMaxSlope,          false, tupletMaxSlope,          resetTupletMaxSlope },
        { StyleId::tupletOufOfStaff,        false, tupletOutOfStaff,        0 },

        { StyleId::repeatBarTips,            false, showRepeatBarTips,            resetShowRepeatBarTips },
        { StyleId::startBarlineSingle,       false, showStartBarlineSingle,       resetShowStartBarlineSingle },
        { StyleId::startBarlineMultiple,     false, showStartBarlineMultiple,     resetShowStartBarlineMultiple },
        { StyleId::dividerLeftSym,           false, dividerLeftSym,               0 },
        { StyleId::dividerRightSym,          false, dividerRightSym,              0 },

        { StyleId::showMeasureNumber,        false, showMeasureNumber,            0 },
        { StyleId::showMeasureNumberOne,     false, showFirstMeasureNumber,       0 },
        { StyleId::measureNumberInterval,    false, intervalMeasureNumber,        0 },
        { StyleId::measureNumberSystem,      false, showEverySystemMeasureNumber, 0 },
        { StyleId::measureNumberAllStaves,   false, showAllStavesMeasureNumber,   0 },
        { StyleId::measureNumberVPlacement,  false, measureNumberVPlacement,      resetMeasureNumberVPlacement },
        { StyleId::measureNumberHPlacement,  false, measureNumberHPlacement,      resetMeasureNumberHPlacement },
        { StyleId::measureNumberPosAbove,    false, measureNumberPosAbove,        resetMeasureNumberPosAbove },
        { StyleId::measureNumberPosBelow,    false, measureNumberPosBelow,        resetMeasureNumberPosBelow },

        { StyleId::mmRestShowMeasureNumberRange, false, mmRestShowMeasureNumberRange, 0 },
        { StyleId::mmRestRangeBracketType,   false, mmRestRangeBracketType,       resetMmRestRangeBracketType },
        { StyleId::mmRestRangeVPlacement,    false, mmRestRangeVPlacement,        resetMmRestRangeVPlacement },
        { StyleId::mmRestRangeHPlacement,    false, mmRestRangeHPlacement,        resetMmRestRangeHPlacement },
        { StyleId::mmRestRangePosAbove,      false, mmRestRangePosAbove,          resetMMRestRangePosAbove },
        { StyleId::mmRestRangePosBelow,      false, mmRestRangePosBelow,          resetMMRestRangePosBelow },

        { StyleId::beamDistance,             true,  beamDistance,                 0 },
        { StyleId::beamNoSlope,              false, beamNoSlope,                  0 },
        { StyleId::graceNoteMag,             true,  graceNoteSize,                resetGraceNoteSize },
        { StyleId::smallStaffMag,            true,  smallStaffSize,               resetSmallStaffSize },
        { StyleId::smallNoteMag,             true,  smallNoteSize,                resetSmallNoteSize },
        { StyleId::smallClefMag,             true,  smallClefSize,                resetSmallClefSize },
        { StyleId::lastSystemFillLimit,      true,  lastSystemFillThreshold,      resetLastSystemFillThreshold },
        { StyleId::genClef,                  false, genClef,                      0 },
        { StyleId::genKeysig,                false, genKeysig,                    0 },
        { StyleId::genCourtesyTimesig,       false, genCourtesyTimesig,           0 },
        { StyleId::genCourtesyKeysig,        false, genCourtesyKeysig,            0 },
        { StyleId::genCourtesyClef,          false, genCourtesyClef,              0 },
        { StyleId::swingRatio,               false, swingBox,                     0 },
        { StyleId::chordsXmlFile,            false, chordsXmlFile,                0 },
        { StyleId::dotMag,                   true,  dotMag,                       0 },
        { StyleId::articulationMag,          true,  articulationMag,              resetArticulationMag },
        { StyleId::shortenStem,              false, shortenStem,                  0 },
        { StyleId::showHeader,               false, showHeader,                   0 },
        { StyleId::headerFirstPage,          false, showHeaderFirstPage,          0 },
        { StyleId::headerOddEven,            false, headerOddEven,                0 },
        { StyleId::evenHeaderL,              false, evenHeaderL,                  0 },
        { StyleId::evenHeaderC,              false, evenHeaderC,                  0 },
        { StyleId::evenHeaderR,              false, evenHeaderR,                  0 },
        { StyleId::oddHeaderL,               false, oddHeaderL,                   0 },
        { StyleId::oddHeaderC,               false, oddHeaderC,                   0 },
        { StyleId::oddHeaderR,               false, oddHeaderR,                   0 },
        { StyleId::showFooter,               false, showFooter,                   0 },
        { StyleId::footerFirstPage,          false, showFooterFirstPage,          0 },
        { StyleId::footerOddEven,            false, footerOddEven,                0 },
        { StyleId::evenFooterL,              false, evenFooterL,                  0 },
        { StyleId::evenFooterC,              false, evenFooterC,                  0 },
        { StyleId::evenFooterR,              false, evenFooterR,                  0 },
        { StyleId::oddFooterL,               false, oddFooterL,                   0 },
        { StyleId::oddFooterC,               false, oddFooterC,                   0 },
        { StyleId::oddFooterR,               false, oddFooterR,                   0 },

        { StyleId::ottavaNumbersOnly,        false, ottavaNumbersOnly,            resetOttavaNumbersOnly },
        { StyleId::capoPosition,             false, capoPosition,                 0 },
        { StyleId::fretNumMag,               true,  fretNumMag,                   0 },
        { StyleId::fretNumPos,               false, fretNumGroup,                 0 },
        { StyleId::fretY,                    false, fretY,                        0 },
        { StyleId::barreLineWidth,           false, barreLineWidth,               0 },
        { StyleId::fretMag,                  false, fretMag,                      0 },
        { StyleId::fretDotSize,              false, fretDotSize,                  0 },
        { StyleId::fretStringSpacing,        false, fretStringSpacing,            0 },
        { StyleId::fretFretSpacing,          false, fretFretSpacing,              0 },
        { StyleId::maxFretShiftAbove,        false, maxFretShiftAbove,            resetMaxFretShiftAbove },
        { StyleId::maxFretShiftBelow,        false, maxFretShiftBelow,            resetMaxFretShiftBelow },
        { StyleId::scaleBarlines,            false, scaleBarlines,                resetScaleBarlines },
        { StyleId::crossMeasureValues,       false, crossMeasureValues,           0 },

        { StyleId::MusicalSymbolFont,        false, musicalSymbolFont,            0 },
        { StyleId::MusicalTextFont,          false, musicalTextFont,              0 },
        { StyleId::autoplaceHairpinDynamicsDistance, false, autoplaceHairpinDynamicsDistance,
          resetAutoplaceHairpinDynamicsDistance },

        { StyleId::dynamicsPlacement,       false, dynamicsPlacement,          resetDynamicsPlacement },
        { StyleId::dynamicsPosAbove,        false, dynamicsPosAbove,           resetDynamicsPosAbove },
        { StyleId::dynamicsPosBelow,        false, dynamicsPosBelow,           resetDynamicsPosBelow },
        { StyleId::dynamicsMinDistance,     false, dynamicsMinDistance,        resetDynamicsMinDistance },

        { StyleId::tempoPlacement,          false, tempoTextPlacement,          resetTempoTextPlacement },
        { StyleId::tempoPosAbove,           false, tempoTextPosAbove,           resetTempoTextPosAbove },
        { StyleId::tempoPosBelow,           false, tempoTextPosBelow,           resetTempoTextPosBelow },
        { StyleId::tempoMinDistance,        false, tempoTextMinDistance,        resetTempoTextMinDistance },

        { StyleId::rehearsalMarkPlacement,   false, rehearsalMarkPlacement,     resetRehearsalMarkPlacement },
        { StyleId::rehearsalMarkPosAbove,    false, rehearsalMarkPosAbove,      resetRehearsalMarkPosAbove },
        { StyleId::rehearsalMarkPosBelow,    false, rehearsalMarkPosBelow,      resetRehearsalMarkPosBelow },
        { StyleId::rehearsalMarkMinDistance, false, rehearsalMarkMinDistance,   resetRehearsalMarkMinDistance },

        { StyleId::autoplaceVerticalAlignRange, false, autoplaceVerticalAlignRange, resetAutoplaceVerticalAlignRange },
        { StyleId::minVerticalDistance,         false, minVerticalDistance,         resetMinVerticalDistance },
        { StyleId::textLinePlacement,           false, textLinePlacement,           resetTextLinePlacement },
        { StyleId::textLinePosAbove,            false, textLinePosAbove,            resetTextLinePosAbove },
        { StyleId::textLinePosBelow,            false, textLinePosBelow,            resetTextLinePosBelow },

        { StyleId::systemTextLinePlacement,     false, systemTextLinePlacement,     resetSystemTextLinePlacement },
        { StyleId::systemTextLinePosAbove,      false, systemTextLinePosAbove,      resetSystemTextLinePosAbove },
        { StyleId::systemTextLinePosBelow,      false, systemTextLinePosBelow,      resetSystemTextLinePosBelow },

        { StyleId::fermataPosAbove,         false, fermataPosAbove,       resetFermataPosAbove },
        { StyleId::fermataPosBelow,         false, fermataPosBelow,       resetFermataPosBelow },
        { StyleId::fermataMinDistance,      false, fermataMinDistance,    resetFermataMinDistance },

        { StyleId::staffTextPlacement,      false, staffTextPlacement,    resetStaffTextPlacement },
        { StyleId::staffTextPosAbove,       false, staffTextPosAbove,     resetStaffTextPosAbove },
        { StyleId::staffTextPosBelow,       false, staffTextPosBelow,     resetStaffTextPosBelow },
        { StyleId::staffTextMinDistance,    false, staffTextMinDistance,  resetStaffTextMinDistance },

        { StyleId::bendLineWidth,     false, bendLineWidth,     resetBendLineWidth },
        { StyleId::bendArrowWidth,    false, bendArrowWidth,    resetBendArrowWidth },
    };

    lineStyleComboBoxes = {
        voltaLineStyle,
        ottavaLineStyle,
        pedalLineStyle
    };
    for (QComboBox* cb : lineStyleComboBoxes) {
        cb->clear();
        int idx = 0;
        for (const char* p : lineStyles) {
            cb->addItem(qtrc("notation", p), idx);
            ++idx;
        }
    }

    verticalPlacementComboBoxes = {
        lyricsPlacement,
        textLinePlacement,
        hairpinPlacement,
        pedalLinePlacement,
        trillLinePlacement,
        vibratoLinePlacement,
        dynamicsPlacement,
        tempoTextPlacement,
        staffTextPlacement,
        rehearsalMarkPlacement,
        measureNumberVPlacement
    };
    for (QComboBox* cb : verticalPlacementComboBoxes) {
        cb->clear();
        cb->addItem(tr("Above"), int(Ms::Placement::ABOVE));
        cb->addItem(tr("Below"), int(Ms::Placement::BELOW));
    }

    horizontalPlacementComboBoxes = {
        measureNumberHPlacement,
        mmRestRangeHPlacement
    };
    for (QComboBox* cb : horizontalPlacementComboBoxes) {
        cb->clear();
        cb->addItem(tr("Left"),   int(Ms::HPlacement::LEFT));
        cb->addItem(tr("Center"), int(Ms::HPlacement::CENTER));
        cb->addItem(tr("Right"),  int(Ms::HPlacement::RIGHT));
    }

    mmRestRangeBracketType->clear();
    mmRestRangeBracketType->addItem(tr("None"),        int(Ms::MMRestRangeBracketType::NONE));
    mmRestRangeBracketType->addItem(tr("Brackets"),    int(Ms::MMRestRangeBracketType::BRACKETS));
    mmRestRangeBracketType->addItem(tr("Parentheses"), int(Ms::MMRestRangeBracketType::PARENTHESES));

    autoplaceVerticalAlignRange->clear();
    autoplaceVerticalAlignRange->addItem(tr("Segment"), int(Ms::VerticalAlignRange::SEGMENT));
    autoplaceVerticalAlignRange->addItem(tr("Measure"), int(Ms::VerticalAlignRange::MEASURE));
    autoplaceVerticalAlignRange->addItem(tr("System"),  int(Ms::VerticalAlignRange::SYSTEM));

    tupletNumberType->clear();
    tupletNumberType->addItem(tr("Number"), int(TupletNumberType::SHOW_NUMBER));
    tupletNumberType->addItem(tr("Ratio"), int(TupletNumberType::SHOW_RELATION));
    tupletNumberType->addItem(tr("None", "no tuplet number type"), int(TupletNumberType::NO_TEXT));

    tupletBracketType->clear();
    tupletBracketType->addItem(tr("Automatic"), int(TupletBracketType::AUTO_BRACKET));
    tupletBracketType->addItem(tr("Bracket"), int(TupletBracketType::SHOW_BRACKET));
    tupletBracketType->addItem(tr("None", "no tuplet bracket type"), int(TupletBracketType::SHOW_NO_BRACKET));

    pageList->setCurrentRow(0);
    accidentalsGroup->setVisible(false);   // disable, not yet implemented

    musicalSymbolFont->clear();
    int idx = 0;
    for (auto i : Ms::ScoreFont::scoreFonts()) {
        musicalSymbolFont->addItem(i.name(), i.name());
        ++idx;
    }

    static const Ms::SymId ids[] = {
        Ms::SymId::systemDivider, Ms::SymId::systemDividerLong, Ms::SymId::systemDividerExtraLong
    };
    for (Ms::SymId id : ids) {
        const QString& un = Ms::Sym::id2userName(id);
        const char* n  = Ms::Sym::id2name(id);
        dividerLeftSym->addItem(un,  QVariant(QString(n)));
        dividerRightSym->addItem(un, QVariant(QString(n)));
    }

    // figured bass init
    QList<QString> fbFontNames = Ms::FiguredBass::fontNames();
    for (const QString& family: fbFontNames) {
        comboFBFont->addItem(family);
    }
    comboFBFont->setCurrentIndex(0);
    connect(comboFBFont, SIGNAL(currentIndexChanged(int)), SLOT(on_comboFBFont_currentIndexChanged(int)));

    // chord symbol init
    harmonyPlay->setChecked(true);

    voicingSelectWidget->interpretBox->clear();
    voicingSelectWidget->interpretBox->addItem(tr("Jazz"), int(0));   // two-item combobox for boolean style variant
    voicingSelectWidget->interpretBox->addItem(tr("Literal"), int(1));   // true = literal

    voicingSelectWidget->voicingBox->clear();
    voicingSelectWidget->voicingBox->addItem(tr("Automatic"), int(Ms::Voicing::AUTO));
    voicingSelectWidget->voicingBox->addItem(tr("Root Only"), int(Ms::Voicing::ROOT_ONLY));
    voicingSelectWidget->voicingBox->addItem(tr("Close"), int(Ms::Voicing::CLOSE));
    voicingSelectWidget->voicingBox->addItem(tr("Drop Two"), int(Ms::Voicing::DROP_2));
    voicingSelectWidget->voicingBox->addItem(tr("Six Note"), int(Ms::Voicing::SIX_NOTE));
    voicingSelectWidget->voicingBox->addItem(tr("Four Note"), int(Ms::Voicing::FOUR_NOTE));
    voicingSelectWidget->voicingBox->addItem(tr("Three Note"), int(Ms::Voicing::THREE_NOTE));

    voicingSelectWidget->durationBox->clear();
    voicingSelectWidget->durationBox->addItem(tr("Until Next Chord Symbol"), int(Ms::HDuration::UNTIL_NEXT_CHORD_SYMBOL));
    voicingSelectWidget->durationBox->addItem(tr("Until End of Measure"), int(Ms::HDuration::STOP_AT_MEASURE_END));
    voicingSelectWidget->durationBox->addItem(tr("Chord/Rest Duration"), int(Ms::HDuration::SEGMENT_DURATION));

    setHeaderFooterToolTip();

    connect(buttonBox,             SIGNAL(clicked(QAbstractButton*)), SLOT(buttonClicked(QAbstractButton*)));
    connect(enableVerticalSpread,  SIGNAL(toggled(bool)),             SLOT(enableVerticalSpreadClicked(bool)));
    connect(disableVerticalSpread, SIGNAL(toggled(bool)),             SLOT(disableVerticalSpreadClicked(bool)));
    connect(headerOddEven,         SIGNAL(toggled(bool)),             SLOT(toggleHeaderOddEven(bool)));
    connect(footerOddEven,         SIGNAL(toggled(bool)),             SLOT(toggleFooterOddEven(bool)));
    connect(chordDescriptionFileButton, SIGNAL(clicked()),            SLOT(selectChordDescriptionFile()));
    connect(chordsStandard,        SIGNAL(toggled(bool)),             SLOT(setChordStyle(bool)));
    connect(chordsJazz,            SIGNAL(toggled(bool)),             SLOT(setChordStyle(bool)));
    connect(chordsCustom,          SIGNAL(toggled(bool)),             SLOT(setChordStyle(bool)));
    connect(chordsXmlFile,         SIGNAL(toggled(bool)),             SLOT(setChordStyle(bool)));
    connect(chordDescriptionFile,  &QLineEdit::editingFinished,       [=]() { setChordStyle(true); });

    chordDescriptionFileButton->setText(OPEN_FILE_ICON);

    connect(swingOff,            SIGNAL(toggled(bool)),             SLOT(setSwingParams(bool)));
    connect(swingEighth,         SIGNAL(toggled(bool)),             SLOT(setSwingParams(bool)));
    connect(swingSixteenth,      SIGNAL(toggled(bool)),             SLOT(setSwingParams(bool)));

    connect(concertPitch,        SIGNAL(toggled(bool)),             SLOT(concertPitchToggled(bool)));
    connect(lyricsDashMinLength, SIGNAL(valueChanged(double)),      SLOT(lyricsDashMinLengthValueChanged(double)));
    connect(lyricsDashMaxLength, SIGNAL(valueChanged(double)),      SLOT(lyricsDashMaxLengthValueChanged(double)));
    connect(minSystemDistance,   SIGNAL(valueChanged(double)),      SLOT(systemMinDistanceValueChanged(double)));
    connect(maxSystemDistance,   SIGNAL(valueChanged(double)),      SLOT(systemMaxDistanceValueChanged(double)));

    QSignalMapper* mapper  = new QSignalMapper(this);       // reset style signals
    QSignalMapper* mapper2 = new QSignalMapper(this);       // value change signals

    for (const StyleWidget& sw : styleWidgets) {
        const char* type = styleValue(sw.idx).typeName();

        if (!strcmp("Direction", type)) {
            QComboBox* cb = qobject_cast<QComboBox*>(sw.widget);
            Ms::fillComboBoxDirection(cb);
        }
        if (sw.reset) {
            sw.reset->setText(RESET_ICON);
            connect(sw.reset, SIGNAL(clicked()), mapper, SLOT(map()));
            mapper->setMapping(sw.reset, int(sw.idx));
        }
        if (qobject_cast<QSpinBox*>(sw.widget)) {
            connect(qobject_cast<QSpinBox*>(sw.widget), SIGNAL(valueChanged(int)), mapper2, SLOT(map()));
        } else if (qobject_cast<QDoubleSpinBox*>(sw.widget)) {
            connect(qobject_cast<QDoubleSpinBox*>(sw.widget), SIGNAL(valueChanged(double)), mapper2, SLOT(map()));
        } else if (qobject_cast<QFontComboBox*>(sw.widget)) {
            connect(qobject_cast<QFontComboBox*>(sw.widget), SIGNAL(currentFontChanged(const QFont&)), mapper2,
                    SLOT(map()));
        } else if (qobject_cast<QComboBox*>(sw.widget)) {
            connect(qobject_cast<QComboBox*>(sw.widget), SIGNAL(currentIndexChanged(int)), mapper2, SLOT(map()));
        } else if (qobject_cast<QRadioButton*>(sw.widget)) {
            connect(qobject_cast<QRadioButton*>(sw.widget), SIGNAL(toggled(bool)), mapper2, SLOT(map()));
        } else if (qobject_cast<QPushButton*>(sw.widget)) {
            connect(qobject_cast<QPushButton*>(sw.widget), SIGNAL(toggled(bool)), mapper2, SLOT(map()));
        } else if (qobject_cast<QToolButton*>(sw.widget)) {
            connect(qobject_cast<QToolButton*>(sw.widget), SIGNAL(toggled(bool)), mapper2, SLOT(map()));
        } else if (qobject_cast<QGroupBox*>(sw.widget)) {
            connect(qobject_cast<QGroupBox*>(sw.widget), SIGNAL(toggled(bool)), mapper2, SLOT(map()));
        } else if (qobject_cast<QCheckBox*>(sw.widget)) {
            connect(qobject_cast<QCheckBox*>(sw.widget), SIGNAL(stateChanged(int)), mapper2, SLOT(map()));
        } else if (qobject_cast<QTextEdit*>(sw.widget)) {
            connect(qobject_cast<QTextEdit*>(sw.widget), SIGNAL(textChanged()), mapper2, SLOT(map()));
        } else if (qobject_cast<QButtonGroup*>(sw.widget)) {
            connect(qobject_cast<QButtonGroup*>(sw.widget), SIGNAL(buttonClicked(int)), mapper2, SLOT(map()));
        } else if (qobject_cast<AlignSelect*>(sw.widget)) {
            connect(qobject_cast<AlignSelect*>(sw.widget), SIGNAL(alignChanged(Align)), mapper2, SLOT(map()));
        } else if (qobject_cast<OffsetSelect*>(sw.widget)) {
            connect(qobject_cast<OffsetSelect*>(sw.widget), SIGNAL(offsetChanged(const QPointF&)), mapper2,
                    SLOT(map()));
        } else if (FontStyleSelect* fontStyle = qobject_cast<FontStyleSelect*>(sw.widget)) {
            connect(fontStyle, &FontStyleSelect::fontStyleChanged, mapper2, QOverload<>::of(&QSignalMapper::map));
        }

        mapper2->setMapping(sw.widget, int(sw.idx));
    }

    int topBottomMargin = automaticCapitalization->rect().height() - configuration()->fontSize();
    topBottomMargin /= 2;
    topBottomMargin = topBottomMargin > 4 ? topBottomMargin - 4 : 0;
    automaticCapitalization->layout()->setContentsMargins(9, topBottomMargin, 9, topBottomMargin);

    connect(mapper,  SIGNAL(mapped(int)), SLOT(resetStyleValue(int)));
    connect(mapper2, SIGNAL(mapped(int)), SLOT(valueChanged(int)));
    textStyles->clear();
    for (auto ss : Ms::allTextStyles()) {
        QListWidgetItem* item = new QListWidgetItem(Ms::textStyleUserName(ss));
        item->setData(Qt::UserRole, int(ss));
        textStyles->addItem(item);
    }

    textStyleFrameType->clear();
    textStyleFrameType->addItem(tr("None", "no frame for text"), int(Ms::FrameType::NO_FRAME));
    textStyleFrameType->addItem(tr("Rectangle"), int(Ms::FrameType::SQUARE));
    textStyleFrameType->addItem(tr("Circle"), int(Ms::FrameType::CIRCLE));

    resetTextStyleName->setText(RESET_ICON);
    connect(resetTextStyleName, &QToolButton::clicked, [=]() { resetUserStyleName(); });
    connect(styleName, &QLineEdit::textEdited, [=]() { editUserStyleName(); });
    connect(styleName, &QLineEdit::editingFinished, [=]() { endEditUserStyleName(); });

    // font face
    resetTextStyleFontFace->setText(RESET_ICON);
    connect(resetTextStyleFontFace, &QToolButton::clicked,
            [=]() { resetTextStyle(Ms::Pid::FONT_FACE); }
            );
    connect(textStyleFontFace, &QFontComboBox::currentFontChanged,
            [=]() { textStyleValueChanged(Ms::Pid::FONT_FACE, QVariant(textStyleFontFace->currentFont().family())); }
            );

    // font size
    resetTextStyleFontSize->setText(RESET_ICON);
    connect(resetTextStyleFontSize, &QToolButton::clicked,
            [=]() { resetTextStyle(Ms::Pid::FONT_SIZE); }
            );
    connect(textStyleFontSize, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [=]() { textStyleValueChanged(Ms::Pid::FONT_SIZE, QVariant(textStyleFontSize->value())); }
            );

    // line spacing
    resetTextStyleLineSpacing->setText(RESET_ICON);
    connect(resetTextStyleLineSpacing, &QToolButton::clicked,
            [=]() { resetTextStyle(Ms::Pid::TEXT_LINE_SPACING); }
            );
    connect(textStyleLineSpacing, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [=]() { textStyleValueChanged(Ms::Pid::TEXT_LINE_SPACING, QVariant(textStyleLineSpacing->value())); }
            );

    // font style
    resetTextStyleFontStyle->setText(RESET_ICON);
    connect(resetTextStyleFontStyle, &QToolButton::clicked,
            [=]() { resetTextStyle(Ms::Pid::FONT_STYLE); }
            );
    connect(textStyleFontStyle, &notation::FontStyleSelect::fontStyleChanged,
            [=]() { textStyleValueChanged(Ms::Pid::FONT_STYLE, QVariant(int(textStyleFontStyle->fontStyle()))); }
            );

    // align
    resetTextStyleAlign->setText(RESET_ICON);
    connect(resetTextStyleAlign, &QToolButton::clicked, [=]() { resetTextStyle(Ms::Pid::ALIGN); });
    connect(textStyleAlign, &AlignSelect::alignChanged,
            [=]() { textStyleValueChanged(Ms::Pid::ALIGN, QVariant::fromValue(textStyleAlign->align())); }
            );

    // offset
    resetTextStyleOffset->setText(RESET_ICON);
    connect(resetTextStyleOffset, &QToolButton::clicked, [=]() { resetTextStyle(Ms::Pid::OFFSET); });
    connect(textStyleOffset, &OffsetSelect::offsetChanged,
            [=]() { textStyleValueChanged(Ms::Pid::OFFSET, QVariant(textStyleOffset->offset())); }
            );

    // spatium dependent
    resetTextStyleSpatiumDependent->setText(RESET_ICON);
    connect(resetTextStyleSpatiumDependent, &QToolButton::clicked, [=]() {
        resetTextStyle(Ms::Pid::SIZE_SPATIUM_DEPENDENT);
    });
    connect(textStyleSpatiumDependent, &QCheckBox::toggled,
            [=]() { textStyleValueChanged(Ms::Pid::SIZE_SPATIUM_DEPENDENT, textStyleSpatiumDependent->isChecked()); }
            );

    resetTextStyleFrameType->setText(RESET_ICON);
    connect(resetTextStyleFrameType, &QToolButton::clicked, [=]() { resetTextStyle(Ms::Pid::FRAME_TYPE); });
    connect(textStyleFrameType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [=]() { textStyleValueChanged(Ms::Pid::FRAME_TYPE, textStyleFrameType->currentIndex()); }
            );

    resetTextStyleFramePadding->setText(RESET_ICON);
    connect(resetTextStyleFramePadding, &QToolButton::clicked, [=]() { resetTextStyle(Ms::Pid::FRAME_PADDING); });
    connect(textStyleFramePadding, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [=]() { textStyleValueChanged(Ms::Pid::FRAME_PADDING, textStyleFramePadding->value()); }
            );

    resetTextStyleFrameBorder->setText(RESET_ICON);
    connect(resetTextStyleFrameBorder, &QToolButton::clicked, [=]() { resetTextStyle(Ms::Pid::FRAME_WIDTH); });
    connect(textStyleFrameBorder, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [=]() { textStyleValueChanged(Ms::Pid::FRAME_WIDTH, textStyleFrameBorder->value()); }
            );

    resetTextStyleFrameBorderRadius->setText(RESET_ICON);
    connect(resetTextStyleFrameBorderRadius, &QToolButton::clicked, [=]() { resetTextStyle(Ms::Pid::FRAME_ROUND); });
    connect(textStyleFrameBorderRadius, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [=]() { textStyleValueChanged(Ms::Pid::FRAME_ROUND, textStyleFrameBorderRadius->value()); }
            );

    resetTextStyleFrameForeground->setText(RESET_ICON);
    connect(resetTextStyleFrameForeground, &QToolButton::clicked, [=]() { resetTextStyle(Ms::Pid::FRAME_FG_COLOR); });
    connect(textStyleFrameForeground, &Awl::ColorLabel::colorChanged,
            [=]() { textStyleValueChanged(Ms::Pid::FRAME_FG_COLOR, textStyleFrameForeground->color()); }
            );

    resetTextStyleFrameBackground->setText(RESET_ICON);
    connect(resetTextStyleFrameBackground, &QToolButton::clicked, [=]() { resetTextStyle(Ms::Pid::FRAME_BG_COLOR); });
    connect(textStyleFrameBackground, &Awl::ColorLabel::colorChanged,
            [=]() { textStyleValueChanged(Ms::Pid::FRAME_BG_COLOR, textStyleFrameBackground->color()); }
            );

    resetTextStyleColor->setText(RESET_ICON);
    connect(resetTextStyleColor, &QToolButton::clicked, [=]() { resetTextStyle(Ms::Pid::COLOR); });
    connect(textStyleColor, &Awl::ColorLabel::colorChanged,
            [=]() { textStyleValueChanged(Ms::Pid::COLOR, textStyleColor->color()); }
            );

    connect(textStyles, SIGNAL(currentRowChanged(int)), SLOT(textStyleChanged(int)));
    textStyles->setCurrentRow(0);

    adjustPagesStackSize(0);

    WidgetStateStore::restoreGeometry(this);
}

EditStyle::EditStyle(const EditStyle& other)
    : QDialog(other.parentWidget())
{
}

//---------------------------------------------------------
//   showEvent
//---------------------------------------------------------

void EditStyle::showEvent(QShowEvent* ev)
{
    setValues();
    pageList->setFocus();
    globalContext()->currentNotation()->undoStack()->prepareChanges();
    buttonApplyToAllParts->setEnabled(globalContext()->currentNotation()->style()->canApplyToAllParts());
    QWidget::showEvent(ev);
}

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void EditStyle::hideEvent(QHideEvent* ev)
{
    WidgetStateStore::saveGeometry(this);
    QWidget::hideEvent(ev);
}

//---------------------------------------------------------
//   changeEvent
//---------------------------------------------------------

void EditStyle::changeEvent(QEvent* event)
{
    QDialog::changeEvent(event);
    if (event->type() == QEvent::LanguageChange) {
        retranslate();
    }
}

//---------------------------------------------------------
//   retranslate
///   NOTE: keep in sync with constructor.
//---------------------------------------------------------

void EditStyle::retranslate()
{
    retranslateUi(this);

    buttonApplyToAllParts->setText(tr("Apply to all Parts"));

    for (QComboBox* cb : lineStyleComboBoxes) {
        int idx = 0;
        for (const char* p : lineStyles) {
            cb->setItemText(idx, qtrc("notation", p));
            ++idx;
        }
    }

    for (QComboBox* cb : verticalPlacementComboBoxes) {
        cb->setItemText(0, tr("Above"));
        cb->setItemText(1, tr("Below"));
    }

    for (QComboBox* cb : horizontalPlacementComboBoxes) {
        cb->setItemText(0, tr("Left"));
        cb->setItemText(1, tr("Center"));
        cb->setItemText(2, tr("Right"));
    }

    mmRestRangeBracketType->setItemText(0, tr("None"));
    mmRestRangeBracketType->setItemText(1, tr("Brackets"));
    mmRestRangeBracketType->setItemText(2, tr("Parentheses"));

    autoplaceVerticalAlignRange->setItemText(0, tr("Segment"));
    autoplaceVerticalAlignRange->setItemText(1, tr("Measure"));
    autoplaceVerticalAlignRange->setItemText(2, tr("System"));

    tupletNumberType->setItemText(0, tr("Number"));
    tupletNumberType->setItemText(1, tr("Ratio"));
    tupletNumberType->setItemText(2, tr("None", "no tuplet number type"));

    tupletBracketType->setItemText(0, tr("Automatic"));
    tupletBracketType->setItemText(1, tr("Bracket"));
    tupletBracketType->setItemText(2, tr("None", "no tuplet bracket type"));

    voicingSelectWidget->interpretBox->setItemText(0, tr("Jazz"));
    voicingSelectWidget->interpretBox->setItemText(1, tr("Literal"));

    voicingSelectWidget->voicingBox->setItemText(0, tr("Automatic"));
    voicingSelectWidget->voicingBox->setItemText(1, tr("Root Only"));
    voicingSelectWidget->voicingBox->setItemText(2, tr("Close"));
    voicingSelectWidget->voicingBox->setItemText(3, tr("Drop Two"));
    voicingSelectWidget->voicingBox->setItemText(4, tr("Six Note"));
    voicingSelectWidget->voicingBox->setItemText(5, tr("Four Note"));
    voicingSelectWidget->voicingBox->setItemText(6, tr("Three Note"));

    voicingSelectWidget->durationBox->setItemText(0, tr("Until Next Chord Symbol"));
    voicingSelectWidget->durationBox->setItemText(1, tr("Until End of Measure"));
    voicingSelectWidget->durationBox->setItemText(2, tr("Chord/Rest Duration"));

    setHeaderFooterToolTip();

    int idx = 0;
    for (auto ss : Ms::allTextStyles()) {
        QString name = Ms::textStyleUserName(ss);
        textStyles->item(idx)->setText(name);
        ++idx;
    }

    textStyleFrameType->setItemText(0, tr("None", "no frame for text"));
    textStyleFrameType->setItemText(1, tr("Rectangle"));
    textStyleFrameType->setItemText(2, tr("Circle"));
}

//---------------------------------------------------------
//   setHeaderFooterToolTip
//---------------------------------------------------------

void EditStyle::setHeaderFooterToolTip()
{
    // keep in sync with implementation in Page::replaceTextMacros (page.cpp)
    // jumping thru hoops here to make the job of translators easier, yet have a nice display
    QString toolTipHeaderFooter
        = QString("<html><head></head><body><p><b>")
          + tr("Special symbols in header/footer")
          + QString("</b></p>")
          + QString("<table><tr><td>$p</td><td>-</td><td><i>")
          + tr("Page number, except on first page")
          + QString("</i></td></tr><tr><td>$N</td><td>-</td><td><i>")
          + tr("Page number, if there is more than one page")
          + QString("</i></td></tr><tr><td>$P</td><td>-</td><td><i>")
          + tr("Page number, on all pages")
          + QString("</i></td></tr><tr><td>$n</td><td>-</td><td><i>")
          + tr("Number of pages")
          + QString("</i></td></tr><tr><td>$f</td><td>-</td><td><i>")
          + tr("File name")
          + QString("</i></td></tr><tr><td>$F</td><td>-</td><td><i>")
          + tr("File path+name")
          + QString("</i></td></tr><tr><td>$i</td><td>-</td><td><i>")
          + tr("Part name, except on first page")
          + QString("</i></td></tr><tr><td>$I</td><td>-</td><td><i>")
          + tr("Part name, on all pages")
          + QString("</i></td></tr><tr><td>$d</td><td>-</td><td><i>")
          + tr("Current date")
          + QString("</i></td></tr><tr><td>$D</td><td>-</td><td><i>")
          + tr("Creation date")
          + QString("</i></td></tr><tr><td>$m</td><td>-</td><td><i>")
          + tr("Last modification time")
          + QString("</i></td></tr><tr><td>$M</td><td>-</td><td><i>")
          + tr("Last modification date")
          + QString("</i></td></tr><tr><td>$C</td><td>-</td><td><i>")
          + tr("Copyright, on first page only")
          + QString("</i></td></tr><tr><td>$c</td><td>-</td><td><i>")
          + tr("Copyright, on all pages")
          + QString("</i></td></tr><tr><td>$v</td><td>-</td><td><i>")
          + tr("MuseScore version this score was last saved with")
          + QString("</i></td></tr><tr><td>$r</td><td>-</td><td><i>")
          + tr("MuseScore revision this score was last saved with")
          + QString("</i></td></tr><tr><td>$$</td><td>-</td><td><i>")
          + tr("The $ sign itself")
          + QString("</i></td></tr><tr><td>$:tag:</td><td>-</td><td><i>")
          + tr("Metadata tag, see below")
          + QString("</i></td></tr></table><p>")
          + tr("Available metadata tags and their current values")
          + QString("<br />")
          + tr("(in File > Score Properties…):")
          + QString("</p><table>");

    // show all tags for current score/part, see also Score::init()
    QList<QMap<QString, QString> > tags; // FIXME
    for (const QMap<QString, QString>& tag: tags) {
        QMapIterator<QString, QString> i(tag);
        while (i.hasNext()) {
            i.next();
            toolTipHeaderFooter += QString("<tr><td>%1</td><td>-</td><td>%2</td></tr>").arg(i.key()).arg(i.value());
        }
    }

    toolTipHeaderFooter += QString("</table></body></html>");
    showHeader->setToolTip(toolTipHeaderFooter);
    showHeader->setToolTipDuration(5000);   // leaving the default value of -1 calculates the duration automatically and it takes too long
    showFooter->setToolTip(toolTipHeaderFooter);
    showFooter->setToolTipDuration(5000);
}

//---------------------------------------------------------
//   adjustPagesStackSize
//---------------------------------------------------------

void EditStyle::adjustPagesStackSize(int currentPageIndex)
{
    QSize preferredSize = pageStack->widget(currentPageIndex)->sizeHint();
    pageStack->setMinimumSize(preferredSize);

    connect(pageStack, &QStackedWidget::currentChanged, [this](int currentIndex) {
        QWidget* currentPage = pageStack->widget(currentIndex);
        if (!currentPage) {
            return;
        }
        pageStack->setMinimumSize(currentPage->sizeHint());
    });
}

//---------------------------------------------------------
//   pageForElement
///   Returns the page related to the element `e`, to allow the creation of a 'Style...'
///   menu for every possible element on the score.
//---------------------------------------------------------

EditStylePage EditStyle::pageForElement(Element* e)
{
    switch (e->type()) {
    case ElementType::SCORE:
        return &EditStyle::PageScore;
    case ElementType::PAGE:
        return &EditStyle::PagePage;
    case ElementType::TEXT:
        if (toText(e)->tid() == Ms::Tid::FOOTER || toText(e)->tid() == Ms::Tid::HEADER) {
            return &EditStyle::PageHeaderFooter;
        }
        return nullptr;
    case ElementType::MEASURE_NUMBER:
    case ElementType::MMREST_RANGE:
        return &EditStyle::PageMeasureNumbers;
    case ElementType::BRACKET:
    case ElementType::BRACKET_ITEM:
    case ElementType::SYSTEM_DIVIDER:
        return &EditStyle::PageSystem;
    case ElementType::CLEF:
        return &EditStyle::PageClefs;
    case ElementType::KEYSIG:
        return &EditStyle::PageAccidentals;
    case ElementType::MEASURE:
        return &EditStyle::PageMeasure;
    case ElementType::BAR_LINE:
        return &EditStyle::PageBarlines;
    case ElementType::NOTE:
    case ElementType::CHORD:
    case ElementType::ACCIDENTAL:
    case ElementType::STEM:
    case ElementType::STEM_SLASH:
    case ElementType::LEDGER_LINE:
        return &EditStyle::PageNotes;
    case ElementType::REST:
    case ElementType::MMREST:
        return &EditStyle::PageRests;
    case ElementType::MEASURE_REPEAT:
        return &EditStyle::PageMeasureRepeats;
    case ElementType::BEAM:
        return &EditStyle::PageBeams;
    case ElementType::TUPLET:
        return &EditStyle::PageTuplets;
    case ElementType::ARPEGGIO:
        return &EditStyle::PageArpeggios;
    case ElementType::SLUR:
    case ElementType::SLUR_SEGMENT:
    case ElementType::TIE:
    case ElementType::TIE_SEGMENT:
        return &EditStyle::PageSlursTies;
    case ElementType::HAIRPIN:
    case ElementType::HAIRPIN_SEGMENT:
        return &EditStyle::PageHairpins;
    case ElementType::VOLTA:
    case ElementType::VOLTA_SEGMENT:
        return &EditStyle::PageVolta;
    case ElementType::OTTAVA:
    case ElementType::OTTAVA_SEGMENT:
        return &EditStyle::PageOttava;
    case ElementType::PEDAL:
    case ElementType::PEDAL_SEGMENT:
        return &EditStyle::PagePedal;
    case ElementType::TRILL:
    case ElementType::TRILL_SEGMENT:
        return &EditStyle::PageTrill;
    case ElementType::VIBRATO:
    case ElementType::VIBRATO_SEGMENT:
        return &EditStyle::PageVibrato;
    case ElementType::BEND:
        return &EditStyle::PageBend;
    case ElementType::TEXTLINE:
    case ElementType::TEXTLINE_SEGMENT:
        return &EditStyle::PageTextLine;
    case ElementType::ARTICULATION:
        return &EditStyle::PageArticulationsOrnaments;
    case ElementType::FERMATA:
        return &EditStyle::PageFermatas;
    case ElementType::STAFF_TEXT:
        return &EditStyle::PageStaffText;
    case ElementType::TEMPO_TEXT:
        return &EditStyle::PageTempoText;
    case ElementType::LYRICS:
    case ElementType::LYRICSLINE:
    case ElementType::LYRICSLINE_SEGMENT:
        return &EditStyle::PageLyrics;
    case ElementType::DYNAMIC:
        return &EditStyle::PageDynamics;
    case ElementType::REHEARSAL_MARK:
        return &EditStyle::PageRehearsalMarks;
    case ElementType::FIGURED_BASS:
        return &EditStyle::PageFiguredBass;
    case ElementType::HARMONY:
        return &EditStyle::PageChordSymbols;
    case ElementType::FRET_DIAGRAM:
        return &EditStyle::PageFretboardDiagrams;
    default:
        return nullptr;
    }
}

//---------------------------------------------------------
//   elementHasPage
///   check if the element `e` has a style page related to it
//---------------------------------------------------------

bool EditStyle::elementHasPage(Element* e)
{
    return pageForElement(e) != nullptr;
}

//---------------------------------------------------------
//   gotoElement
///   switch to the page related to the element `e`
//---------------------------------------------------------

void EditStyle::gotoElement(Element* e)
{
    if (auto pagePointer = pageForElement(e)) {
        if (QWidget* page = this->*pagePointer) {
            setPage(pageStack->indexOf(page));
        }
    }
}

//---------------------------------------------------------
//   setPage
//---------------------------------------------------------

void EditStyle::setPage(int idx)
{
    if (idx >= 0) {
        pageList->setCurrentRow(idx);
    }
}

//---------------------------------------------------------
//   buttonClicked
//---------------------------------------------------------

void EditStyle::buttonClicked(QAbstractButton* b)
{
    switch (buttonBox->standardButton(b)) {
    case QDialogButtonBox::Ok:
        accept();
        break;
    case QDialogButtonBox::Cancel:
        reject();
        break;
    default:
        if (b == buttonApplyToAllParts) {
            globalContext()->currentNotation()->style()->applyToAllParts();
        }
        break;
    }
}

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void EditStyle::accept()
{
    globalContext()->currentNotation()->undoStack()->commitChanges();
    QDialog::accept();
}

//---------------------------------------------------------
//   reject
//---------------------------------------------------------

void EditStyle::reject()
{
    globalContext()->currentNotation()->undoStack()->rollbackChanges();
    QDialog::reject();
}

//---------------------------------------------------------
//   On comboFBFont currentIndex changed
//---------------------------------------------------------

void EditStyle::on_comboFBFont_currentIndexChanged(int index)
{
    qreal size, lineHeight;

    if (Ms::FiguredBass::fontData(index, 0, 0, &size, &lineHeight)) {
        doubleSpinFBSize->setValue(size);
        spinFBLineHeight->setValue(static_cast<int>(lineHeight * 100.0));
    }
}

//---------------------------------------------------------
//    On buttonTogglePagelist clicked
//---------------------------------------------------------

void EditStyle::on_buttonTogglePagelist_clicked()
{
    bool isVis = !pageList->isVisible();   // toggle it

    pageList->setVisible(isVis);
    buttonTogglePagelist->setText(isVis ? GO_NEXT_ICON : GO_PREV_ICON);
}

//---------------------------------------------------------
//    On resetStylesButton clicked
//---------------------------------------------------------

void EditStyle::on_resetStylesButton_clicked()
{
    NOT_IMPLEMENTED;
}

//---------------------------------------------------------
//   unhandledType
//---------------------------------------------------------

void EditStyle::unhandledType(const StyleWidget sw)
{
    const char* type = Ms::MStyle::valueType(sw.idx);
    qFatal("%s <%s>: widget: %s\n", type, Ms::MStyle::valueName(sw.idx), sw.widget->metaObject()->className());
}

//---------------------------------------------------------
//   getValue
//    return current gui value
//---------------------------------------------------------

QVariant EditStyle::getValue(StyleId idx)
{
    const StyleWidget& sw = styleWidget(idx);
    const char* type = styleValue(sw.idx).typeName();

    if (!strcmp("Ms::Spatium", type)) {
        QDoubleSpinBox* sb = qobject_cast<QDoubleSpinBox*>(sw.widget);
        return QVariant(Ms::Spatium(sb->value() * (sw.showPercent ? 0.01 : 1.0)));
    } else if (!strcmp("double", type)) {
        QVariant v = sw.widget->property("value");
        if (!v.isValid()) {
            unhandledType(sw);
        }
        if (sw.showPercent) {
            v = v.toDouble() * 0.01;
        }
        return v;
    } else if (!strcmp("bool", type)) {
        QVariant v;
        if (sw.idx == StyleId::harmonyVoiceLiteral) { // special case for bool represented by a two-item combobox
            QComboBox* cb = qobject_cast<QComboBox*>(sw.widget);
            v = cb->currentIndex();
        } else {
            v = sw.widget->property("checked");
            if (!v.isValid()) {
                unhandledType(sw);
            }
        }
        return v;
    } else if (!strcmp("int", type)) {
        if (qobject_cast<QComboBox*>(sw.widget)) {
            QComboBox* cb = qobject_cast<QComboBox*>(sw.widget);
            return cb->currentData().toInt();
        } else if (qobject_cast<QSpinBox*>(sw.widget)) {
            return qobject_cast<QSpinBox*>(sw.widget)->value() / (sw.showPercent ? 100 : 1);
        } else if (qobject_cast<QButtonGroup*>(sw.widget)) {
            QButtonGroup* bg = qobject_cast<QButtonGroup*>(sw.widget);
            return bg->checkedId();
        } else if (FontStyleSelect* fontStyle = qobject_cast<FontStyleSelect*>(sw.widget)) {
            return int(fontStyle->fontStyle());
        } else {
            qFatal("unhandled int");
        }
    } else if (!strcmp("QString", type)) {
        if (qobject_cast<QFontComboBox*>(sw.widget)) {
            return static_cast<QFontComboBox*>(sw.widget)->currentFont().family();
        }
        if (qobject_cast<QComboBox*>(sw.widget)) {
            QComboBox* cb = qobject_cast<QComboBox*>(sw.widget);
            return cb->currentData().toString();
        }
        if (qobject_cast<QTextEdit*>(sw.widget)) {
            QTextEdit* te = qobject_cast<QTextEdit*>(sw.widget);
            return te->toPlainText();
        }
    } else if (!strcmp("QPointF", type)) {
        OffsetSelect* cb = qobject_cast<OffsetSelect*>(sw.widget);
        if (cb) {
            return cb->offset();
        } else {
            qFatal("unhandled QPointF");
        }
    } else if (!strcmp("Ms::Direction", type)) {
        QComboBox* cb = qobject_cast<QComboBox*>(sw.widget);
        if (cb) {
            return cb->currentIndex();
        } else {
            qFatal("unhandled Direction");
        }
    } else if (!strcmp("Ms::Align", type)) {
        AlignSelect* as = qobject_cast<AlignSelect*>(sw.widget);
        return QVariant::fromValue(as->align());
    } else {
        qFatal("EditStyle::getValue: unhandled type <%s>", type);
    }
    return QVariant();
}

//---------------------------------------------------------
//   setValues
//---------------------------------------------------------

void EditStyle::setValues()
{
    for (const StyleWidget& sw : styleWidgets) {
        if (sw.widget) {
            sw.widget->blockSignals(true);
        }
        QVariant val = styleValue(sw.idx);
        const char* type = styleValue(sw.idx).typeName();
        if (sw.reset) {
            sw.reset->setEnabled(hasDefaultStyleValue(sw.idx));
        }

        if (!strcmp("Ms::Spatium", type)) {
            if (sw.showPercent) {
                qobject_cast<QSpinBox*>(sw.widget)->setValue(int(val.value<Ms::Spatium>().val() * 100.0));
            } else {
                sw.widget->setProperty("value", val);
            }
        } else if (!strcmp("double", type)) {
            if (sw.showPercent) {
                val = QVariant(val.toDouble() * 100);
            }
            if (!sw.widget->setProperty("value", val)) {
                unhandledType(sw);
            }
        } else if (!strcmp("bool", type)) {
            if (sw.idx == StyleId::harmonyVoiceLiteral) { // special case for bool represented by a two-item combobox
                voicingSelectWidget->interpretBox->setCurrentIndex(val.toBool());
            } else {
                if (!sw.widget->setProperty("checked", val)) {
                    unhandledType(sw);
                }
                if (sw.idx == StyleId::measureNumberSystem && !val.toBool()) {
                    showIntervalMeasureNumber->setChecked(true);
                }
            }
        } else if (!strcmp("int", type)) {
            if (qobject_cast<QComboBox*>(sw.widget)) {
                QComboBox* cb = qobject_cast<QComboBox*>(sw.widget);
                cb->setCurrentIndex(cb->findData(val));
            } else if (qobject_cast<QSpinBox*>(sw.widget)) {
                qobject_cast<QSpinBox*>(sw.widget)->setValue(val.toInt()
                                                             * (sw.showPercent ? 100 : 1));
            } else if (qobject_cast<QButtonGroup*>(sw.widget)) {
                QButtonGroup* bg = qobject_cast<QButtonGroup*>(sw.widget);
                for (auto a : bg->buttons()) {
                    if (bg->id(a) == val.toInt()) {
                        a->setChecked(true);
                        break;
                    }
                }
            } else if (FontStyleSelect* fontStyle = qobject_cast<FontStyleSelect*>(sw.widget)) {
                fontStyle->setFontStyle(Ms::FontStyle(val.toInt()));
            } else {
                unhandledType(sw);
            }
        } else if (!strcmp("QString", type)) {
            if (qobject_cast<QFontComboBox*>(sw.widget)) {
                static_cast<QFontComboBox*>(sw.widget)->setCurrentFont(QFont(val.toString()));
            } else if (qobject_cast<QComboBox*>(sw.widget)) {
                QComboBox* cb = qobject_cast<QComboBox*>(sw.widget);
                for (int i = 0; i < cb->count(); ++i) {
                    if (cb->itemData(i) == val.toString()) {
                        cb->setCurrentIndex(i);
                        break;
                    }
                }
            } else if (qobject_cast<QTextEdit*>(sw.widget)) {
                static_cast<QTextEdit*>(sw.widget)->setPlainText(val.toString());
            } else {
                unhandledType(sw);
            }
        } else if (!strcmp("Ms::Direction", type)) {
            QComboBox* cb = qobject_cast<QComboBox*>(sw.widget);
            if (cb) {
                cb->setCurrentIndex(int(val.value<Ms::Direction>()));
            } else {
                unhandledType(sw);
            }
        } else if (!strcmp("Ms::Align", type)) {
            AlignSelect* as = qobject_cast<AlignSelect*>(sw.widget);
            as->setAlign(val.value<Ms::Align>());
        } else if (!strcmp("QPointF", type)) {
            OffsetSelect* as = qobject_cast<OffsetSelect*>(sw.widget);
            if (as) {
                as->setOffset(val.value<QPointF>());
            }
        } else {
            unhandledType(sw);
        }

        if (sw.widget) {
            sw.widget->blockSignals(false);
        }
    }

    textStyleChanged(textStyles->currentRow());

    //TODO: convert the rest:

    QString unit(styleValue(StyleId::swingUnit).toString());

    if (unit == Ms::TDuration(Ms::TDuration::DurationType::V_EIGHTH).name()) {
        swingEighth->setChecked(true);
        swingBox->setEnabled(true);
    } else if (unit == Ms::TDuration(Ms::TDuration::DurationType::V_16TH).name()) {
        swingSixteenth->setChecked(true);
        swingBox->setEnabled(true);
    } else if (unit == Ms::TDuration(Ms::TDuration::DurationType::V_ZERO).name()) {
        swingOff->setChecked(true);
        swingBox->setEnabled(false);
    }
    QString s(styleValue(StyleId::chordDescriptionFile).toString());
    chordDescriptionFile->setText(s);
    QString cstyle(styleValue(StyleId::chordStyle).toString());
    if (cstyle == "std") {
        chordsStandard->setChecked(true);
        chordDescriptionGroup->setEnabled(false);
    } else if (cstyle == "jazz") {
        chordsJazz->setChecked(true);
        chordDescriptionGroup->setEnabled(false);
    } else {
        chordsCustom->setChecked(true);
        chordDescriptionGroup->setEnabled(true);
    }
    //formattingGroup->setEnabled(lstyle.chordList()->autoAdjust());

    // figured bass
    for (int i = 0; i < comboFBFont->count(); i++) {
        if (comboFBFont->itemText(i) == styleValue(StyleId::figuredBassFontFamily).toString()) {
            comboFBFont->setCurrentIndex(i);
            break;
        }
    }
    doubleSpinFBSize->setValue(styleValue(StyleId::figuredBassFontSize).toDouble());
    doubleSpinFBVertPos->setValue(styleValue(StyleId::figuredBassYOffset).toDouble());
    spinFBLineHeight->setValue(styleValue(StyleId::figuredBassLineHeight).toDouble() * 100.0);

    QString mfont(styleValue(StyleId::MusicalSymbolFont).toString());
    int idx = 0;
    for (const auto& i : Ms::ScoreFont::scoreFonts()) {
        if (i.name().toLower() == mfont.toLower()) {
            musicalSymbolFont->setCurrentIndex(idx);
            break;
        }
        ++idx;
    }
    musicalTextFont->blockSignals(true);
    musicalTextFont->clear();
    // CAUTION: the second element, the itemdata, is a font family name!
    // It's also stored in score file as the musicalTextFont
    musicalTextFont->addItem("Leland Text", "Leland Text");
    musicalTextFont->addItem("Bravura Text", "Bravura Text");
    musicalTextFont->addItem("Emmentaler Text", "MScore Text");
    musicalTextFont->addItem("Gonville Text", "Gootville Text");
    musicalTextFont->addItem("MuseJazz Text", "MuseJazz Text");
    musicalTextFont->addItem("Petaluma Text", "Petaluma Text");
    QString tfont(styleValue(StyleId::MusicalTextFont).toString());
    idx = musicalTextFont->findData(tfont);
    musicalTextFont->setCurrentIndex(idx);
    musicalTextFont->blockSignals(false);

    toggleHeaderOddEven(styleValue(StyleId::headerOddEven).toBool());
    toggleFooterOddEven(styleValue(StyleId::footerOddEven).toBool());
    disableVerticalSpread->setChecked(!styleValue(StyleId::enableVerticalSpread).toBool());
}

//---------------------------------------------------------
//   selectChordDescriptionFile
//---------------------------------------------------------

void EditStyle::selectChordDescriptionFile()
{
    io::path dir = configuration()->stylesPath().val;
    QString filter = qtrc("notation", "MuseScore Styles") + " (*.mss)";

    mu::io::path path = interactive()->selectOpeningFile(qtrc("notation", "Load Style"), dir, filter);
    if (path.empty()) {
        return;
    }

    chordDescriptionFile->setText(path.toQString());
    setChordStyle(true);
}

//---------------------------------------------------------
//   setSwingParams
//---------------------------------------------------------

void EditStyle::setSwingParams(bool checked)
{
    if (!checked) {
        return;
    }
    QVariant val;
    if (swingOff->isChecked()) {
        val = Ms::TDuration(Ms::TDuration::DurationType::V_ZERO).name();
        swingBox->setEnabled(false);
    } else if (swingEighth->isChecked()) {
        val = Ms::TDuration(Ms::TDuration::DurationType::V_EIGHTH).name();
        swingBox->setEnabled(true);
    } else if (swingSixteenth->isChecked()) {
        val = Ms::TDuration(Ms::TDuration::DurationType::V_16TH).name();
        swingBox->setEnabled(true);
    }

    setStyleValue(StyleId::swingUnit, val);
}

QVariant EditStyle::styleValue(StyleId id) const
{
    return globalContext()->currentNotation()->style()->styleValue(id);
}

QVariant EditStyle::defaultStyleValue(StyleId id) const
{
    return globalContext()->currentNotation()->style()->defaultStyleValue(id);
}

bool EditStyle::hasDefaultStyleValue(StyleId id) const
{
    return defaultStyleValue(id) == styleValue(id);
}

void EditStyle::setStyleValue(StyleId id, const QVariant& value)
{
    globalContext()->currentNotation()->style()->setStyleValue(id, value);
}

//---------------------------------------------------------
//   concertPitchToggled
//---------------------------------------------------------

void EditStyle::concertPitchToggled(bool checked)
{
    setStyleValue(StyleId::concertPitch, checked);
}

//---------------------------------------------------------
//   setChordStyle
//---------------------------------------------------------

void EditStyle::setChordStyle(bool checked)
{
    if (!checked) {
        return;
    }
    QVariant val;
    QString file;
    bool chordsXml;
    if (chordsStandard->isChecked()) {
        val  = QString("std");
        file = "chords_std.xml";
        chordsXml = false;
    } else if (chordsJazz->isChecked()) {
        val  = QString("jazz");
        file = "chords_jazz.xml";
        chordsXml = false;
    } else {
        val = QString("custom");
        chordDescriptionGroup->setEnabled(true);
        file = chordDescriptionFile->text();
        chordsXml = chordsXmlFile->isChecked();
    }
    if (val != "custom") {
        chordsXmlFile->setChecked(chordsXml);
        chordDescriptionGroup->setEnabled(false);
        chordDescriptionFile->setText(file);
    }

    setStyleValue(StyleId::chordsXmlFile, chordsXml);
    setStyleValue(StyleId::chordStyle, val);

    if (!file.isEmpty()) {
        setStyleValue(StyleId::chordDescriptionFile, file);
    }
}

//---------------------------------------------------------
//   toggleHeaderOddEven
//---------------------------------------------------------

void EditStyle::toggleHeaderOddEven(bool checked)
{
    if (!showHeader->isChecked()) {
        return;
    }
    labelEvenHeader->setEnabled(checked);
    evenHeaderL->setEnabled(checked);
    evenHeaderC->setEnabled(checked);
    evenHeaderR->setEnabled(checked);
    static QString odd  = labelOddHeader->text();    // save on 1st round
    static QString even = labelEvenHeader->text();   // save on 1st round
    if (checked) {
        labelOddHeader->setText(odd);     // restore
    } else {
        labelOddHeader->setText(odd + "\n" + even);     // replace
    }
    return;
}

//---------------------------------------------------------
//   enableStyleWidget
//---------------------------------------------------------

void EditStyle::enableStyleWidget(const StyleId idx, bool enable)
{
    const StyleWidget& sw { styleWidget(idx) };
    static_cast<QWidget*>(sw.widget)->setEnabled(enable);
    if (sw.reset) {
        sw.reset->setEnabled(enable && hasDefaultStyleValue(idx));
    }
}

//---------------------------------------------------------
//   enableVerticalSpreadClicked
//---------------------------------------------------------

void EditStyle::enableVerticalSpreadClicked(bool checked)
{
    disableVerticalSpread->setChecked(!checked);
}

//---------------------------------------------------------
//   disableVerticalSpreadClicked
//---------------------------------------------------------

void EditStyle::disableVerticalSpreadClicked(bool checked)
{
    setStyleValue(StyleId::enableVerticalSpread, !checked);
    enableVerticalSpread->setChecked(!checked);
}

//---------------------------------------------------------
//   toggleFooterOddEven
//---------------------------------------------------------

void EditStyle::toggleFooterOddEven(bool checked)
{
    if (!showFooter->isChecked()) {
        return;
    }
    labelEvenFooter->setEnabled(checked);
    evenFooterL->setEnabled(checked);
    evenFooterC->setEnabled(checked);
    evenFooterR->setEnabled(checked);
    static QString odd  = labelOddFooter->text();    // save on 1st round
    static QString even = labelEvenFooter->text();   // save on 1st round
    if (checked) {
        labelOddFooter->setText(odd);     // restore
    } else {
        labelOddFooter->setText(odd + "\n" + even);     // replace
    }
    return;
}

//---------------------------------------------------------
//   lyricsDashMin/MaxLengthValueChanged
///   Ensure lyricsDashMinLength <= lyricsDashMaxLength
//---------------------------------------------------------

void EditStyle::lyricsDashMaxLengthValueChanged(double val)
{
    double otherVal = lyricsDashMinLength->value();
    if (otherVal > val) {
        lyricsDashMaxLength->setValue(otherVal);
    }
}

void EditStyle::lyricsDashMinLengthValueChanged(double val)
{
    double otherVal = lyricsDashMaxLength->value();
    if (otherVal < val) {
        lyricsDashMinLength->setValue(otherVal);
    }
}

//---------------------------------------------------------
//   systemMin/MaxDistanceValueChanged
///   Ensure minSystemDistance <= maxSystemDistance
//---------------------------------------------------------

void EditStyle::systemMaxDistanceValueChanged(double val)
{
    double otherVal = minSystemDistance->value();
    if (otherVal > val) {
        maxSystemDistance->setValue(otherVal);
    }
}

void EditStyle::systemMinDistanceValueChanged(double val)
{
    double otherVal = maxSystemDistance->value();
    if (otherVal < val) {
        minSystemDistance->setValue(otherVal);
    }
}

//---------------------------------------------------------
//   styleWidget
//---------------------------------------------------------

const StyleWidget& EditStyle::styleWidget(StyleId idx) const
{
    for (const StyleWidget& sw : styleWidgets) {
        if (sw.idx == idx) {
            return sw;
        }
    }
#if (!defined (_MSCVER) && !defined (_MSC_VER))
    __builtin_unreachable();
#else
    // The MSVC __assume() optimizer hint is similar, though not identical, to __builtin_unreachable()
    __assume(0);
#endif
}

//---------------------------------------------------------
//   valueChanged
//---------------------------------------------------------

void EditStyle::valueChanged(int i)
{
    StyleId idx       = (StyleId)i;
    QVariant val  = getValue(idx);
    bool setValue = false;
    if (idx == StyleId::MusicalSymbolFont && optimizeStyleCheckbox->isChecked()) {
        Ms::ScoreFont* scoreFont = Ms::ScoreFont::fontFactory(val.toString());
        if (scoreFont) {
            for (auto j : scoreFont->engravingDefaults()) {
                setStyleValue(j.first, j.second);
            }

            // fix values, the distances are defined different in MuseScore
            double barWidthD = styleValue(StyleId::barWidth).toDouble() + styleValue(StyleId::endBarWidth).toDouble() * .5;

            double newEndBarDistance = styleValue(StyleId::endBarDistance).toDouble() + barWidthD;
            setStyleValue(StyleId::endBarDistance, newEndBarDistance);

            double newDoubleBarDistance = styleValue(StyleId::doubleBarDistance).toDouble() + barWidthD;
            setStyleValue(StyleId::doubleBarDistance, newDoubleBarDistance);

            // guess the repeat dot width = spatium * .3
            double newRepeatBarlineDotSepration = styleValue(StyleId::repeatBarlineDotSeparation).toDouble()
                                                  + (styleValue(StyleId::barWidth).toDouble() + .3) * .5;
            setStyleValue(StyleId::repeatBarlineDotSeparation, newRepeatBarlineDotSepration);

            // adjust mmrest, which is not in engravingDefaults
            // TODO: create generalized method for setting style vals based on font
            if (scoreFont->name() == "Bravura") {
                setStyleValue(StyleId::mmRestHBarThickness, 1.0);
                setStyleValue(StyleId::multiMeasureRestMargin, 3.0);
            } else {
                setStyleValue(StyleId::mmRestHBarThickness, defaultStyleValue(StyleId::mmRestHBarThickness));
                setStyleValue(StyleId::multiMeasureRestMargin, defaultStyleValue(StyleId::multiMeasureRestMargin));
            }
        }
        setValue = true;
    }

    setStyleValue(idx, val);
    if (setValue) {
        setValues();
    }
    const StyleWidget& sw = styleWidget(idx);
    if (sw.reset) {
        sw.reset->setEnabled(!hasDefaultStyleValue(idx));
    }
}

//---------------------------------------------------------
//   resetStyleValue
//---------------------------------------------------------

void EditStyle::resetStyleValue(int i)
{
    StyleId idx = (StyleId)i;

    setStyleValue(idx, defaultStyleValue(idx));
    setValues();
}

//---------------------------------------------------------
//   textStyleChanged
//---------------------------------------------------------

void EditStyle::textStyleChanged(int row)
{
    Ms::Tid tid = Ms::Tid(textStyles->item(row)->data(Qt::UserRole).toInt());
    const Ms::TextStyle* ts = Ms::textStyle(tid);

    for (const Ms::StyledProperty& a : *ts) {
        switch (a.pid) {
        case Ms::Pid::FONT_FACE: {
            QVariant val = styleValue(a.sid);
            textStyleFontFace->setCurrentFont(QFont(val.toString()));
            resetTextStyleFontFace->setEnabled(val != defaultStyleValue(a.sid));
        }
        break;

        case Ms::Pid::FONT_SIZE:
            textStyleFontSize->setValue(styleValue(a.sid).toDouble());
            resetTextStyleFontSize->setEnabled(styleValue(a.sid) != defaultStyleValue(a.sid));
            break;

        case Ms::Pid::TEXT_LINE_SPACING:
            textStyleLineSpacing->setValue(styleValue(a.sid).toDouble());
            resetTextStyleLineSpacing->setEnabled(styleValue(a.sid) != defaultStyleValue(a.sid));
            break;

        case Ms::Pid::FONT_STYLE:
            textStyleFontStyle->setFontStyle(Ms::FontStyle(styleValue(a.sid).toInt()));
            resetTextStyleFontStyle->setEnabled(styleValue(a.sid) != defaultStyleValue(a.sid));
            break;

        case Ms::Pid::ALIGN:
            textStyleAlign->setAlign(styleValue(a.sid).value<Ms::Align>());
            resetTextStyleAlign->setEnabled(styleValue(a.sid) != defaultStyleValue(a.sid));
            break;

        case Ms::Pid::OFFSET:
            textStyleOffset->setOffset(styleValue(a.sid).toPointF());
            resetTextStyleOffset->setEnabled(styleValue(a.sid) != defaultStyleValue(a.sid));
            break;

        case Ms::Pid::SIZE_SPATIUM_DEPENDENT: {
            QVariant val = styleValue(a.sid);
            textStyleSpatiumDependent->setChecked(val.toBool());
            resetTextStyleSpatiumDependent->setEnabled(val != defaultStyleValue(a.sid));
            textStyleOffset->setSuffix(val.toBool() ? tr("sp") : tr("mm"));
        }
        break;

        case Ms::Pid::FRAME_TYPE:
            textStyleFrameType->setCurrentIndex(styleValue(a.sid).toInt());
            resetTextStyleFrameType->setEnabled(styleValue(a.sid) != defaultStyleValue(a.sid));
            frameWidget->setEnabled(styleValue(a.sid).toInt() != 0);             // disable if no frame
            break;

        case Ms::Pid::FRAME_PADDING:
            textStyleFramePadding->setValue(styleValue(a.sid).toDouble());
            resetTextStyleFramePadding->setEnabled(styleValue(a.sid) != defaultStyleValue(a.sid));
            break;

        case Ms::Pid::FRAME_WIDTH:
            textStyleFrameBorder->setValue(styleValue(a.sid).toDouble());
            resetTextStyleFrameBorder->setEnabled(styleValue(a.sid) != defaultStyleValue(a.sid));
            break;

        case Ms::Pid::FRAME_ROUND:
            textStyleFrameBorderRadius->setValue(styleValue(a.sid).toDouble());
            resetTextStyleFrameBorderRadius->setEnabled(styleValue(a.sid) != defaultStyleValue(a.sid));
            break;

        case Ms::Pid::FRAME_FG_COLOR:
            textStyleFrameForeground->setColor(styleValue(a.sid).value<QColor>());
            resetTextStyleFrameForeground->setEnabled(styleValue(a.sid) != defaultStyleValue(a.sid));
            break;

        case Ms::Pid::FRAME_BG_COLOR:
            textStyleFrameBackground->setColor(styleValue(a.sid).value<QColor>());
            resetTextStyleFrameBackground->setEnabled(styleValue(a.sid) != defaultStyleValue(a.sid));
            break;

        case Ms::Pid::COLOR:
            textStyleColor->setColor(styleValue(a.sid).value<QColor>());
            resetTextStyleColor->setEnabled(styleValue(a.sid) != defaultStyleValue(a.sid));
            break;

        default:
            break;
        }
    }

    styleName->setText(Ms::textStyleUserName(tid));
    styleName->setEnabled(int(tid) >= int(Ms::Tid::USER1));
    resetTextStyleName->setEnabled(styleName->text() != Ms::textStyleUserName(tid));
}

//---------------------------------------------------------
//   textStyleValueChanged
//---------------------------------------------------------

void EditStyle::textStyleValueChanged(Ms::Pid pid, QVariant value)
{
    Ms::Tid tid = Ms::Tid(textStyles->item(textStyles->currentRow())->data(Qt::UserRole).toInt());
    const Ms::TextStyle* ts = Ms::textStyle(tid);

    for (const Ms::StyledProperty& a : *ts) {
        if (a.pid == pid) {
            setStyleValue(a.sid, value);
            break;
        }
    }
    textStyleChanged(textStyles->currentRow()); // update GUI (reset buttons)
}

//---------------------------------------------------------
//   resetTextStyle
//---------------------------------------------------------

void EditStyle::resetTextStyle(Ms::Pid pid)
{
    Ms::Tid tid = Ms::Tid(textStyles->item(textStyles->currentRow())->data(Qt::UserRole).toInt());
    const Ms::TextStyle* ts = Ms::textStyle(tid);

    for (const Ms::StyledProperty& a : *ts) {
        if (a.pid == pid) {
            setStyleValue(a.sid, defaultStyleValue(a.sid));
            break;
        }
    }
    textStyleChanged(textStyles->currentRow()); // update GUI
}

//---------------------------------------------------------
//   editUserStyleName
//---------------------------------------------------------

void EditStyle::editUserStyleName()
{
    int row = textStyles->currentRow();
    Ms::Tid tid = Ms::Tid(textStyles->item(row)->data(Qt::UserRole).toInt());
    textStyles->item(row)->setText(styleName->text());
    resetTextStyleName->setEnabled(styleName->text() != Ms::textStyleUserName(tid));
}

//---------------------------------------------------------
//   endEditUserStyleName
//---------------------------------------------------------

void EditStyle::endEditUserStyleName()
{
    int row = textStyles->currentRow();
    Ms::Tid tid = Ms::Tid(textStyles->item(row)->data(Qt::UserRole).toInt());
    int idx = int(tid) - int(Ms::Tid::USER1);
    if (int(tid) < int(Ms::Tid::USER1) || int(tid) > int(Ms::Tid::USER12)) {
        qDebug("User style index %d outside of range.", idx);
        return;
    }
    StyleId sid[]
        = { StyleId::user1Name, StyleId::user2Name, StyleId::user3Name, StyleId::user4Name, StyleId::user5Name, StyleId::user6Name,
            StyleId::user7Name, StyleId::user8Name, StyleId::user9Name, StyleId::user10Name, StyleId::user11Name, StyleId::user12Name };
    QString name = styleName->text();
    setStyleValue(sid[idx], name);
    if (name == "") {
        name = Ms::textStyleUserName(tid);
        styleName->setText(name);
        textStyles->item(row)->setText(name);
        resetTextStyleName->setEnabled(false);
    }
//    MuseScoreCore::mscoreCore->updateInspector(); TODO
}

//---------------------------------------------------------
//   resetUserStyleName
//---------------------------------------------------------

void EditStyle::resetUserStyleName()
{
    styleName->clear();
    endEditUserStyleName();
}
