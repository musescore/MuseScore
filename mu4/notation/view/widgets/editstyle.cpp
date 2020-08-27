//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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

#include "alignSelect.h"
#include "offsetSelect.h"
#include "fontStyleSelect.h"

#include "libmscore/figuredbass.h"
#include "libmscore/layout.h"
#include "libmscore/sym.h"

#include "ui/view/iconcodes.h"

#include "framework/global/widgetstatestore.h"

#include "translation.h"

using namespace Ms;
using namespace mu::notation;
using namespace mu::notation;
using namespace mu::framework;

QChar iconCodeToChar(IconCode::Code code)
{
    return QChar(static_cast<char16_t>(code));
}

static const QChar GO_NEXT_ICON = iconCodeToChar(IconCode::Code::ARROW_RIGHT);
static const QChar GO_PREV_ICON = iconCodeToChar(IconCode::Code::ARROW_LEFT);
static const QChar OPEN_FILE_ICON = iconCodeToChar(IconCode::Code::OPEN_FILE);
static const QChar RESET_ICON = iconCodeToChar(IconCode::Code::REDO);

//---------------------------------------------------------
//   EditStyle
//---------------------------------------------------------

EditStyle::EditStyle(QWidget* parent)
    : QDialog(parent)
{
    setObjectName("EditStyle");
    setupUi(this);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    buttonApplyToAllParts = buttonBox->addButton(tr("Apply to all Parts"), QDialogButtonBox::ApplyRole);
    buttonTogglePagelist->setText(GO_NEXT_ICON);
    setModal(true);

    // create button groups for every set of radio button widgets
    // use this group widgets in list styleWidgets
    // This works for groups which represent an int enumeration.

    QButtonGroup* fretNumGroup = new QButtonGroup(this);
    fretNumGroup->addButton(radioFretNumLeft, 0);
    fretNumGroup->addButton(radioFretNumRight, 1);

    QButtonGroup* ksng = new QButtonGroup(this);
    ksng->addButton(radioKeySigNatNone, int(KeySigNatural::NONE));
    ksng->addButton(radioKeySigNatBefore, int(KeySigNatural::BEFORE));
    ksng->addButton(radioKeySigNatAfter, int(KeySigNatural::AFTER));

    QButtonGroup* ctg = new QButtonGroup(this);
    ctg->addButton(clefTab1, int(ClefType::TAB));
    ctg->addButton(clefTab2, int(ClefType::TAB_SERIF));

    QButtonGroup* fbAlign = new QButtonGroup(this);
    fbAlign->addButton(radioFBTop, 0);
    fbAlign->addButton(radioFBBottom, 1);

    QButtonGroup* fbStyle = new QButtonGroup(this);
    fbStyle->addButton(radioFBModern, 0);
    fbStyle->addButton(radioFBHistoric, 1);

    const char* styles[] = {
        QT_TRANSLATE_NOOP("EditStyleBase", "Continuous"),
        QT_TRANSLATE_NOOP("EditStyleBase", "Dashed"),
        QT_TRANSLATE_NOOP("EditStyleBase", "Dotted"),
        QT_TRANSLATE_NOOP("EditStyleBase", "Dash-dotted"),
        QT_TRANSLATE_NOOP("EditStyleBase", "Dash-dot-dotted")
    };
    int dta = 1;
    voltaLineStyle->clear();
    ottavaLineStyle->clear();
    pedalLineStyle->clear();
    for (const char* p : styles) {
        QString trs = qApp->translate("EditStyleBase", p);
        voltaLineStyle->addItem(trs, dta);
        ottavaLineStyle->addItem(trs, dta);
        pedalLineStyle->addItem(trs, dta);
        ++dta;
    }

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
        { StyleId::minSystemDistance,       false, minSystemDistance,       resetMinSystemDistance },
        { StyleId::maxSystemDistance,       false, maxSystemDistance,       resetMaxSystemDistance },

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
        { StyleId::alwaysShowBracketsWhenEmptyStavesAreHidden, false, alwaysShowBrackets, 0 },
        { StyleId::hideInstrumentNameIfOneInstrument, false, hideInstrumentNameIfOneInstrument, 0 },
        { StyleId::accidentalNoteDistance,  false, accidentalNoteDistance,  0 },
        { StyleId::accidentalDistance,      false, accidentalDistance,      0 },

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
        { StyleId::systemHeaderTimeSigDistance,    false, systemHeaderTimeSigDistance,
          resetSystemHeaderTimeSigDistance },

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

        { StyleId::vibratoPlacement,        false, vibratoLinePlacement,      resetVibratoLinePlacement },
        { StyleId::vibratoPosAbove,         false, vibratoLinePosAbove,       resetVibratoLinePosAbove },
        { StyleId::vibratoPosBelow,         false, vibratoLinePosBelow,       resetVibratoLinePosBelow },

        { StyleId::harmonyFretDist,         false, harmonyFretDist,         0 },
        { StyleId::minHarmonyDistance,      false, minHarmonyDistance,      0 },
        { StyleId::maxHarmonyBarDistance,   false, maxHarmonyBarDistance,   0 },

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

    for (QComboBox* cb : std::vector<QComboBox*> {
        lyricsPlacement, textLinePlacement, hairpinPlacement, pedalLinePlacement,
        trillLinePlacement, vibratoLinePlacement, dynamicsPlacement,
        tempoTextPlacement, staffTextPlacement, rehearsalMarkPlacement,
        measureNumberVPlacement
    }) {
        cb->clear();
        cb->addItem(tr("Above"), int(Placement::ABOVE));
        cb->addItem(tr("Below"), int(Placement::BELOW));
    }

    measureNumberHPlacement->clear();
    measureNumberHPlacement->addItem(tr("Left"),   int(HPlacement::LEFT));
    measureNumberHPlacement->addItem(tr("Center"), int(HPlacement::CENTER));
    measureNumberHPlacement->addItem(tr("Right"),  int(HPlacement::RIGHT));

    autoplaceVerticalAlignRange->clear();
    autoplaceVerticalAlignRange->addItem(tr("Segment"), int(VerticalAlignRange::SEGMENT));
    autoplaceVerticalAlignRange->addItem(tr("Measure"), int(VerticalAlignRange::MEASURE));
    autoplaceVerticalAlignRange->addItem(tr("System"),  int(VerticalAlignRange::SYSTEM));

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
    for (auto i : ScoreFont::scoreFonts()) {
        musicalSymbolFont->addItem(i.name(), i.name());
        ++idx;
    }

    static const SymId ids[] = {
        SymId::systemDivider, SymId::systemDividerLong, SymId::systemDividerExtraLong
    };
    for (SymId id : ids) {
        const QString& un = Sym::id2userName(id);
        const char* n  = Sym::id2name(id);
        dividerLeftSym->addItem(un,  QVariant(QString(n)));
        dividerRightSym->addItem(un, QVariant(QString(n)));
    }

    // figured bass init
    QList<QString> fbFontNames = FiguredBass::fontNames();
    for (const QString& family: fbFontNames) {
        comboFBFont->addItem(family);
    }
    comboFBFont->setCurrentIndex(0);
    connect(comboFBFont, SIGNAL(currentIndexChanged(int)), SLOT(on_comboFBFont_currentIndexChanged(int)));

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

    connect(buttonBox,           SIGNAL(clicked(QAbstractButton*)), SLOT(buttonClicked(QAbstractButton*)));
    connect(headerOddEven,       SIGNAL(toggled(bool)),             SLOT(toggleHeaderOddEven(bool)));
    connect(footerOddEven,       SIGNAL(toggled(bool)),             SLOT(toggleFooterOddEven(bool)));
    connect(chordDescriptionFileButton, SIGNAL(clicked()),          SLOT(selectChordDescriptionFile()));
    connect(chordsStandard,      SIGNAL(toggled(bool)),             SLOT(setChordStyle(bool)));
    connect(chordsJazz,          SIGNAL(toggled(bool)),             SLOT(setChordStyle(bool)));
    connect(chordsCustom,        SIGNAL(toggled(bool)),             SLOT(setChordStyle(bool)));
    connect(chordsXmlFile,       SIGNAL(toggled(bool)),             SLOT(setChordStyle(bool)));
    connect(chordDescriptionFile,&QLineEdit::editingFinished,       [=]() { setChordStyle(true); });

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
            fillComboBoxDirection(cb);
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
    for (auto ss : allTextStyles()) {
        QListWidgetItem* item = new QListWidgetItem(textStyleUserName(ss));
        item->setData(Qt::UserRole, int(ss));
        textStyles->addItem(item);
    }

    textStyleFrameType->clear();
    textStyleFrameType->addItem(tr("None", "no frame for text"), int(FrameType::NO_FRAME));
    textStyleFrameType->addItem(tr("Rectangle"), int(FrameType::SQUARE));
    textStyleFrameType->addItem(tr("Circle"), int(FrameType::CIRCLE));

    resetTextStyleName->setText(RESET_ICON);
    connect(resetTextStyleName, &QToolButton::clicked, [=]() { resetUserStyleName(); });
    connect(styleName, &QLineEdit::textEdited, [=]() { editUserStyleName(); });
    connect(styleName, &QLineEdit::editingFinished, [=]() { endEditUserStyleName(); });

    // font face
    resetTextStyleFontFace->setText(RESET_ICON);
    connect(resetTextStyleFontFace, &QToolButton::clicked,
            [=]() { resetTextStyle(Pid::FONT_FACE); }
            );
    connect(textStyleFontFace, &QFontComboBox::currentFontChanged,
            [=]() { textStyleValueChanged(Pid::FONT_FACE, QVariant(textStyleFontFace->currentFont().family())); }
            );

    // font size
    resetTextStyleFontSize->setText(RESET_ICON);
    connect(resetTextStyleFontSize, &QToolButton::clicked,
            [=]() { resetTextStyle(Pid::FONT_SIZE); }
            );
    connect(textStyleFontSize, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [=]() { textStyleValueChanged(Pid::FONT_SIZE, QVariant(textStyleFontSize->value())); }
            );

    // font style
    resetTextStyleFontStyle->setText(RESET_ICON);
    connect(resetTextStyleFontStyle, &QToolButton::clicked,
            [=]() { resetTextStyle(Pid::FONT_STYLE); }
            );

    connect(textStyleFontStyle, &notation::FontStyleSelect::fontStyleChanged,
            [=]() { textStyleValueChanged(Pid::FONT_STYLE, QVariant(int(textStyleFontStyle->fontStyle()))); }
            );

    // align
    resetTextStyleAlign->setText(RESET_ICON);
    connect(resetTextStyleAlign, &QToolButton::clicked, [=]() { resetTextStyle(Pid::ALIGN); });
    connect(textStyleAlign, &AlignSelect::alignChanged,
            [=]() { textStyleValueChanged(Pid::ALIGN, QVariant::fromValue(textStyleAlign->align())); }
            );

    // offset
    resetTextStyleOffset->setText(RESET_ICON);
    connect(resetTextStyleOffset, &QToolButton::clicked, [=]() { resetTextStyle(Pid::OFFSET); });
    connect(textStyleOffset, &OffsetSelect::offsetChanged,
            [=]() { textStyleValueChanged(Pid::OFFSET, QVariant(textStyleOffset->offset())); }
            );

    // spatium dependent
    resetTextStyleSpatiumDependent->setText(RESET_ICON);
    connect(resetTextStyleSpatiumDependent, &QToolButton::clicked, [=]() {
        resetTextStyle(Pid::SIZE_SPATIUM_DEPENDENT);
    });
    connect(textStyleSpatiumDependent, &QCheckBox::toggled,
            [=]() { textStyleValueChanged(Pid::SIZE_SPATIUM_DEPENDENT, textStyleSpatiumDependent->isChecked()); }
            );

    resetTextStyleFrameType->setText(RESET_ICON);
    connect(resetTextStyleFrameType, &QToolButton::clicked, [=]() { resetTextStyle(Pid::FRAME_TYPE); });
    connect(textStyleFrameType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [=]() { textStyleValueChanged(Pid::FRAME_TYPE, textStyleFrameType->currentIndex()); }
            );

    resetTextStyleFramePadding->setText(RESET_ICON);
    connect(resetTextStyleFramePadding, &QToolButton::clicked, [=]() { resetTextStyle(Pid::FRAME_PADDING); });
    connect(textStyleFramePadding, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [=]() { textStyleValueChanged(Pid::FRAME_PADDING, textStyleFramePadding->value()); }
            );

    resetTextStyleFrameBorder->setText(RESET_ICON);
    connect(resetTextStyleFrameBorder, &QToolButton::clicked, [=]() { resetTextStyle(Pid::FRAME_WIDTH); });
    connect(textStyleFrameBorder, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [=]() { textStyleValueChanged(Pid::FRAME_WIDTH, textStyleFrameBorder->value()); }
            );

    resetTextStyleFrameBorderRadius->setText(RESET_ICON);
    connect(resetTextStyleFrameBorderRadius, &QToolButton::clicked, [=]() { resetTextStyle(Pid::FRAME_ROUND); });
    connect(textStyleFrameBorderRadius, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [=]() { textStyleValueChanged(Pid::FRAME_ROUND, textStyleFrameBorderRadius->value()); }
            );

    resetTextStyleFrameForeground->setText(RESET_ICON);
    connect(resetTextStyleFrameForeground, &QToolButton::clicked, [=]() { resetTextStyle(Pid::FRAME_FG_COLOR); });
    connect(textStyleFrameForeground, &Awl::ColorLabel::colorChanged,
            [=]() { textStyleValueChanged(Pid::FRAME_FG_COLOR, textStyleFrameForeground->color()); }
            );

    resetTextStyleFrameBackground->setText(RESET_ICON);
    connect(resetTextStyleFrameBackground, &QToolButton::clicked, [=]() { resetTextStyle(Pid::FRAME_BG_COLOR); });
    connect(textStyleFrameBackground, &Awl::ColorLabel::colorChanged,
            [=]() { textStyleValueChanged(Pid::FRAME_BG_COLOR, textStyleFrameBackground->color()); }
            );

    resetTextStyleColor->setText(RESET_ICON);
    connect(resetTextStyleColor, &QToolButton::clicked, [=]() { resetTextStyle(Pid::COLOR); });
    connect(textStyleColor, &Awl::ColorLabel::colorChanged,
            [=]() { textStyleValueChanged(Pid::COLOR, textStyleColor->color()); }
            );

    connect(textStyles, SIGNAL(currentRowChanged(int)), SLOT(textStyleChanged(int)));
    textStyles->setCurrentRow(0);

    QRect scr = QGuiApplication::primaryScreen()->availableGeometry();
    QRect dlg = this->frameGeometry();
    isTooBig  = dlg.width() > scr.width() || dlg.height() > scr.height();
    if (isTooBig) {
        this->setMinimumSize(scr.width() / 2, scr.height() / 2);
    }
    hasShown = false;
    WidgetStateStore::restoreGeometry(this);
}

EditStyle::EditStyle(const EditStyle& other)
    : QDialog(other.parentWidget())
{
}

int EditStyle::metaTypeId()
{
    return QMetaType::type("EditStyle");
}

//---------------------------------------------------------
//   showEvent
//---------------------------------------------------------

void EditStyle::showEvent(QShowEvent* ev)
{
    if (!hasShown && isTooBig) {
        // Add scroll bars to pageStack - this cannot be in the constructor
        // or the Header, Footer text input boxes size themselves too large.
        QScrollArea* scrollArea = new QScrollArea(splitter);
        scrollArea->setWidget(pageStack);
        hasShown = true;     // so that it only happens once
    }
    setValues();
    pageList->setFocus();
    globalContext()->currentNotation()->undoStack()->prepareChanges();
    buttonApplyToAllParts->setEnabled(!globalContext()->currentNotation()->style()->canApplyToAllParts());
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
//   buttonClicked
//---------------------------------------------------------

void EditStyle::buttonClicked(QAbstractButton* b)
{
    switch (buttonBox->standardButton(b)) {
    case QDialogButtonBox::Ok:
        done(1);
        globalContext()->currentNotation()->undoStack()->commitChanges();
        break;
    case QDialogButtonBox::Cancel:
        done(0);
        globalContext()->currentNotation()->undoStack()->rollbackChanges();
        break;
    case QDialogButtonBox::NoButton:
    default:
        if (b == buttonApplyToAllParts) {
            globalContext()->currentNotation()->style()->applyToAllParts();
        }
        break;
    }
}

//---------------------------------------------------------
//   On comboFBFont currentIndex changed
//---------------------------------------------------------

void EditStyle::on_comboFBFont_currentIndexChanged(int index)
{
    qreal size, lineHeight;

    if (FiguredBass::fontData(index, 0, 0, &size, &lineHeight)) {
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
//   getValue
//    return current gui value
//---------------------------------------------------------

QVariant EditStyle::getValue(StyleId idx)
{
    const StyleWidget& sw = styleWidget(idx);
    const char* type = styleValue(sw.idx).typeName();

    if (!strcmp("Ms::Spatium", type)) {
        QDoubleSpinBox* sb = qobject_cast<QDoubleSpinBox*>(sw.widget);
        return QVariant(Spatium(sb->value() * (sw.showPercent ? 0.01 : 1.0)));
    } else if (!strcmp("double", type)) {
        QVariant v = sw.widget->property("value");
        if (sw.showPercent) {
            v = v.toDouble() * 0.01;
        }
        return v;
    } else if (!strcmp("bool", type)) {
        return sw.widget->property("checked");
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
                qobject_cast<QSpinBox*>(sw.widget)->setValue(int(val.value<Spatium>().val() * 100.0));
            } else {
                sw.widget->setProperty("value", val);
            }
        } else if (!strcmp("double", type)) {
            if (sw.showPercent) {
                val = QVariant(val.toDouble() * 100);
            }
            sw.widget->setProperty("value", val);
        } else if (!strcmp("bool", type)) {
            sw.widget->setProperty("checked", val);

            if (sw.idx == StyleId::measureNumberSystem && !val.toBool()) {
                showIntervalMeasureNumber->setChecked(true);
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
                fontStyle->setFontStyle(FontStyle(val.toInt()));
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
            }
        } else if (!strcmp("Ms::Direction", type)) {
            QComboBox* cb = qobject_cast<QComboBox*>(sw.widget);
            if (cb) {
                cb->setCurrentIndex(int(val.value<Direction>()));
            }
        } else if (!strcmp("Ms::Align", type)) {
            AlignSelect* as = qobject_cast<AlignSelect*>(sw.widget);
            as->setAlign(val.value<Align>());
        } else if (!strcmp("QPointF", type)) {
            OffsetSelect* as = qobject_cast<OffsetSelect*>(sw.widget);
            if (as) {
                as->setOffset(val.value<QPointF>());
            }
        }

        if (sw.widget) {
            sw.widget->blockSignals(false);
        }
    }

    textStyleChanged(textStyles->currentRow());

    //TODO: convert the rest:

    QString unit(styleValue(StyleId::swingUnit).toString());

    if (unit == TDuration(TDuration::DurationType::V_EIGHTH).name()) {
        swingEighth->setChecked(true);
        swingBox->setEnabled(true);
    } else if (unit == TDuration(TDuration::DurationType::V_16TH).name()) {
        swingSixteenth->setChecked(true);
        swingBox->setEnabled(true);
    } else if (unit == TDuration(TDuration::DurationType::V_ZERO).name()) {
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
    for (const auto& i : ScoreFont::scoreFonts()) {
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
    musicalTextFont->addItem("Bravura Text", "Bravura Text");
    musicalTextFont->addItem("Emmentaler Text", "MScore Text");
    musicalTextFont->addItem("Gonville Text", "Gootville Text");
    musicalTextFont->addItem("MuseJazz Text", "MuseJazz Text");
    QString tfont(styleValue(StyleId::MusicalTextFont).toString());
    idx = musicalTextFont->findData(tfont);
    musicalTextFont->setCurrentIndex(idx);
    musicalTextFont->blockSignals(false);

    toggleHeaderOddEven(styleValue(StyleId::headerOddEven).toBool());
    toggleFooterOddEven(styleValue(StyleId::footerOddEven).toBool());
}

//---------------------------------------------------------
//   selectChordDescriptionFile
//---------------------------------------------------------

void EditStyle::selectChordDescriptionFile()
{
    io::path dir = configuration()->stylesDirPath();
    QString filter = mu::qtrc("notation", "MuseScore Styles") + " (*.mss)";

    mu::io::path path = interactive()->selectOpeningFile(mu::qtrc("notation", "Load Style"), dir, filter);
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
        val = TDuration(TDuration::DurationType::V_ZERO).name();
        swingBox->setEnabled(false);
    } else if (swingEighth->isChecked()) {
        val = TDuration(TDuration::DurationType::V_EIGHTH).name();
        swingBox->setEnabled(true);
    } else if (swingSixteenth->isChecked()) {
        val = TDuration(TDuration::DurationType::V_16TH).name();
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
//
//    Ensure lyricsDashMinLength <= lyricsDashMaxLength
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
//
//    Ensure minSystemDistance <= maxSystemDistance
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
        ScoreFont* scoreFont = ScoreFont::fontFactory(val.toString());
        if (scoreFont) {
            for (auto j : scoreFont->engravingDefaults()) {
                setStyleValue(j.first, j.second);
            }

            // fix values, the distances are defined different in MuseScore
            double barWidth = styleValue(StyleId::barWidth).toDouble() + styleValue(StyleId::endBarWidth).toDouble() * .5;

            double newEndBarDistance = styleValue(StyleId::endBarDistance).toDouble() + barWidth;
            setStyleValue(StyleId::endBarDistance, newEndBarDistance);

            double newDoubleBarDistance = styleValue(StyleId::doubleBarDistance).toDouble() + barWidth;
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
    Tid tid = Tid(textStyles->item(row)->data(Qt::UserRole).toInt());
    const TextStyle* ts = textStyle(tid);

    for (const StyledProperty& a : *ts) {
        switch (a.pid) {
        case Pid::FONT_FACE: {
            QVariant val = styleValue(a.sid);
            textStyleFontFace->setCurrentFont(QFont(val.toString()));
            resetTextStyleFontFace->setEnabled(val != defaultStyleValue(a.sid));
        }
        break;

        case Pid::FONT_SIZE:
            textStyleFontSize->setValue(styleValue(a.sid).toDouble());
            resetTextStyleFontSize->setEnabled(styleValue(a.sid) != defaultStyleValue(a.sid));
            break;

        case Pid::FONT_STYLE:
            textStyleFontStyle->setFontStyle(FontStyle(styleValue(a.sid).toInt()));
            resetTextStyleFontStyle->setEnabled(styleValue(a.sid) != defaultStyleValue(a.sid));
            break;

        case Pid::ALIGN:
            textStyleAlign->setAlign(styleValue(a.sid).value<Align>());
            resetTextStyleAlign->setEnabled(styleValue(a.sid) != defaultStyleValue(a.sid));
            break;

        case Pid::OFFSET:
            textStyleOffset->setOffset(styleValue(a.sid).toPointF());
            resetTextStyleOffset->setEnabled(styleValue(a.sid) != defaultStyleValue(a.sid));
            break;

        case Pid::SIZE_SPATIUM_DEPENDENT: {
            QVariant val = styleValue(a.sid);
            textStyleSpatiumDependent->setChecked(val.toBool());
            resetTextStyleSpatiumDependent->setEnabled(val != defaultStyleValue(a.sid));
            textStyleOffset->setSuffix(val.toBool() ? tr("sp") : tr("mm"));
        }
        break;

        case Pid::FRAME_TYPE:
            textStyleFrameType->setCurrentIndex(styleValue(a.sid).toInt());
            resetTextStyleFrameType->setEnabled(styleValue(a.sid) != defaultStyleValue(a.sid));
            frameWidget->setEnabled(styleValue(a.sid).toInt() != 0);             // disable if no frame
            break;

        case Pid::FRAME_PADDING:
            textStyleFramePadding->setValue(styleValue(a.sid).toDouble());
            resetTextStyleFramePadding->setEnabled(styleValue(a.sid) != defaultStyleValue(a.sid));
            break;

        case Pid::FRAME_WIDTH:
            textStyleFrameBorder->setValue(styleValue(a.sid).toDouble());
            resetTextStyleFrameBorder->setEnabled(styleValue(a.sid) != defaultStyleValue(a.sid));
            break;

        case Pid::FRAME_ROUND:
            textStyleFrameBorderRadius->setValue(styleValue(a.sid).toDouble());
            resetTextStyleFrameBorderRadius->setEnabled(styleValue(a.sid) != defaultStyleValue(a.sid));
            break;

        case Pid::FRAME_FG_COLOR:
            textStyleFrameForeground->setColor(styleValue(a.sid).value<QColor>());
            resetTextStyleFrameForeground->setEnabled(styleValue(a.sid) != defaultStyleValue(a.sid));
            break;

        case Pid::FRAME_BG_COLOR:
            textStyleFrameBackground->setColor(styleValue(a.sid).value<QColor>());
            resetTextStyleFrameBackground->setEnabled(styleValue(a.sid) != defaultStyleValue(a.sid));
            break;

        case Pid::COLOR:
            textStyleColor->setColor(styleValue(a.sid).value<QColor>());
            resetTextStyleColor->setEnabled(styleValue(a.sid) != defaultStyleValue(a.sid));
            break;

        default:
            break;
        }
    }

    styleName->setText(textStyleUserName(tid));
    styleName->setEnabled(int(tid) >= int(Tid::USER1));
    resetTextStyleName->setEnabled(styleName->text() != textStyleUserName(tid));
}

//---------------------------------------------------------
//   textStyleValueChanged
//---------------------------------------------------------

void EditStyle::textStyleValueChanged(Pid pid, QVariant value)
{
    Tid tid = Tid(textStyles->item(textStyles->currentRow())->data(Qt::UserRole).toInt());
    const TextStyle* ts = textStyle(tid);

    for (const StyledProperty& a : *ts) {
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

void EditStyle::resetTextStyle(Pid pid)
{
    Tid tid = Tid(textStyles->item(textStyles->currentRow())->data(Qt::UserRole).toInt());
    const TextStyle* ts = textStyle(tid);

    for (const StyledProperty& a : *ts) {
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
    Tid tid = Tid(textStyles->item(row)->data(Qt::UserRole).toInt());
    textStyles->item(row)->setText(styleName->text());
    resetTextStyleName->setEnabled(styleName->text() != textStyleUserName(tid));
}

//---------------------------------------------------------
//   endEditUserStyleName
//---------------------------------------------------------

void EditStyle::endEditUserStyleName()
{
    int row = textStyles->currentRow();
    Tid tid = Tid(textStyles->item(row)->data(Qt::UserRole).toInt());
    int idx = int(tid) - int(Tid::USER1);
    if ((idx < 0) || (idx > 5)) {
        qDebug("User style index %d outside of range [0,5].", idx);
        return;
    }
    StyleId sid[]
        = { StyleId::user1Name, StyleId::user2Name, StyleId::user3Name, StyleId::user4Name, StyleId::user5Name, StyleId::user6Name };
    QString name = styleName->text();
    setStyleValue(sid[idx], name);
    if (name == "") {
        name = textStyleUserName(tid);
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
