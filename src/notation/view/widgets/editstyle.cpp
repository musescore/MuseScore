/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "editstyle.h"

#include <QButtonGroup>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickView>
#include <QSignalMapper>

#include "translation.h"
#include "types/translatablestring.h"

#include "alignSelect.h"
#include "colorlabel.h"
#include "fontStyleSelect.h"
#include "offsetSelect.h"

#include "engraving/dom/figuredbass.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/realizedharmony.h"
#include "engraving/dom/stafftype.h"
#include "engraving/dom/text.h"
#include "engraving/style/textstyle.h"
#include "engraving/types/symnames.h"
#include "engraving/types/typesconv.h"
#include "engraving/types/types.h"

#include "ui/view/widgetstatestore.h"
#include "ui/view/widgetutils.h"

#include "defer.h"
#include "log.h"

using namespace mu;
using namespace mu::notation;
using namespace mu::engraving;
using namespace muse::ui;

static const QStringList ALL_PAGE_CODES {
    "score",
    "page",
    "sizes",
    "header-and-footer",
    "measure-number",
    "system",
    "clefs-key-and-time-signatures",
    "accidentals",
    "measure",
    "barlines",
    "notes",
    "rests",
    "measure-repeats",
    "beams",
    "tuplets",
    "arpeggios",
    "slurs-and-ties",
    "dynamics-hairpins",
    "volta",
    "ottava",
    "pedal",
    "trill",
    "vibrato",
    "glissando-note-line",
    "bend",
    "text-line",
    "system-text-line",
    "articulations-and-ornaments",
    "fermatas",
    "staff-text",
    "tempo-text",
    "lyrics",
    "expression",
    "rehearsal-marks",
    "figured-bass",
    "chord-symbols",
    "fretboard-diagrams",
    "tablature-styles",
    "text-styles"
};

static const QStringList ALL_TEXT_STYLE_SUBPAGE_CODES {
    "title",
    "subtitle",
    "composer",
    "poet",
    "translator",
    "frame",
    "instrument-name-part",
    "instrument-name-long",
    "instrument-name-short",
    "instrument-change",
    "header",
    "footer",
    "copyright",
    "page-number",
    "measure-number",
    "multimeasure-rest-range",
    "tempo",
    "tempo-change",
    "metronome",
    "repeat-text-left",
    "repeat-text-right",
    "rehearsal-mark",
    "system",
    "staff",
    "expression",
    "hairpin",
    "lyrics-odd-lines",
    "lyrics-even-lines",
    "chord-symbols",
    "chord-symbols-alternate",
    "roman-numeral-analysis",
    "nashville-number",
    "tuplet",
    "sticking",
    "fingering",
    "lh-guitar-fingering",
    "rh-guitar-fingering",
    "string-number",
    "string-tunings",
    "fretboard-diagram-fingering",
    "fretboard-diagram-fret-number",
    "harp-pedal-diagram",
    "harp-pedal-text-diagram",
    "text-line",
    "note-line",
    "volta",
    "ottava",
    "glissando",
    "pedal",
    "bend",
    "let-ring",
    "palm-mute",
    "user1",
    "user2",
    "user3",
    "user4",
    "user5",
    "user6",
    "user7",
    "user8",
    "user9",
    "user10",
    "user11",
    "user12"
};

class EditStyle::LineStyleSelect : public QObject
{
public:
    LineStyleSelect(QObject* parent, QComboBox* lineStyleComboBox, const QList<QWidget*>& dashSpecificWidgets)
        : QObject(parent), lineStyleComboBox(lineStyleComboBox), dashSpecificWidgets(dashSpecificWidgets)
    {
        connect(lineStyleComboBox, &QComboBox::currentIndexChanged, this, &LineStyleSelect::update);
    }

    void update() const
    {
        bool isDash = lineStyleComboBox->currentIndex() == int(LineType::DASHED);

        for (QWidget* w : dashSpecificWidgets) {
            w->setVisible(isDash);
        }
    }

    QComboBox* lineStyleComboBox = nullptr;
    QList<QWidget*> dashSpecificWidgets;
};

static const muse::TranslatableString lineStyles[] = {
    //: line style
    muse::TranslatableString("notation/editstyle", "Continuous"),
    //: line style
    muse::TranslatableString("notation/editstyle", "Dashed"),
    //: line style
    muse::TranslatableString("notation/editstyle", "Dotted"),
};

static void fillDirectionComboBox(QComboBox* comboBox)
{
    comboBox->clear();
    comboBox->addItem(TConv::translatedUserName(DirectionV::AUTO), static_cast<int>(DirectionV::AUTO));
    comboBox->addItem(TConv::translatedUserName(DirectionV::UP),   static_cast<int>(DirectionV::UP));
    comboBox->addItem(TConv::translatedUserName(DirectionV::DOWN), static_cast<int>(DirectionV::DOWN));
}

static void fillDynamicHairpinComboBox(QComboBox* comboBox)
{
    comboBox->clear();
    comboBox->addItem(muse::qtrc("notation/editstyle", "Based on voice"), int(DirectionV::AUTO));
    comboBox->addItem(muse::qtrc("notation/editstyle", "Above"), int(DirectionV::UP));
    comboBox->addItem(muse::qtrc("notation/editstyle", "Below"), int(DirectionV::DOWN));
}

//---------------------------------------------------------
//   EditStyle
//---------------------------------------------------------

EditStyle::EditStyle(QWidget* parent)
    : QDialog(parent), muse::Injectable(muse::iocCtxForQWidget(this))
{
    //! NOTE: suppress all accessibility events causing a long delay when opening the dialog (massive spam from setupUi)
    accessibilityController()->setIgnoreQtAccessibilityEvents(true);
    DEFER {
        accessibilityController()->setIgnoreQtAccessibilityEvents(false);
    };

    setObjectName("EditStyle");
    setupUi(this);
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    setModal(true);

    buttonApplyToAllParts = buttonBox->addButton(muse::qtrc("notation/editstyle", "Apply to all parts"), QDialogButtonBox::ApplyRole);
    WidgetUtils::setWidgetIcon(buttonTogglePagelist, IconCode::Code::ARROW_RIGHT);

    // ====================================================
    // Button Groups
    // ====================================================

    // create button groups for every set of radio button widgets
    // use this group widgets in list styleWidgets
    // This works for groups which represent an int enumeration.

    QButtonGroup* ksng = new QButtonGroup(this);
    ksng->addButton(radioKeySigNatNone, int(KeySigNatural::NONE));
    ksng->addButton(radioKeySigNatBefore, int(KeySigNatural::BEFORE));
    ksng->addButton(radioKeySigNatAfter, int(KeySigNatural::AFTER));

    QButtonGroup* ksbl = new QButtonGroup(this);
    ksbl->addButton(radioKeySigCourtesyBarlineAlwaysSingle, int(CourtesyBarlineMode::ALWAYS_SINGLE));
    ksbl->addButton(radioKeySigCourtesyBarlineAlwaysDouble, int(CourtesyBarlineMode::ALWAYS_DOUBLE));
    ksbl->addButton(radioKeySigCourtesyBarlineDoubleBeforeNewSystem, int(CourtesyBarlineMode::DOUBLE_BEFORE_COURTESY));

    QButtonGroup* tsbl = new QButtonGroup(this);
    tsbl->addButton(radioTimeSigCourtesyBarlineAlwaysSingle, int(CourtesyBarlineMode::ALWAYS_SINGLE));
    tsbl->addButton(radioTimeSigCourtesyBarlineAlwaysDouble, int(CourtesyBarlineMode::ALWAYS_DOUBLE));
    tsbl->addButton(radioTimeSigCourtesyBarlineDoubleBeforeNewSystem, int(CourtesyBarlineMode::DOUBLE_BEFORE_COURTESY));

    QButtonGroup* ctg = new QButtonGroup(this);
    ctg->addButton(clefTab1, int(ClefType::TAB));
    ctg->addButton(clefTab2, int(ClefType::TAB_SERIF));

    QButtonGroup* fbAlign = new QButtonGroup(this);
    fbAlign->addButton(radioFBTop, 0);
    fbAlign->addButton(radioFBBottom, 1);

    QButtonGroup* fbStyle = new QButtonGroup(this);
    fbStyle->addButton(radioFBModern, 0);
    fbStyle->addButton(radioFBHistoric, 1);

    QButtonGroup* articulationStemSide = new QButtonGroup(this);
    articulationStemSide->addButton(radioArticAlignStem, int(ArticulationStemSideAlign::STEM));
    articulationStemSide->addButton(radioArticAlignNoteHead, int(ArticulationStemSideAlign::NOTEHEAD));
    articulationStemSide->addButton(radioArticAlignCenter, int(ArticulationStemSideAlign::AVERAGE));

    QButtonGroup* articulationKeepTogether = new QButtonGroup(this);
    articulationKeepTogether->addButton(radioArticKeepTogether, 1);
    articulationKeepTogether->addButton(radioArticAllowSeparate, 0);

    QButtonGroup* trillAlwaysShowCueNote = new QButtonGroup(this);
    trillAlwaysShowCueNote->addButton(ornamentShowCueNoteAlways, 1);
    trillAlwaysShowCueNote->addButton(ornamentShowCueNoteOnlyNonSeconds, 0);

    QButtonGroup* tabShowTiedFrets = new QButtonGroup(this);
    tabShowTiedFrets->addButton(tabShowTiesAndFret, int(ShowTiedFret::TIE_AND_FRET));
    tabShowTiedFrets->addButton(tabShowTies, int(ShowTiedFret::TIE));
    tabShowTiedFrets->addButton(tabShowNone, int(ShowTiedFret::NONE));

    QButtonGroup* tabParenthFrets = new QButtonGroup(this);
    tabParenthFrets->addButton(tabParenthSystem, int(ParenthesizeTiedFret::START_OF_SYSTEM));
    tabParenthFrets->addButton(tabParenthMeasure, int(ParenthesizeTiedFret::START_OF_MEASURE));
    tabParenthFrets->addButton(tabParenthNone, int(ParenthesizeTiedFret::NEVER));

    void (QButtonGroup::* tabShowTiedFretsButtonClicked)(QAbstractButton*) = &QButtonGroup::buttonClicked;
    connect(tabShowTiedFrets, tabShowTiedFretsButtonClicked, this, [this](QAbstractButton*){
        updateParenthesisIndicatingTiesGroupState();
    });

    QButtonGroup* clefVisibility = new QButtonGroup(this);
    clefVisibility->addButton(radioShowAllClefs, true);
    clefVisibility->addButton(radioHideClefs, false);

    QButtonGroup* keysigVisibility = new QButtonGroup(this);
    keysigVisibility->addButton(radioShowAllKeys, true);
    keysigVisibility->addButton(radioHideKeys, false);

    QButtonGroup* firstSysLabels = new QButtonGroup(this);
    firstSysLabels->addButton(firstLongBtn, int(InstrumentLabelVisibility::LONG));
    firstSysLabels->addButton(firstShortBtn, int(InstrumentLabelVisibility::SHORT));
    firstSysLabels->addButton(firstHideBtn, int(InstrumentLabelVisibility::HIDE));

    QButtonGroup* subsSysLabels = new QButtonGroup(this);
    subsSysLabels->addButton(subsLongBtn, int(InstrumentLabelVisibility::LONG));
    subsSysLabels->addButton(subsShortBtn, int(InstrumentLabelVisibility::SHORT));
    subsSysLabels->addButton(subsHideBtn, int(InstrumentLabelVisibility::HIDE));

    QButtonGroup* singleMeasureMMRest = new QButtonGroup(this);
    singleMeasureMMRest->addButton(mmRestSingleUseHBar, 0);
    singleMeasureMMRest->addButton(mmRestSingleUseNormalRest, 1);

    QButtonGroup* mmRestConstantWidth = new QButtonGroup(this);
    mmRestConstantWidth->addButton(mmRestWidthProportional, 0);
    mmRestConstantWidth->addButton(mmRestWidthConstant, 1);

    // ====================================================
    // Style widgets
    // ====================================================

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
        { StyleId::voltaDashLineLen,        false, voltaLineStyleDashSize,  resetVoltaLineStyleDashSize },
        { StyleId::voltaDashGapLen,         false, voltaLineStyleGapSize,   resetVoltaLineStyleGapSize },
        { StyleId::ottavaLineStyle,         false, ottavaLineStyle,         resetOttavaLineStyle },
        { StyleId::ottavaDashLineLen,       false, ottavaLineStyleDashSize, resetOttavaLineStyleDashSize },
        { StyleId::ottavaDashGapLen,        false, ottavaLineStyleGapSize,  resetOttavaLineStyleGapSize },
        { StyleId::pedalLineStyle,          false, pedalLineStyle,          resetPedalLineStyle },
        { StyleId::pedalDashLineLen,        false, pedalLineStyleDashSize,  resetPedalLineStyleDashSize },
        { StyleId::pedalDashGapLen,         false, pedalLineStyleGapSize,   resetPedalLineStyleGapSize },

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
        { StyleId::lyricsPosAbove,          false, yLyricsPosAbove,         resetLyricsPosAbove },
        { StyleId::lyricsPosBelow,          false, yLyricsPosBelow,         resetLyricsPosBelow },
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
        { StyleId::lyricsDashPad,           false, lyricsDashPad,           resetLyricsDashPad },
        { StyleId::lyricsDashLineThickness, false, lyricsDashLineThickness, resetLyricsDashLineThickness },
        { StyleId::lyricsDashYposRatio,     false, lyricsDashYposRatio,     resetLyricsDashYposRatio },
        { StyleId::lyricsShowDashIfSyllableOnFirstNote, false, lyricsShowDashIfSyllableOnFirstNote,
          resetLyricsShowDashIfSyllableOnFirstNote },
        { StyleId::lyricsMelismaMinLength,  false, minMelismaLength,     resetMinMelismaLength },
        { StyleId::lyricsMelismaForce,      false, lyricsMelismaForce,   resetLyricsMelismaForce },
        { StyleId::lyricsDashPosAtStartOfSystem, false, lyricsDashStartSystemPlacement, resetLyricsDashStartSystemPlacement },

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
        { StyleId::singleMeasureMMRestUseNormalRest, false, singleMeasureMMRest, 0 },
        { StyleId::singleMeasureMMRestShowNumber,    false, singleMMRestShowNumber, 0 },
        { StyleId::mmRestConstantWidth,     false, mmRestConstantWidth, 0 },
        { StyleId::mmRestReferenceWidth,    false, mmRestRefDuration, resetMMRestRefDuration },
        { StyleId::mmRestMaxWidthIncrease,  false, mmRestMaxMeasures, mmRestMaxMeasuresReset },
        { StyleId::minMMRestWidth,          false, minMeasureWidth,         resetMinMMRestWidth },
        { StyleId::mmRestNumberPos,         false, mmRestNumberPos,         resetMMRestNumberPos },
        { StyleId::mmRestBetweenStaves,     false, placeBetweenStaves,      resetPlaceBetweenStaves },
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
        { StyleId::firstSystemInstNameVisibility, false, firstSysLabels, resetFirstSystemLabel },
        { StyleId::subsSystemInstNameVisibility, false, subsSysLabels, resetSubsSystemLabel },
        { StyleId::accidentalNoteDistance,  false, accidentalNoteDistance,  0 },
        { StyleId::accidentalDistance,      false, accidentalDistance,      0 },
        { StyleId::bracketedAccidentalPadding, false, accidentalsBracketsBadding, resetAccidentalsBracketPadding },
        { StyleId::minNoteDistance,         false, minNoteDistance,         resetMinNoteDistance },
        { StyleId::barNoteDistance,         false, barNoteDistance,         resetBarNoteDistance },
        { StyleId::barAccidentalDistance,   false, barAccidentalDistance,   resetBarAccidentalDistance },
        { StyleId::noteBarDistance,         false, noteBarDistance,         resetNoteBarDistance },
        { StyleId::clefLeftMargin,          false, clefLeftMargin,          resetClefLeftMargin },
        { StyleId::keysigLeftMargin,        false, keysigLeftMargin,        resetKeysigLeftMargin },
        { StyleId::timesigLeftMargin,       false, timesigLeftMargin,       resetTimesigLeftMargin },
        { StyleId::clefKeyRightMargin,      false, clefKeyRightMargin,      resetClefKeyRightMargin },
        { StyleId::clefKeyDistance,         false, clefKeyDistance,         resetClefKeyDistance },
        { StyleId::clefTimesigDistance,     false, clefTimesigDistance,     resetClefTimesigDistance },
        { StyleId::keyTimesigDistance,      false, keyTimesigDistance,      resetKeyTimesigDistance },
        { StyleId::keyBarlineDistance,      false, keyBarlineDistance,      resetKeyBarlineDistance },
        { StyleId::systemHeaderDistance,    false, systemHeaderDistance,    resetSystemHeaderDistance },
        { StyleId::systemHeaderTimeSigDistance, false, systemHeaderTimeSigDistance, resetSystemHeaderTimeSigDistance },

        { StyleId::clefBarlineDistance,     false, clefBarlineDistance,     resetClefBarlineDistance },
        { StyleId::timesigBarlineDistance,  false, timesigBarlineDistance,  resetTimesigBarlineDistance },
        { StyleId::staffLineWidth,          false, staffLineWidth,          resetStaffLineWidth },

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
        { StyleId::shortestStem,            false, shortestStem,            0 },
        { StyleId::combineVoice,            false, combineVoices,           resetCombineVoices },
        { StyleId::arpeggioNoteDistance,    false, arpeggioNoteDistance,    0 },
        { StyleId::arpeggioLineWidth,       false, arpeggioLineWidth,       0 },
        { StyleId::arpeggioHookLen,         false, arpeggioHookLen,         0 },
        { StyleId::arpeggioHiddenInStdIfTab, false, arpeggioHiddenInStdIfTab, 0 },
        { StyleId::slurEndWidth,            false, slurEndLineWidth,        resetSlurEndLineWidth },
        { StyleId::slurMidWidth,            false, slurMidLineWidth,        resetSlurMidLineWidth },
        { StyleId::slurDottedWidth,         false, slurDottedLineWidth,     resetSlurDottedLineWidth },
        { StyleId::slurMinDistance,         false, slurMinDistance,         resetSlurMinDistance },
        { StyleId::tieEndWidth,             false, tieEndLineWidth,         resetTieEndLineWidth },
        { StyleId::tieMidWidth,             false, tieMidLineWidth,         resetTieMidLineWidth },
        { StyleId::tieDottedWidth,          false, tieDottedLineWidth,      resetTieDottedLineWidth },
        { StyleId::tieMinDistance,          false, tieMinDistance,          resetTieMinDistance },
        { StyleId::minTieLength,            false, minTieLength,            resetMinTieLength },
        { StyleId::minHangingTieLength,     false, minHangingTieLength,     resetMinHangingTieLength },

        { StyleId::minLaissezVibLength,            false, minLaissezVibLength,            resetMinLaissezVibLength },
        { StyleId::laissezVibUseSmuflSym,          false, laissezVibUseSmufl,            0 },

        { StyleId::bracketWidth,            false, bracketWidth,            resetBracketThickness },
        { StyleId::bracketDistance,         false, bracketDistance,         resetBracketDistance },
        { StyleId::akkoladeWidth,           false, akkoladeWidth,           resetBraceThickness },
        { StyleId::akkoladeBarDistance,     false, akkoladeBarDistance,     resetBraceDistance },
        { StyleId::dividerLeft,             false, dividerLeft,             0 },
        { StyleId::dividerLeftX,            false, dividerLeftX,            0 },
        { StyleId::dividerLeftY,            false, dividerLeftY,            0 },
        { StyleId::dividerRight,            false, dividerRight,            0 },
        { StyleId::dividerRightX,           false, dividerRightX,           0 },
        { StyleId::dividerRightY,           false, dividerRightY,           0 },
        { StyleId::articulationMinDistance, false, articMinVerticalDist,    resetArticMinVerticalDist },
        { StyleId::propertyDistanceHead,    false, articNoteHeadDist,       resetArticNoteHeadDist },
        { StyleId::propertyDistanceStem,    false, articStemDist,           resetArticStemDist },
        { StyleId::propertyDistance,        false, articStaffDist,          resetArticStaffDist },
        { StyleId::articulationStemHAlign,  false, articulationStemSide,    0 },
        { StyleId::articulationKeepTogether, false, articulationKeepTogether, 0 },
        { StyleId::trillAlwaysShowCueNote, false, trillAlwaysShowCueNote, 0 },
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
        { StyleId::tupletOutOfStaff,        false, tupletOutOfStaff,        0 },
        { StyleId::tupletUseSymbols,        false, tupletUseSymbols,        resetTupletUseSymbols },

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

        { StyleId::graceNoteMag,             true,  graceNoteSize,                resetGraceNoteSize },
        { StyleId::smallStaffMag,            true,  smallStaffSize,               resetSmallStaffSize },
        { StyleId::smallNoteMag,             true,  smallNoteSize,                resetSmallNoteSize },
        { StyleId::scaleRythmicSpacingForSmallNotes, true, reduceRythmicSpacing, 0 },
        { StyleId::smallClefMag,             true,  smallClefSize,                resetSmallClefSize },
        { StyleId::lastSystemFillLimit,      true,  lastSystemFillThreshold,      resetLastSystemFillThreshold },
        { StyleId::hideTabClefAfterFirst,    false, hideTabClefs,                 0 },
        { StyleId::genClef,                  false, clefVisibility,               0 },
        { StyleId::genKeysig,                false, keysigVisibility,             0 },
        { StyleId::genCourtesyTimesig,       false, genCourtesyTimesig,           0 },
        { StyleId::genCourtesyKeysig,        false, genCourtesyKeysig,            0 },
        { StyleId::genCourtesyClef,          false, genCourtesyClef,              0 },
        { StyleId::keySigCourtesyBarlineMode, false, ksbl,                        0 },
        { StyleId::timeSigCourtesyBarlineMode, false, tsbl,                       0 },
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
        { StyleId::scaleBarlines,            false, scaleBarlines,                resetScaleBarlines },
        { StyleId::crossMeasureValues,       false, crossMeasureValues,           0 },

        { StyleId::musicalSymbolFont,        false, musicalSymbolFont,            0 },
        { StyleId::musicalTextFont,          false, musicalTextFont,              0 },
        { StyleId::autoplaceHairpinDynamicsDistance, false, autoplaceHairpinDynamicsDistance,
          resetAutoplaceHairpinDynamicsDistance },

        { StyleId::dynamicsPosAbove,        false, dynamicsPosAbove,           resetDynamicsPosAbove },
        { StyleId::dynamicsPosBelow,        false, dynamicsPosBelow,           resetDynamicsPosBelow },
        { StyleId::dynamicsMinDistance,     false, dynamicsMinDistance,        resetDynamicsMinDistance },
        { StyleId::avoidBarLines,           false, avoidBarLines,              resetAvoidBarLines },
        { StyleId::snapToDynamics,          false, snapExpression,             resetSnapExpression },
        { StyleId::dynamicsSize,            true,  dynamicsSize,               resetDynamicsSize },
        { StyleId::dynamicsOverrideFont,    false, dynamicsOverrideFont,       0 },
        { StyleId::dynamicsFont,            false, dynamicsFont,               0 },

        { StyleId::dynamicsHairpinVoiceBasedPlacement, false, dynamicsAndHairpinPos, resetDynamicsAndHairpinPos },
        { StyleId::dynamicsHairpinsAutoCenterOnGrandStaff, false, dynamicsAndHairpinsCenterOnGrandStaff, 0 },
        { StyleId::dynamicsHairpinsAboveForVocalStaves, false, dynamicsAndHairpinsAboveOnVocalStaves, 0 },

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

        { StyleId::guitarBendLineWidth,     false, bendLineWidth,     resetBendLineWidth },
        { StyleId::guitarBendLineWidthTab,  false, bendLineWidthTab,  resetBendLineWidthTab },
        { StyleId::guitarBendArrowWidth,    false, bendArrowWidth,    resetBendArrowWidth },
        { StyleId::guitarBendArrowHeight,   false, bendArrowHeight,   resetBendArrowHeight },
        { StyleId::useCueSizeFretForGraceBends, false, guitarBendCueSizedGraceFrets, 0 },

        /// Tablature styles

        { StyleId::slurShowTabSimple, false, slurShowTabSimple, 0 },
        { StyleId::slurShowTabCommon, false, slurShowTabCommon, 0 },
        { StyleId::fermataShowTabSimple, false, fermataShowTabSimple, 0 },
        { StyleId::fermataShowTabCommon, false, fermataShowTabCommon, 0 },
        { StyleId::dynamicsShowTabSimple, false, dynamicsShowTabSimple, 0 },
        { StyleId::dynamicsShowTabCommon, false, dynamicsShowTabCommon, 0 },
        { StyleId::hairpinShowTabSimple, false, hairpinShowTabSimple, 0 },
        { StyleId::hairpinShowTabCommon, false, hairpinShowTabCommon, 0 },
        { StyleId::accentShowTabSimple, false, accentShowTabSimple, 0 },
        { StyleId::accentShowTabCommon, false, accentShowTabCommon, 0 },
        { StyleId::staccatoShowTabSimple, false, staccatoShowTabSimple, 0 },
        { StyleId::staccatoShowTabCommon, false, staccatoShowTabCommon, 0 },
        { StyleId::harmonicMarkShowTabSimple, false, harmonicMarkShowTabSimple, 0 },
        { StyleId::harmonicMarkShowTabCommon, false, harmonicMarkShowTabCommon, 0 },
        { StyleId::letRingShowTabSimple, false, letRingShowTabSimple, 0 },
        { StyleId::letRingShowTabCommon, false, letRingShowTabCommon, 0 },
        { StyleId::palmMuteShowTabSimple, false, palmMuteShowTabSimple, 0 },
        { StyleId::palmMuteShowTabCommon, false, palmMuteShowTabCommon, 0 },
        { StyleId::rasgueadoShowTabSimple, false, rasgueadoShowTabSimple, 0 },
        { StyleId::rasgueadoShowTabCommon, false, rasgueadoShowTabCommon, 0 },
        { StyleId::mordentShowTabSimple, false, mordentShowTabSimple, 0 },
        { StyleId::mordentShowTabCommon, false, mordentShowTabCommon, 0 },
        { StyleId::turnShowTabSimple, false, turnShowTabSimple, 0 },
        { StyleId::turnShowTabCommon, false, turnShowTabCommon, 0 },
        { StyleId::wahShowTabSimple, false, wahShowTabSimple, 0 },
        { StyleId::wahShowTabCommon, false, wahShowTabCommon, 0 },
        { StyleId::golpeShowTabSimple, false, golpeShowTabSimple, 0 },
        { StyleId::golpeShowTabCommon, false, golpeShowTabCommon, 0 },

        { StyleId::tabShowTiedFret, false, tabShowTiedFrets, 0 },
        { StyleId::tabParenthesizeTiedFret, false, tabParenthFrets, 0 },
        { StyleId::parenthesizeTiedFretIfArticulation, false, tabParenthArticulation, 0 },
    };

    // ====================================================
    // Combo Boxes
    // ====================================================

    m_lineStyleSelects = {
        new LineStyleSelect(this, voltaLineStyle, {
            label_volta_lineStyle_dashSize,
            voltaLineStyleDashSize,
            resetVoltaLineStyleDashSize,
            label_volta_lineStyle_gapSize,
            voltaLineStyleGapSize,
            resetVoltaLineStyleGapSize
        }),

        new LineStyleSelect(this, ottavaLineStyle, {
            label_ottava_lineStyle_dashSize,
            ottavaLineStyleDashSize,
            resetOttavaLineStyleDashSize,
            label_ottava_lineStyle_gapSize,
            ottavaLineStyleGapSize,
            resetOttavaLineStyleGapSize
        }),

        new LineStyleSelect(this, pedalLineStyle, {
            label_pedalLine_lineStyle_dashSize,
            pedalLineStyleDashSize,
            resetPedalLineStyleDashSize,
            label_pedalLine_lineStyle_gapSize,
            pedalLineStyleGapSize,
            resetPedalLineStyleGapSize
        })
    };

    for (const LineStyleSelect* lineStyleSelect : m_lineStyleSelects) {
        lineStyleSelect->lineStyleComboBox->clear();

        int idx = 0;
        for (const muse::TranslatableString& lineStyle : lineStyles) {
            lineStyleSelect->lineStyleComboBox->addItem(lineStyle.qTranslated(), idx);
            ++idx;
        }
    }

    verticalPlacementComboBoxes = {
        lyricsPlacement,
        textLinePlacement,
        systemTextLinePlacement,
        pedalLinePlacement,
        trillLinePlacement,
        vibratoLinePlacement,
        tempoTextPlacement,
        staffTextPlacement,
        rehearsalMarkPlacement,
        measureNumberVPlacement,
        mmRestRangeVPlacement
    };

    for (QComboBox* cb : verticalPlacementComboBoxes) {
        cb->clear();
        cb->addItem(muse::qtrc("notation/editstyle", "Above"), int(PlacementV::ABOVE));
        cb->addItem(muse::qtrc("notation/editstyle", "Below"), int(PlacementV::BELOW));
    }

    horizontalPlacementComboBoxes = {
        measureNumberHPlacement,
        mmRestRangeHPlacement
    };

    for (QComboBox* cb : horizontalPlacementComboBoxes) {
        cb->clear();
        cb->addItem(muse::qtrc("notation/editstyle", "Left"),   int(PlacementH::LEFT));
        cb->addItem(muse::qtrc("notation/editstyle", "Center"), int(PlacementH::CENTER));
        cb->addItem(muse::qtrc("notation/editstyle", "Right"),  int(PlacementH::RIGHT));
    }

    mmRestRangeBracketType->clear();
    mmRestRangeBracketType->addItem(muse::qtrc("notation/editstyle", "None"),        int(MMRestRangeBracketType::NONE));
    mmRestRangeBracketType->addItem(muse::qtrc("notation/editstyle", "Brackets"),    int(MMRestRangeBracketType::BRACKETS));
    mmRestRangeBracketType->addItem(muse::qtrc("notation/editstyle", "Parentheses"), int(MMRestRangeBracketType::PARENTHESES));

    autoplaceVerticalAlignRange->clear();
    autoplaceVerticalAlignRange->addItem(muse::qtrc("notation/editstyle", "Segment"), int(VerticalAlignRange::SEGMENT));
    autoplaceVerticalAlignRange->addItem(muse::qtrc("notation/editstyle", "Measure"), int(VerticalAlignRange::MEASURE));
    autoplaceVerticalAlignRange->addItem(muse::qtrc("notation/editstyle", "System"),  int(VerticalAlignRange::SYSTEM));

    tupletNumberType->clear();
    tupletNumberType->addItem(muse::qtrc("notation/editstyle", "Number"), int(TupletNumberType::SHOW_NUMBER));
    tupletNumberType->addItem(muse::qtrc("notation/editstyle", "Ratio"), int(TupletNumberType::SHOW_RELATION));
    tupletNumberType->addItem(muse::qtrc("notation/editstyle", "None", "no tuplet number type"), int(TupletNumberType::NO_TEXT));

    tupletBracketType->clear();
    tupletBracketType->addItem(muse::qtrc("notation/editstyle", "Automatic"), int(TupletBracketType::AUTO_BRACKET));
    tupletBracketType->addItem(muse::qtrc("notation/editstyle", "Bracket"), int(TupletBracketType::SHOW_BRACKET));
    tupletBracketType->addItem(muse::qtrc("notation/editstyle", "None", "no tuplet bracket type"), int(TupletBracketType::SHOW_NO_BRACKET));

    lyricsDashStartSystemPlacement->clear();
    lyricsDashStartSystemPlacement->addItem(muse::qtrc("notation/editstyle", "Standard"), int(LyricsDashSystemStart::STANDARD));
    lyricsDashStartSystemPlacement->addItem(muse::qtrc("notation/editstyle", "Inside the header"),
                                            int(LyricsDashSystemStart::UNDER_HEADER));
    lyricsDashStartSystemPlacement->addItem(muse::qtrc("notation/editstyle", "Under the first note"),
                                            int(LyricsDashSystemStart::UNDER_FIRST_NOTE));

    musicalSymbolFont->clear();
    dynamicsFont->clear();
    for (auto i : engravingFonts()->fonts()) {
        musicalSymbolFont->addItem(QString::fromStdString(i->name()), QString::fromStdString(i->name()));
        dynamicsFont->addItem(QString::fromStdString(i->name()), QString::fromStdString(i->name()));
    }

    static const SymId ids[] = {
        SymId::systemDivider, SymId::systemDividerLong, SymId::systemDividerExtraLong
    };
    for (SymId id : ids) {
        const QString& un = SymNames::translatedUserNameForSymId(id);
        AsciiStringView n = SymNames::nameForSymId(id);
        dividerLeftSym->addItem(un,  QVariant(QString(n.toQLatin1String())));
        dividerRightSym->addItem(un, QVariant(QString(n.toQLatin1String())));
    }

    // ====================================================
    // Notes (QML)
    // ====================================================

    auto noteFlagsTypeSelector = createQmlWidget(
        groupBox_noteFlags,
        QUrl(QString::fromUtf8("qrc:/qml/MuseScore/NotationScene/internal/EditStyle/NoteFlagsTypeSelector.qml")));
    noteFlagsTypeSelector.widget->setMinimumSize(224, 70);
    groupBox_noteFlags->layout()->addWidget(noteFlagsTypeSelector.widget);

    // ====================================================
    // Rests (QML)
    // ====================================================

    auto restOffsetSelector = createQmlWidget(
        groupBox_rests,
        QUrl(QString::fromUtf8("qrc:/qml/MuseScore/NotationScene/internal/EditStyle/RestOffsetSelector.qml")));
    restOffsetSelector.widget->setMinimumSize(224, 70);
    groupBox_rests->layout()->addWidget(restOffsetSelector.widget);

    // ====================================================
    // Measure repeats / (Multimeasure) rests
    // ====================================================

    // Define string here instead of in the .ui file to avoid MSVC compiler warning C4125, which would
    // be triggered by the decimal digit immediately following a non-ASCII character (curly quote).
    oneMeasureRepeatShow1->setText(muse::qtrc("EditStyleBase", "Show ‘1’ on 1-measure repeats"));
    singleMMRestShowNumber->setText(muse::qtrc("EditStyleBase", "Show number ‘1’"));

    // ====================================================
    // BEAMS (QML)
    // ====================================================

    auto beamsPage = createQmlWidget(
        groupBox_beams,
        QUrl(QString::fromUtf8("qrc:/qml/MuseScore/NotationScene/internal/EditStyle/BeamsPage.qml")));
    beamsPage.widget->setMinimumSize(224, 418);
    groupBox_beams->layout()->addWidget(beamsPage.widget);

    // ====================================================
    // BENDS (QML)
    // ====================================================

    auto fullBendStyleSelector = createQmlWidget(
        fullBendStyleBoxSelector,
        QUrl(QString::fromUtf8("qrc:/qml/MuseScore/NotationScene/internal/EditStyle/FullBendStyleSelector.qml")));
    fullBendStyleSelector.widget->setMinimumSize(224, 60);
    fullBendStyleBoxSelector->layout()->addWidget(fullBendStyleSelector.widget);

    // ====================================================
    // TIE PLACEMENT (QML)
    // ====================================================

    auto tiePlacementSelector = createQmlWidget(
        groupBox_ties,
        QUrl(QString::fromUtf8("qrc:/qml/MuseScore/NotationScene/internal/EditStyle/TiePlacementSelector.qml")));
    tiePlacementSelector.widget->setMinimumSize(224, 240);
    groupBox_ties->layout()->addWidget(tiePlacementSelector.widget);

    // ====================================================
    // ACCIDENTAL GROUP PLACEMENT (QML)
    // ====================================================

    auto accidPlacementSelector = createQmlWidget(
        groupBoxAccidentalStacking,
        QUrl(QString::fromUtf8("qrc:/qml/MuseScore/NotationScene/internal/EditStyle/AccidentalGroupPage.qml")));
    accidPlacementSelector.widget->setMinimumSize(224, 440);
    groupBoxAccidentalStacking->layout()->addWidget(accidPlacementSelector.widget);

    // ====================================================
    // FRETBOARDS STYLE PAGE (QML)
    // ====================================================

    auto fretboardsPage = createQmlWidget(
        fretboardsWidget,
        QUrl(QString::fromUtf8("qrc:/qml/MuseScore/NotationScene/internal/EditStyle/FretboardsPage.qml")));
    fretboardsPage.widget->setMinimumSize(224, 1006);
    connect(fretboardsPage.view->rootObject(), SIGNAL(goToTextStylePage(QString)), this, SLOT(goToTextStylePage(QString)));
    fretboardsWidget->layout()->addWidget(fretboardsPage.widget);

    // ====================================================
    // GLISSANDO STYLE SECTION (QML)
    // ====================================================

    auto glissandoSection = createQmlWidget(
        groupBox_glissando,
        QUrl(QString::fromUtf8("qrc:/qml/MuseScore/NotationScene/internal/EditStyle/GlissandoSection.qml")));
    glissandoSection.widget->setMinimumSize(224, 284);
    connect(glissandoSection.view->rootObject(), SIGNAL(goToTextStylePage(QString)), this, SLOT(goToTextStylePage(QString)));
    groupBox_glissando->layout()->addWidget(glissandoSection.widget);

    // ====================================================
    // NOTE LINE STYLE SECTION (QML)
    // ====================================================

    auto noteLineSection = createQmlWidget(
        groupBox_noteline,
        QUrl(QString::fromUtf8("qrc:/qml/MuseScore/NotationScene/internal/EditStyle/NoteLineSection.qml")));
    noteLineSection.widget->setMinimumSize(224, 200);
    connect(noteLineSection.view->rootObject(), SIGNAL(goToTextStylePage(QString)), this, SLOT(goToTextStylePage(QString)));
    groupBox_noteline->layout()->addWidget(noteLineSection.widget);

    // ====================================================
    // Figured Bass
    // ====================================================

    std::list<String> fbFontNames = FiguredBass::fontNames();
    for (const String& family : fbFontNames) {
        comboFBFont->addItem(family);
    }
    comboFBFont->setCurrentIndex(0);
    connect(comboFBFont, &QComboBox::currentIndexChanged, this, &EditStyle::on_comboFBFont_currentIndexChanged);

    // ====================================================
    // Chord Symbols
    // ====================================================

    voicingSelectWidget->interpretBox->clear();
    voicingSelectWidget->interpretBox->addItem(muse::qtrc("notation/editstyle", "Jazz"), int(0));   // two-item combobox for boolean style variant
    voicingSelectWidget->interpretBox->addItem(muse::qtrc("notation/editstyle", "Literal"), int(1));   // true = literal

    voicingSelectWidget->voicingBox->clear();
    voicingSelectWidget->voicingBox->addItem(muse::qtrc("notation/editstyle", "Automatic"), int(Voicing::AUTO));
    voicingSelectWidget->voicingBox->addItem(muse::qtrc("notation/editstyle", "Root only"), int(Voicing::ROOT_ONLY));
    voicingSelectWidget->voicingBox->addItem(muse::qtrc("notation/editstyle", "Close"), int(Voicing::CLOSE));
    voicingSelectWidget->voicingBox->addItem(muse::qtrc("notation/editstyle", "Drop two"), int(Voicing::DROP_2));
    voicingSelectWidget->voicingBox->addItem(muse::qtrc("notation/editstyle", "Six note"), int(Voicing::SIX_NOTE));
    voicingSelectWidget->voicingBox->addItem(muse::qtrc("notation/editstyle", "Four note"), int(Voicing::FOUR_NOTE));
    voicingSelectWidget->voicingBox->addItem(muse::qtrc("notation/editstyle", "Three note"), int(Voicing::THREE_NOTE));

    voicingSelectWidget->durationBox->clear();
    voicingSelectWidget->durationBox->addItem(muse::qtrc("notation/editstyle", "Until next chord symbol"),
                                              int(HDuration::UNTIL_NEXT_CHORD_SYMBOL));
    voicingSelectWidget->durationBox->addItem(muse::qtrc("notation/editstyle", "Until end of measure"),
                                              int(HDuration::STOP_AT_MEASURE_END));
    voicingSelectWidget->durationBox->addItem(muse::qtrc("notation/editstyle", "Chord/rest duration"),
                                              int(HDuration::SEGMENT_DURATION));

    // ====================================================
    // Miscellaneous
    // ====================================================

    setHeaderFooterToolTip();

    connect(buttonBox,             &QDialogButtonBox::clicked,  this, &EditStyle::buttonClicked);
    connect(enableVerticalSpread,  &QGroupBox::toggled,         this, &EditStyle::enableVerticalSpreadClicked);
    connect(disableVerticalSpread, &QGroupBox::toggled,         this, &EditStyle::disableVerticalSpreadClicked);
    connect(headerOddEven,         &QCheckBox::toggled,         this, &EditStyle::toggleHeaderOddEven);
    connect(footerOddEven,         &QCheckBox::toggled,         this, &EditStyle::toggleFooterOddEven);
    connect(chordDescriptionFileButton, &QToolButton::clicked,  this, &EditStyle::selectChordDescriptionFile);
    connect(chordsStandard,        &QRadioButton::toggled,      this, &EditStyle::setChordStyle);
    connect(chordsJazz,            &QRadioButton::toggled,      this, &EditStyle::setChordStyle);
    connect(chordsCustom,          &QRadioButton::toggled,      this, &EditStyle::setChordStyle);
    connect(chordsXmlFile,         &QCheckBox::toggled,         this, &EditStyle::setChordStyle);
    connect(chordDescriptionFile,  &QLineEdit::editingFinished, this, [=]() { setChordStyle(true); });

    WidgetUtils::setWidgetIcon(chordDescriptionFileButton, IconCode::Code::OPEN_FILE);

    connect(swingOff,       &QRadioButton::toggled, this, &EditStyle::setSwingParams);
    connect(swingEighth,    &QRadioButton::toggled, this, &EditStyle::setSwingParams);
    connect(swingSixteenth, &QRadioButton::toggled, this, &EditStyle::setSwingParams);

    connect(concertPitch,        &QCheckBox::toggled, this, &EditStyle::concertPitchToggled);
    connect(lyricsDashMinLength, &QDoubleSpinBox::valueChanged, this, &EditStyle::lyricsDashMinLengthValueChanged);
    connect(lyricsDashMaxLength, &QDoubleSpinBox::valueChanged, this, &EditStyle::lyricsDashMaxLengthValueChanged);
    connect(minSystemDistance,   &QDoubleSpinBox::valueChanged, this, &EditStyle::systemMinDistanceValueChanged);
    connect(maxSystemDistance,   &QDoubleSpinBox::valueChanged, this, &EditStyle::systemMaxDistanceValueChanged);

    connect(radioShowAllClefs, &QRadioButton::toggled, this, &EditStyle::clefVisibilityChanged);
    connect(radioHideClefs,    &QRadioButton::toggled, this, &EditStyle::clefVisibilityChanged);

    accidentalsGroup->setVisible(false);   // disable, not yet implemented

    // ====================================================
    // Signal Mappers
    // ====================================================

    QSignalMapper* setSignalMapper = new QSignalMapper(this); // value change signals
    QSignalMapper* resetSignalMapper = new QSignalMapper(this); // reset style signals

    const auto mapFunction = QOverload<>::of(&QSignalMapper::map);

    for (const StyleWidget& sw : styleWidgets) {
        P_TYPE type = MStyle::valueType(sw.idx);

        if (P_TYPE::DIRECTION_V == type) {
            QComboBox* cb = qobject_cast<QComboBox*>(sw.widget);
            if (sw.idx == StyleId::dynamicsHairpinVoiceBasedPlacement) {
                fillDynamicHairpinComboBox(cb);
            } else {
                fillDirectionComboBox(cb);
            }
        }

        if (sw.reset) {
            WidgetUtils::setWidgetIcon(sw.reset, IconCode::Code::UNDO);
            connect(sw.reset, &QToolButton::clicked, resetSignalMapper, mapFunction);
            resetSignalMapper->setMapping(sw.reset, static_cast<int>(sw.idx));
        }

        if (auto spinBox = qobject_cast<QSpinBox*>(sw.widget)) {
            connect(spinBox, &QSpinBox::valueChanged, setSignalMapper, mapFunction);
        } else if (auto doubleSpinBox = qobject_cast<QDoubleSpinBox*>(sw.widget)) {
            connect(doubleSpinBox, &QDoubleSpinBox::valueChanged, setSignalMapper, mapFunction);
        } else if (auto fontComboBox = qobject_cast<QFontComboBox*>(sw.widget)) {
            connect(fontComboBox, &QFontComboBox::currentFontChanged, setSignalMapper, mapFunction);
        } else if (auto comboBox = qobject_cast<QComboBox*>(sw.widget)) {
            connect(comboBox, &QComboBox::currentIndexChanged, setSignalMapper, mapFunction);
        } else if (auto radioButton = qobject_cast<QRadioButton*>(sw.widget)) {
            connect(radioButton, &QRadioButton::toggled, setSignalMapper, mapFunction);
        } else if (auto checkBox = qobject_cast<QCheckBox*>(sw.widget)) {
            connect(checkBox, &QCheckBox::stateChanged, setSignalMapper, mapFunction);
        } else if (auto button = qobject_cast<QAbstractButton*>(sw.widget)) {
            connect(button, &QAbstractButton::toggled, setSignalMapper, mapFunction);
        } else if (auto groupBox = qobject_cast<QGroupBox*>(sw.widget)) {
            connect(groupBox, &QGroupBox::toggled, setSignalMapper, mapFunction);
        } else if (auto textEdit = qobject_cast<QTextEdit*>(sw.widget)) {
            connect(textEdit, &QTextEdit::textChanged, setSignalMapper, mapFunction);
        } else if (auto buttonGroup = qobject_cast<QButtonGroup*>(sw.widget)) {
            connect(buttonGroup, &QButtonGroup::buttonClicked, setSignalMapper, mapFunction);
        } else if (auto alignSelect = qobject_cast<AlignSelect*>(sw.widget)) {
            connect(alignSelect, &AlignSelect::alignChanged, setSignalMapper, mapFunction);
        } else if (auto offsetSelect = qobject_cast<OffsetSelect*>(sw.widget)) {
            connect(offsetSelect, &OffsetSelect::offsetChanged, setSignalMapper, mapFunction);
        } else if (auto fontStyle = qobject_cast<FontStyleSelect*>(sw.widget)) {
            connect(fontStyle, &FontStyleSelect::fontStyleChanged, setSignalMapper, mapFunction);
        }

        setSignalMapper->setMapping(sw.widget, static_cast<int>(sw.idx));
    }

    connect(setSignalMapper, &QSignalMapper::mappedInt, this, &EditStyle::valueChanged);
    connect(resetSignalMapper, &QSignalMapper::mappedInt, this, &EditStyle::resetStyleValue);

    Score* score = globalContext()->currentNotation()->elements()->msScore();

    textStyles->clear();
    for (TextStyleType textStyleType : editableTextStyles()) {
        QListWidgetItem* item = new QListWidgetItem(score->getTextStyleUserName(textStyleType).qTranslated());
        item->setData(Qt::UserRole, int(textStyleType));
        textStyles->addItem(item);
    }

    textStyleFrameType->clear();
    textStyleFrameType->addItem(muse::qtrc("notation/editstyle", "None", "no frame for text"), int(FrameType::NO_FRAME));
    textStyleFrameType->addItem(muse::qtrc("notation/editstyle", "Rectangle"), int(FrameType::SQUARE));
    textStyleFrameType->addItem(muse::qtrc("notation/editstyle", "Circle"), int(FrameType::CIRCLE));

    connect(dynamicsAndHairpinPos, &QComboBox::currentIndexChanged, dynamicsAndHairpinPosDescription, [=]() {
        dynamicsAndHairpinPosDescription->setVisible(dynamicsAndHairpinPos->currentIndex() == int(DirectionV::AUTO));
    });
    connect(dynamicsAndHairpinPos, &QComboBox::currentIndexChanged, dynamicsAndHairpinsAboveOnVocalStaves, [=]() {
        dynamicsAndHairpinsAboveOnVocalStaves->setEnabled(dynamicsAndHairpinPos->currentIndex() != int(DirectionV::UP));
    });

    WidgetUtils::setWidgetIcon(resetTextStyleName, IconCode::Code::UNDO);

    connect(resetTextStyleName, &QToolButton::clicked, this, &EditStyle::resetUserStyleName);
    connect(styleName, &QLineEdit::textEdited, this, &EditStyle::editUserStyleName);
    connect(styleName, &QLineEdit::editingFinished, this, &EditStyle::endEditUserStyleName);

    // font face
    WidgetUtils::setWidgetIcon(resetTextStyleFontFace, IconCode::Code::UNDO);
    connect(resetTextStyleFontFace, &QToolButton::clicked, this, [=]() {
        resetTextStyle(TextStylePropertyType::FontFace);
    });
    connect(textStyleFontFace, &QFontComboBox::currentFontChanged, this, [=]() {
        textStyleValueChanged(TextStylePropertyType::FontFace, QVariant(textStyleFontFace->currentFont().family()));
    });

    // font size
    WidgetUtils::setWidgetIcon(resetTextStyleFontSize, IconCode::Code::UNDO);
    connect(resetTextStyleFontSize, &QToolButton::clicked, this, [=]() {
        resetTextStyle(TextStylePropertyType::FontSize);
    });
    connect(textStyleFontSize, &QDoubleSpinBox::valueChanged, this, [=]() {
        textStyleValueChanged(TextStylePropertyType::FontSize, QVariant(textStyleFontSize->value()));
    });

    // musical symbols scale
    WidgetUtils::setWidgetIcon(resetTextStyleMusicalSymbolsScale, IconCode::Code::UNDO);
    connect(resetTextStyleMusicalSymbolsScale, &QToolButton::clicked, this, [=]() {
        resetTextStyle(TextStylePropertyType::MusicalSymbolsScale);
    });
    connect(textStyleMusicalSymbolsScale, &QDoubleSpinBox::valueChanged, this, [=]() {
        textStyleValueChanged(TextStylePropertyType::MusicalSymbolsScale, QVariant(textStyleMusicalSymbolsScale->value()));
    });

    // line spacing
    WidgetUtils::setWidgetIcon(resetTextStyleLineSpacing, IconCode::Code::UNDO);
    connect(resetTextStyleLineSpacing, &QToolButton::clicked, this, [=]() {
        resetTextStyle(TextStylePropertyType::LineSpacing);
    });
    connect(textStyleLineSpacing, &QDoubleSpinBox::valueChanged, this, [=]() {
        textStyleValueChanged(TextStylePropertyType::LineSpacing, QVariant(textStyleLineSpacing->value()));
    });

    // font style
    WidgetUtils::setWidgetIcon(resetTextStyleFontStyle, IconCode::Code::UNDO);
    connect(resetTextStyleFontStyle, &QToolButton::clicked, this, [=]() {
        resetTextStyle(TextStylePropertyType::FontStyle);
    });
    connect(textStyleFontStyle, &FontStyleSelect::fontStyleChanged, this, [=]() {
        textStyleValueChanged(TextStylePropertyType::FontStyle, QVariant(int(textStyleFontStyle->fontStyle())));
    });

    // align
    WidgetUtils::setWidgetIcon(resetTextStyleAlign, IconCode::Code::UNDO);
    connect(resetTextStyleAlign, &QToolButton::clicked, this, [=]() {
        resetTextStyle(TextStylePropertyType::TextAlign);
    });
    connect(textStyleAlign, &AlignSelect::alignChanged, this, [=]() {
        textStyleValueChanged(TextStylePropertyType::TextAlign, PropertyValue(textStyleAlign->align()).toQVariant());
    });

    // offset
    WidgetUtils::setWidgetIcon(resetTextStyleOffset, IconCode::Code::UNDO);
    connect(resetTextStyleOffset, &QToolButton::clicked, this, [=]() {
        resetTextStyle(TextStylePropertyType::Offset);
    });
    connect(textStyleOffset, &OffsetSelect::offsetChanged, this, [=]() {
        textStyleValueChanged(TextStylePropertyType::Offset, QVariant(textStyleOffset->offset()));
    });

    // spatium dependent
    WidgetUtils::setWidgetIcon(resetTextStyleSpatiumDependent, IconCode::Code::UNDO);
    connect(resetTextStyleSpatiumDependent, &QToolButton::clicked, this, [=]() {
        resetTextStyle(TextStylePropertyType::SizeSpatiumDependent);
    });
    connect(textStyleSpatiumDependent, &QCheckBox::toggled, this, [=]() {
        textStyleValueChanged(TextStylePropertyType::SizeSpatiumDependent, textStyleSpatiumDependent->isChecked());
    });

    WidgetUtils::setWidgetIcon(resetTextStyleFrameType, IconCode::Code::UNDO);
    connect(resetTextStyleFrameType, &QToolButton::clicked, this, [=]() {
        resetTextStyle(TextStylePropertyType::FrameType);
    });
    connect(textStyleFrameType, &QComboBox::currentIndexChanged, this, [=]() {
        textStyleValueChanged(TextStylePropertyType::FrameType, textStyleFrameType->currentIndex());
    });

    WidgetUtils::setWidgetIcon(resetTextStyleFramePadding, IconCode::Code::UNDO);
    connect(resetTextStyleFramePadding, &QToolButton::clicked, this, [=]() {
        resetTextStyle(TextStylePropertyType::FramePadding);
    });
    connect(textStyleFramePadding, &QDoubleSpinBox::valueChanged, this, [=]() {
        textStyleValueChanged(TextStylePropertyType::FramePadding, textStyleFramePadding->value());
    });

    WidgetUtils::setWidgetIcon(resetTextStyleFrameBorder, IconCode::Code::UNDO);
    connect(resetTextStyleFrameBorder, &QToolButton::clicked, this, [=]() {
        resetTextStyle(TextStylePropertyType::FrameWidth);
    });
    connect(textStyleFrameBorder, &QDoubleSpinBox::valueChanged, this, [=]() {
        textStyleValueChanged(TextStylePropertyType::FrameWidth, textStyleFrameBorder->value());
    });

    WidgetUtils::setWidgetIcon(resetTextStyleFrameBorderRadius, IconCode::Code::UNDO);
    connect(resetTextStyleFrameBorderRadius, &QToolButton::clicked, this, [=]() {
        resetTextStyle(TextStylePropertyType::FrameRound);
    });
    connect(textStyleFrameBorderRadius, &QDoubleSpinBox::valueChanged, this, [=]() {
        textStyleValueChanged(TextStylePropertyType::FrameRound, textStyleFrameBorderRadius->value());
    });

    WidgetUtils::setWidgetIcon(resetTextStyleFrameForeground, IconCode::Code::UNDO);
    connect(resetTextStyleFrameForeground, &QToolButton::clicked, this, [=]() {
        resetTextStyle(TextStylePropertyType::FrameBorderColor);
    });
    connect(textStyleFrameForeground, &Awl::ColorLabel::colorChanged, this, [=]() {
        textStyleValueChanged(TextStylePropertyType::FrameBorderColor, textStyleFrameForeground->color());
    });

    WidgetUtils::setWidgetIcon(resetTextStyleFrameBackground, IconCode::Code::UNDO);
    connect(resetTextStyleFrameBackground, &QToolButton::clicked, this, [=]() {
        resetTextStyle(TextStylePropertyType::FrameFillColor);
    });
    connect(textStyleFrameBackground, &Awl::ColorLabel::colorChanged, this, [=]() {
        textStyleValueChanged(TextStylePropertyType::FrameFillColor, textStyleFrameBackground->color());
    });

    WidgetUtils::setWidgetIcon(resetTextStyleColor, IconCode::Code::UNDO);
    connect(resetTextStyleColor, &QToolButton::clicked, this, [=]() {
        resetTextStyle(TextStylePropertyType::Color);
    });
    connect(textStyleColor, &Awl::ColorLabel::colorChanged, this, [=]() {
        textStyleValueChanged(TextStylePropertyType::Color, textStyleColor->color());
    });

    connect(textStyles, &QListWidget::currentRowChanged, this, &EditStyle::textStyleChanged);
    textStyles->setCurrentRow(configuration()->styleDialogLastSubPageIndex());

    connect(pageList, &QListWidget::currentRowChanged, pageStack, &QStackedWidget::setCurrentIndex);
    connect(pageList, &QListWidget::currentRowChanged, this, &EditStyle::on_pageRowSelectionChanged);
    pageList->setCurrentRow(configuration()->styleDialogLastPageIndex());

    editLyricsTextStyleButton->setChecked(false);
    connect(editLyricsTextStyleButton, &QPushButton::clicked, pageList, [=](){
        pageList->setCurrentRow(ALL_PAGE_CODES.indexOf("text-styles"));
    });
    connect(editLyricsTextStyleButton, &QPushButton::clicked, textStyles, [=](){
        textStyles->setCurrentRow(ALL_TEXT_STYLE_SUBPAGE_CODES.indexOf("lyrics-odd-lines"));
    });

    adjustPagesStackSize(0);

    WidgetStateStore::restoreGeometry(this);
}

//---------------------------------------------------------
//   showEvent
//---------------------------------------------------------

void EditStyle::showEvent(QShowEvent* ev)
{
    setValues();
    pageList->setFocus();
    globalContext()->currentNotation()->undoStack()->prepareChanges(muse::TranslatableString("undoableAction", "Edit style"));
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

    buttonApplyToAllParts->setText(muse::qtrc("notation/editstyle", "Apply to all parts"));

    for (const LineStyleSelect* lineStyleSelect : m_lineStyleSelects) {
        int idx = 0;
        for (const muse::TranslatableString& lineStyle : lineStyles) {
            lineStyleSelect->lineStyleComboBox->setItemText(idx, lineStyle.qTranslated());
            ++idx;
        }
    }

    for (QComboBox* cb : verticalPlacementComboBoxes) {
        cb->setItemText(0, muse::qtrc("notation/editstyle", "Above"));
        cb->setItemText(1, muse::qtrc("notation/editstyle", "Below"));
    }

    for (QComboBox* cb : horizontalPlacementComboBoxes) {
        cb->setItemText(0, muse::qtrc("notation/editstyle", "Left"));
        cb->setItemText(1, muse::qtrc("notation/editstyle", "Center"));
        cb->setItemText(2, muse::qtrc("notation/editstyle", "Right"));
    }

    mmRestRangeBracketType->setItemText(0, muse::qtrc("notation/editstyle", "None"));
    mmRestRangeBracketType->setItemText(1, muse::qtrc("notation/editstyle", "Brackets"));
    mmRestRangeBracketType->setItemText(2, muse::qtrc("notation/editstyle", "Parentheses"));

    autoplaceVerticalAlignRange->setItemText(0, muse::qtrc("notation/editstyle", "Segment"));
    autoplaceVerticalAlignRange->setItemText(1, muse::qtrc("notation/editstyle", "Measure"));
    autoplaceVerticalAlignRange->setItemText(2, muse::qtrc("notation/editstyle", "System"));

    tupletNumberType->setItemText(0, muse::qtrc("notation/editstyle", "Number"));
    tupletNumberType->setItemText(1, muse::qtrc("notation/editstyle", "Ratio"));
    tupletNumberType->setItemText(2, muse::qtrc("notation/editstyle", "None", "no tuplet number type"));

    tupletBracketType->setItemText(0, muse::qtrc("notation/editstyle", "Automatic"));
    tupletBracketType->setItemText(1, muse::qtrc("notation/editstyle", "Bracket"));
    tupletBracketType->setItemText(2, muse::qtrc("notation/editstyle", "None", "no tuplet bracket type"));

    voicingSelectWidget->interpretBox->setItemText(0, muse::qtrc("notation/editstyle", "Jazz"));
    voicingSelectWidget->interpretBox->setItemText(1, muse::qtrc("notation/editstyle", "Literal"));

    voicingSelectWidget->voicingBox->setItemText(0, muse::qtrc("notation/editstyle", "Automatic"));
    voicingSelectWidget->voicingBox->setItemText(1, muse::qtrc("notation/editstyle", "Root only"));
    voicingSelectWidget->voicingBox->setItemText(2, muse::qtrc("notation/editstyle", "Close"));
    voicingSelectWidget->voicingBox->setItemText(3, muse::qtrc("notation/editstyle", "Drop two"));
    voicingSelectWidget->voicingBox->setItemText(4, muse::qtrc("notation/editstyle", "Six note"));
    voicingSelectWidget->voicingBox->setItemText(5, muse::qtrc("notation/editstyle", "Four note"));
    voicingSelectWidget->voicingBox->setItemText(6, muse::qtrc("notation/editstyle", "Three note"));

    voicingSelectWidget->durationBox->setItemText(0, muse::qtrc("notation/editstyle", "Until next chord symbol"));
    voicingSelectWidget->durationBox->setItemText(1, muse::qtrc("notation/editstyle", "Until end of measure"));
    voicingSelectWidget->durationBox->setItemText(2, muse::qtrc("notation/editstyle", "Chord/rest duration"));

    setHeaderFooterToolTip();

    Score* score = globalContext()->currentNotation()->elements()->msScore();

    int idx = 0;
    for (TextStyleType textStyleType : allTextStyles()) {
        textStyles->item(idx)->setText(score->getTextStyleUserName(textStyleType).qTranslated());
        ++idx;
    }

    textStyleFrameType->setItemText(0, muse::qtrc("notation/editstyle", "None", "no frame for text"));
    textStyleFrameType->setItemText(1, muse::qtrc("notation/editstyle", "Rectangle"));
    textStyleFrameType->setItemText(2, muse::qtrc("notation/editstyle", "Circle"));
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
          + muse::qtrc("notation/editstyle", "Special symbols in header/footer")
          + QString("</b></p>")
          + QString("<table><tr><td>$p</td><td>-</td><td><i>")
          + muse::qtrc("notation/editstyle", "Page number, except on first page")
          + QString("</i></td></tr><tr><td>$N</td><td>-</td><td><i>")
          + muse::qtrc("notation/editstyle", "Page number, if there is more than one page")
          + QString("</i></td></tr><tr><td>$P</td><td>-</td><td><i>")
          + muse::qtrc("notation/editstyle", "Page number, on all pages")
          + QString("</i></td></tr><tr><td>$n</td><td>-</td><td><i>")
          + muse::qtrc("notation/editstyle", "Number of pages")
          + QString("</i></td></tr><tr><td>$f</td><td>-</td><td><i>")
          + muse::qtrc("notation/editstyle", "File name")
          + QString("</i></td></tr><tr><td>$F</td><td>-</td><td><i>")
          + muse::qtrc("notation/editstyle", "File path+name")
          + QString("</i></td></tr><tr><td>$i</td><td>-</td><td><i>")
          + muse::qtrc("notation/editstyle", "Part name, except on first page")
          + QString("</i></td></tr><tr><td>$I</td><td>-</td><td><i>")
          + muse::qtrc("notation/editstyle", "Part name, on all pages")
          + QString("</i></td></tr><tr><td>$d</td><td>-</td><td><i>")
          + muse::qtrc("notation/editstyle", "Current date")
          + QString("</i></td></tr><tr><td>$D</td><td>-</td><td><i>")
          + muse::qtrc("notation/editstyle", "Creation date")
          + QString("</i></td></tr><tr><td>$m</td><td>-</td><td><i>")
          + muse::qtrc("notation/editstyle", "Last modification time")
          + QString("</i></td></tr><tr><td>$M</td><td>-</td><td><i>")
          + muse::qtrc("notation/editstyle", "Last modification date")
          + QString("</i></td></tr><tr><td>$C</td><td>-</td><td><i>")
          + muse::qtrc("notation/editstyle", "Copyright, on first page only")
          + QString("</i></td></tr><tr><td>$c</td><td>-</td><td><i>")
          + muse::qtrc("notation/editstyle", "Copyright, on all pages")
          + QString("</i></td></tr><tr><td>$v</td><td>-</td><td><i>")
          + muse::qtrc("notation/editstyle", "MuseScore Studio version this score was last saved with")
          + QString("</i></td></tr><tr><td>$r</td><td>-</td><td><i>")
          + muse::qtrc("notation/editstyle", "MuseScore Studio revision this score was last saved with")
          + QString("</i></td></tr><tr><td>$$</td><td>-</td><td><i>")
          + muse::qtrc("notation/editstyle", "The $ sign itself")
          + QString("</i></td></tr><tr><td>$:tag:</td><td>-</td><td><i>")
          + muse::qtrc("notation/editstyle", "Metadata tag, see below")
          + QString("</i></td></tr></table><p>")
          + muse::qtrc("notation/editstyle", "Available metadata tags and their current values")
          + QString("<br />")
          + muse::qtrc("notation/editstyle", "(in File > Project properties…):")
          + QString("</p><table>");

    // show all tags for current score/part
    Score* score = globalContext()->currentNotation()->elements()->msScore();
    if (!score->isMaster()) {
        for (const auto& tag : score->masterScore()->metaTags()) {
            toolTipHeaderFooter += QString("<tr><td>%1</td><td>-</td><td>%2</td></tr>")
                                   .arg(tag.first.toQString(), tag.second.toQString());
        }
    }
    for (const auto& tag : score->masterScore()->metaTags()) {
        toolTipHeaderFooter += QString("<tr><td>%1</td><td>-</td><td>%2</td></tr>")
                               .arg(tag.first.toQString(), tag.second.toQString());
    }

    QList<QMap<QString, QString> > tags; // FIXME
    for (const QMap<QString, QString>& tag: tags) {
        QMapIterator<QString, QString> i(tag);
        while (i.hasNext()) {
            i.next();
            toolTipHeaderFooter += QString("<tr><td>%1</td><td>-</td><td>%2</td></tr>").arg(i.key(), i.value());
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

    connect(pageStack, &QStackedWidget::currentChanged, this, [this](int currentIndex) {
        QWidget* currentPage = pageStack->widget(currentIndex);
        if (!currentPage) {
            return;
        }
        pageStack->setMinimumSize(currentPage->sizeHint());
    });
}

EditStyle::WidgetAndView EditStyle::createQmlWidget(QWidget* parent, const QUrl& source)
{
    QQuickView* view = new QQuickView(uiEngine()->qmlEngine(), nullptr);
    view->setResizeMode(QQuickView::SizeRootObjectToView);

    connect(view, &QQuickView::statusChanged, view, [source, view](QQuickView::Status status) {
        if (status == QQuickView::Error) {
            LOGE() << "Errors while loading QML file from " << source << ":";

            for (const QQmlError& error : view->errors()) {
                LOGE() << error.toString();
            }
        }
    });

    connect(view, &QQuickView::sceneGraphError, view, [source](QQuickWindow::SceneGraphError error, const QString& message) {
        LOGE() << "Scene graph error in QML file from " << source << ": [" << error << "] " << message;
    });

    QString bgColorStr = uiConfiguration()->currentTheme().values.value(muse::ui::BACKGROUND_PRIMARY_COLOR).toString();
    view->setColor(QColor(bgColorStr));

    view->setSource(source);

    QWidget* container = QWidget::createWindowContainer(view, parent);
    container->setMinimumSize(view->size());
    container->setFocusPolicy(Qt::TabFocus);

    return { container, view };
}

QString EditStyle::currentPageCode() const
{
    return m_currentPageCode;
}

QString EditStyle::currentSubPageCode() const
{
    return m_currentSubPageCode;
}

QString EditStyle::pageCodeForElement(const EngravingItem* element)
{
    IF_ASSERT_FAILED(element) {
        return QString();
    }

    switch (element->type()) {
    case ElementType::SCORE:
        return "score";

    case ElementType::PAGE:
        return "page";

    case ElementType::INSTRUMENT_NAME:
    case ElementType::TEXT:
    case ElementType::HARP_DIAGRAM: {
        if (element->isText()) {
            if (toText(element)->textStyleType() == TextStyleType::FOOTER
                || toText(element)->textStyleType() == TextStyleType::HEADER) {
                return "header-and-footer";
            }
        }
        return "text-styles";
    }

    case ElementType::MEASURE_NUMBER:
    case ElementType::MMREST_RANGE:
        return "measure-number";

    case ElementType::BRACKET:
    case ElementType::BRACKET_ITEM:
    case ElementType::SYSTEM_DIVIDER:
        return "system";

    case ElementType::CLEF:
    case ElementType::KEYSIG:
    case ElementType::TIMESIG:
        return "clefs-key-and-time-signatures";

    case ElementType::ACCIDENTAL:
        return "accidentals";

    case ElementType::MEASURE:
        return "measure";

    case ElementType::BAR_LINE:
        return "barlines";

    case ElementType::NOTE:
    case ElementType::CHORD:
    case ElementType::STEM:
    case ElementType::STEM_SLASH:
    case ElementType::LEDGER_LINE:
    case ElementType::NOTEDOT:
        return "notes";

    case ElementType::REST:
    case ElementType::MMREST:
        return "rests";

    case ElementType::MEASURE_REPEAT:
        return "measure-repeats";

    case ElementType::BEAM:
        return "beams";

    case ElementType::TUPLET:
        return "tuplets";

    case ElementType::ARPEGGIO:
        return "arpeggios";

    case ElementType::SLUR:
    case ElementType::SLUR_SEGMENT:
    case ElementType::TIE:
    case ElementType::TIE_SEGMENT:
    case ElementType::LAISSEZ_VIB:
    case ElementType::LAISSEZ_VIB_SEGMENT:
    case ElementType::PARTIAL_TIE:
    case ElementType::PARTIAL_TIE_SEGMENT:
        return "slurs-and-ties";

    case ElementType::HAIRPIN:
    case ElementType::HAIRPIN_SEGMENT:
        return "dynamics-hairpins";

    case ElementType::VOLTA:
    case ElementType::VOLTA_SEGMENT:
        return "volta";

    case ElementType::OTTAVA:
    case ElementType::OTTAVA_SEGMENT:
        return "ottava";

    case ElementType::PEDAL:
    case ElementType::PEDAL_SEGMENT:
        return "pedal";

    case ElementType::TRILL:
    case ElementType::TRILL_SEGMENT:
        return "trill";

    case ElementType::VIBRATO:
    case ElementType::VIBRATO_SEGMENT:
        return "vibrato";

    case ElementType::BEND:
    case ElementType::GUITAR_BEND:
    case ElementType::GUITAR_BEND_SEGMENT:
    case ElementType::GUITAR_BEND_HOLD:
    case ElementType::GUITAR_BEND_HOLD_SEGMENT:
    case ElementType::GUITAR_BEND_TEXT:
        return "bend";

    case ElementType::TEXTLINE:
    case ElementType::TEXTLINE_SEGMENT:
        return "text-line";

    case ElementType::GLISSANDO:
    case ElementType::GLISSANDO_SEGMENT:
    case ElementType::NOTELINE:
    case ElementType::NOTELINE_SEGMENT:
        return "glissando-note-line";

    case ElementType::ARTICULATION:
        return "articulations-and-ornaments";

    case ElementType::FERMATA:
        return "fermatas";

    case ElementType::PLAYTECH_ANNOTATION:
    case ElementType::STAFF_TEXT:
        return "staff-text";

    case ElementType::TEMPO_TEXT:
        return "tempo-text";

    case ElementType::LYRICS:
    case ElementType::LYRICSLINE:
    case ElementType::LYRICSLINE_SEGMENT:
        return "lyrics";

    case ElementType::EXPRESSION:
        return "expression";

    case ElementType::DYNAMIC:
        return "dynamics-hairpins";

    case ElementType::REHEARSAL_MARK:
        return "rehearsal-marks";

    case ElementType::FIGURED_BASS:
        return "figured-bass";

    case ElementType::HARMONY:
        return "chord-symbols";

    case ElementType::FRET_DIAGRAM:
        return "fretboard-diagrams";

    default: return QString();
    }
}

QString EditStyle::subPageCodeForElement(const EngravingItem* element)
{
    IF_ASSERT_FAILED(element) {
        return QString();
    }

    if (pageCodeForElement(element) == "text-styles" && element->isTextBase()) {
        switch (toTextBase(element)->textStyleType()) {
        case TextStyleType::TITLE:
            return "title";

        case TextStyleType::SUBTITLE:
            return "subtitle";

        case TextStyleType::COMPOSER:
            return "composer";

        case TextStyleType::LYRICIST:
            return "poet";

        case TextStyleType::TRANSLATOR:
            return "translator";

        case TextStyleType::FRAME:
            return "frame";

        case TextStyleType::INSTRUMENT_EXCERPT:
            return "instrument-name-part";

        case TextStyleType::INSTRUMENT_LONG:
            return "instrument-name-long";

        case TextStyleType::INSTRUMENT_SHORT:
            return "instrument-name-short";

        case TextStyleType::INSTRUMENT_CHANGE:
            return "instrument-change";

        case TextStyleType::HEADER:
            return "header";

        case TextStyleType::FOOTER:
            return "footer";

        case TextStyleType::COPYRIGHT:
            return "copyright";

        case TextStyleType::PAGE_NUMBER:
            return "page-number";

        case TextStyleType::MEASURE_NUMBER:
            return "measure-number";

        case TextStyleType::MMREST_RANGE:
            return "multimeasure-rest-range";

        case TextStyleType::TEMPO:
            return "tempo";

        case TextStyleType::TEMPO_CHANGE:
            return "tempo-change";

        case TextStyleType::METRONOME:
            return "metronome";

        case TextStyleType::REPEAT_LEFT:
            return "repeat-text-left";

        case TextStyleType::REPEAT_RIGHT:
            return "repeat-text-right";

        case TextStyleType::REHEARSAL_MARK:
            return "rehearsal-mark";

        case TextStyleType::SYSTEM:
            return "system";

        case TextStyleType::STAFF:
            return "staff";

        case TextStyleType::EXPRESSION:
            return "expression";

        case TextStyleType::HAIRPIN:
            return "hairpin";

        case TextStyleType::LYRICS_ODD:
            return "lyrics-odd-lines";

        case TextStyleType::LYRICS_EVEN:
            return "lyrics-even-lines";

        case TextStyleType::HARMONY_A:
            return "chord-symbols";

        case TextStyleType::HARMONY_B:
            return "chord-symbols-alternate";

        case TextStyleType::HARMONY_ROMAN:
            return "roman-numeral-analysis";

        case TextStyleType::HARMONY_NASHVILLE:
            return "nashville-number";

        case TextStyleType::TUPLET:
            return "tuplet";

        case TextStyleType::STICKING:
            return "sticking";

        case TextStyleType::FINGERING:
            return "fingering";

        case TextStyleType::LH_GUITAR_FINGERING:
            return "lh-guitar-fingering";

        case TextStyleType::RH_GUITAR_FINGERING:
            return "rh-guitar-fingering";

        case TextStyleType::STRING_NUMBER:
            return "string-number";

        case TextStyleType::STRING_TUNINGS:
            return "string-tunings";

        case TextStyleType::FRET_DIAGRAM_FINGERING:
            return "fretboard-diagram-fingering";

        case TextStyleType::FRET_DIAGRAM_FRET_NUMBER:
            return "fretboard-diagram-fret-number";

        case TextStyleType::HARP_PEDAL_DIAGRAM:
            return "harp-pedal-diagram";

        case TextStyleType::HARP_PEDAL_TEXT_DIAGRAM:
            return "harp-pedal-text-diagram";

        case TextStyleType::TEXTLINE:
            return "text-line";

        case TextStyleType::NOTELINE:
            return "note-line";

        case TextStyleType::VOLTA:
            return "volta";

        case TextStyleType::OTTAVA:
            return "ottava";

        case TextStyleType::GLISSANDO:
            return "glissando";

        case TextStyleType::PEDAL:
            return "pedal";

        case TextStyleType::BEND:
            return "bend";

        case TextStyleType::LET_RING:
            return "let-ring";

        case TextStyleType::PALM_MUTE:
            return "palm-mute";

        case TextStyleType::USER1:
            return "user1";

        case TextStyleType::USER2:
            return "user2";

        case TextStyleType::USER3:
            return "user3";

        case TextStyleType::USER4:
            return "user4";

        case TextStyleType::USER5:
            return "user5";

        case TextStyleType::USER6:
            return "user6";

        case TextStyleType::USER7:
            return "user7";

        case TextStyleType::USER8:
            return "user8";

        case TextStyleType::USER9:
            return "user9";

        case TextStyleType::USER10:
            return "user10";

        case TextStyleType::USER11:
            return "user11";

        case TextStyleType::USER12:
            return "user12";

        case TextStyleType::DYNAMICS:
        case TextStyleType::DEFAULT:
        case TextStyleType::TEXT_TYPES:
        case TextStyleType::IGNORED_TYPES:
            return QString();
        }
    }
    return QString();
}

void EditStyle::setCurrentPageCode(const QString& code)
{
    if (m_currentPageCode == code) {
        return;
    }

    int index = ALL_PAGE_CODES.indexOf(code);
    IF_ASSERT_FAILED(index >= 0) {
        return;
    }

    pageList->setCurrentRow(index);

    m_currentPageCode = code;
    emit currentPageChanged();
}

void EditStyle::setCurrentSubPageCode(const QString& code)
{
    if (m_currentSubPageCode == code) {
        return;
    }

    IF_ASSERT_FAILED(m_currentPageCode == "text-styles") {
        return;
    }

    int index = ALL_TEXT_STYLE_SUBPAGE_CODES.indexOf(code);
    IF_ASSERT_FAILED(index >= 0) {
        return;
    }

    textStyles->setCurrentRow(index);

    m_currentSubPageCode = code;
    emit currentSubPageChanged();
}

void EditStyle::goToTextStylePage(const QString& code)
{
    int index = ALL_PAGE_CODES.indexOf("text-styles");

    int subIndex = ALL_TEXT_STYLE_SUBPAGE_CODES.indexOf(code);
    IF_ASSERT_FAILED(index >= 0) {
        return;
    }

    pageList->setCurrentRow(index);
    m_currentPageCode = "text-styles";

    textStyles->setCurrentRow(subIndex);
    m_currentSubPageCode = code;

    emit currentPageChanged();
    emit currentSubPageChanged();
}

//---------------------------------------------------------
//   keyPressEvent
//---------------------------------------------------------

void EditStyle::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        return;
    }
    QDialog::keyPressEvent(event);
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
    globalContext()->currentNotation()->style()->styleChanged().notify();

    QDialog::accept();
}

//---------------------------------------------------------
//   reject
//---------------------------------------------------------

void EditStyle::reject()
{
    globalContext()->currentNotation()->undoStack()->rollbackChanges();
    globalContext()->currentNotation()->style()->styleChanged().notify();

    QDialog::reject();
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
    WidgetUtils::setWidgetIcon(buttonTogglePagelist, isVis ? IconCode::Code::ARROW_RIGHT : IconCode::Code::ARROW_LEFT);
}

//---------------------------------------------------------
//    On resetStylesButton clicked
//---------------------------------------------------------

void EditStyle::on_resetStylesButton_clicked()
{
    StyleIdSet dontResetTheseStyles {
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

    globalContext()->currentNotation()->style()->resetAllStyleValues(dontResetTheseStyles);
    setValues();
}

//---------------------------------------------------------
//    On pageRowSelectionChanged
//---------------------------------------------------------

void EditStyle::on_pageRowSelectionChanged()
{
    configuration()->setStyleDialogLastPageIndex(pageList->currentRow());
}

//---------------------------------------------------------
//   unhandledType
//---------------------------------------------------------

void EditStyle::unhandledType(const StyleWidget sw)
{
    P_TYPE type = MStyle::valueType(sw.idx);
    ASSERT_X(QString::asprintf("%d <%s>: widget: %s\n", int(type), MStyle::valueName(sw.idx),
                               sw.widget->metaObject()->className()));
}

bool EditStyle::isBoolStyleRepresentedByButtonGroup(StyleId id)
{
    switch (id) {
    case StyleId::articulationKeepTogether:
    case StyleId::trillAlwaysShowCueNote:
    case StyleId::genClef:
    case StyleId::genKeysig:
    case StyleId::singleMeasureMMRestUseNormalRest:
    case StyleId::mmRestConstantWidth:
        return true;
    default:
        return false;
    }
}

//---------------------------------------------------------
//   getValue
//    return current gui value
//---------------------------------------------------------

PropertyValue EditStyle::getValue(StyleId idx)
{
    const StyleWidget& sw = styleWidget(idx);
    P_TYPE type = MStyle::valueType(idx);
    switch (type) {
    case P_TYPE::SPATIUM: {
        QDoubleSpinBox* sb = qobject_cast<QDoubleSpinBox*>(sw.widget);
        return Spatium(sb->value() * (sw.showPercent ? 0.01 : 1.0));
    } break;
    case P_TYPE::REAL: {
        QVariant v = sw.widget->property("value");
        if (!v.isValid()) {
            unhandledType(sw);
            return PropertyValue();
        }
        qreal value = v.toReal();
        if (sw.showPercent) {
            value = value * 0.01;
        }
        return value;
    } break;
    case P_TYPE::BOOL: {
        QVariant v;
        if (sw.idx == StyleId::harmonyVoiceLiteral) { // special case for bool represented by a two-item combobox
            QComboBox* cb = qobject_cast<QComboBox*>(sw.widget);
            v = cb->currentIndex();
        } else if (isBoolStyleRepresentedByButtonGroup(idx)) { // special case for bool represented by a two-item buttonGroup
            QButtonGroup* bg = qobject_cast<QButtonGroup*>(sw.widget);
            v = bool(bg->checkedId());
        } else if (sw.idx == StyleId::lyricsDashForce || sw.idx == StyleId::lyricsMelismaForce) { // special case where UI is presented with opposite wording
            v = sw.widget->property("checked");
            return !v.toBool();
        } else {
            v = sw.widget->property("checked");
            if (!v.isValid()) {
                unhandledType(sw);
                return PropertyValue();
            }
        }
        return v.toBool();
    } break;
    case P_TYPE::PLACEMENT_H:
    case P_TYPE::PLACEMENT_V:
    case P_TYPE::LINE_TYPE:
    case P_TYPE::INT: {
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
            ASSERT_X("unhandled int");
        }
    } break;
    case P_TYPE::STRING: {
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
    } break;
    case P_TYPE::POINT: {
        if (idx == StyleId::lyricsPosAbove || idx == StyleId::lyricsPosBelow) {
            QDoubleSpinBox* dsb = qobject_cast<QDoubleSpinBox*>(sw.widget);
            return PointF(0.0, dsb->value());
        }
        OffsetSelect* cb = qobject_cast<OffsetSelect*>(sw.widget);
        if (cb) {
            return PointF::fromQPointF(cb->offset());
        } else {
            ASSERT_X("unhandled muse::PointF");
        }
    } break;
    case P_TYPE::DIRECTION_V: {
        QComboBox* cb = qobject_cast<QComboBox*>(sw.widget);
        if (cb) {
            return DirectionV(cb->currentIndex());
        } else {
            ASSERT_X("unhandled Direction");
        }
    } break;
    case P_TYPE::ALIGN: {
        AlignSelect* as = qobject_cast<AlignSelect*>(sw.widget);
        return as->align();
    } break;
    case P_TYPE::TIE_PLACEMENT: {
        QButtonGroup* bg = qobject_cast<QButtonGroup*>(sw.widget);
        return TiePlacement(bg->checkedId());
    } break;
    default: {
        ASSERT_X(QString::asprintf("EditStyle::getValue: unhandled type <%d>", static_cast<int>(type)));
    } break;
    }

    return PropertyValue();
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
        PropertyValue val = sw.idx == StyleId::lyricsPosAbove || sw.idx == StyleId::lyricsPosBelow
                            ? Spatium(styleValue(sw.idx).value<PointF>().y())
                            : styleValue(sw.idx);
        if (sw.reset) {
            sw.reset->setEnabled(!hasDefaultStyleValue(sw.idx));
        }

        switch (val.type()) {
        case P_TYPE::SPATIUM: {
            qreal value = val.value<Spatium>().val();
            if (sw.showPercent) {
                qobject_cast<QSpinBox*>(sw.widget)->setValue(int(value * 100.0));
            } else {
                sw.widget->setProperty("value", value);
            }
        } break;
        case P_TYPE::REAL: {
            qreal value = val.toReal();
            if (sw.showPercent) {
                value = value * 100;
            }
            if (!sw.widget->setProperty("value", value)) {
                unhandledType(sw);
            }
        } break;
        case P_TYPE::BOOL: {
            bool value = val.toBool();
            if (sw.idx == StyleId::harmonyVoiceLiteral) { // special case for bool represented by a two-item combobox
                voicingSelectWidget->interpretBox->setCurrentIndex(value);
            } else if (isBoolStyleRepresentedByButtonGroup(sw.idx)) { // special case for bool represented by a two-item buttonGroup
                qobject_cast<QButtonGroup*>(sw.widget)->button(1)->setChecked(value);
                qobject_cast<QButtonGroup*>(sw.widget)->button(0)->setChecked(!value);
            } else if (sw.idx == StyleId::lyricsDashForce || sw.idx == StyleId::lyricsMelismaForce) { // special case where UI is presented with opposite wording
                sw.widget->setProperty("checked", !value);
            } else {
                if (!sw.widget->setProperty("checked", value)) {
                    unhandledType(sw);
                }
                if (sw.idx == StyleId::measureNumberSystem && !value) {
                    showIntervalMeasureNumber->setChecked(true);
                }
            }
        } break;
        case P_TYPE::PLACEMENT_H:
        case P_TYPE::PLACEMENT_V:
        case P_TYPE::BARLINE_TYPE:
        case P_TYPE::LINE_TYPE:
        case P_TYPE::HOOK_TYPE:
        case P_TYPE::DYNAMIC_TYPE:
        case P_TYPE::ACCIDENTAL_ROLE:
        case P_TYPE::TIE_PLACEMENT:
        case P_TYPE::INT: {
            int value = val.toInt();
            if (qobject_cast<QComboBox*>(sw.widget)) {
                QComboBox* cb = qobject_cast<QComboBox*>(sw.widget);
                cb->setCurrentIndex(cb->findData(value));
            } else if (qobject_cast<QSpinBox*>(sw.widget)) {
                qobject_cast<QSpinBox*>(sw.widget)->setValue(value * (sw.showPercent ? 100 : 1));
            } else if (qobject_cast<QButtonGroup*>(sw.widget)) {
                QButtonGroup* bg = qobject_cast<QButtonGroup*>(sw.widget);
                for (auto a : bg->buttons()) {
                    if (bg->id(a) == val.toInt()) {
                        a->setChecked(true);
                        break;
                    }
                }
            } else if (FontStyleSelect* fontStyle = qobject_cast<FontStyleSelect*>(sw.widget)) {
                fontStyle->setFontStyle(FontStyle(value));
            } else {
                unhandledType(sw);
            }
        } break;
        case P_TYPE::STRING: {
            QString value = val.value<String>();
            if (qobject_cast<QFontComboBox*>(sw.widget)) {
                static_cast<QFontComboBox*>(sw.widget)->setCurrentFont(QFont(value));
            } else if (qobject_cast<QComboBox*>(sw.widget)) {
                QComboBox* cb = qobject_cast<QComboBox*>(sw.widget);
                for (int i = 0; i < cb->count(); ++i) {
                    if (cb->itemData(i) == value) {
                        cb->setCurrentIndex(i);
                        break;
                    }
                }
            } else if (qobject_cast<QTextEdit*>(sw.widget)) {
                static_cast<QTextEdit*>(sw.widget)->setPlainText(value);
            } else {
                unhandledType(sw);
            }
        } break;
        case P_TYPE::DIRECTION_V: {
            QComboBox* cb = qobject_cast<QComboBox*>(sw.widget);
            if (cb) {
                cb->setCurrentIndex(int(val.value<DirectionV>()));
            } else {
                unhandledType(sw);
            }
        } break;
        case P_TYPE::ALIGN: {
            AlignSelect* as = qobject_cast<AlignSelect*>(sw.widget);
            as->setAlign(val.value<Align>());
        } break;
        case P_TYPE::POINT: {
            OffsetSelect* as = qobject_cast<OffsetSelect*>(sw.widget);
            if (as) {
                as->setOffset(val.value<muse::PointF>().toQPointF());
            }
        } break;
        default: {
            unhandledType(sw);
        } break;
        }

        if (sw.widget) {
            sw.widget->blockSignals(false);
        }
    }

    textStyleChanged(textStyles->currentRow());

    emit dynamicsAndHairpinPos->currentIndexChanged(dynamicsAndHairpinPos->currentIndex());
    resetDynamicsAndHairpinPos->setEnabled(!dynamicsAndHairpinPosPropertiesHaveDefaultStyleValue());

    //TODO: convert the rest:

    muse::ByteArray ba = styleValue(StyleId::swingUnit).value<String>().toAscii();
    DurationType unit = TConv::fromXml(ba.constChar(), DurationType::V_INVALID);

    if (unit == DurationType::V_EIGHTH) {
        swingEighth->setChecked(true);
        swingBox->setEnabled(true);
    } else if (unit == DurationType::V_16TH) {
        swingSixteenth->setChecked(true);
        swingBox->setEnabled(true);
    } else if (unit == DurationType::V_ZERO) {
        swingOff->setChecked(true);
        swingBox->setEnabled(false);
    }
    QString s(styleValue(StyleId::chordDescriptionFile).value<String>());
    chordDescriptionFile->setText(s);
    QString cstyle(styleValue(StyleId::chordStyle).value<String>());
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
        if (comboFBFont->itemText(i) == styleValue(StyleId::figuredBassFontFamily).value<String>()) {
            comboFBFont->setCurrentIndex(i);
            break;
        }
    }
    doubleSpinFBSize->setValue(styleValue(StyleId::figuredBassFontSize).toDouble());
    doubleSpinFBVertPos->setValue(styleValue(StyleId::figuredBassYOffset).toDouble());
    spinFBLineHeight->setValue(styleValue(StyleId::figuredBassLineHeight).toDouble() * 100.0);

    QString mfont(styleValue(StyleId::musicalSymbolFont).value<String>());
    int idx = 0;
    for (const auto& i : engravingFonts()->fonts()) {
        if (QString::fromStdString(i->name()).toLower() == mfont.toLower()) {
            musicalSymbolFont->setCurrentIndex(idx);
            break;
        }
        ++idx;
    }

    QString dynFont(styleValue(StyleId::dynamicsFont).value<String>());
    idx = 0;
    for (const auto& i : engravingFonts()->fonts()) {
        if (QString::fromStdString(i->name()).toLower() == dynFont.toLower()) {
            dynamicsFont->setCurrentIndex(idx);
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
    musicalTextFont->addItem("Finale Maestro Text", "Finale Maestro Text");
    musicalTextFont->addItem("Finale Broadway Text", "Finale Broadway Text");
    QString tfont(styleValue(StyleId::musicalTextFont).value<String>());
    idx = musicalTextFont->findData(tfont);
    musicalTextFont->setCurrentIndex(idx);
    musicalTextFont->blockSignals(false);

    toggleHeaderOddEven(styleValue(StyleId::headerOddEven).toBool());
    toggleFooterOddEven(styleValue(StyleId::footerOddEven).toBool());
    disableVerticalSpread->setChecked(!styleValue(StyleId::enableVerticalSpread).toBool());

    for (const LineStyleSelect* lineStyleSelect : m_lineStyleSelects) {
        lineStyleSelect->update();
    }

    singleMeasureMMRestOption->setEnabled(styleValue(StyleId::minEmptyMeasures).toInt() == 1);
    singleMMRestShowNumber->setEnabled(styleValue(StyleId::singleMeasureMMRestUseNormalRest).toBool());
    mmRestSingleUseHBar->setEnabled(!styleValue(StyleId::oldStyleMultiMeasureRests).toBool());
    mmRestRefDuration->setEnabled(styleValue(StyleId::mmRestConstantWidth).toBool());

    updateParenthesisIndicatingTiesGroupState();
}

//---------------------------------------------------------
//   selectChordDescriptionFile
//---------------------------------------------------------

void EditStyle::selectChordDescriptionFile()
{
    muse::io::path_t dir = configuration()->userStylesPath();
    std::vector<std::string> filter = { muse::trc("notation", "MuseScore chord symbol style files") + " (*.xml)" };

    muse::io::path_t path = interactive()->selectOpeningFile(muse::qtrc("notation", "Load style"), dir, filter);
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
    QString val;
    if (swingOff->isChecked()) {
        val = TConv::toXml(DurationType::V_ZERO).ascii();
        swingBox->setEnabled(false);
    } else if (swingEighth->isChecked()) {
        val = TConv::toXml(DurationType::V_EIGHTH).ascii();
        swingBox->setEnabled(true);
    } else if (swingSixteenth->isChecked()) {
        val = TConv::toXml(DurationType::V_16TH).ascii();
        swingBox->setEnabled(true);
    }

    setStyleQVariantValue(StyleId::swingUnit, val);
}

PropertyValue EditStyle::styleValue(StyleId id) const
{
    return globalContext()->currentNotation()->style()->styleValue(id);
}

PropertyValue EditStyle::defaultStyleValue(StyleId id) const
{
    return globalContext()->currentNotation()->style()->defaultStyleValue(id);
}

bool EditStyle::hasDefaultStyleValue(StyleId id) const
{
    return defaultStyleValue(id) == styleValue(id);
}

bool EditStyle::dynamicsAndHairpinPosPropertiesHaveDefaultStyleValue() const
{
    return hasDefaultStyleValue(StyleId::dynamicsHairpinsAutoCenterOnGrandStaff)
           && hasDefaultStyleValue(StyleId::dynamicsHairpinsAboveForVocalStaves)
           && hasDefaultStyleValue(StyleId::dynamicsHairpinVoiceBasedPlacement);
}

void EditStyle::setStyleQVariantValue(StyleId id, const QVariant& value)
{
    setStyleValue(id, PropertyValue::fromQVariant(value, MStyle::valueType(id)));
}

void EditStyle::setStyleValue(StyleId id, const PropertyValue& value)
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
    String val;
    String file;
    bool chordsXml;
    if (chordsStandard->isChecked()) {
        val  = u"std";
        file = u"chords_std.xml";
        chordsXml = false;
    } else if (chordsJazz->isChecked()) {
        val  = u"jazz";
        file = u"chords_jazz.xml";
        chordsXml = false;
    } else {
        val = u"custom";
        chordDescriptionGroup->setEnabled(true);
        file = chordDescriptionFile->text();
        chordsXml = chordsXmlFile->isChecked();
    }
    if (val != u"custom") {
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
        sw.reset->setEnabled(enable && !hasDefaultStyleValue(idx));
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

const EditStyle::StyleWidget& EditStyle::styleWidget(StyleId idx) const
{
    for (const StyleWidget& sw : styleWidgets) {
        if (sw.idx == idx) {
            return sw;
        }
    }
    UNREACHABLE;
    static EditStyle::StyleWidget dummy;
    return dummy;
}

//---------------------------------------------------------
//   valueChanged
//---------------------------------------------------------

void EditStyle::valueChanged(int i)
{
    StyleId idx       = (StyleId)i;
    PropertyValue val  = getValue(idx);
    bool setValue = false;
    if (idx == StyleId::musicalSymbolFont) {
        bool overrideDynamicsFont = getValue(StyleId::dynamicsOverrideFont).toBool();
        if (!overrideDynamicsFont) {
            setStyleValue(StyleId::dynamicsFont, val); // Match dynamics font
        }
        if (optimizeStyleCheckbox->isChecked()) {
            IEngravingFontPtr scoreFont = engravingFonts()->fontByName(val.value<String>().toStdString());
            if (scoreFont) {
                for (auto j : scoreFont->engravingDefaults()) {
                    setStyleValue(j.first, j.second);
                }
            }
            setValue = true;
        }
    }

    setStyleValue(idx, val);
    if (setValue) {
        setValues();
    }
    const StyleWidget& sw = styleWidget(idx);
    if (sw.reset) {
        sw.reset->setEnabled(!hasDefaultStyleValue(idx));
    }

    if (idx == StyleId::dynamicsHairpinVoiceBasedPlacement || idx == StyleId::dynamicsHairpinsAutoCenterOnGrandStaff
        || idx == StyleId::dynamicsHairpinsAboveForVocalStaves) {
        resetDynamicsAndHairpinPos->setEnabled(!dynamicsAndHairpinPosPropertiesHaveDefaultStyleValue());
    }

    if (idx == StyleId::minEmptyMeasures) {
        singleMeasureMMRestOption->setEnabled(styleValue(StyleId::minEmptyMeasures).toInt() == 1);
    }
    if (idx == StyleId::singleMeasureMMRestUseNormalRest) {
        singleMMRestShowNumber->setEnabled(styleValue(StyleId::singleMeasureMMRestUseNormalRest).toBool());
    }
    if (idx == StyleId::mmRestConstantWidth) {
        mmRestRefDuration->setEnabled(styleValue(StyleId::mmRestConstantWidth).toBool());
    }
    if (idx == StyleId::oldStyleMultiMeasureRests) {
        bool useOldStyle = styleValue(StyleId::oldStyleMultiMeasureRests).toBool();
        if (useOldStyle && !styleValue(StyleId::singleMeasureMMRestUseNormalRest).toBool()) {
            setStyleValue(StyleId::singleMeasureMMRestUseNormalRest, true);
            setValues();
        }
        mmRestSingleUseHBar->setEnabled(!useOldStyle);
    }
}

//---------------------------------------------------------
//   resetStyleValue
//---------------------------------------------------------

void EditStyle::resetStyleValue(int i)
{
    StyleId idx = (StyleId)i;

    setStyleValue(idx, defaultStyleValue(idx));
    if (idx == StyleId::dynamicsHairpinVoiceBasedPlacement) {
        for (StyleId id : { StyleId::dynamicsHairpinsAutoCenterOnGrandStaff, StyleId::dynamicsHairpinsAboveForVocalStaves }) {
            setStyleValue(id, defaultStyleValue(id));
        }
    }
    setValues();
}

//---------------------------------------------------------
//   textStyleChanged
//---------------------------------------------------------

void EditStyle::textStyleChanged(int row)
{
    TextStyleType tid = TextStyleType(textStyles->item(row)->data(Qt::UserRole).toInt());
    const TextStyle* ts = textStyle(tid);

    for (const auto& a : *ts) {
        switch (a.type) {
        case TextStylePropertyType::FontFace: {
            PropertyValue val = styleValue(a.sid);
            textStyleFontFace->setCurrentFont(QFont(val.value<String>()));
            resetTextStyleFontFace->setEnabled(val != defaultStyleValue(a.sid));
        }
        break;

        case TextStylePropertyType::FontSize:
            textStyleFontSize->setValue(styleValue(a.sid).toDouble());
            resetTextStyleFontSize->setEnabled(styleValue(a.sid) != defaultStyleValue(a.sid));
            break;

        case TextStylePropertyType::LineSpacing:
            textStyleLineSpacing->setValue(styleValue(a.sid).toDouble());
            resetTextStyleLineSpacing->setEnabled(styleValue(a.sid) != defaultStyleValue(a.sid));
            break;

        case TextStylePropertyType::MusicalSymbolsScale:
            textStyleMusicalSymbolsScale->setValue(styleValue(a.sid).toDouble() * 100);
            resetTextStyleMusicalSymbolsScale->setEnabled(styleValue(a.sid) != defaultStyleValue(a.sid));
            break;

        case TextStylePropertyType::FontStyle:
            textStyleFontStyle->setFontStyle(FontStyle(styleValue(a.sid).toInt()));
            resetTextStyleFontStyle->setEnabled(styleValue(a.sid) != defaultStyleValue(a.sid));
            break;

        case TextStylePropertyType::TextAlign:
            textStyleAlign->setAlign(styleValue(a.sid).value<Align>());
            resetTextStyleAlign->setEnabled(styleValue(a.sid) != defaultStyleValue(a.sid));
            break;

        case TextStylePropertyType::Offset:
            textStyleOffset->setOffset(styleValue(a.sid).value<PointF>().toQPointF());
            resetTextStyleOffset->setEnabled(styleValue(a.sid) != defaultStyleValue(a.sid));
            break;

        case TextStylePropertyType::SizeSpatiumDependent: {
            PropertyValue val = styleValue(a.sid);
            textStyleSpatiumDependent->setChecked(val.toBool());
            resetTextStyleSpatiumDependent->setEnabled(val != defaultStyleValue(a.sid));
            textStyleOffset->setSuffix(val.toBool() ? muse::qtrc("global", "sp") : muse::qtrc("global", "mm"));
        }
        break;

        case TextStylePropertyType::FrameType:
            textStyleFrameType->setCurrentIndex(styleValue(a.sid).toInt());
            resetTextStyleFrameType->setEnabled(styleValue(a.sid) != defaultStyleValue(a.sid));
            frameWidget->setEnabled(styleValue(a.sid).toInt() != 0);             // disable if no frame
            break;

        case TextStylePropertyType::FramePadding:
            textStyleFramePadding->setValue(styleValue(a.sid).toDouble());
            resetTextStyleFramePadding->setEnabled(styleValue(a.sid) != defaultStyleValue(a.sid));
            break;

        case TextStylePropertyType::FrameWidth:
            textStyleFrameBorder->setValue(styleValue(a.sid).toDouble());
            resetTextStyleFrameBorder->setEnabled(styleValue(a.sid) != defaultStyleValue(a.sid));
            break;

        case TextStylePropertyType::FrameRound:
            textStyleFrameBorderRadius->setValue(double(styleValue(a.sid).toInt()));
            resetTextStyleFrameBorderRadius->setEnabled(styleValue(a.sid) != defaultStyleValue(a.sid));
            break;

        case TextStylePropertyType::FrameBorderColor:
            textStyleFrameForeground->setColor(styleValue(a.sid).value<Color>().toQColor());
            resetTextStyleFrameForeground->setEnabled(styleValue(a.sid) != defaultStyleValue(a.sid));
            break;

        case TextStylePropertyType::FrameFillColor:
            textStyleFrameBackground->setColor(styleValue(a.sid).value<Color>().toQColor());
            resetTextStyleFrameBackground->setEnabled(styleValue(a.sid) != defaultStyleValue(a.sid));
            break;

        case TextStylePropertyType::Color:
            textStyleColor->setColor(styleValue(a.sid).value<Color>().toQColor());
            resetTextStyleColor->setEnabled(styleValue(a.sid) != defaultStyleValue(a.sid));
            break;

        default:
            break;
        }
    }

    Score* score = globalContext()->currentNotation()->elements()->msScore();

    styleName->setText(score->getTextStyleUserName(tid).qTranslated());
    styleName->setEnabled(int(tid) >= int(TextStyleType::USER1));
    resetTextStyleName->setEnabled(styleName->text() != TConv::translatedUserName(tid));

    tupletUseSymbols->setVisible(tid == TextStyleType::TUPLET);
    resetTupletUseSymbols->setVisible(tid == TextStyleType::TUPLET);

    configuration()->setStyleDialogLastSubPageIndex(row);
}

//---------------------------------------------------------
//   textStyleValueChanged
//---------------------------------------------------------

void EditStyle::textStyleValueChanged(TextStylePropertyType type, QVariant value)
{
    TextStyleType tid = TextStyleType(textStyles->currentItem()->data(Qt::UserRole).toInt());
    const TextStyle* ts = textStyle(tid);

    for (const auto& a : *ts) {
        if (a.type == type) {
            if (type == TextStylePropertyType::MusicalSymbolsScale) {
                setStyleQVariantValue(a.sid, value.toDouble() / 100);
            } else {
                setStyleQVariantValue(a.sid, value);
            }
            break;
        }
    }
    textStyleChanged(textStyles->currentRow()); // update GUI (reset buttons)
}

//---------------------------------------------------------
//   resetTextStyle
//---------------------------------------------------------

void EditStyle::resetTextStyle(TextStylePropertyType type)
{
    TextStyleType tid = TextStyleType(textStyles->currentItem()->data(Qt::UserRole).toInt());
    const TextStyle* ts = textStyle(tid);

    for (const auto& a : *ts) {
        if (a.type == type) {
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
    TextStyleType tid = TextStyleType(textStyles->item(row)->data(Qt::UserRole).toInt());
    textStyles->item(row)->setText(styleName->text());
    resetTextStyleName->setEnabled(styleName->text() != TConv::translatedUserName(tid));
}

//---------------------------------------------------------
//   endEditUserStyleName
//---------------------------------------------------------

void EditStyle::endEditUserStyleName()
{
    int row = textStyles->currentRow();
    TextStyleType tid = TextStyleType(textStyles->item(row)->data(Qt::UserRole).toInt());
    int idx = int(tid) - int(TextStyleType::USER1);
    if (int(tid) < int(TextStyleType::USER1) || int(tid) > int(TextStyleType::USER12)) {
        LOGD("User style index %d outside of range.", idx);
        return;
    }
    StyleId sid[]
        = { StyleId::user1Name, StyleId::user2Name, StyleId::user3Name, StyleId::user4Name, StyleId::user5Name, StyleId::user6Name,
            StyleId::user7Name, StyleId::user8Name, StyleId::user9Name, StyleId::user10Name, StyleId::user11Name, StyleId::user12Name };
    QString name = styleName->text();
    setStyleValue(sid[idx], name);
    if (name == "") {
        name = TConv::translatedUserName(tid);
        styleName->setText(name);
        textStyles->item(row)->setText(name);
        resetTextStyleName->setEnabled(false);
    }
}

//---------------------------------------------------------
//   resetUserStyleName
//---------------------------------------------------------

void EditStyle::resetUserStyleName()
{
    styleName->clear();
    endEditUserStyleName();
}

void EditStyle::updateParenthesisIndicatingTiesGroupState()
{
    tieParen->setEnabled(tabShowTies->isChecked() || tabShowNone->isChecked());
}

void EditStyle::clefVisibilityChanged(bool checked)
{
    if (!checked) {
        return;
    }
    if (radioHideClefs->isChecked()) {
        hideTabClefs->setChecked(true);
        hideTabClefs->setEnabled(false);
    } else {
        hideTabClefs->setEnabled(true);
    }
}
