//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2016 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "editstyle.h"
#include "icons.h"
#include "musescore.h"
#include "preferences.h"
#include "scoreview.h"

#include "global/log.h"

#include "libmscore/clef.h"
#include "libmscore/excerpt.h"
#include "libmscore/figuredbass.h"
#include "libmscore/layout.h"
#include "libmscore/score.h"
#include "libmscore/style.h"
#include "libmscore/sym.h"
#include "libmscore/tuplet.h"
#include "libmscore/undo.h"

#include "inspector/alignSelect.h"
#include "inspector/fontStyleSelect.h"
#include "inspector/offsetSelect.h"

namespace Ms {

const char* lineStyles[] = {
      QT_TRANSLATE_NOOP("EditStyleBase", "Continuous"),
      QT_TRANSLATE_NOOP("EditStyleBase", "Dashed"),
      QT_TRANSLATE_NOOP("EditStyleBase", "Dotted"),
      QT_TRANSLATE_NOOP("EditStyleBase", "Dash-dotted"),
      QT_TRANSLATE_NOOP("EditStyleBase", "Dash-dot-dotted")
};

//---------------------------------------------------------
//   EditStyle
//---------------------------------------------------------

EditStyle::EditStyle(Score* s, QWidget* parent)
   : AbstractDialog(parent), cs(s)
      {
      setObjectName("EditStyle");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      buttonApplyToAllParts = buttonBox->addButton(tr("Apply to all Parts"), QDialogButtonBox::ApplyRole);
      buttonTogglePagelist->setIcon(QIcon(*icons[int(Icons::goNext_ICON)]));

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

      int dta = 0;
      voltaLineStyle->clear();
      ottavaLineStyle->clear();
      pedalLineStyle->clear();
      for (const char* p : lineStyles) {
            QString trs = qApp->translate("EditStyleBase", p);
            voltaLineStyle->addItem(trs, dta);
            ottavaLineStyle->addItem(trs, dta);
            pedalLineStyle->addItem(trs, dta);
            ++dta;
            }

      styleWidgets = {
      //   idx                --- showPercent      --- widget          --- resetButton
      { Sid::figuredBassAlignment,    false, fbAlign,                 0                        },
      { Sid::figuredBassStyle,        false, fbStyle,                 0                        },
      { Sid::figuredBassFontSize,     false, doubleSpinFBSize,        resetDoubleSpinFBSize    },
      { Sid::figuredBassYOffset,      false, doubleSpinFBVertPos,     resetDoubleSpinFBVertPos },
      { Sid::figuredBassLineHeight,   true,  spinFBLineHeight,        resetSpinFBLineHeight    },
      { Sid::tabClef,                 false, ctg,                     0                        },
      { Sid::keySigNaturals,          false, ksng,                    0                        },
      { Sid::voltaLineStyle,          false, voltaLineStyle,          resetVoltaLineStyle      },
      { Sid::ottavaLineStyle,         false, ottavaLineStyle,         resetOttavaLineStyle     },
      { Sid::pedalLineStyle,          false, pedalLineStyle,          resetPedalLineStyle      },

      { Sid::staffUpperBorder,        false, staffUpperBorder,        resetStaffUpperBorder    },
      { Sid::staffLowerBorder,        false, staffLowerBorder,        resetStaffLowerBorder    },
      { Sid::staffDistance,           false, staffDistance,           resetStaffDistance       },
      { Sid::akkoladeDistance,        false, akkoladeDistance,        resetAkkoladeDistance    },
      { Sid::enableVerticalSpread,    false, enableVerticalSpread,    0                        },
      { Sid::minSystemDistance,       false, minSystemDistance,       resetMinSystemDistance   },
      { Sid::maxSystemDistance,       false, maxSystemDistance,       resetMaxSystemDistance   },
      { Sid::spreadSystem,            false, spreadSystem,            resetSpreadSystem        },
      { Sid::spreadSquareBracket,     false, spreadSquareBracket,     resetSpreadSquareBracket },
      { Sid::spreadCurlyBracket,      false, spreadCurlyBracket,      resetSpreadCurlyBracket  },
      { Sid::minSystemSpread,         false, minSystemSpread,         resetMinSystemSpread     },
      { Sid::maxSystemSpread,         false, maxSystemSpread,         resetMaxSystemSpread     },
      { Sid::minStaffSpread,          false, minStaffSpread,          resetMinStaffSpread      },
      { Sid::maxStaffSpread,          false, maxStaffSpread,          resetMaxStaffSpread      },
      { Sid::maxAkkoladeDistance,     false, maxAkkoladeDistance,     resetMaxAkkoladeDistance },
      { Sid::maxPageFillSpread,       false, maxPageFillSpread,       resetMaxPageFillSpread },

      { Sid::lyricsPlacement,         false, lyricsPlacement,         resetLyricsPlacement         },
      { Sid::lyricsPosAbove,          false, lyricsPosAbove,          resetLyricsPosAbove          },
      { Sid::lyricsPosBelow,          false, lyricsPosBelow,          resetLyricsPosBelow          },
      { Sid::lyricsMinTopDistance,    false, lyricsMinTopDistance,    resetLyricsMinTopDistance    },
      { Sid::lyricsMinBottomDistance, false, lyricsMinBottomDistance, resetLyricsMinBottomDistance },
      { Sid::lyricsMinDistance,       false, lyricsMinDistance,       resetLyricsMinDistance       },
      { Sid::lyricsLineHeight,        true,  lyricsLineHeight,        resetLyricsLineHeight        },
      { Sid::lyricsDashMinLength,     false, lyricsDashMinLength,     resetLyricsDashMinLength     },
      { Sid::lyricsDashMaxLength,     false, lyricsDashMaxLength,     resetLyricsDashMaxLength     },
      { Sid::lyricsDashMaxDistance,   false, lyricsDashMaxDistance,   resetLyricsDashMaxDistance   },
      { Sid::lyricsDashForce,         false, lyricsDashForce,         resetLyricsDashForce         },
      { Sid::lyricsAlignVerseNumber,  false, lyricsAlignVerseNumber,  resetLyricsAlignVerseNumber  },
      { Sid::lyricsLineThickness,     false, lyricsLineThickness,     resetLyricsLineThickness     },
      { Sid::lyricsMelismaPad,        false, lyricsMelismaPad,        resetLyricsMelismaPad        },
      { Sid::lyricsMelismaAlign,      false, lyricsMelismaAlign,      resetLyricsMelismaAlign      },
      { Sid::lyricsDashPad,           false, lyricsDashPad,           resetLyricsDashPad           },
      { Sid::lyricsDashLineThickness, false, lyricsDashLineThickness, resetLyricsDashLineThickness },
      { Sid::lyricsDashYposRatio,     false, lyricsDashYposRatio,     resetLyricsDashYposRatio     },

      { Sid::systemFrameDistance,     false, systemFrameDistance,     resetSystemFrameDistance },
      { Sid::frameSystemDistance,     false, frameSystemDistance,     resetFrameSystemDistance },
      { Sid::minMeasureWidth,         false, minMeasureWidth_2,       resetMinMeasureWidth },
      { Sid::measureSpacing,          false, measureSpacing,          resetMeasureSpacing },

      { Sid::barWidth,                false, barWidth,                resetBarWidth },
      { Sid::endBarWidth,             false, endBarWidth,             resetEndBarWidth },
      { Sid::endBarDistance,          false, endBarDistance,          resetEndBarDistance },
      { Sid::doubleBarWidth,          false, doubleBarWidth,          resetDoubleBarWidth },
      { Sid::doubleBarDistance,       false, doubleBarDistance,       resetDoubleBarDistance },
      { Sid::repeatBarlineDotSeparation, false, repeatBarlineDotSeparation, resetRepeatBarlineDotSeparation },

      { Sid::barGraceDistance,        false, barGraceDistance,        resetBarGraceDistance },
      { Sid::chordExtensionMag,       false, extensionMag,            resetExtensionMag },
      { Sid::chordExtensionAdjust,    false, extensionAdjust,         resetExtensionAdjust },
      { Sid::chordModifierMag,        false, modifierMag,             resetModifierMag },
      { Sid::chordModifierAdjust,     false, modifierAdjust,          resetModifierAdjust },
      { Sid::useStandardNoteNames,    false, useStandardNoteNames,    0 },
      { Sid::useGermanNoteNames,      false, useGermanNoteNames,      0 },
      { Sid::useFullGermanNoteNames,  false, useFullGermanNoteNames,  0 },
      { Sid::useSolfeggioNoteNames,   false, useSolfeggioNoteNames,   0 },
      { Sid::useFrenchNoteNames,      false, useFrenchNoteNames,      0 },
      { Sid::automaticCapitalization, false, automaticCapitalization, 0 },

      { Sid::lowerCaseMinorChords,    false, lowerCaseMinorChords,    0 },

      { Sid::lowerCaseBassNotes,      false, lowerCaseBassNotes,      0 },
      { Sid::allCapsNoteNames,        false, allCapsNoteNames,        0 },
      { Sid::concertPitch,            false, concertPitch,            0 },
      { Sid::createMultiMeasureRests, false, multiMeasureRests,       0 },
      { Sid::minEmptyMeasures,        false, minEmptyMeasures,        resetMinEmptyMeasures },
      { Sid::minMMRestWidth,          false, minMeasureWidth,         resetMinMMRestWidth   },
      { Sid::mmRestNumberPos,         false, mmRestNumberPos,         resetMMRestNumberPos  },
      { Sid::hideEmptyStaves,         false, hideEmptyStaves,         0 },
      { Sid::dontHideStavesInFirstSystem, false, dontHideStavesInFirstSystem, 0 },
      { Sid::enableIndentationOnFirstSystem, false, enableIndentationOnFirstSystem, 0 },
      { Sid::firstSystemIndentationValue, false, indentationValue, resetFirstSystemIndentation },
      { Sid::alwaysShowBracketsWhenEmptyStavesAreHidden, false, alwaysShowBrackets, 0 },
      { Sid::hideInstrumentNameIfOneInstrument, false, hideInstrumentNameIfOneInstrument, 0 },
      { Sid::accidentalNoteDistance,  false, accidentalNoteDistance,  resetAccidentalNoteDistance },
      { Sid::accidentalDistance,      false, accidentalDistance,      resetAccidentalDistance },
      { Sid::bracketedAccidentalPadding, false, accidentalsBracketsBadding,     resetAccidentalsBracketPadding },
      { Sid::alignAccidentalsLeft,    false, accidentalsOctaveColumnsAlignLeft, resetAccidentalsOctaveColumnsAlignLeft },

      { Sid::minNoteDistance,         false, minNoteDistance,         resetMinNoteDistance },
      { Sid::barNoteDistance,         false, barNoteDistance,         resetBarNoteDistance },
      { Sid::barAccidentalDistance,   false, barAccidentalDistance,   resetBarAccidentalDistance },
      { Sid::multiMeasureRestMargin,  false, multiMeasureRestMargin,  resetMultiMeasureRestMargin },
      { Sid::noteBarDistance,         false, noteBarDistance,         resetNoteBarDistance },
      { Sid::clefLeftMargin,          false, clefLeftMargin,          resetClefLeftMargin },
      { Sid::keysigLeftMargin,        false, keysigLeftMargin,        resetKeysigLeftMargin },
      { Sid::timesigLeftMargin,       false, timesigLeftMargin,       resetTimesigLeftMargin },
      { Sid::midClefKeyRightMargin,   false, clefKeyRightMargin,      resetClefKeyRightMargin },
      { Sid::clefKeyDistance,         false, clefKeyDistance,         resetClefKeyDistance },
      { Sid::clefTimesigDistance,     false, clefTimesigDistance,     resetClefTimesigDistance },
      { Sid::keyTimesigDistance,      false, keyTimesigDistance,      resetKeyTimesigDistance },
      { Sid::keyBarlineDistance,      false, keyBarlineDistance,      resetKeyBarlineDistance },
      { Sid::systemHeaderDistance,    false, systemHeaderDistance,    resetSystemHeaderDistance },
      { Sid::systemHeaderTimeSigDistance,    false, systemHeaderTimeSigDistance,    resetSystemHeaderTimeSigDistance },

      { Sid::clefBarlineDistance,     false, clefBarlineDistance,     resetClefBarlineDistance },
      { Sid::timesigBarlineDistance,  false, timesigBarlineDistance,  resetTimesigBarlineDistance },
      { Sid::staffLineWidth,          false, staffLineWidth,          resetStaffLineWidth },
      { Sid::beamWidth,               false, beamWidth,               resetBeamWidth },
      { Sid::beamMinLen,              false, beamMinLen,              resetBeamMinLen },

      { Sid::hairpinPlacement,        false, hairpinPlacement,        resetHairpinPlacement },
      { Sid::hairpinPosAbove,         false, hairpinPosAbove,         resetHairpinPosAbove },
      { Sid::hairpinPosBelow,         false, hairpinPosBelow,         resetHairpinPosBelow },
      { Sid::hairpinLineWidth,        false, hairpinLineWidth,        resetHairpinLineWidth },
      { Sid::hairpinHeight,           false, hairpinHeight,           resetHairpinHeight },
      { Sid::hairpinContHeight,       false, hairpinContinueHeight,   resetHairpinContinueHeight },

      { Sid::dotNoteDistance,         false, noteDotDistance,         resetNoteDotDistance },
      { Sid::dotDotDistance,          false, dotDotDistance,          resetDotDotDistance },
      { Sid::stemWidth,               false, stemWidth,               resetStemWidth },
      { Sid::ledgerLineWidth,         false, ledgerLineWidth,         resetLedgerLineWidth },
      { Sid::ledgerLineLength,        false, ledgerLineLength,        resetLedgerLineLength },
      { Sid::shortStemProgression,    false, shortStemProgression,    resetShortStemProgression },
      { Sid::shortestStem,            false, shortestStem,            resetShortestStem },
      { Sid::arpeggioNoteDistance,    false, arpeggioNoteDistance,    resetArpeggioNoteDistance },
      { Sid::arpeggioLineWidth,       false, arpeggioLineWidth,       resetArpeggioLineWidth },
      { Sid::arpeggioHookLen,         false, arpeggioHookLen,         resetArpeggioHookLen },
      { Sid::arpeggioHiddenInStdIfTab,false, arpeggioHiddenInStdIfTab,0 },
      { Sid::slurEndWidth,            false, slurEndLineWidth,        resetSlurEndLineWidth    },
      { Sid::slurMidWidth,            false, slurMidLineWidth,        resetSlurMidLineWidth    },
      { Sid::slurDottedWidth,         false, slurDottedLineWidth,     resetSlurDottedLineWidth },
      { Sid::slurMinDistance,         false, slurMinDistance,         resetSlurMinDistance     },
      { Sid::tieEndWidth,             false, tieEndLineWidth,         resetTieEndLineWidth     },
      { Sid::tieMidWidth,             false, tieMidLineWidth,         resetTieMidLineWidth     },
      { Sid::tieDottedWidth,          false, tieDottedLineWidth,      resetTieDottedLineWidth  },
      { Sid::tieMinDistance,          false, tieMinDistance,          resetTieMinDistance      },
      { Sid::minTieLength,            false, minTieLength,            resetMinTieLength        },
      { Sid::bracketWidth,            false, bracketWidth,            resetBracketWidth },
      { Sid::bracketDistance,         false, bracketDistance,         resetBracketDistance },
      { Sid::akkoladeWidth,           false, akkoladeWidth,           resetAkkoladeWidth },
      { Sid::akkoladeBarDistance,     false, akkoladeBarDistance,     resetAkkoladeBarDistance },
      { Sid::dividerLeft,             false, dividerLeft,             0 },
      { Sid::dividerLeftX,            false, dividerLeftX,            0 },
      { Sid::dividerLeftY,            false, dividerLeftY,            0 },
      { Sid::dividerRight,            false, dividerRight,            0 },
      { Sid::dividerRightX,           false, dividerRightX,           0 },
      { Sid::dividerRightY,           false, dividerRightY,           0 },
      { Sid::propertyDistanceHead,    false, propertyDistanceHead,    resetPropertyDistanceHead },
      { Sid::propertyDistanceStem,    false, propertyDistanceStem,    resetPropertyDistanceStem },
      { Sid::propertyDistance,        false, propertyDistance,        resetPropertyDistance },
      { Sid::voltaPosAbove,           false, voltaPosAbove,           resetVoltaPosAbove },
      { Sid::voltaHook,               false, voltaHook,               resetVoltaHook },
      { Sid::voltaLineWidth,          false, voltaLineWidth,          resetVoltaLineWidth  },

      { Sid::ottavaPosAbove,          false, ottavaPosAbove,          resetOttavaPosAbove  },
      { Sid::ottavaPosBelow,          false, ottavaPosBelow,          resetOttavaPosBelow  },
      { Sid::ottavaHookAbove,         false, ottavaHookAbove,         resetOttavaHookAbove },
      { Sid::ottavaHookBelow,         false, ottavaHookBelow,         resetOttavaHookBelow },
      { Sid::ottavaLineWidth,         false, ottavaLineWidth,         resetOttavaLineWidth },

      { Sid::pedalPlacement,          false, pedalLinePlacement,      resetPedalLinePlacement  },
      { Sid::pedalPosAbove,           false, pedalLinePosAbove,       resetPedalLinePosAbove   },
      { Sid::pedalPosBelow,           false, pedalLinePosBelow,       resetPedalLinePosBelow   },
      { Sid::pedalLineWidth,          false, pedalLineWidth,          resetPedalLineWidth  },

      { Sid::trillPlacement,          false, trillLinePlacement,      resetTrillLinePlacement  },
      { Sid::trillPosAbove,           false, trillLinePosAbove,       resetTrillLinePosAbove   },
      { Sid::trillPosBelow,           false, trillLinePosBelow,       resetTrillLinePosBelow   },

      { Sid::vibratoPlacement,        false, vibratoLinePlacement,    resetVibratoLinePlacement  },
      { Sid::vibratoPosAbove,         false, vibratoLinePosAbove,     resetVibratoLinePosAbove   },
      { Sid::vibratoPosBelow,         false, vibratoLinePosBelow,     resetVibratoLinePosBelow   },

      { Sid::harmonyFretDist,         false, harmonyFretDist,         resetHarmonyFretDist       },
      { Sid::minHarmonyDistance,      false, minHarmonyDistance,      resetMinHarmonyDistance    },
      { Sid::maxHarmonyBarDistance,   false, maxHarmonyBarDistance,   resetMaxHarmonyBarDistance },
      { Sid::maxChordShiftAbove,      false, maxChordShiftAbove,      resetMaxChordShiftAbove    },
      { Sid::maxChordShiftBelow,      false, maxChordShiftBelow,      resetMaxChordShiftBelow    },
      { Sid::harmonyPlay,             false, harmonyPlay,             0                          },
      { Sid::harmonyVoiceLiteral,     false, voicingSelectWidget->interpretBox, 0 },
      { Sid::harmonyVoicing,          false, voicingSelectWidget->voicingBox, 0 },
      { Sid::harmonyDuration,         false, voicingSelectWidget->durationBox, 0 },

      { Sid::tupletVHeadDistance,     false, tupletVHeadDistance,     resetTupletVHeadDistance      },
      { Sid::tupletVStemDistance,     false, tupletVStemDistance,     resetTupletVStemDistance      },
      { Sid::tupletStemLeftDistance,  false, tupletStemLeftDistance,  resetTupletStemLeftDistance   },
      { Sid::tupletStemRightDistance, false, tupletStemRightDistance, resetTupletStemRightDistance  },
      { Sid::tupletNoteLeftDistance,  false, tupletNoteLeftDistance,  resetTupletNoteLeftDistance   },
      { Sid::tupletNoteRightDistance, false, tupletNoteRightDistance, resetTupletNoteRightDistance  },
      { Sid::tupletBracketWidth,      false, tupletBracketWidth,      resetTupletBracketWidth       },
      { Sid::tupletBracketHookHeight, false, tupletBracketHookHeight, resetTupletBracketHookHeight  },
      { Sid::tupletDirection,         false, tupletDirection,         resetTupletDirection          },
      { Sid::tupletNumberType,        false, tupletNumberType,        resetTupletNumberType         },
      { Sid::tupletBracketType,       false, tupletBracketType,       resetTupletBracketType        },
      { Sid::tupletMaxSlope,          false, tupletMaxSlope,          resetTupletMaxSlope           },
      { Sid::tupletOutOfStaff,        false, tupletOutOfStaff,        0                             },

      { Sid::repeatBarTips,            false, showRepeatBarTips,            resetShowRepeatBarTips },
      { Sid::startBarlineSingle,       false, showStartBarlineSingle,       resetShowStartBarlineSingle },
      { Sid::startBarlineMultiple,     false, showStartBarlineMultiple,     resetShowStartBarlineMultiple },
      { Sid::dividerLeftSym,           false, dividerLeftSym,               0 },
      { Sid::dividerRightSym,          false, dividerRightSym,              0 },

      { Sid::showMeasureNumber,        false, showMeasureNumber,            0 },
      { Sid::showMeasureNumberOne,     false, showFirstMeasureNumber,       0 },
      { Sid::measureNumberInterval,    false, intervalMeasureNumber,        0 },
      { Sid::measureNumberSystem,      false, showEverySystemMeasureNumber, 0 },
      { Sid::measureNumberAllStaves,   false, showAllStavesMeasureNumber,   0 },
      { Sid::measureNumberVPlacement,  false, measureNumberVPlacement,      resetMeasureNumberVPlacement },
      { Sid::measureNumberHPlacement,  false, measureNumberHPlacement,      resetMeasureNumberHPlacement },
      { Sid::measureNumberPosAbove,    false, measureNumberPosAbove,        resetMeasureNumberPosAbove },
      { Sid::measureNumberPosBelow,    false, measureNumberPosBelow,        resetMeasureNumberPosBelow },

      { Sid::mmRestShowMeasureNumberRange, false, mmRestShowMeasureNumberRange,  0 },
      { Sid::mmRestRangeBracketType,       false, mmRestRangeBracketType,        resetMmRestRangeBracketType },
      { Sid::mmRestRangeVPlacement,        false, mmRestRangeVPlacement,         resetMmRestRangeVPlacement },
      { Sid::mmRestRangeHPlacement,        false, mmRestRangeHPlacement,         resetMmRestRangeHPlacement },
      { Sid::mmRestRangePosAbove,          false, mmRestRangePosAbove,           resetMMRestRangePosAbove },
      { Sid::mmRestRangePosBelow,          false, mmRestRangePosBelow,           resetMMRestRangePosBelow },

      { Sid::beamDistance,             true,  beamDistance,                 resetBeamDistance   },
      { Sid::beamNoSlope,              false, beamNoSlope,                  resetBeamNoSlope    },
      { Sid::graceNoteMag,             true,  graceNoteSize,                resetGraceNoteSize  },
      { Sid::smallStaffMag,            true,  smallStaffSize,               resetSmallStaffSize },
      { Sid::smallNoteMag,             true,  smallNoteSize,                resetSmallNoteSize  },
      { Sid::smallClefMag,             true,  smallClefSize,                resetSmallClefSize  },
      { Sid::lastSystemFillLimit,      true,  lastSystemFillThreshold,      resetLastSystemFillThreshold },
      { Sid::genClef,                  false, genClef,                      0 },
      { Sid::genKeysig,                false, genKeysig,                    0 },
      { Sid::genCourtesyTimesig,       false, genCourtesyTimesig,           0 },
      { Sid::genCourtesyKeysig,        false, genCourtesyKeysig,            0 },
      { Sid::genCourtesyClef,          false, genCourtesyClef,              0 },
      { Sid::swingRatio,               false, swingBox,                     0 },
      { Sid::chordsXmlFile,            false, chordsXmlFile,                0 },
      { Sid::dotMag,                   true,  dotMag,                       resetDotMag },
      { Sid::articulationMag,          true,  articulationMag,              resetArticulationMag },
      { Sid::shortenStem,              false, shortenStem,                  0 },
      { Sid::showHeader,               false, showHeader,                   0 },
      { Sid::headerFirstPage,          false, showHeaderFirstPage,          0 },
      { Sid::headerOddEven,            false, headerOddEven,                0 },
      { Sid::evenHeaderL,              false, evenHeaderL,                  0 },
      { Sid::evenHeaderC,              false, evenHeaderC,                  0 },
      { Sid::evenHeaderR,              false, evenHeaderR,                  0 },
      { Sid::oddHeaderL,               false, oddHeaderL,                   0 },
      { Sid::oddHeaderC,               false, oddHeaderC,                   0 },
      { Sid::oddHeaderR,               false, oddHeaderR,                   0 },
      { Sid::showFooter,               false, showFooter,                   0 },
      { Sid::footerFirstPage,          false, showFooterFirstPage,          0 },
      { Sid::footerOddEven,            false, footerOddEven,                0 },
      { Sid::footerInsideMargins,      false, footerInsideMargins,          0 },
      { Sid::evenFooterL,              false, evenFooterL,                  0 },
      { Sid::evenFooterC,              false, evenFooterC,                  0 },
      { Sid::evenFooterR,              false, evenFooterR,                  0 },
      { Sid::oddFooterL,               false, oddFooterL,                   0 },
      { Sid::oddFooterC,               false, oddFooterC,                   0 },
      { Sid::oddFooterR,               false, oddFooterR,                   0 },

      { Sid::ottavaNumbersOnly,        false, ottavaNumbersOnly,            resetOttavaNumbersOnly },
      { Sid::capoPosition,             false, capoPosition,                 resetCapoPosition },
      { Sid::fretNumMag,               true,  fretNumMag,                   resetFretNumMag },
      { Sid::fretNumPos,               false, fretNumGroup,                 resetFretNumPos },
      { Sid::fretY,                    false, fretY,                        resetFretY },
      { Sid::barreLineWidth,           false, barreLineWidth,               resetBarreLineWidth },
      { Sid::fretMag,                  false, fretMag,                      resetFretMag },
      { Sid::fretDotSize,              false, fretDotSize,                  resetFretDotSize },
      { Sid::fretStringSpacing,        false, fretStringSpacing,            resetFretStringSpacing },
      { Sid::fretFretSpacing,          false, fretFretSpacing,              resetFretFretSpacing },
      { Sid::maxFretShiftAbove,        false, maxFretShiftAbove,            resetMaxFretShiftAbove   },
      { Sid::maxFretShiftBelow,        false, maxFretShiftBelow,            resetMaxFretShiftBelow   },
      { Sid::scaleBarlines,            false, scaleBarlines,                resetScaleBarlines},
      { Sid::crossMeasureValues,       false, crossMeasureValues,           0 },

      { Sid::musicalSymbolFont,        false, musicalSymbolFontComboBox,    0 },
      { Sid::musicalTextFont,          false, musicalTextFontComboBox,      0 },
      { Sid::autoplaceHairpinDynamicsDistance, false, autoplaceHairpinDynamicsDistance, resetAutoplaceHairpinDynamicsDistance },

      { Sid::dynamicsPlacement,       false, dynamicsPlacement,          resetDynamicsPlacement },
      { Sid::dynamicsPosAbove,        false, dynamicsPosAbove,           resetDynamicsPosAbove },
      { Sid::dynamicsPosBelow,        false, dynamicsPosBelow,           resetDynamicsPosBelow },
      { Sid::dynamicsMinDistance,     false, dynamicsMinDistance,        resetDynamicsMinDistance },

      { Sid::tempoPlacement,          false, tempoTextPlacement,          resetTempoTextPlacement },
      { Sid::tempoPosAbove,           false, tempoTextPosAbove,           resetTempoTextPosAbove },
      { Sid::tempoPosBelow,           false, tempoTextPosBelow,           resetTempoTextPosBelow },
      { Sid::tempoMinDistance,        false, tempoTextMinDistance,        resetTempoTextMinDistance },

      { Sid::rehearsalMarkPlacement,   false, rehearsalMarkPlacement,     resetRehearsalMarkPlacement },
      { Sid::rehearsalMarkPosAbove,    false, rehearsalMarkPosAbove,      resetRehearsalMarkPosAbove },
      { Sid::rehearsalMarkPosBelow,    false, rehearsalMarkPosBelow,      resetRehearsalMarkPosBelow },
      { Sid::rehearsalMarkMinDistance, false, rehearsalMarkMinDistance,   resetRehearsalMarkMinDistance },

      { Sid::autoplaceVerticalAlignRange, false, autoplaceVerticalAlignRange, resetAutoplaceVerticalAlignRange },
      { Sid::minVerticalDistance,         false, minVerticalDistance,         resetMinVerticalDistance },
      { Sid::textLinePlacement,           false, textLinePlacement,           resetTextLinePlacement },
      { Sid::textLinePosAbove,            false, textLinePosAbove,            resetTextLinePosAbove },
      { Sid::textLinePosBelow,            false, textLinePosBelow,            resetTextLinePosBelow },

      { Sid::systemTextLinePlacement,     false, systemTextLinePlacement,     resetSystemTextLinePlacement },
      { Sid::systemTextLinePosAbove,      false, systemTextLinePosAbove,      resetSystemTextLinePosAbove },
      { Sid::systemTextLinePosBelow,      false, systemTextLinePosBelow,      resetSystemTextLinePosBelow },

      { Sid::fermataPosAbove,         false, fermataPosAbove,       resetFermataPosAbove    },
      { Sid::fermataPosBelow,         false, fermataPosBelow,       resetFermataPosBelow    },
      { Sid::fermataMinDistance,      false, fermataMinDistance,    resetFermataMinDistance },

      { Sid::staffTextPlacement,      false, staffTextPlacement,    resetStaffTextPlacement   },
      { Sid::staffTextPosAbove,       false, staffTextPosAbove,     resetStaffTextPosAbove    },
      { Sid::staffTextPosBelow,       false, staffTextPosBelow,     resetStaffTextPosBelow    },
      { Sid::staffTextMinDistance,    false, staffTextMinDistance,  resetStaffTextMinDistance },

      { Sid::bendLineWidth,     false, bendLineWidth,     resetBendLineWidth     },
      { Sid::bendArrowWidth,    false, bendArrowWidth,    resetBendArrowWidth    },
      };

      for (QComboBox* cb : std::vector<QComboBox*> {
            lyricsPlacement, textLinePlacement, systemTextLinePlacement, hairpinPlacement, pedalLinePlacement,
            trillLinePlacement, vibratoLinePlacement, dynamicsPlacement,
            tempoTextPlacement, staffTextPlacement, rehearsalMarkPlacement,
            measureNumberVPlacement, mmRestRangeVPlacement
            }) {
            cb->clear();
            cb->addItem(tr("Above"), int(Placement::ABOVE));
            cb->addItem(tr("Below"), int(Placement::BELOW));
            }
      for (QComboBox* cb : std::vector<QComboBox*> {
           measureNumberHPlacement, mmRestRangeHPlacement
           }) {
            cb->clear();
            cb->addItem(tr("Left"),   int(HPlacement::LEFT));
            cb->addItem(tr("Center"), int(HPlacement::CENTER));
            cb->addItem(tr("Right"),  int(HPlacement::RIGHT));
            }

      mmRestRangeBracketType->clear();
      mmRestRangeBracketType->addItem(tr("None"),        int(MMRestRangeBracketType::NONE));
      mmRestRangeBracketType->addItem(tr("Brackets"),    int(MMRestRangeBracketType::BRACKETS));
      mmRestRangeBracketType->addItem(tr("Parentheses"), int(MMRestRangeBracketType::PARENTHESES));

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
      accidentalsGroup->setVisible(false); // disable, not yet implemented

      fillScoreFontsComboBoxes();

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
      for (QString& family: fbFontNames)
            comboFBFont->addItem(family);
      comboFBFont->setCurrentIndex(0);
      connect(comboFBFont, SIGNAL(currentIndexChanged(int)), SLOT(on_comboFBFont_currentIndexChanged(int)));

      // chord symbol init
      harmonyPlay->setChecked(true);

      voicingSelectWidget->interpretBox->clear();
      voicingSelectWidget->interpretBox->addItem(tr("Jazz"), int(0)); // two-item combobox for boolean style variant
      voicingSelectWidget->interpretBox->addItem(tr("Literal"), int(1)); // true = literal

      voicingSelectWidget->voicingBox->clear();
      voicingSelectWidget->voicingBox->addItem(tr("Automatic"), int(Voicing::AUTO));
      voicingSelectWidget->voicingBox->addItem(tr("Root Only"), int(Voicing::ROOT_ONLY));
      voicingSelectWidget->voicingBox->addItem(tr("Close"), int(Voicing::CLOSE));
      voicingSelectWidget->voicingBox->addItem(tr("Drop Two"), int(Voicing::DROP_2));
      voicingSelectWidget->voicingBox->addItem(tr("Six Note"), int(Voicing::SIX_NOTE));
      voicingSelectWidget->voicingBox->addItem(tr("Four Note"), int(Voicing::FOUR_NOTE));
      voicingSelectWidget->voicingBox->addItem(tr("Three Note"), int(Voicing::THREE_NOTE));

      voicingSelectWidget->durationBox->clear();
      voicingSelectWidget->durationBox->addItem(tr("Until Next Chord Symbol"), int(HDuration::UNTIL_NEXT_CHORD_SYMBOL));
      voicingSelectWidget->durationBox->addItem(tr("Until End of Measure"), int(HDuration::STOP_AT_MEASURE_END));
      voicingSelectWidget->durationBox->addItem(tr("Chord/Rest Duration"), int(HDuration::SEGMENT_DURATION));

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
      connect(chordDescriptionFile,  &QLineEdit::editingFinished, this, [=]() { setChordStyle(true); });
      //chordDescriptionFile->setEnabled(false);

      chordDescriptionFileButton->setIcon(*icons[int(Icons::fileOpen_ICON)]);

      connect(swingOff,            SIGNAL(toggled(bool)),             SLOT(setSwingParams(bool)));
      connect(swingEighth,         SIGNAL(toggled(bool)),             SLOT(setSwingParams(bool)));
      connect(swingSixteenth,      SIGNAL(toggled(bool)),             SLOT(setSwingParams(bool)));

      connect(concertPitch,        SIGNAL(toggled(bool)),             SLOT(concertPitchToggled(bool)));
      connect(lyricsDashMinLength, SIGNAL(valueChanged(double)),      SLOT(lyricsDashMinLengthValueChanged(double)));
      connect(lyricsDashMaxLength, SIGNAL(valueChanged(double)),      SLOT(lyricsDashMaxLengthValueChanged(double)));
      connect(minSystemDistance,   SIGNAL(valueChanged(double)),      SLOT(systemMinDistanceValueChanged(double)));
      connect(maxSystemDistance,   SIGNAL(valueChanged(double)),      SLOT(systemMaxDistanceValueChanged(double)));

      QSignalMapper* mapper  = new QSignalMapper(this);     // reset style signals
      QSignalMapper* mapper2 = new QSignalMapper(this);     // value change signals

      for (StyleWidget& sw : styleWidgets) {
            const char* type = MStyle::valueType(sw.idx);

            if (!strcmp("Direction", type)) {
                  QComboBox* cb = qobject_cast<QComboBox*>(sw.widget);
                  fillComboBoxDirection(cb);
                  }
            if (sw.reset) {
                  sw.reset->setIcon(*icons[int(Icons::reset_ICON)]);
                  connect(sw.reset, SIGNAL(clicked()), mapper, SLOT(map()));
                  mapper->setMapping(sw.reset, int(sw.idx));
                  }
            if (qobject_cast<QSpinBox*>(sw.widget))
                  connect(qobject_cast<QSpinBox*>(sw.widget), SIGNAL(valueChanged(int)), mapper2, SLOT(map()));
            else if (qobject_cast<QDoubleSpinBox*>(sw.widget))
                  connect(qobject_cast<QDoubleSpinBox*>(sw.widget), SIGNAL(valueChanged(double)), mapper2, SLOT(map()));
            else if (qobject_cast<QFontComboBox*>(sw.widget))
                  connect(qobject_cast<QFontComboBox*>(sw.widget), SIGNAL(currentFontChanged(QFont)), mapper2, SLOT(map()));
            else if (qobject_cast<QComboBox*>(sw.widget))
                  connect(qobject_cast<QComboBox*>(sw.widget), SIGNAL(currentIndexChanged(int)), mapper2, SLOT(map()));
            else if (qobject_cast<QRadioButton*>(sw.widget))
                  connect(qobject_cast<QRadioButton*>(sw.widget), SIGNAL(toggled(bool)), mapper2, SLOT(map()));
            else if (qobject_cast<QPushButton*>(sw.widget))
                  connect(qobject_cast<QPushButton*>(sw.widget), SIGNAL(toggled(bool)), mapper2, SLOT(map()));
            else if (qobject_cast<QToolButton*>(sw.widget))
                  connect(qobject_cast<QToolButton*>(sw.widget), SIGNAL(toggled(bool)), mapper2, SLOT(map()));
            else if (qobject_cast<QGroupBox*>(sw.widget))
                  connect(qobject_cast<QGroupBox*>(sw.widget), SIGNAL(toggled(bool)), mapper2, SLOT(map()));
            else if (qobject_cast<QCheckBox*>(sw.widget))
                  connect(qobject_cast<QCheckBox*>(sw.widget), SIGNAL(stateChanged(int)), mapper2, SLOT(map()));
            else if (qobject_cast<QTextEdit*>(sw.widget))
                  connect(qobject_cast<QTextEdit*>(sw.widget), SIGNAL(textChanged()), mapper2, SLOT(map()));
            else if (qobject_cast<QButtonGroup*>(sw.widget))
                  connect(qobject_cast<QButtonGroup*>(sw.widget), SIGNAL(buttonClicked(int)), mapper2, SLOT(map()));
            else if (qobject_cast<AlignSelect*>(sw.widget))
                  connect(qobject_cast<AlignSelect*>(sw.widget), SIGNAL(alignChanged(Align)), mapper2, SLOT(map()));
            else if (qobject_cast<OffsetSelect*>(sw.widget))
                  connect(qobject_cast<OffsetSelect*>(sw.widget), SIGNAL(offsetChanged(QPointF)), mapper2, SLOT(map()));
            else if (FontStyleSelect* fontStyle = qobject_cast<FontStyleSelect*>(sw.widget))
                  connect(fontStyle, &FontStyleSelect::fontStyleChanged, mapper2, QOverload<>::of(&QSignalMapper::map));
            else {
                  qFatal("unhandled gui widget type %s valueType %s",
                     sw.widget->metaObject()->className(),
                     MStyle::valueName(sw.idx)
                  );
                  }

            mapper2->setMapping(sw.widget, int(sw.idx));
            }

      int topBottomMargin = automaticCapitalization->rect().height() - preferences.getDouble(PREF_UI_THEME_FONTSIZE);
      topBottomMargin /= 2;
      topBottomMargin = topBottomMargin > 4 ? topBottomMargin - 4 : 0;
      automaticCapitalization->layout()->setContentsMargins(9, topBottomMargin, 9, topBottomMargin);

      connect(mapper,  SIGNAL(mapped(int)), SLOT(resetStyleValue(int)));
      connect(mapper2, SIGNAL(mapped(int)), SLOT(valueChanged(int)));
      textStyles->clear();
      for (auto ss : allTextStyles()) {
            QListWidgetItem* item = new QListWidgetItem(s->getTextStyleUserName(ss));
            item->setData(Qt::UserRole, int(ss));
            textStyles->addItem(item);
            }

      textStyleFrameType->clear();
      textStyleFrameType->addItem(tr("None", "no frame for text"), int(FrameType::NO_FRAME));
      textStyleFrameType->addItem(tr("Rectangle"), int(FrameType::SQUARE));
      textStyleFrameType->addItem(tr("Circle"), int(FrameType::CIRCLE));

      resetTextStyleName->setIcon(*icons[int(Icons::reset_ICON)]);
      connect(resetTextStyleName, &QToolButton::clicked, this, [=](){ resetUserStyleName(); });
      connect(styleName, &QLineEdit::textEdited, this, [=]() { editUserStyleName(); });
      connect(styleName, &QLineEdit::editingFinished, this, [=]() { endEditUserStyleName(); });

      // font face
      resetTextStyleFontFace->setIcon(*icons[int(Icons::reset_ICON)]);
      connect(resetTextStyleFontFace, &QToolButton::clicked, this,
         [=](){ resetTextStyle(Pid::FONT_FACE); }
         );
      connect(textStyleFontFace, &QFontComboBox::currentFontChanged, this,
         [=](){ textStyleValueChanged(Pid::FONT_FACE, QVariant(textStyleFontFace->currentFont().family())); }
         );

      // font size
      resetTextStyleFontSize->setIcon(*icons[int(Icons::reset_ICON)]);
      connect(resetTextStyleFontSize, &QToolButton::clicked, this,
         [=](){ resetTextStyle(Pid::FONT_SIZE); }
         );
      connect(textStyleFontSize, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
         [=](){ textStyleValueChanged(Pid::FONT_SIZE, QVariant(textStyleFontSize->value())); }
         );

      // line spacing
      resetTextStyleLineSpacing->setIcon(*icons[int(Icons::reset_ICON)]);
      connect(resetTextStyleLineSpacing, &QToolButton::clicked, this,
          [=]() { resetTextStyle(Pid::TEXT_LINE_SPACING); }
      );
      connect(textStyleLineSpacing, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
          [=]() { textStyleValueChanged(Pid::TEXT_LINE_SPACING, QVariant(textStyleLineSpacing->value())); }
      );

      // font style
      resetTextStyleFontStyle->setIcon(*icons[int(Icons::reset_ICON)]);
      connect(resetTextStyleFontStyle, &QToolButton::clicked, this,
         [=](){ resetTextStyle(Pid::FONT_STYLE); }
         );
      connect(textStyleFontStyle, &FontStyleSelect::fontStyleChanged, this,
         [=](){ textStyleValueChanged(Pid::FONT_STYLE, QVariant(int(textStyleFontStyle->fontStyle()))); }
         );

      // align
      resetTextStyleAlign->setIcon(*icons[int(Icons::reset_ICON)]);
      connect(resetTextStyleAlign, &QToolButton::clicked, this, [=](){ resetTextStyle(Pid::ALIGN); });
      connect(textStyleAlign, &AlignSelect::alignChanged, this,
         [=](){ textStyleValueChanged(Pid::ALIGN, QVariant::fromValue(textStyleAlign->align())); }
         );

      // offset
      resetTextStyleOffset->setIcon(*icons[int(Icons::reset_ICON)]);
      connect(resetTextStyleOffset, &QToolButton::clicked, this, [=](){ resetTextStyle(Pid::OFFSET); });
      connect(textStyleOffset, &OffsetSelect::offsetChanged, this,
         [=](){ textStyleValueChanged(Pid::OFFSET, QVariant(textStyleOffset->offset())); }
         );

      // spatium dependent
      resetTextStyleSpatiumDependent->setIcon(*icons[int(Icons::reset_ICON)]);
      connect(resetTextStyleSpatiumDependent, &QToolButton::clicked, this, [=](){ resetTextStyle(Pid::SIZE_SPATIUM_DEPENDENT); });
      connect(textStyleSpatiumDependent, &QCheckBox::toggled, this,
         [=](){ textStyleValueChanged(Pid::SIZE_SPATIUM_DEPENDENT, textStyleSpatiumDependent->isChecked()); }
         );

      resetTextStyleFrameType->setIcon(*icons[int(Icons::reset_ICON)]);
      connect(resetTextStyleFrameType, &QToolButton::clicked, this, [=](){ resetTextStyle(Pid::FRAME_TYPE); });
      connect(textStyleFrameType, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
         [=](){ textStyleValueChanged(Pid::FRAME_TYPE, textStyleFrameType->currentIndex()); }
         );

      resetTextStyleFramePadding->setIcon(*icons[int(Icons::reset_ICON)]);
      connect(resetTextStyleFramePadding, &QToolButton::clicked, this, [=](){ resetTextStyle(Pid::FRAME_PADDING); });
      connect(textStyleFramePadding, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
         [=](){ textStyleValueChanged(Pid::FRAME_PADDING, textStyleFramePadding->value()); }
         );

      resetTextStyleFrameBorder->setIcon(*icons[int(Icons::reset_ICON)]);
      connect(resetTextStyleFrameBorder, &QToolButton::clicked, this, [=](){ resetTextStyle(Pid::FRAME_WIDTH); });
      connect(textStyleFrameBorder, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
         [=](){ textStyleValueChanged(Pid::FRAME_WIDTH, textStyleFrameBorder->value()); }
         );

      resetTextStyleFrameBorderRadius->setIcon(*icons[int(Icons::reset_ICON)]);
      connect(resetTextStyleFrameBorderRadius, &QToolButton::clicked, this, [=](){ resetTextStyle(Pid::FRAME_ROUND); });
      connect(textStyleFrameBorderRadius, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
         [=](){ textStyleValueChanged(Pid::FRAME_ROUND, textStyleFrameBorderRadius->value()); }
         );

      resetTextStyleFrameForeground->setIcon(*icons[int(Icons::reset_ICON)]);
      connect(resetTextStyleFrameForeground, &QToolButton::clicked, this, [=](){ resetTextStyle(Pid::FRAME_FG_COLOR); });
      connect(textStyleFrameForeground, &Awl::ColorLabel::colorChanged, this,
         [=](){ textStyleValueChanged(Pid::FRAME_FG_COLOR, textStyleFrameForeground->color()); }
         );

      resetTextStyleFrameBackground->setIcon(*icons[int(Icons::reset_ICON)]);
      connect(resetTextStyleFrameBackground, &QToolButton::clicked, this, [=](){ resetTextStyle(Pid::FRAME_BG_COLOR); });
      connect(textStyleFrameBackground, &Awl::ColorLabel::colorChanged, this,
         [=](){ textStyleValueChanged(Pid::FRAME_BG_COLOR, textStyleFrameBackground->color()); }
         );

      resetTextStyleColor->setIcon(*icons[int(Icons::reset_ICON)]);
      connect(resetTextStyleColor, &QToolButton::clicked, this, [=](){ resetTextStyle(Pid::COLOR); });
      connect(textStyleColor, &Awl::ColorLabel::colorChanged, this,
         [=](){ textStyleValueChanged(Pid::COLOR, textStyleColor->color()); }
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

      adjustPagesStackSize(0);

      MuseScore::restoreGeometry(this);
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
            if (!currentPage)
                  return;

            pageStack->setMinimumSize(currentPage->sizeHint());

            if (scrollArea)
                  scrollArea->ensureVisible(0, 0);
            });
      }

//---------------------------------------------------------
//   MuseScore::showStyleDialog
///   Opens the Style Dialog, and if an element `e` is specified, switches to the
///   page related to the element.
//---------------------------------------------------------

void MuseScore::showStyleDialog(Element* e)
      {
      if (!_styleDlg)
            _styleDlg = new EditStyle { cs, this };
      else
            _styleDlg->setScore(cs);

      if (e)
            _styleDlg->gotoElement(e);

      if (_styleDlg->isVisible()) {
            _styleDlg->raise();
            _styleDlg->activateWindow();
            }
      else
            // use `_styleDlg->show();` to show non-modally to allow
            // user to scroll/zoom score while selecting style options;
            // however, that must be properly implemented, otherwise it
            // will cause problems.
            _styleDlg->exec();
      }

//---------------------------------------------------------
//   retranslate
//    NOTE: keep in sync with constructor.
//---------------------------------------------------------

void EditStyle::retranslate()
      {
      retranslateUi(this);

      int index = 0;
      for (const char* p : lineStyles) {
            QString trs = qApp->translate("EditStyleBase", p);
            voltaLineStyle->setItemText(index, trs);
            ottavaLineStyle->setItemText(index, trs);
            pedalLineStyle->setItemText(index, trs);
            ++index;
            }

      for (QComboBox* cb : std::vector<QComboBox*> {
            lyricsPlacement, textLinePlacement, systemTextLinePlacement, hairpinPlacement, pedalLinePlacement,
            trillLinePlacement, vibratoLinePlacement, dynamicsPlacement,
            tempoTextPlacement, staffTextPlacement, rehearsalMarkPlacement,
            measureNumberVPlacement, mmRestRangeVPlacement
            }) {
            cb->setItemText(0, tr("Above"));
            cb->setItemText(1, tr("Below"));
            }

      for (QComboBox* cb : std::vector<QComboBox*> {
            measureNumberHPlacement, mmRestRangeHPlacement
            }) {
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

      voicingSelectWidget->interpretBox->setItemText(0, tr("Jazz")); // two-item combobox for boolean style variant
      voicingSelectWidget->interpretBox->setItemText(1, tr("Literal")); // true = literal

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

      index = 0;
      for (auto ss : allTextStyles()) {
            QString name = cs->getTextStyleUserName(ss);
            textStyles->item(index)->setText(name);
            ++index;
            }

      textStyleFrameType->setItemText(0, tr("None", "no frame for text"));
      textStyleFrameType->setItemText(1, tr("Rectangle"));
      textStyleFrameType->setItemText(2, tr("Circle"));
      }

//---------------------------------------------------------
//   setHeaderFooterToolTip
//---------------------------------------------------------

void EditStyle::setHeaderFooterToolTip() {
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
      if (!cs->isMaster()) {
            QMapIterator<QString, QString> j(cs->masterScore()->metaTags());
            while (j.hasNext()) {
                  j.next();
                  toolTipHeaderFooter += QString("<tr><td>%1</td><td>-</td><td>%2</td></tr>").arg(j.key(),j.value());
                  }
            }
      QMapIterator<QString, QString> i(cs->metaTags());
      while (i.hasNext()) {
            i.next();
            toolTipHeaderFooter += QString("<tr><td>%1</td><td>-</td><td>%2</td></tr>").arg(i.key(), i.value());
            }
      toolTipHeaderFooter += QString("</table></body></html>");
      showHeader->setToolTip(toolTipHeaderFooter);
      showHeader->setToolTipDuration(10000); // leaving the default value of -1 calculates the duration automatically and it takes too long
      showFooter->setToolTip(toolTipHeaderFooter);
      showFooter->setToolTipDuration(10000);
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
            case ElementType::INSTRUMENT_NAME:
                  return &EditStyle::PageTextStyles;
            case ElementType::TEXT:
                  if (toText(e)->tid() == Tid::FOOTER || toText(e)->tid() == Tid::HEADER)
                        return &EditStyle::PageHeaderFooter;
                  return &EditStyle::PageTextStyles;
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
            case ElementType::REST:
                  return &EditStyle::PageMeasure;
            case ElementType::BAR_LINE:
                  return &EditStyle::PageBarlines;
            case ElementType::NOTE:
            case ElementType::CHORD:
            case ElementType::ACCIDENTAL:
            case ElementType::STEM:
            case ElementType::STEM_SLASH:
            case ElementType::LEDGER_LINE:
            case ElementType::NOTEDOT:
                  return &EditStyle::PageNotes;
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
      return (pageForElement(e) != nullptr);
      }

//---------------------------------------------------------
//   gotoElement
///   switch to the page related to the element `e`
//---------------------------------------------------------

void EditStyle::gotoElement(Element* e)
      {
      if (auto pagePointer = pageForElement(e))
            if (QWidget* page = this->*pagePointer)
                  setPage(pageStack->indexOf(page));
      }

//---------------------------------------------------------
//   showEvent
//---------------------------------------------------------

void EditStyle::showEvent(QShowEvent* ev)
      {
      if (!hasShown && isTooBig) {
            // Add scroll bars to pageStack - this cannot be in the constructor
            // or the Header, Footer text input boxes size themselves too large.
            scrollArea = new QScrollArea(splitter);
            scrollArea->setWidget(pageStack);
            scrollArea->setWidgetResizable(true);
            hasShown = true; // so that it only happens once
            }
      setValues();
      pageList->setFocus();
      cs->startCmd();
      buttonApplyToAllParts->setEnabled(!cs->isMaster());
      needResetStyle = false;
      QWidget::showEvent(ev);
      }

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void EditStyle::hideEvent(QHideEvent* ev)
      {
      MuseScore::saveGeometry(this);
      QWidget::hideEvent(ev);
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
            case QDialogButtonBox::NoButton:
            default:
                  if (b == buttonApplyToAllParts)
                        applyToAllParts();
                  break;
            }
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void EditStyle::accept()
      {
      cs->endCmd();
      QDialog::accept();
      }

//---------------------------------------------------------
//   reject
//---------------------------------------------------------

void EditStyle::reject()
      {
      cs->endCmd(true);
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
      bool isVis = !pageList->isVisible(); // toggle it

      pageList->setVisible(isVis);
      buttonTogglePagelist->setIcon(QIcon(*icons[int(isVis ? Icons::goNext_ICON
                                                           : Icons::goPrevious_ICON)]));
      }

void EditStyle::on_resetStylesButton_clicked()
{
    needResetStyle = true;
    resetStyle(cs);
    cs->update();
    setValues();
}

//---------------------------------------------------------
//   applyToAllParts
//---------------------------------------------------------

void EditStyle::resetStyle(Score* score)
{
      auto ignoreStyles = pageStyles();
      ignoreStyles.insert(Sid::concertPitch);
      ignoreStyles.insert(Sid::createMultiMeasureRests);
      score->style().resetAllStyles(score, ignoreStyles);
}

void EditStyle::applyToAllParts()
      {
      for (Excerpt*& e : cs->masterScore()->excerpts()) {
            if (needResetStyle) {
                resetStyle(e->partScore());
            } else {
                e->partScore()->undo(new ChangeStyle(e->partScore(), cs->style()));
            }

            e->partScore()->update();
      }
      }

//---------------------------------------------------------
//   unhandledType
//---------------------------------------------------------

static void unhandledType(const StyleWidget* sw)
      {
      IF_ASSERT_FAILED(!sw) {
            return;
            }
      const char* type = MStyle::valueType(sw->idx);
      qFatal("%s <%s>: widget: %s\n", type, MStyle::valueName(sw->idx), sw->widget->metaObject()->className());
      }

//---------------------------------------------------------
//   getValue
//    return current gui value
//---------------------------------------------------------

QVariant EditStyle::getValue(Sid idx)
      {
      const StyleWidget& sw = styleWidget(idx);
      const char* type = MStyle::valueType(sw.idx);

      if (!strcmp("Ms::Spatium", type)) {
            QDoubleSpinBox* sb = qobject_cast<QDoubleSpinBox*>(sw.widget);
            return QVariant(Spatium(sb->value() * (sw.showPercent ? 0.01 : 1.0)));
            }
      else if (!strcmp("double", type)) {
            QVariant v = sw.widget->property("value");
            if (!v.isValid())
                  unhandledType(&sw);
            if (sw.showPercent)
                  v = v.toDouble() * 0.01;
            return v;
            }
      else if (!strcmp("bool", type)) {
            QVariant v;
            if (sw.idx == Sid::harmonyVoiceLiteral) { // special case for bool represented by a two-item combobox
                  QComboBox* cb = qobject_cast<QComboBox*>(sw.widget);
                  v = cb->currentIndex();
                  }
            else {
                  v = sw.widget->property("checked");
                  if (!v.isValid())
                        unhandledType(&sw);
                  }
            return v;
            }
      else if (!strcmp("int", type)) {
            if (qobject_cast<QComboBox*>(sw.widget)) {
                  QComboBox* cb = qobject_cast<QComboBox*>(sw.widget);
                  return cb->currentData().toInt();
                  }
            else if (qobject_cast<QSpinBox*>(sw.widget))
                  return qobject_cast<QSpinBox*>(sw.widget)->value() / (sw.showPercent ? 100 : 1);
            else if (qobject_cast<QButtonGroup*>(sw.widget)) {
                  QButtonGroup* bg = qobject_cast<QButtonGroup*>(sw.widget);
                  return bg->checkedId();
                  }
            else if (FontStyleSelect* fontStyle = qobject_cast<FontStyleSelect*>(sw.widget))
                  return int(fontStyle->fontStyle());
            else
                  qFatal("unhandled int");
            }
      else if (!strcmp("QString", type)) {
            if (qobject_cast<QFontComboBox*>(sw.widget))
                  return static_cast<QFontComboBox*>(sw.widget)->currentFont().family();
            if (qobject_cast<QComboBox*>(sw.widget)) {
                  QComboBox* cb = qobject_cast<QComboBox*>(sw.widget);
                  return cb->currentData().toString();
                  }
            if (qobject_cast<QTextEdit*>(sw.widget)) {
                  QTextEdit* te = qobject_cast<QTextEdit*>(sw.widget);
                  return te->toPlainText();
                  }
            qFatal("getValue: unhandled widget type %s valueType %s",
               sw.widget->metaObject()->className(),
               MStyle::valueName(idx));

            }
      else if (!strcmp("QPointF", type)) {
            OffsetSelect* cb = qobject_cast<Ms::OffsetSelect*>(sw.widget);
            if (cb)
                  return cb->offset();
            else
                  qFatal("unhandled QPointF");
            }
      else if (!strcmp("Ms::Direction", type)) {
            QComboBox* cb = qobject_cast<QComboBox*>(sw.widget);
            if (cb)
                  return cb->currentIndex();
            else
                  qFatal("unhandled Direction");
            }
      else if (!strcmp("Ms::Align", type)) {
            AlignSelect* as = qobject_cast<Ms::AlignSelect*>(sw.widget);
            return QVariant::fromValue(as->align());
            }
      else {
            qFatal("EditStyle::getValue: unhandled type <%s>", type);
            }
      return QVariant();
      }

//---------------------------------------------------------
//   setValues
//---------------------------------------------------------

void EditStyle::setValues()
      {
      const MStyle& lstyle = cs->style();
      for (StyleWidget& sw : styleWidgets) {
            if (sw.widget)
                  sw.widget->blockSignals(true);
            QVariant val = lstyle.value(sw.idx);
            const char* type = MStyle::valueType(sw.idx);
            if (sw.reset)
                  sw.reset->setEnabled(!lstyle.isDefault(sw.idx));

            if (!strcmp("Ms::Spatium", type)) {
                  if (sw.showPercent)
                        qobject_cast<QSpinBox*>(sw.widget)->setValue(int(val.value<Spatium>().val() * 100.0));
                  else if (sw.widget)
                        sw.widget->setProperty("value", val);
                  }
            else if (!strcmp("double", type)) {
                  if (sw.showPercent)
                        val = QVariant(val.toDouble() * 100);
                  if (sw.widget && !sw.widget->setProperty("value", val))
                        unhandledType(&sw);
                  }
            else if (!strcmp("bool", type)) {
                  if (sw.idx == Sid::harmonyVoiceLiteral) { // special case for bool represented by a two-item combobox
                        voicingSelectWidget->interpretBox->setCurrentIndex(val.toBool());
                        }
                  else {
                        if (sw.widget && !sw.widget->setProperty("checked", val))
                              unhandledType(&sw);
                        if (sw.idx == Sid::measureNumberSystem && !val.toBool())
                              showIntervalMeasureNumber->setChecked(true);
                        }
                  }
            else if (!strcmp("int", type)) {
                  if (qobject_cast<QComboBox*>(sw.widget)) {
                        QComboBox* cb = qobject_cast<QComboBox*>(sw.widget);
                        cb->setCurrentIndex(cb->findData(val));
                        }
                  else if (qobject_cast<QSpinBox*>(sw.widget)) {
                        qobject_cast<QSpinBox*>(sw.widget)->setValue(val.toInt()
                           * (sw.showPercent ? 100 : 1));
                        }
                  else if (qobject_cast<QButtonGroup*>(sw.widget)) {
                        QButtonGroup* bg = qobject_cast<QButtonGroup*>(sw.widget);
                        for (auto& a : bg->buttons()) {
                              if (bg->id(a) == val.toInt()) {
                                    a->setChecked(true);
                                    break;
                                    }
                              }
                        }
                  else if (FontStyleSelect* fontStyle = qobject_cast<FontStyleSelect*>(sw.widget))
                        fontStyle->setFontStyle(FontStyle(val.toInt()));
                  else
                        unhandledType(&sw);
                  }
            else if (!strcmp("QString", type)) {
                  if (sw.widget && qobject_cast<QFontComboBox*>(sw.widget))
                        static_cast<QFontComboBox*>(sw.widget)->setCurrentFont(QFont(val.toString()));
                  else if (qobject_cast<QComboBox*>(sw.widget)) {
                        QComboBox* cb = qobject_cast<QComboBox*>(sw.widget);
                        for (int i = 0; i < cb->count(); ++i) {
                              if (cb->itemData(i) == val.toString()) {
                                    cb->setCurrentIndex(i);
                                    break;
                                    }
                              }
                        }
                  else if (sw.widget && qobject_cast<QTextEdit*>(sw.widget))
                        static_cast<QTextEdit*>(sw.widget)->setPlainText(val.toString());
                  else
                        unhandledType(&sw);
                  }
            else if (!strcmp("Ms::Direction", type)) {
                  QComboBox* cb = qobject_cast<QComboBox*>(sw.widget);
                  if (cb)
                        cb->setCurrentIndex(int(val.value<Direction>()));
                  else
                        unhandledType(&sw);
                  }
            else if (!strcmp("Ms::Align", type)) {
                  AlignSelect* as = qobject_cast<Ms::AlignSelect*>(sw.widget);
                  as->setAlign(val.value<Align>());
                  }
            else if (!strcmp("QPointF", type)) {
                  OffsetSelect* as = qobject_cast<Ms::OffsetSelect*>(sw.widget);
                  if (as)
                        as->setOffset(val.value<QPointF>());
#if 0  // debug
                  else {
                        printf("no widget for QPointF <%s><%s>\n",
                               sw.widget->metaObject()->className(), MStyle::valueName(sw.idx));
                        }
#endif
                  }
            else
                  unhandledType(&sw);
            if (sw.widget)
                  sw.widget->blockSignals(false);
            }

      textStyleChanged(textStyles->currentRow());

      //TODO: convert the rest:

      QString unit(lstyle.value(Sid::swingUnit).toString());

      if (unit == TDuration(TDuration::DurationType::V_EIGHTH).name()) {
            swingEighth->setChecked(true);
            swingBox->setEnabled(true);
            }
      else if (unit == TDuration(TDuration::DurationType::V_16TH).name()) {
            swingSixteenth->setChecked(true);
            swingBox->setEnabled(true);
            }
      else if (unit == TDuration(TDuration::DurationType::V_ZERO).name()) {
            swingOff->setChecked(true);
            swingBox->setEnabled(false);
            }
      QString s(lstyle.value(Sid::chordDescriptionFile).toString());
      chordDescriptionFile->setText(s);
      QString cstyle(lstyle.value(Sid::chordStyle).toString());
      if (cstyle == "std") {
            chordsStandard->setChecked(true);
            chordDescriptionGroup->setEnabled(false);
            }
      else if (cstyle == "jazz") {
            chordsJazz->setChecked(true);
            chordDescriptionGroup->setEnabled(false);
            }
      else {
            chordsCustom->setChecked(true);
            chordDescriptionGroup->setEnabled(true);
            }
      //formattingGroup->setEnabled(lstyle.chordList()->autoAdjust());

      // figured bass
      for (int i = 0; i < comboFBFont->count(); i++)
            if (comboFBFont->itemText(i) == lstyle.value(Sid::figuredBassFontFamily).toString()) {
                  comboFBFont->setCurrentIndex(i);
                  break;
            }
      doubleSpinFBSize->setValue(lstyle.value(Sid::figuredBassFontSize).toDouble());
      doubleSpinFBVertPos->setValue(lstyle.value(Sid::figuredBassYOffset).toDouble());
      spinFBLineHeight->setValue(lstyle.value(Sid::figuredBassLineHeight).toDouble() * 100.0);

      fillScoreFontsComboBoxes();

      toggleHeaderOddEven(lstyle.value(Sid::headerOddEven).toBool());
      toggleFooterOddEven(lstyle.value(Sid::footerOddEven).toBool());
      disableVerticalSpread->setChecked(!lstyle.value(Sid::enableVerticalSpread).toBool());
      }

void EditStyle::fillScoreFontsComboBoxes()
      {
      QString selectedMusicalSymbolFontName = cs->styleSt(Sid::musicalSymbolFont);
      QString selectedMusicalTextFontFamily = cs->styleSt(Sid::musicalTextFont);

      musicalSymbolFontComboBox->blockSignals(true);
      musicalSymbolFontComboBox->clear();
      musicalTextFontComboBox->blockSignals(true);
      musicalTextFontComboBox->clear();

      int idx = 0;
      for (const ScoreFont& font : ScoreFont::scoreFonts()) {
            musicalSymbolFontComboBox->addItem(font.name(), font.name());
            if (font.name().toLower() == selectedMusicalSymbolFontName.toLower())
                  musicalSymbolFontComboBox->setCurrentIndex(idx);

            musicalTextFontComboBox->addItem(font.correspondingTextFontName(), font.correspondingTextFontFamily());
            if (font.correspondingTextFontFamily().toLower() == selectedMusicalTextFontFamily.toLower())
                  musicalTextFontComboBox->setCurrentIndex(idx);

            ++idx;
            }

      musicalSymbolFontComboBox->blockSignals(false);
      musicalTextFontComboBox->blockSignals(false);
      }

//---------------------------------------------------------
//   selectChordDescriptionFile
//---------------------------------------------------------

void EditStyle::selectChordDescriptionFile()
      {
      QString fn = mscore->getChordStyleFilename(true);
      if (fn.isEmpty())
            return;
      chordDescriptionFile->setText(fn);
      setChordStyle(true);
      }

//---------------------------------------------------------
//   setSwingParams
//---------------------------------------------------------

void EditStyle::setSwingParams(bool checked)
      {
      if (!checked)
            return;
      QVariant val;
      if (swingOff->isChecked()) {
            val = TDuration(TDuration::DurationType::V_ZERO).name();
            swingBox->setEnabled(false);
            }
      else if (swingEighth->isChecked()) {
            val = TDuration(TDuration::DurationType::V_EIGHTH).name();
            swingBox->setEnabled(true);
            }
      else if (swingSixteenth->isChecked()) {
            val = TDuration(TDuration::DurationType::V_16TH).name();
            swingBox->setEnabled(true);
            }
      cs->undo(new ChangeStyleVal(cs, Sid::swingUnit, val));
      cs->update();
      }

//---------------------------------------------------------
//   concertPitchToggled
//---------------------------------------------------------

void EditStyle::concertPitchToggled(bool checked)
      {
      cs->cmdConcertPitchChanged(checked, true);
      }

//---------------------------------------------------------
//   setChordStyle
//---------------------------------------------------------

void EditStyle::setChordStyle(bool checked)
      {
      if (!checked)
            return;
      QVariant val;
      QString file;
      bool chordsXml;
      if (chordsStandard->isChecked()) {
            val  = QString("std");
            file = "chords_std.xml";
            chordsXml = false;
            }
      else if (chordsJazz->isChecked()) {
            val  = QString("jazz");
            file = "chords_jazz.xml";
            chordsXml = false;
            }
      else {
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
      cs->undo(new ChangeStyleVal(cs, Sid::chordsXmlFile, chordsXml));
      cs->undo(new ChangeStyleVal(cs, Sid::chordStyle, val));
      if (!file.isEmpty()) {
            cs->undo(new ChangeStyleVal(cs, Sid::chordDescriptionFile, file));
            cs->update();
            }
      //formattingGroup->setEnabled(cs->style().chordList()->autoAdjust());
      }

//---------------------------------------------------------
//   enableStyleWidget
//---------------------------------------------------------

void EditStyle::enableStyleWidget(const Sid idx, bool enable)
      {
      const StyleWidget& sw { styleWidget(idx) };
      static_cast<QWidget*>(sw.widget)->setEnabled(enable);
      if (sw.reset)
            sw.reset->setEnabled(enable && !cs->style().isDefault(idx));
      }

//---------------------------------------------------------
//   enableVerticalSpreadClicked
//---------------------------------------------------------

void EditStyle::enableVerticalSpreadClicked(bool checked)
      {
      disableVerticalSpread->setChecked(!checked);
      cs->setLayoutAll();
      }

//---------------------------------------------------------
//   disableVerticalSpreadClicked
//---------------------------------------------------------

void EditStyle::disableVerticalSpreadClicked(bool checked)
      {
      cs->undo(new ChangeStyleVal(cs, Sid::enableVerticalSpread, !checked));
      enableVerticalSpread->setChecked(!checked);
      cs->setLayoutAll();
      }

//---------------------------------------------------------
//   toggleHeaderOddEven
//---------------------------------------------------------

void EditStyle::toggleHeaderOddEven(bool checked)
      {
      if (!showHeader->isChecked())
            return;
      labelEvenHeader->setEnabled(checked);
      evenHeaderL->setEnabled(checked);
      evenHeaderC->setEnabled(checked);
      evenHeaderR->setEnabled(checked);
      static QString odd  = labelOddHeader->text();  // save on 1st round
      static QString even = labelEvenHeader->text(); // save on 1st round
      if (checked)
            labelOddHeader->setText(odd); // restore
      else
            labelOddHeader->setText(odd + "\n" + even); // replace
      return;
      }

//---------------------------------------------------------
//   toggleFooterOddEven
//---------------------------------------------------------

void EditStyle::toggleFooterOddEven(bool checked)
      {
      if (!showFooter->isChecked())
            return;
      labelEvenFooter->setEnabled(checked);
      evenFooterL->setEnabled(checked);
      evenFooterC->setEnabled(checked);
      evenFooterR->setEnabled(checked);
      static QString odd  = labelOddFooter->text();  // save on 1st round
      static QString even = labelEvenFooter->text(); // save on 1st round
      if (checked)
            labelOddFooter->setText(odd); // restore
      else
            labelOddFooter->setText(odd + "\n" + even); // replace
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
      if (otherVal > val)
            lyricsDashMaxLength->setValue(otherVal);
      }

void EditStyle::lyricsDashMinLengthValueChanged(double val)
      {
      double otherVal = lyricsDashMaxLength->value();
      if (otherVal < val)
            lyricsDashMinLength->setValue(otherVal);
      }

//---------------------------------------------------------
//   systemMin/MaxDistanceValueChanged
//
//    Ensure minSystemDistance <= maxSystemDistance
//---------------------------------------------------------

void EditStyle::systemMaxDistanceValueChanged(double val)
      {
      double otherVal = minSystemDistance->value();
      if (otherVal > val)
            maxSystemDistance->setValue(otherVal);
      }


void EditStyle::systemMinDistanceValueChanged(double val)
      {
      double otherVal = maxSystemDistance->value();
      if (otherVal < val)
            minSystemDistance->setValue(otherVal);
      }
//---------------------------------------------------------
//   setPage
//---------------------------------------------------------

void EditStyle::setPage(int row)
      {
      if (row >= 0)
            pageList->setCurrentRow(row);
      }

//---------------------------------------------------------
//   styleWidget
//---------------------------------------------------------

const StyleWidget& EditStyle::styleWidget(Sid idx) const
      {
      for (const StyleWidget& sw : styleWidgets) {
            if (sw.idx == idx)
                  return sw;
            }
      Q_UNREACHABLE();
      }

//---------------------------------------------------------
//   valueChanged
//---------------------------------------------------------

void EditStyle::valueChanged(int i)
      {
      Sid idx       = (Sid)i;
      QVariant val  = getValue(idx);
      bool setValue = false;
      if (idx == Sid::musicalSymbolFont && optimizeStyleCheckbox->isChecked()) {
            ScoreFont* scoreFont = ScoreFont::fontFactory(val.toString());
            if (scoreFont) {
                  for (auto& j : scoreFont->engravingDefaults()) {
#if 0  // debug
                        if (cs->styleV(j.first) != j.second) {
                              printf("change style <%s>(%s) %f -> %f (%f %f)\n",
                                 MStyle::valueName(j.first),
                                 MStyle::valueType(j.first),
                                 cs->styleV(j.first).toDouble(),
                                 j.second.toDouble(),
                                 SPATIUM20,
                                 cs->spatium()
                                 );
                              }
#endif
                          cs->undo(new ChangeStyleVal(cs, j.first, j.second));
                          }
                  // fix values, the distances are defined different in MuseScore
                  cs->undo(new ChangeStyleVal(cs, Sid::endBarDistance,
                    cs->styleV(Sid::endBarDistance).toDouble()
                    + (cs->styleV(Sid::barWidth).toDouble() + cs->styleV(Sid::endBarWidth).toDouble()) * .5));
                  cs->undo(new ChangeStyleVal(cs, Sid::doubleBarDistance,
                    cs->styleV(Sid::doubleBarDistance).toDouble()
                    + (cs->styleV(Sid::barWidth).toDouble() + cs->styleV(Sid::barWidth).toDouble()) * .5));

                  // guess the repeat dot width = spatium * .3
                  cs->undo(new ChangeStyleVal(cs, Sid::repeatBarlineDotSeparation,
                    cs->styleV(Sid::repeatBarlineDotSeparation).toDouble()
                    + (cs->styleV(Sid::barWidth).toDouble() + .3) * .5));

//                  if (scoreFont->textEnclosureThickness()) {
//                        TextStyle ts = cs->textStyle(TextStyleType::REHEARSAL_MARK);
//                        ts.setFrameWidth(Spatium(scoreFont->textEnclosureThickness()));
//TODO                        cs->undo(new ChangeTextStyle(cs, ts));
//                        }
                  }
            setValue = true;
            }
      cs->undo(new ChangeStyleVal(cs, idx, val));
      cs->update();
      if (setValue)
            setValues();
      const StyleWidget& sw = styleWidget(idx);
      if (sw.reset)
            sw.reset->setEnabled(!cs->style().isDefault(idx));
      }

//---------------------------------------------------------
//   resetStyleValue
//---------------------------------------------------------

void EditStyle::resetStyleValue(int i)
      {
      Sid idx = (Sid)i;
      cs->undo(new ChangeStyleVal(cs, idx, MScore::defaultStyle().value(idx)));
      setValues();
      cs->update();
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
                        QVariant val = cs->styleV(a.sid);
                        textStyleFontFace->setCurrentFont(QFont(val.toString()));
                        resetTextStyleFontFace->setEnabled(val != MScore::defaultStyle().value(a.sid));
                        }
                        break;

                  case Pid::FONT_SIZE:
                        textStyleFontSize->setValue(cs->styleD(a.sid));
                        resetTextStyleFontSize->setEnabled(cs->styleV(a.sid) != MScore::defaultStyle().value(a.sid));
                        break;

                  case Pid::TEXT_LINE_SPACING:
                      textStyleLineSpacing->setValue(cs->styleD(a.sid));
                      resetTextStyleLineSpacing->setEnabled(cs->styleV(a.sid) != MScore::defaultStyle().value(a.sid));
                      break;

                  case Pid::FONT_STYLE:
                        textStyleFontStyle->setFontStyle(FontStyle(cs->styleV(a.sid).toInt()));
                        resetTextStyleFontStyle->setEnabled(cs->styleV(a.sid) != MScore::defaultStyle().value(a.sid));
                        break;

                  case Pid::ALIGN:
                        textStyleAlign->setAlign(cs->styleV(a.sid).value<Align>());
                        resetTextStyleAlign->setEnabled(cs->styleV(a.sid) != MScore::defaultStyle().value(a.sid));
                        break;

                  case Pid::OFFSET:
                        textStyleOffset->setOffset(cs->styleV(a.sid).toPointF());
                        resetTextStyleOffset->setEnabled(cs->styleV(a.sid) != MScore::defaultStyle().value(a.sid));
                        break;

                  case Pid::SIZE_SPATIUM_DEPENDENT: {
                        QVariant val = cs->styleV(a.sid);
                        textStyleSpatiumDependent->setChecked(val.toBool());
                        resetTextStyleSpatiumDependent->setEnabled(val != MScore::defaultStyle().value(a.sid));
                        textStyleOffset->setSuffix(val.toBool() ? tr("sp") : tr("mm"));
                        }
                        break;

                  case Pid::FRAME_TYPE:
                        textStyleFrameType->setCurrentIndex(cs->styleV(a.sid).toInt());
                        resetTextStyleFrameType->setEnabled(cs->styleV(a.sid) != MScore::defaultStyle().value(a.sid));
                        frameWidget->setEnabled(cs->styleV(a.sid).toInt() != 0); // disable if no frame
                        break;

                  case Pid::FRAME_PADDING:
                        textStyleFramePadding->setValue(cs->styleV(a.sid).toDouble());
                        resetTextStyleFramePadding->setEnabled(cs->styleV(a.sid) != MScore::defaultStyle().value(a.sid));
                        break;

                  case Pid::FRAME_WIDTH:
                        textStyleFrameBorder->setValue(cs->styleV(a.sid).toDouble());
                        resetTextStyleFrameBorder->setEnabled(cs->styleV(a.sid) != MScore::defaultStyle().value(a.sid));
                        break;

                  case Pid::FRAME_ROUND:
                        textStyleFrameBorderRadius->setValue(cs->styleV(a.sid).toDouble());
                        resetTextStyleFrameBorderRadius->setEnabled(cs->styleV(a.sid) != MScore::defaultStyle().value(a.sid));
                        break;

                  case Pid::FRAME_FG_COLOR:
                        textStyleFrameForeground->setColor(cs->styleV(a.sid).value<QColor>());
                        resetTextStyleFrameForeground->setEnabled(cs->styleV(a.sid) != MScore::defaultStyle().value(a.sid));
                        break;

                  case Pid::FRAME_BG_COLOR:
                        textStyleFrameBackground->setColor(cs->styleV(a.sid).value<QColor>());
                        resetTextStyleFrameBackground->setEnabled(cs->styleV(a.sid) != MScore::defaultStyle().value(a.sid));
                        break;

                  case Pid::COLOR:
                        textStyleColor->setColor(cs->styleV(a.sid).value<QColor>());
                        resetTextStyleColor->setEnabled(cs->styleV(a.sid) != MScore::defaultStyle().value(a.sid));
                        break;

                  default:
                        break;
                  }
            }
      styleName->setText(cs->getTextStyleUserName(tid));
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
                  cs->undoChangeStyleVal(a.sid, value);
                  break;
                  }
            }
      textStyleChanged(textStyles->currentRow());     // update GUI (reset buttons)
      cs->update();
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
                  cs->undoChangeStyleVal(a.sid, MScore::defaultStyle().value(a.sid));
                  break;
                  }
            }
      textStyleChanged(textStyles->currentRow());     // update GUI
      cs->update();
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
      if (int(tid) < int(Tid::USER1) || int(tid) > int(Tid::USER12)) {
            qDebug("User style index %d outside of range.", idx);
            return;
            }
      Sid sid[] = { Sid::user1Name, Sid::user2Name, Sid::user3Name, Sid::user4Name, Sid::user5Name, Sid::user6Name,
                    Sid::user7Name, Sid::user8Name, Sid::user9Name, Sid::user10Name, Sid::user11Name, Sid::user12Name };
      QString name = styleName->text();
      cs->undoChangeStyleVal(sid[idx], name);
      if (name == "") {
            name = textStyleUserName(tid);
            styleName->setText(name);
            textStyles->item(row)->setText(name);
            resetTextStyleName->setEnabled(false);
            }
      MuseScoreCore::mscoreCore->updateInspector();
      }
//---------------------------------------------------------
//   resetUserStyleName
//---------------------------------------------------------

void EditStyle::resetUserStyleName()
      {
      styleName->clear();
      endEditUserStyleName();
      }

} //namespace Ms
