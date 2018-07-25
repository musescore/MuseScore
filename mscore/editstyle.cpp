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

#include "libmscore/score.h"
#include "scoreview.h"
#include "libmscore/style.h"
#include "editstyle.h"
#include "libmscore/articulation.h"
#include "libmscore/sym.h"
#include "icons.h"
#include "musescore.h"
#include "libmscore/undo.h"
#include "icons.h"
#include "libmscore/harmony.h"
#include "libmscore/chordlist.h"
#include "libmscore/figuredbass.h"
#include "libmscore/clef.h"
#include "libmscore/excerpt.h"
#include "libmscore/tuplet.h"
#include "libmscore/layout.h"

namespace Ms {

//---------------------------------------------------------
//   EditStyle
//---------------------------------------------------------

EditStyle::EditStyle(Score* s, QWidget* parent)
   : QDialog(parent)
      {
      setObjectName("EditStyle");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      cs = s;
      buttonApplyToAllParts = buttonBox->addButton(tr("Apply to all Parts"), QDialogButtonBox::ApplyRole);
      buttonApplyToAllParts->setEnabled(!cs->isMaster());
      setModal(true);

      // create button groups for every set of radio button widgets
      // use this group widgets in list styleWidgets
      // This works for groups which represent an int enumeration.

      QButtonGroup* fretNumGroup = new QButtonGroup(this);
      fretNumGroup->addButton(radioFretNumLeft, 0);
      fretNumGroup->addButton(radioFretNumRight, 1);

      QButtonGroup* keySigNatGroup = new QButtonGroup(this);
      keySigNatGroup->addButton(radioKeySigNatNone, int(KeySigNatural::NONE));
      keySigNatGroup->addButton(radioKeySigNatBefore, int(KeySigNatural::BEFORE));
      keySigNatGroup->addButton(radioKeySigNatAfter, int(KeySigNatural::AFTER));

      QButtonGroup* clefTypeGroup = new QButtonGroup(this);
      clefTypeGroup->addButton(clefTab1, int(ClefType::TAB));
      clefTypeGroup->addButton(clefTab2, int(ClefType::TAB_SERIF));

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
      int data = 1;
      voltaLineStyle->clear();
      ottavaLineStyle->clear();
      pedalLineStyle->clear();
      for (const char* p : styles) {
            QString trs = qApp->translate("EditStyleBase", p);
            voltaLineStyle->addItem(trs, data);
            ottavaLineStyle->addItem(trs, data);
            pedalLineStyle->addItem(trs, data);
            ++data;
            }

      styleWidgets = {
      //   idx                --- showPercent      --- widget          --- resetButton
      { Sid::figuredBassAlignment,    false, fbAlign,                 0                    },
      { Sid::figuredBassStyle,        false, fbStyle,                 0                    },
      { Sid::tabClef,                 false, clefTypeGroup,           0                    },
      { Sid::keySigNaturals,          false, keySigNatGroup,          0                    },
      { Sid::voltaLineStyle,          false, voltaLineStyle,          resetVoltaLineStyle  },
      { Sid::ottavaLineStyle,         false, ottavaLineStyle,         resetOttavaLineStyle },
      { Sid::pedalLineStyle,          false, pedalLineStyle,          resetPedalLineStyle  },

      { Sid::staffUpperBorder,        false, staffUpperBorder,        resetStaffUpperBorder  },
      { Sid::staffLowerBorder,        false, staffLowerBorder,        resetStaffLowerBorder  },
      { Sid::staffDistance,           false, staffDistance,           resetStaffDistance     },
      { Sid::akkoladeDistance,        false, akkoladeDistance,        resetAkkoladeDistance  },
      { Sid::minSystemDistance,       false, minSystemDistance,       resetMinSystemDistance },
      { Sid::maxSystemDistance,       false, maxSystemDistance,       resetMaxSystemDistance },

      { Sid::lyricsPlacement,         false, lyricsPlacement,         resetLyricsPlacement },
      { Sid::lyricsPosAbove,          false, lyricsPosAbove,          resetLyricsPosAbove },
      { Sid::lyricsPosBelow,          false, lyricsPosBelow,          resetLyricsPosBelow },
      { Sid::lyricsMinTopDistance,    false, lyricsMinTopDistance,    resetLyricsMinTopDistance },
      { Sid::lyricsMinBottomDistance, false, lyricsMinBottomDistance, resetLyricsMinBottomDistance },
      { Sid::lyricsLineHeight,        true,  lyricsLineHeight,        resetLyricsLineHeight },
      { Sid::lyricsDashMinLength,     false, lyricsDashMinLength,     resetLyricsDashMinLength },
      { Sid::lyricsDashMaxLength,     false, lyricsDashMaxLength,     resetLyricsDashMaxLength },
      { Sid::lyricsDashMaxDistance,   false, lyricsDashMaxDistance,   resetLyricsDashMaxDistance },
      { Sid::lyricsDashForce,         false, lyricsDashForce,         resetLyricsDashForce },
      { Sid::lyricsAlignVerseNumber,  false, lyricsAlignVerseNumber,  resetLyricsAlignVerseNumber },
      { Sid::lyricsLineThickness,     false, lyricsLineThickness,     resetLyricsLineThickness },

      { Sid::lyricsOddFontFace,       false, lyricsOddFontFace,       resetLyricsOddFontFace       },
      { Sid::lyricsOddFontSize,       false, lyricsOddFontSize,       resetLyricsOddFontSize       },
      { Sid::lyricsOddFontBold,       false, lyricsOddFontBold,       resetLyricsOddFontBold       },
      { Sid::lyricsOddFontItalic,     false, lyricsOddFontItalic,     resetLyricsOddFontItalic     },
      { Sid::lyricsOddFontUnderline,  false, lyricsOddFontUnderline,  resetLyricsOddFontUnderline  },
      { Sid::lyricsEvenFontFace,      false, lyricsEvenFontFace,      resetLyricsEvenFontFace      },
      { Sid::lyricsEvenFontSize,      false, lyricsEvenFontSize,      resetLyricsEvenFontSize      },
      { Sid::lyricsEvenFontBold,      false, lyricsEvenFontBold,      resetLyricsEvenFontBold      },
      { Sid::lyricsEvenFontItalic,    false, lyricsEvenFontItalic,    resetLyricsEvenFontItalic    },
      { Sid::lyricsEvenFontUnderline, false, lyricsEvenFontUnderline, resetLyricsEvenFontUnderline },

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
      { Sid::minEmptyMeasures,        false, minEmptyMeasures,        0 },
      { Sid::minMMRestWidth,          false, minMeasureWidth,         resetMinMMRestWidth },
      { Sid::hideEmptyStaves,         false, hideEmptyStaves,         0 },
      { Sid::dontHideStavesInFirstSystem, false, dontHideStavesInFirstSystem,             0 },
      { Sid::hideInstrumentNameIfOneInstrument, false, hideInstrumentNameIfOneInstrument, 0 },
      { Sid::accidentalNoteDistance,  false, accidentalNoteDistance,  0 },
      { Sid::accidentalDistance,      false, accidentalDistance,      0 },

      { Sid::minNoteDistance,         false, minNoteDistance,         resetMinNoteDistance },
      { Sid::barNoteDistance,         false, barNoteDistance,         resetBarNoteDistance },
      { Sid::barAccidentalDistance,   false, barAccidentalDistance,   resetBarAccidentalDistance },
      { Sid::multiMeasureRestMargin,  false, multiMeasureRestMargin,  resetMultiMeasureRestMargin },
      { Sid::noteBarDistance,         false, noteBarDistance,         resetNoteBarDistance },
      { Sid::clefLeftMargin,          false, clefLeftMargin,          resetClefLeftMargin },
      { Sid::keysigLeftMargin,        false, keysigLeftMargin,        resetKeysigLeftMargin },
      { Sid::timesigLeftMargin,       false, timesigLeftMargin,       resetTimesigLeftMargin },
      { Sid::clefKeyRightMargin,      false, clefKeyRightMargin,      resetClefKeyRightMargin },
      { Sid::clefKeyDistance,         false, clefKeyDistance,         resetClefKeyDistance },
      { Sid::clefTimesigDistance,     false, clefTimesigDistance,     resetClefTimesigDistance },
      { Sid::keyTimesigDistance,      false, keyTimesigDistance,      resetKeyTimesigDistance },
      { Sid::keyBarlineDistance,      false, keyBarlineDistance,      resetKeyBarlineDistance },
      { Sid::systemHeaderDistance,    false, systemHeaderDistance,    resetSystemHeaderDistance },
      { Sid::systemHeaderTimeSigDistance,    false, systemHeaderTimeSigDistance,    resetSystemHeaderTimeSigDistance },

      { Sid::clefBarlineDistance,     false, clefBarlineDistance,     resetClefBarlineDistance },
      { Sid::timesigBarlineDistance,  false, timesigBarlineDistance,  resetTimesigBarlineDistance },
      { Sid::staffLineWidth,          false, staffLineWidth,          resetStaffLineWidth },
      { Sid::beamWidth,               false, beamWidth,               0 },
      { Sid::beamMinLen,              false, beamMinLen,              0 },

      { Sid::hairpinPlacement,        false, hairpinPlacement,        resetHairpinPlacement },
      { Sid::hairpinPosAbove,         false, hairpinPosAbove,         resetHairpinPosAbove },
      { Sid::hairpinPosBelow,         false, hairpinPosBelow,         resetHairpinPosBelow },
      { Sid::hairpinLineWidth,        false, hairpinLineWidth,        resetHairpinLineWidth },
      { Sid::hairpinHeight,           false, hairpinHeight,           resetHairpinHeight },
      { Sid::hairpinContHeight,       false, hairpinContinueHeight,   resetHairpinContinueHeight },

      { Sid::dotNoteDistance,         false, noteDotDistance,         0 },
      { Sid::dotDotDistance,          false, dotDotDistance,          0 },
      { Sid::stemWidth,               false, stemWidth,               0 },
      { Sid::ledgerLineWidth,         false, ledgerLineWidth,         0 },
      { Sid::ledgerLineLength,        false, ledgerLineLength,        0 },
      { Sid::shortStemProgression,    false, shortStemProgression,    0 },
      { Sid::shortestStem,            false, shortestStem,            0 },
      { Sid::ArpeggioNoteDistance,    false, arpeggioNoteDistance,    0 },
      { Sid::ArpeggioLineWidth,       false, arpeggioLineWidth,       0 },
      { Sid::ArpeggioHookLen,         false, arpeggioHookLen,         0 },
      { Sid::ArpeggioHiddenInStdIfTab,false, arpeggioHiddenInStdIfTab,0 },
      { Sid::SlurEndWidth,            false, slurEndLineWidth,        resetSlurEndLineWidth    },
      { Sid::SlurMidWidth,            false, slurMidLineWidth,        resetSlurMidLineWidth    },
      { Sid::SlurDottedWidth,         false, slurDottedLineWidth,     resetSlurDottedLineWidth },
      { Sid::SlurMinDistance,         false, slurMinDistance,         resetSlurMinDistance     },
      { Sid::MinTieLength,            false, minTieLength,            resetMinTieLength        },
      { Sid::bracketWidth,            false, bracketWidth,            0 },
      { Sid::bracketDistance,         false, bracketDistance,         0 },
      { Sid::akkoladeWidth,           false, akkoladeWidth,           0 },
      { Sid::akkoladeBarDistance,     false, akkoladeBarDistance,     0 },
      { Sid::dividerLeft,             false, dividerLeft,             0 },
      { Sid::dividerLeftX,            false, dividerLeftX,            0 },
      { Sid::dividerLeftY,            false, dividerLeftY,            0 },
      { Sid::dividerRight,            false, dividerRight,            0 },
      { Sid::dividerRightX,           false, dividerRightX,           0 },
      { Sid::dividerRightY,           false, dividerRightY,           0 },
      { Sid::propertyDistanceHead,    false, propertyDistanceHead,    0 },
      { Sid::propertyDistanceStem,    false, propertyDistanceStem,    0 },
      { Sid::propertyDistance,        false, propertyDistance,        0 },
      { Sid::voltaY,                  false, voltaY,                  resetVoltaY },
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

      { Sid::vibratoPlacement,        false, vibratoLinePlacement,      resetVibratoLinePlacement  },
      { Sid::vibratoPosAbove,         false, vibratoLinePosAbove,       resetVibratoLinePosAbove   },
      { Sid::vibratoPosBelow,         false, vibratoLinePosBelow,       resetVibratoLinePosBelow   },

      { Sid::chordSymbolAPosAbove,    false, chordSymbolPosAboveA,    resetChordSymbolPosAboveA },
      { Sid::chordSymbolBPosAbove,    false, chordSymbolPosAboveB,    resetChordSymbolPosAboveB },
      { Sid::harmonyFretDist,         false, harmonyFretDist,         0 },
      { Sid::minHarmonyDistance,      false, minHarmonyDistance,      0 },
      { Sid::maxHarmonyBarDistance,   false, maxHarmonyBarDistance,   0 },

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
      { Sid::tupletOufOfStaff,        false, tupletOutOfStaff,        0                             },
      { Sid::tupletFontFace,          false, tupletFontFace,          resetTupletFontFace           },
      { Sid::tupletFontSize,          false, tupletFontSize,          resetTupletFontSize           },
      { Sid::tupletFontBold,          false, tupletFontBold,          resetTupletFontBold           },
      { Sid::tupletFontItalic,        false, tupletFontItalic,        resetTupletFontItalic         },
      { Sid::tupletFontUnderline,     false, tupletFontUnderline,     resetTupletFontUnderline      },

      { Sid::repeatBarTips,           false, showRepeatBarTips,            resetShowRepeatBarTips },
      { Sid::startBarlineSingle,      false, showStartBarlineSingle,       resetShowStartBarlineSingle },
      { Sid::startBarlineMultiple,    false, showStartBarlineMultiple,     resetShowStartBarlineMultiple },
      { Sid::dividerLeftSym,          false, dividerLeftSym,               0 },
      { Sid::dividerRightSym,         false, dividerRightSym,              0 },

      { Sid::showMeasureNumber,          false, showMeasureNumber,            0 },
      { Sid::showMeasureNumberOne,       false, showFirstMeasureNumber,       0 },
      { Sid::measureNumberInterval,      false, intervalMeasureNumber,        0 },
      { Sid::measureNumberSystem,        false, showEverySystemMeasureNumber, 0 },
      { Sid::measureNumberAllStaffs,     false, showAllStaffsMeasureNumber,   0 },
      { Sid::measureNumberFontFace,      false, measureNumberFontFace,        resetMeasureNumberFontFace },
      { Sid::measureNumberFontSize,      false, measureNumberFontSize,        resetMeasureNumberFontSize },
      { Sid::measureNumberFontBold,      false, measureNumberBold,            resetMeasureNumberBold },
      { Sid::measureNumberFontItalic,    false, measureNumberItalic,          resetMeasureNumberItalic },
      { Sid::measureNumberFontUnderline, false, measureNumberUnderline,       resetMeasureNumberUnderline },
//      { Sid::measureNumberOffset,           "measureNumberOffset",          QPointF(0.0, -2.0) },

      { Sid::shortInstrumentFontFace,      false, shortInstrumentFontFace,      resetShortInstrumentFontFace },
      { Sid::shortInstrumentFontSize,      false, shortInstrumentFontSize,      resetShortInstrumentFontSize },
      { Sid::shortInstrumentFontBold,      false, shortInstrumentFontBold,      resetShortInstrumentFontBold },
      { Sid::shortInstrumentFontItalic,    false, shortInstrumentFontItalic,    resetShortInstrumentFontItalic },
      { Sid::shortInstrumentFontUnderline, false, shortInstrumentFontUnderline, resetShortInstrumentFontUnderline },

      { Sid::longInstrumentFontFace,      false, longInstrumentFontFace,        resetLongInstrumentFontFace },
      { Sid::longInstrumentFontSize,      false, longInstrumentFontSize,        resetLongInstrumentFontSize },
      { Sid::longInstrumentFontBold,      false, longInstrumentFontBold,        resetLongInstrumentFontBold },
      { Sid::longInstrumentFontItalic,    false, longInstrumentFontItalic,      resetLongInstrumentFontItalic },
      { Sid::longInstrumentFontUnderline, false, longInstrumentFontUnderline,   resetLongInstrumentFontUnderline },

      { Sid::headerFontFace,      false, headerFontFace,        resetHeaderFontFace },
      { Sid::headerFontSize,      false, headerFontSize,        resetHeaderFontSize },
      { Sid::headerFontBold,      false, headerFontBold,        resetHeaderFontBold },
      { Sid::headerFontItalic,    false, headerFontItalic,      resetHeaderFontItalic },
      { Sid::headerFontUnderline, false, headerFontUnderline,   resetHeaderFontUnderline },

      { Sid::footerFontFace,      false, footerFontFace,        resetFooterFontFace },
      { Sid::footerFontSize,      false, footerFontSize,        resetFooterFontSize },
      { Sid::footerFontBold,      false, footerFontBold,        resetFooterFontBold },
      { Sid::footerFontItalic,    false, footerFontItalic,      resetFooterFontItalic },
      { Sid::footerFontUnderline, false, footerFontUnderline,   resetFooterFontUnderline },

      { Sid::beamDistance,            true,  beamDistance,                 0 },
      { Sid::beamNoSlope,             false, beamNoSlope,                  0 },
      { Sid::graceNoteMag,            true,  graceNoteSize,                resetGraceNoteSize  },
      { Sid::smallStaffMag,           true,  smallStaffSize,               resetSmallStaffSize },
      { Sid::smallNoteMag,            true,  smallNoteSize,                resetSmallNoteSize  },
      { Sid::smallClefMag,            true,  smallClefSize,                resetSmallClefSize  },
      { Sid::lastSystemFillLimit,     true,  lastSystemFillThreshold,      resetLastSystemFillThreshold },
      { Sid::genClef,                 false, genClef,                      0 },
      { Sid::genKeysig,               false, genKeysig,                    0 },
      { Sid::genCourtesyTimesig,      false, genCourtesyTimesig,           0 },
      { Sid::genCourtesyKeysig,       false, genCourtesyKeysig,            0 },
      { Sid::genCourtesyClef,         false, genCourtesyClef,              0 },
      { Sid::swingRatio,              false, swingBox,                     0 },
      { Sid::chordsXmlFile,           false, chordsXmlFile,                0 },
      { Sid::dotMag,                  true,  dotMag,                       0 },
      { Sid::articulationMag,         true,  articulationMag,              0 },
      { Sid::shortenStem,             false, shortenStem,                  0 },
      { Sid::showHeader,              false, showHeader,                   0 },
      { Sid::headerFirstPage,         false, showHeaderFirstPage,          0 },
      { Sid::headerOddEven,           false, headerOddEven,                0 },
      { Sid::evenHeaderL,             false, evenHeaderL,                  0 },
      { Sid::evenHeaderC,             false, evenHeaderC,                  0 },
      { Sid::evenHeaderR,             false, evenHeaderR,                  0 },
      { Sid::oddHeaderL,              false, oddHeaderL,                   0 },
      { Sid::oddHeaderC,              false, oddHeaderC,                   0 },
      { Sid::oddHeaderR,              false, oddHeaderR,                   0 },
      { Sid::showFooter,              false, showFooter,                   0 },
      { Sid::footerFirstPage,         false, showFooterFirstPage,          0 },
      { Sid::footerOddEven,           false, footerOddEven,                0 },
      { Sid::evenFooterL,             false, evenFooterL,                  0 },
      { Sid::evenFooterC,             false, evenFooterC,                  0 },
      { Sid::evenFooterR,             false, evenFooterR,                  0 },
      { Sid::oddFooterL,              false, oddFooterL,                   0 },
      { Sid::oddFooterC,              false, oddFooterC,                   0 },
      { Sid::oddFooterR,              false, oddFooterR,                   0 },

      { Sid::ottavaNumbersOnly,       false, ottavaNumbersOnly,            resetOttavaNumbersOnly },
      { Sid::capoPosition,            false, capoPosition,                 0 },
      { Sid::fretNumMag,              true,  fretNumMag,                   0 },
      { Sid::fretNumPos,              false, fretNumGroup,                 0 },
      { Sid::fretY,                   false, fretY,                        0 },
      { Sid::barreLineWidth,          false, barreLineWidth,               0 },
      { Sid::fretMag,                 false, fretMag,                      0 },
      { Sid::scaleBarlines,           false, scaleBarlines,                resetScaleBarlines},
      { Sid::crossMeasureValues,      false, crossMeasureValues,           0 },

      { Sid::MusicalSymbolFont,       false, musicalSymbolFont,            0 },
      { Sid::MusicalTextFont,         false, musicalTextFont,              0 },
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
      { Sid::textLinePlacement,           false, textLinePlacement,           resetTextLinePlacement },
      { Sid::textLinePosAbove,            false, textLinePosAbove,            resetTextLinePosAbove },
      { Sid::textLinePosBelow,            false, textLinePosBelow,            resetTextLinePosBelow },

      { Sid::fermataPosAbove,         false, fermataPosAbove,       resetFermataPosAbove    },
      { Sid::fermataPosBelow,         false, fermataPosBelow,       resetFermataPosBelow    },
      { Sid::fermataMinDistance,      false, fermataMinDistance,    resetFermataMinDistance },

      { Sid::staffTextPlacement,      false, staffTextPlacement,    resetStaffTextPlacement   },
      { Sid::staffTextPosAbove,       false, staffTextPosAbove,     resetStaffTextPosAbove    },
      { Sid::staffTextPosBelow,       false, staffTextPosBelow,     resetStaffTextPosBelow    },
      { Sid::staffTextMinDistance,    false, staffTextMinDistance,  resetStaffTextMinDistance },

      { Sid::bendFontFace,      false, bendFontFace,      resetBendFontFace      },
      { Sid::bendFontSize,      false, bendFontSize,      resetBendFontSize      },
      { Sid::bendFontBold,      false, bendFontBold,      resetBendFontBold      },
      { Sid::bendFontItalic,    false, bendFontItalic,    resetBendFontItalic    },
      { Sid::bendFontUnderline, false, bendFontUnderline, resetBendFontUnderline },
      { Sid::bendLineWidth,     false, bendLineWidth,     resetBendLineWidth     },
      { Sid::bendArrowWidth,    false, bendArrowWidth,    resetBendArrowWidth     },
      };

      for (QComboBox* cb : std::vector<QComboBox*> {
            lyricsPlacement, textLinePlacement, hairpinPlacement, pedalLinePlacement,
            trillLinePlacement, vibratoLinePlacement, dynamicsPlacement,
            tempoTextPlacement, staffTextPlacement, rehearsalMarkPlacement
            }) {
            cb->clear();
            cb->addItem(tr("Above"), int(Placement::ABOVE));
            cb->addItem(tr("Below"), int(Placement::BELOW));
            }

      autoplaceVerticalAlignRange->clear();
      autoplaceVerticalAlignRange->addItem(tr("Segment"), int(VerticalAlignRange::SEGMENT));
      autoplaceVerticalAlignRange->addItem(tr("Measure"), int(VerticalAlignRange::MEASURE));
      autoplaceVerticalAlignRange->addItem(tr("System"),  int(VerticalAlignRange::SYSTEM));

      tupletNumberType->clear();
      tupletNumberType->addItem(tr("Number"), int(TupletNumberType::SHOW_NUMBER));
      tupletNumberType->addItem(tr("Ratio"), int(TupletNumberType::SHOW_RELATION));
      tupletNumberType->addItem(tr("Nothing"), int(TupletNumberType::NO_TEXT));

      tupletBracketType->clear();
      tupletBracketType->addItem(tr("Automatic"), int(TupletBracketType::AUTO_BRACKET));
      tupletBracketType->addItem(tr("Bracket"), int(TupletBracketType::SHOW_BRACKET));
      tupletBracketType->addItem(tr("Nothing"), int(TupletBracketType::SHOW_NO_BRACKET));

      pageList->setCurrentRow(0);
      accidentalsGroup->setVisible(false); // disable, not yet implemented

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
      for (const QString& family: fbFontNames)
            comboFBFont->addItem(family);
      comboFBFont->setCurrentIndex(0);
      connect(comboFBFont, SIGNAL(currentIndexChanged(int)), SLOT(on_comboFBFont_currentIndexChanged(int)));

      setValues();

      // keep in sync with implementation in Page::replaceTextMacros (page.cpp)
      // jumping thru hoops here to make the job of translators easier, yet have a nice display
      QString toolTipHeaderFooter
            = QString("<html><head></head><body><p><b>")
            + tr("Special symbols in header/footer")
            + QString("</b></p>")
            + QString("<table><tr><td>$p</td><td>-</td><td><i>")
            + tr("page number, except on first page")
            + QString("</i></td></tr><tr><td>$N</td><td>-</td><td><i>")
            + tr("page number, if there is more than one page")
            + QString("</i></td></tr><tr><td>$P</td><td>-</td><td><i>")
            + tr("page number, on all pages")
            + QString("</i></td></tr><tr><td>$n</td><td>-</td><td><i>")
            + tr("number of pages")
            + QString("</i></td></tr><tr><td>$f</td><td>-</td><td><i>")
            + tr("file name")
            + QString("</i></td></tr><tr><td>$F</td><td>-</td><td><i>")
            + tr("file path+name")
            + QString("</i></td></tr><tr><td>$i</td><td>-</td><td><i>")
            + tr("part name, except on first page")
            + QString("</i></td></tr><tr><td>$I</td><td>-</td><td><i>")
            + tr("part name, on all pages")
            + QString("</i></td></tr><tr><td>$d</td><td>-</td><td><i>")
            + tr("current date")
            + QString("</i></td></tr><tr><td>$D</td><td>-</td><td><i>")
            + tr("creation date")
            + QString("</i></td></tr><tr><td>$m</td><td>-</td><td><i>")
            + tr("last modification time")
            + QString("</i></td></tr><tr><td>$M</td><td>-</td><td><i>")
            + tr("last modification date")
            + QString("</i></td></tr><tr><td>$C</td><td>-</td><td><i>")
            + tr("copyright, on first page only")
            + QString("</i></td></tr><tr><td>$c</td><td>-</td><td><i>")
            + tr("copyright, on all pages")
            + QString("</i></td></tr><tr><td>$$</td><td>-</td><td><i>")
            + tr("the $ sign itself")
            + QString("</i></td></tr><tr><td>$:tag:</td><td>-</td><td><i>")
            + tr("metadata tag, see below")
            + QString("</i></td></tr></table><p>")
            + tr("Available metadata tags and their current values")
            + QString("<br />")
            + tr("(in File > Score Properties...):")
            + QString("</p><table>");
      // show all tags for current score/part, see also Score::init()
      if (!cs->isMaster()) {
            QMapIterator<QString, QString> j(cs->masterScore()->metaTags());
            while (j.hasNext()) {
                  j.next();
                  toolTipHeaderFooter += QString("<tr><td>%1</td><td>-</td><td>%2</td></tr>").arg(j.key()).arg(j.value());
                  }
            }
      QMapIterator<QString, QString> i(cs->metaTags());
      while (i.hasNext()) {
            i.next();
            toolTipHeaderFooter += QString("<tr><td>%1</td><td>-</td><td>%2</td></tr>").arg(i.key()).arg(i.value());
            }
      toolTipHeaderFooter += QString("</table></body></html>");
      showHeader->setToolTip(toolTipHeaderFooter);
      showFooter->setToolTip(toolTipHeaderFooter);

      connect(buttonBox,           SIGNAL(clicked(QAbstractButton*)), SLOT(buttonClicked(QAbstractButton*)));
      connect(headerOddEven,       SIGNAL(toggled(bool)),             SLOT(toggleHeaderOddEven(bool)));
      connect(footerOddEven,       SIGNAL(toggled(bool)),             SLOT(toggleFooterOddEven(bool)));
      connect(chordDescriptionFileButton, SIGNAL(clicked()),          SLOT(selectChordDescriptionFile()));
      connect(chordsStandard,      SIGNAL(toggled(bool)),             SLOT(setChordStyle(bool)));
      connect(chordsJazz,          SIGNAL(toggled(bool)),             SLOT(setChordStyle(bool)));
      connect(chordsCustom,        SIGNAL(toggled(bool)),             SLOT(setChordStyle(bool)));

      connect(SwingOff,            SIGNAL(toggled(bool)),             SLOT(setSwingParams(bool)));
      connect(swingEighth,         SIGNAL(toggled(bool)),             SLOT(setSwingParams(bool)));
      connect(swingSixteenth,      SIGNAL(toggled(bool)),             SLOT(setSwingParams(bool)));

      connect(hideEmptyStaves,     SIGNAL(clicked(bool)), dontHideStavesInFirstSystem, SLOT(setEnabled(bool)));
      connect(lyricsDashMinLength, SIGNAL(valueChanged(double)),      SLOT(lyricsDashMinLengthValueChanged(double)));
      connect(lyricsDashMaxLength, SIGNAL(valueChanged(double)),      SLOT(lyricsDashMaxLengthValueChanged(double)));
      connect(minSystemDistance,   SIGNAL(valueChanged(double)),      SLOT(systemMinDistanceValueChanged(double)));
      connect(maxSystemDistance,   SIGNAL(valueChanged(double)),      SLOT(systemMaxDistanceValueChanged(double)));

      QSignalMapper* mapper  = new QSignalMapper(this);     // reset style signals
      QSignalMapper* mapper2 = new QSignalMapper(this);     // value change signals

      for (const StyleWidget& sw : styleWidgets) {
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
            else {
                  qFatal("unhandled gui widget type %s valueType %s",
                     sw.widget->metaObject()->className(),
                     MStyle::valueName(sw.idx)
                  );
                  }

            mapper2->setMapping(sw.widget, int(sw.idx));
            }

      connect(mapper,  SIGNAL(mapped(int)), SLOT(resetStyleValue(int)));
      connect(mapper2, SIGNAL(mapped(int)), SLOT(valueChanged(int)));

      MuseScore::restoreGeometry(this);
      cs->startCmd();
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
                  done(1);
                  cs->endCmd();
                  break;
            case QDialogButtonBox::Cancel:
                  done(0);
                  cs->endCmd(true);
                  break;
            case QDialogButtonBox::NoButton:
            default:
                  if (b == buttonApplyToAllParts)
                        applyToAllParts();
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
            spinFBLineHeight->setValue((int)(lineHeight * 100.0));
            }
      }

//---------------------------------------------------------
//   applyToAllParts
//---------------------------------------------------------

void EditStyle::applyToAllParts()
      {
      for (Excerpt* e : cs->masterScore()->excerpts()) {
            e->partScore()->undo(new ChangeStyle(e->partScore(), cs->style()));
            e->partScore()->update();
            }
      }

//---------------------------------------------------------
//   unhandledType
//---------------------------------------------------------

static void unhandledType(const StyleWidget* sw)
      {
      const char* type = MStyle::valueType(sw->idx);
      qFatal("unhandled %s <%s>: widget: %s\n", type, MStyle::valueName(sw->idx), sw->widget->metaObject()->className());
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
            QVariant v = sw.widget->property("checked");
            if (!v.isValid())
                  unhandledType(&sw);
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
      else if (!strcmp("Ms::Direction", type)) {
            QComboBox* cb = qobject_cast<QComboBox*>(sw.widget);
            if (cb)
                  return cb->currentIndex();
            else
                  qFatal("unhandled Direction");
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
      for (const StyleWidget& sw : styleWidgets) {
            if (sw.widget)
                  sw.widget->blockSignals(true);
            QVariant val = lstyle.value(sw.idx);
            const char* type = MStyle::valueType(sw.idx);
            if (sw.reset)
                  sw.reset->setEnabled(!lstyle.isDefault(sw.idx));

            if (!strcmp("Ms::Spatium", type)) {
                  if (sw.showPercent)
                        qobject_cast<QSpinBox*>(sw.widget)->setValue(int(val.value<Spatium>().val() * 100.0));
                  else
                        sw.widget->setProperty("value", val);
                  }
            else if (!strcmp("double", type)) {
                  if (sw.showPercent)
                        val = QVariant(val.toDouble() * 100);
                  if (!sw.widget->setProperty("value", val))
                        unhandledType(&sw);
                  }
            else if (!strcmp("bool", type)) {
                  if (!sw.widget->setProperty("checked", val))
                        unhandledType(&sw);
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
                        for (auto a : bg->buttons()) {
                              if (bg->id(a) == val.toInt()) {
                                    a->setChecked(true);
                                    break;
                                    }
                              }
                        }
                  else
                        unhandledType(&sw);
                  }
            else if (!strcmp("QString", type)) {
                  if (qobject_cast<QFontComboBox*>(sw.widget))
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
                  else if (qobject_cast<QTextEdit*>(sw.widget))
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
            else
                  unhandledType(&sw);
            if (sw.widget)
                  sw.widget->blockSignals(false);
            }

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
            SwingOff->setChecked(true);
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

      dontHideStavesInFirstSystem->setEnabled(hideEmptyStaves->isChecked());

      // figured bass
      for(int i = 0; i < comboFBFont->count(); i++)
            if(comboFBFont->itemText(i) == lstyle.value(Sid::figuredBassFontFamily).toString()) {
                  comboFBFont->setCurrentIndex(i);
                  break;
            }
      doubleSpinFBSize->setValue(lstyle.value(Sid::figuredBassFontSize).toDouble());
      doubleSpinFBVertPos->setValue(lstyle.value(Sid::figuredBassYOffset).toDouble());
      spinFBLineHeight->setValue(lstyle.value(Sid::figuredBassLineHeight).toDouble() * 100.0);

      QString mfont(lstyle.value(Sid::MusicalSymbolFont).toString());
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
      QString tfont(lstyle.value(Sid::MusicalTextFont).toString());
      idx = musicalTextFont->findData(tfont);
      musicalTextFont->setCurrentIndex(idx);
      musicalTextFont->blockSignals(false);

      toggleHeaderOddEven(lstyle.value(Sid::headerOddEven).toBool());
      toggleFooterOddEven(lstyle.value(Sid::footerOddEven).toBool());
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
      }

//---------------------------------------------------------
//   setSwingParams
//---------------------------------------------------------

void EditStyle::setSwingParams(bool checked)
      {
      if (!checked)
            return;
      QVariant val;
      if (SwingOff->isChecked()) {
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
//   setChordStyle
//---------------------------------------------------------

void EditStyle::setChordStyle(bool checked)
      {
      if (!checked)
            return;
      QVariant val;
      QString file;
      if (chordsStandard->isChecked()) {
            val  = QString("std");
            file = "chords_std.xml";
            }
      else if (chordsJazz->isChecked()) {
            val  = QString("jazz");
            file = "chords_jazz.xml";
            }
      else {
            val = QString("custom");
            chordDescriptionGroup->setEnabled(true);
            }
      cs->undo(new ChangeStyleVal(cs, Sid::chordStyle, val));
      if (!file.isEmpty()) {
            cs->undo(new ChangeStyleVal(cs, Sid::chordsXmlFile, false));
            chordsXmlFile->setChecked(false);
            chordDescriptionGroup->setEnabled(false);
            chordDescriptionFile->setText(file);
            cs->undo(new ChangeStyleVal(cs, Sid::chordDescriptionFile, file));
            cs->update();
            }
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
      __builtin_unreachable();
      }

//---------------------------------------------------------
//   valueChanged
//---------------------------------------------------------

void EditStyle::valueChanged(int i)
      {
      Sid idx = (Sid)i;
      QVariant val = getValue(idx);
      bool setValue = false;
      if (idx == Sid::MusicalSymbolFont && optimizeStyleCheckbox->isChecked()) {
              ScoreFont* scoreFont = ScoreFont::fontFactory(val.toString());
              if (scoreFont) {
                    for (auto i : scoreFont->engravingDefaults()) {
                          cs->undo(new ChangeStyleVal(cs, i.first, i.second));
                          }
                    if (scoreFont->textEnclosureThickness()) {
//                           TextStyle ts = cs->textStyle(TextStyleType::REHEARSAL_MARK);
//                           ts.setFrameWidth(Spatium(scoreFont->textEnclosureThickness()));
//TODO                           cs->undo(new ChangeTextStyle(cs, ts));
                           }
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

}

