/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include "style.h"

#include <QDebug>

#include "translation.h"

#include "libmscore/mscore.h"
#include "libmscore/xml.h"
#include "libmscore/score.h"
#include "libmscore/articulation.h"
#include "libmscore/harmony.h"
#include "libmscore/chordlist.h"
#include "libmscore/page.h"
#include "libmscore/mscore.h"
#include "libmscore/clef.h"
#include "libmscore/textlinebase.h"
#include "libmscore/tuplet.h"
#include "libmscore/layout.h"
#include "libmscore/property.h"
#include "libmscore/read206.h"
#include "libmscore/undo.h"

//! NOTE It is necessary that `StyleDef::styleTypes` is initialized before static `MStyle` is initialized
#include "styledef.cpp"

using namespace mu;

namespace Ms {
//  20 points        font design size
//  72 points/inch   point size
// 120 dpi           screen resolution
//  spatium = 20/4 points

static const int LEGACY_MSC_VERSION_V3 = 301;
static const int LEGACY_MSC_VERSION_V2 = 206;
static const int LEGACY_MSC_VERSION_V1 = 114;

MStyle MScore::_baseStyle;
MStyle MScore::_defaultStyle;

//---------------------------------------------------------
//   text styles
//---------------------------------------------------------

const TextStyle defaultTextStyle { {
    { Sid::defaultFontFace,                    Pid::FONT_FACE },
    { Sid::defaultFontSize,                    Pid::FONT_SIZE },
    { Sid::defaultLineSpacing,                 Pid::TEXT_LINE_SPACING },
    { Sid::defaultFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::defaultFontStyle,                   Pid::FONT_STYLE },
    { Sid::defaultColor,                       Pid::COLOR },
    { Sid::defaultAlign,                       Pid::ALIGN },
    { Sid::defaultOffset,                      Pid::OFFSET },
    { Sid::defaultFrameType,                   Pid::FRAME_TYPE },
    { Sid::defaultFramePadding,                Pid::FRAME_PADDING },
    { Sid::defaultFrameWidth,                  Pid::FRAME_WIDTH },
    { Sid::defaultFrameRound,                  Pid::FRAME_ROUND },
    { Sid::defaultFrameFgColor,                Pid::FRAME_FG_COLOR },
    { Sid::defaultFrameBgColor,                Pid::FRAME_BG_COLOR },
} };

const TextStyle titleTextStyle { {
    { Sid::titleFontFace,                      Pid::FONT_FACE },
    { Sid::titleFontSize,                      Pid::FONT_SIZE },
    { Sid::titleLineSpacing,                   Pid::TEXT_LINE_SPACING },
    { Sid::titleFontSpatiumDependent,          Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::titleFontStyle,                     Pid::FONT_STYLE },
    { Sid::titleColor,                         Pid::COLOR },
    { Sid::titleAlign,                         Pid::ALIGN },
    { Sid::titleOffset,                        Pid::OFFSET },
    { Sid::titleFrameType,                     Pid::FRAME_TYPE },
    { Sid::titleFramePadding,                  Pid::FRAME_PADDING },
    { Sid::titleFrameWidth,                    Pid::FRAME_WIDTH },
    { Sid::titleFrameRound,                    Pid::FRAME_ROUND },
    { Sid::titleFrameFgColor,                  Pid::FRAME_FG_COLOR },
    { Sid::titleFrameBgColor,                  Pid::FRAME_BG_COLOR },
} };

const TextStyle subTitleTextStyle { {
    { Sid::subTitleFontFace,                   Pid::FONT_FACE },
    { Sid::subTitleFontSize,                   Pid::FONT_SIZE },
    { Sid::subTitleLineSpacing,                Pid::TEXT_LINE_SPACING },
    { Sid::subTitleFontSpatiumDependent,       Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::subTitleFontStyle,                  Pid::FONT_STYLE },
    { Sid::subTitleColor,                      Pid::COLOR },
    { Sid::subTitleAlign,                      Pid::ALIGN },
    { Sid::subTitleOffset,                     Pid::OFFSET },
    { Sid::subTitleFrameType,                  Pid::FRAME_TYPE },
    { Sid::subTitleFramePadding,               Pid::FRAME_PADDING },
    { Sid::subTitleFrameWidth,                 Pid::FRAME_WIDTH },
    { Sid::subTitleFrameRound,                 Pid::FRAME_ROUND },
    { Sid::subTitleFrameFgColor,               Pid::FRAME_FG_COLOR },
    { Sid::subTitleFrameBgColor,               Pid::FRAME_BG_COLOR },
} };

const TextStyle composerTextStyle { {
    { Sid::composerFontFace,                   Pid::FONT_FACE },
    { Sid::composerFontSize,                   Pid::FONT_SIZE },
    { Sid::composerLineSpacing,                Pid::TEXT_LINE_SPACING },
    { Sid::composerFontSpatiumDependent,       Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::composerFontStyle,                  Pid::FONT_STYLE },
    { Sid::composerColor,                      Pid::COLOR },
    { Sid::composerAlign,                      Pid::ALIGN },
    { Sid::composerOffset,                     Pid::OFFSET },
    { Sid::composerFrameType,                  Pid::FRAME_TYPE },
    { Sid::composerFramePadding,               Pid::FRAME_PADDING },
    { Sid::composerFrameWidth,                 Pid::FRAME_WIDTH },
    { Sid::composerFrameRound,                 Pid::FRAME_ROUND },
    { Sid::composerFrameFgColor,               Pid::FRAME_FG_COLOR },
    { Sid::composerFrameBgColor,               Pid::FRAME_BG_COLOR },
} };

const TextStyle lyricistTextStyle { {
    { Sid::lyricistFontFace,                   Pid::FONT_FACE },
    { Sid::lyricistFontSize,                   Pid::FONT_SIZE },
    { Sid::lyricistLineSpacing,                Pid::TEXT_LINE_SPACING },
    { Sid::lyricistFontSpatiumDependent,       Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::lyricistFontStyle,                  Pid::FONT_STYLE },
    { Sid::lyricistColor,                      Pid::COLOR },
    { Sid::lyricistAlign,                      Pid::ALIGN },
    { Sid::lyricistOffset,                     Pid::OFFSET },
    { Sid::lyricistFrameType,                  Pid::FRAME_TYPE },
    { Sid::lyricistFramePadding,               Pid::FRAME_PADDING },
    { Sid::lyricistFrameWidth,                 Pid::FRAME_WIDTH },
    { Sid::lyricistFrameRound,                 Pid::FRAME_ROUND },
    { Sid::lyricistFrameFgColor,               Pid::FRAME_FG_COLOR },
    { Sid::lyricistFrameBgColor,               Pid::FRAME_BG_COLOR },
} };

const TextStyle lyricsEvenTextStyle { {
    { Sid::lyricsEvenFontFace,                 Pid::FONT_FACE },
    { Sid::lyricsEvenFontSize,                 Pid::FONT_SIZE },
    { Sid::lyricsEvenLineSpacing,              Pid::TEXT_LINE_SPACING },
    { Sid::lyricsEvenFontSpatiumDependent,     Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::lyricsEvenFontStyle,                Pid::FONT_STYLE },
    { Sid::lyricsEvenColor,                    Pid::COLOR },
    { Sid::lyricsEvenAlign,                    Pid::ALIGN },
    { Sid::lyricsPosBelow,                     Pid::OFFSET },
    { Sid::lyricsEvenFrameType,                Pid::FRAME_TYPE },
    { Sid::lyricsEvenFramePadding,             Pid::FRAME_PADDING },
    { Sid::lyricsEvenFrameWidth,               Pid::FRAME_WIDTH },
    { Sid::lyricsEvenFrameRound,               Pid::FRAME_ROUND },
    { Sid::lyricsEvenFrameFgColor,             Pid::FRAME_FG_COLOR },
    { Sid::lyricsEvenFrameBgColor,             Pid::FRAME_BG_COLOR },
} };

const TextStyle lyricsOddTextStyle { {
    { Sid::lyricsOddFontFace,                  Pid::FONT_FACE },
    { Sid::lyricsOddFontSize,                  Pid::FONT_SIZE },
    { Sid::lyricsOddLineSpacing,               Pid::TEXT_LINE_SPACING },
    { Sid::lyricsOddFontSpatiumDependent,      Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::lyricsOddFontStyle,                 Pid::FONT_STYLE },
    { Sid::lyricsOddColor,                     Pid::COLOR },
    { Sid::lyricsOddAlign,                     Pid::ALIGN },
    { Sid::lyricsPosBelow,                     Pid::OFFSET },
    { Sid::lyricsOddFrameType,                 Pid::FRAME_TYPE },
    { Sid::lyricsOddFramePadding,              Pid::FRAME_PADDING },
    { Sid::lyricsOddFrameWidth,                Pid::FRAME_WIDTH },
    { Sid::lyricsOddFrameRound,                Pid::FRAME_ROUND },
    { Sid::lyricsOddFrameFgColor,              Pid::FRAME_FG_COLOR },
    { Sid::lyricsOddFrameBgColor,              Pid::FRAME_BG_COLOR },
} };

const TextStyle fingeringTextStyle { {
    { Sid::fingeringFontFace,                  Pid::FONT_FACE },
    { Sid::fingeringFontSize,                  Pid::FONT_SIZE },
    { Sid::fingeringLineSpacing,               Pid::TEXT_LINE_SPACING },
    { Sid::fingeringFontSpatiumDependent,      Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::fingeringFontStyle,                 Pid::FONT_STYLE },
    { Sid::fingeringColor,                     Pid::COLOR },
    { Sid::fingeringAlign,                     Pid::ALIGN },
    { Sid::fingeringOffset,                    Pid::OFFSET },
    { Sid::fingeringFrameType,                 Pid::FRAME_TYPE },
    { Sid::fingeringFramePadding,              Pid::FRAME_PADDING },
    { Sid::fingeringFrameWidth,                Pid::FRAME_WIDTH },
    { Sid::fingeringFrameRound,                Pid::FRAME_ROUND },
    { Sid::fingeringFrameFgColor,              Pid::FRAME_FG_COLOR },
    { Sid::fingeringFrameBgColor,              Pid::FRAME_BG_COLOR },
} };

const TextStyle lhGuitarFingeringTextStyle { {
    { Sid::lhGuitarFingeringFontFace,             Pid::FONT_FACE },
    { Sid::lhGuitarFingeringFontSize,             Pid::FONT_SIZE },
    { Sid::lhGuitarFingeringLineSpacing,          Pid::TEXT_LINE_SPACING },
    { Sid::lhGuitarFingeringFontSpatiumDependent, Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::lhGuitarFingeringFontStyle,            Pid::FONT_STYLE },
    { Sid::lhGuitarFingeringColor,                Pid::COLOR },
    { Sid::lhGuitarFingeringAlign,                Pid::ALIGN },
    { Sid::lhGuitarFingeringOffset,               Pid::OFFSET },
    { Sid::lhGuitarFingeringFrameType,            Pid::FRAME_TYPE },
    { Sid::lhGuitarFingeringFramePadding,         Pid::FRAME_PADDING },
    { Sid::lhGuitarFingeringFrameWidth,           Pid::FRAME_WIDTH },
    { Sid::lhGuitarFingeringFrameRound,           Pid::FRAME_ROUND },
    { Sid::lhGuitarFingeringFrameFgColor,         Pid::FRAME_FG_COLOR },
    { Sid::lhGuitarFingeringFrameBgColor,         Pid::FRAME_BG_COLOR },
} };

const TextStyle rhGuitarFingeringTextStyle { {
    { Sid::rhGuitarFingeringFontFace,             Pid::FONT_FACE },
    { Sid::rhGuitarFingeringFontSize,             Pid::FONT_SIZE },
    { Sid::rhGuitarFingeringLineSpacing,          Pid::TEXT_LINE_SPACING },
    { Sid::rhGuitarFingeringFontSpatiumDependent, Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::rhGuitarFingeringFontStyle,            Pid::FONT_STYLE },
    { Sid::rhGuitarFingeringColor,                Pid::COLOR },
    { Sid::rhGuitarFingeringAlign,                Pid::ALIGN },
    { Sid::rhGuitarFingeringOffset,               Pid::OFFSET },
    { Sid::rhGuitarFingeringFrameType,            Pid::FRAME_TYPE },
    { Sid::rhGuitarFingeringFramePadding,         Pid::FRAME_PADDING },
    { Sid::rhGuitarFingeringFrameWidth,           Pid::FRAME_WIDTH },
    { Sid::rhGuitarFingeringFrameRound,           Pid::FRAME_ROUND },
    { Sid::rhGuitarFingeringFrameFgColor,         Pid::FRAME_FG_COLOR },
    { Sid::rhGuitarFingeringFrameBgColor,         Pid::FRAME_BG_COLOR },
} };

const TextStyle stringNumberTextStyle { {
    { Sid::stringNumberFontFace,               Pid::FONT_FACE },
    { Sid::stringNumberFontSize,               Pid::FONT_SIZE },
    { Sid::stringNumberLineSpacing,            Pid::TEXT_LINE_SPACING },
    { Sid::stringNumberFontSpatiumDependent,   Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::stringNumberFontStyle,              Pid::FONT_STYLE },
    { Sid::stringNumberColor,                  Pid::COLOR },
    { Sid::stringNumberAlign,                  Pid::ALIGN },
    { Sid::stringNumberOffset,                 Pid::OFFSET },
    { Sid::stringNumberFrameType,              Pid::FRAME_TYPE },
    { Sid::stringNumberFramePadding,           Pid::FRAME_PADDING },
    { Sid::stringNumberFrameWidth,             Pid::FRAME_WIDTH },
    { Sid::stringNumberFrameRound,             Pid::FRAME_ROUND },
    { Sid::stringNumberFrameFgColor,           Pid::FRAME_FG_COLOR },
    { Sid::stringNumberFrameBgColor,           Pid::FRAME_BG_COLOR },
} };

const TextStyle longInstrumentTextStyle { {
    { Sid::longInstrumentFontFace,             Pid::FONT_FACE },
    { Sid::longInstrumentFontSize,             Pid::FONT_SIZE },
    { Sid::longInstrumentLineSpacing,          Pid::TEXT_LINE_SPACING },
    { Sid::longInstrumentFontSpatiumDependent, Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::longInstrumentFontStyle,            Pid::FONT_STYLE },
    { Sid::longInstrumentColor,                Pid::COLOR },
    { Sid::longInstrumentAlign,                Pid::ALIGN },
    { Sid::longInstrumentOffset,               Pid::OFFSET },
    { Sid::longInstrumentFrameType,            Pid::FRAME_TYPE },
    { Sid::longInstrumentFramePadding,         Pid::FRAME_PADDING },
    { Sid::longInstrumentFrameWidth,           Pid::FRAME_WIDTH },
    { Sid::longInstrumentFrameRound,           Pid::FRAME_ROUND },
    { Sid::longInstrumentFrameFgColor,         Pid::FRAME_FG_COLOR },
    { Sid::longInstrumentFrameBgColor,         Pid::FRAME_BG_COLOR },
} };

const TextStyle shortInstrumentTextStyle { {
    { Sid::shortInstrumentFontFace,             Pid::FONT_FACE },
    { Sid::shortInstrumentFontSize,             Pid::FONT_SIZE },
    { Sid::shortInstrumentLineSpacing,          Pid::TEXT_LINE_SPACING },
    { Sid::shortInstrumentFontSpatiumDependent, Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::shortInstrumentFontStyle,            Pid::FONT_STYLE },
    { Sid::shortInstrumentColor,                Pid::COLOR },
    { Sid::shortInstrumentAlign,                Pid::ALIGN },
    { Sid::shortInstrumentOffset,               Pid::OFFSET },
    { Sid::shortInstrumentFrameType,            Pid::FRAME_TYPE },
    { Sid::shortInstrumentFramePadding,         Pid::FRAME_PADDING },
    { Sid::shortInstrumentFrameWidth,           Pid::FRAME_WIDTH },
    { Sid::shortInstrumentFrameRound,           Pid::FRAME_ROUND },
    { Sid::shortInstrumentFrameFgColor,         Pid::FRAME_FG_COLOR },
    { Sid::shortInstrumentFrameBgColor,         Pid::FRAME_BG_COLOR },
} };

const TextStyle partInstrumentTextStyle { {
    { Sid::partInstrumentFontFace,             Pid::FONT_FACE },
    { Sid::partInstrumentFontSize,             Pid::FONT_SIZE },
    { Sid::partInstrumentLineSpacing,          Pid::TEXT_LINE_SPACING },
    { Sid::partInstrumentFontSpatiumDependent, Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::partInstrumentFontStyle,            Pid::FONT_STYLE },
    { Sid::partInstrumentColor,                Pid::COLOR },
    { Sid::partInstrumentAlign,                Pid::ALIGN },
    { Sid::partInstrumentOffset,               Pid::OFFSET },
    { Sid::partInstrumentFrameType,            Pid::FRAME_TYPE },
    { Sid::partInstrumentFramePadding,         Pid::FRAME_PADDING },
    { Sid::partInstrumentFrameWidth,           Pid::FRAME_WIDTH },
    { Sid::partInstrumentFrameRound,           Pid::FRAME_ROUND },
    { Sid::partInstrumentFrameFgColor,         Pid::FRAME_FG_COLOR },
    { Sid::partInstrumentFrameBgColor,         Pid::FRAME_BG_COLOR },
} };

const TextStyle dynamicsTextStyle { {
    { Sid::dynamicsFontFace,                   Pid::FONT_FACE },
    { Sid::dynamicsFontSize,                   Pid::FONT_SIZE },
    { Sid::dynamicsLineSpacing,                Pid::TEXT_LINE_SPACING },
    { Sid::dynamicsFontSpatiumDependent,       Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::dynamicsFontStyle,                  Pid::FONT_STYLE },
    { Sid::dynamicsColor,                      Pid::COLOR },
    { Sid::dynamicsAlign,                      Pid::ALIGN },
    { Sid::dynamicsPosBelow,                   Pid::OFFSET },
    { Sid::dynamicsFrameType,                  Pid::FRAME_TYPE },
    { Sid::dynamicsFramePadding,               Pid::FRAME_PADDING },
    { Sid::dynamicsFrameWidth,                 Pid::FRAME_WIDTH },
    { Sid::dynamicsFrameRound,                 Pid::FRAME_ROUND },
    { Sid::dynamicsFrameFgColor,               Pid::FRAME_FG_COLOR },
    { Sid::dynamicsFrameBgColor,               Pid::FRAME_BG_COLOR },
} };

const TextStyle expressionTextStyle { {
    { Sid::expressionFontFace,                 Pid::FONT_FACE },
    { Sid::expressionFontSize,                 Pid::FONT_SIZE },
    { Sid::expressionLineSpacing,              Pid::TEXT_LINE_SPACING },
    { Sid::expressionFontSpatiumDependent,     Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::expressionFontStyle,                Pid::FONT_STYLE },
    { Sid::expressionColor,                    Pid::COLOR },
    { Sid::expressionAlign,                    Pid::ALIGN },
    { Sid::expressionOffset,                   Pid::OFFSET },
    { Sid::expressionFrameType,                Pid::FRAME_TYPE },
    { Sid::expressionFramePadding,             Pid::FRAME_PADDING },
    { Sid::expressionFrameWidth,               Pid::FRAME_WIDTH },
    { Sid::expressionFrameRound,               Pid::FRAME_ROUND },
    { Sid::expressionFrameFgColor,             Pid::FRAME_FG_COLOR },
    { Sid::expressionFrameBgColor,             Pid::FRAME_BG_COLOR },
} };

const TextStyle tempoTextStyle { {
    { Sid::tempoFontFace,                      Pid::FONT_FACE },
    { Sid::tempoFontSize,                      Pid::FONT_SIZE },
    { Sid::tempoLineSpacing,                   Pid::TEXT_LINE_SPACING },
    { Sid::tempoFontSpatiumDependent,          Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::tempoFontStyle,                     Pid::FONT_STYLE },
    { Sid::tempoColor,                         Pid::COLOR },
    { Sid::tempoAlign,                         Pid::ALIGN },
    { Sid::tempoPosAbove,                      Pid::OFFSET },
    { Sid::tempoFrameType,                     Pid::FRAME_TYPE },
    { Sid::tempoFramePadding,                  Pid::FRAME_PADDING },
    { Sid::tempoFrameWidth,                    Pid::FRAME_WIDTH },
    { Sid::tempoFrameRound,                    Pid::FRAME_ROUND },
    { Sid::tempoFrameFgColor,                  Pid::FRAME_FG_COLOR },
    { Sid::tempoFrameBgColor,                  Pid::FRAME_BG_COLOR },
} };

const TextStyle metronomeTextStyle { {
    { Sid::metronomeFontFace,                  Pid::FONT_FACE },
    { Sid::metronomeFontSize,                  Pid::FONT_SIZE },
    { Sid::metronomeLineSpacing,               Pid::TEXT_LINE_SPACING },
    { Sid::metronomeFontSpatiumDependent,      Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::metronomeFontStyle,                 Pid::FONT_STYLE },
    { Sid::metronomeColor,                     Pid::COLOR },
    { Sid::metronomeAlign,                     Pid::ALIGN },
    { Sid::metronomeOffset,                    Pid::OFFSET },
    { Sid::metronomeFrameType,                 Pid::FRAME_TYPE },
    { Sid::metronomeFramePadding,              Pid::FRAME_PADDING },
    { Sid::metronomeFrameWidth,                Pid::FRAME_WIDTH },
    { Sid::metronomeFrameRound,                Pid::FRAME_ROUND },
    { Sid::metronomeFrameFgColor,              Pid::FRAME_FG_COLOR },
    { Sid::metronomeFrameBgColor,              Pid::FRAME_BG_COLOR },
} };

const TextStyle measureNumberTextStyle { {
    { Sid::measureNumberFontFace,              Pid::FONT_FACE },
    { Sid::measureNumberFontSize,              Pid::FONT_SIZE },
    { Sid::measureNumberLineSpacing,           Pid::TEXT_LINE_SPACING },
    { Sid::measureNumberFontSpatiumDependent,  Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::measureNumberFontStyle,             Pid::FONT_STYLE },
    { Sid::measureNumberColor,                 Pid::COLOR },
    { Sid::measureNumberAlign,                 Pid::ALIGN },
    { Sid::measureNumberPosAbove,              Pid::OFFSET },
    { Sid::measureNumberFrameType,             Pid::FRAME_TYPE },
    { Sid::measureNumberFramePadding,          Pid::FRAME_PADDING },
    { Sid::measureNumberFrameWidth,            Pid::FRAME_WIDTH },
    { Sid::measureNumberFrameRound,            Pid::FRAME_ROUND },
    { Sid::measureNumberFrameFgColor,          Pid::FRAME_FG_COLOR },
    { Sid::measureNumberFrameBgColor,          Pid::FRAME_BG_COLOR },
} };

const TextStyle mmRestRangeTextStyle { {
    { Sid::mmRestRangeFontFace,              Pid::FONT_FACE },
    { Sid::mmRestRangeFontSize,              Pid::FONT_SIZE },
    { Sid::mmRestRangeFontSpatiumDependent,  Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::mmRestRangeFontStyle,             Pid::FONT_STYLE },
    { Sid::mmRestRangeColor,                 Pid::COLOR },
    { Sid::mmRestRangeAlign,                 Pid::ALIGN },
    { Sid::mmRestRangePosAbove,              Pid::OFFSET },
    { Sid::mmRestRangeFrameType,             Pid::FRAME_TYPE },
    { Sid::mmRestRangeFramePadding,          Pid::FRAME_PADDING },
    { Sid::mmRestRangeFrameWidth,            Pid::FRAME_WIDTH },
    { Sid::mmRestRangeFrameRound,            Pid::FRAME_ROUND },
    { Sid::mmRestRangeFrameFgColor,          Pid::FRAME_FG_COLOR },
    { Sid::mmRestRangeFrameBgColor,          Pid::FRAME_BG_COLOR },
} };

const TextStyle translatorTextStyle { {
    { Sid::translatorFontFace,                 Pid::FONT_FACE },
    { Sid::translatorFontSize,                 Pid::FONT_SIZE },
    { Sid::translatorLineSpacing,              Pid::TEXT_LINE_SPACING },
    { Sid::translatorFontSpatiumDependent,     Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::translatorFontStyle,                Pid::FONT_STYLE },
    { Sid::translatorColor,                    Pid::COLOR },
    { Sid::translatorAlign,                    Pid::ALIGN },
    { Sid::translatorOffset,                   Pid::OFFSET },
    { Sid::translatorFrameType,                Pid::FRAME_TYPE },
    { Sid::translatorFramePadding,             Pid::FRAME_PADDING },
    { Sid::translatorFrameWidth,               Pid::FRAME_WIDTH },
    { Sid::translatorFrameRound,               Pid::FRAME_ROUND },
    { Sid::translatorFrameFgColor,             Pid::FRAME_FG_COLOR },
    { Sid::translatorFrameBgColor,             Pid::FRAME_BG_COLOR },
} };

const TextStyle tupletTextStyle { {
    { Sid::tupletFontFace,                     Pid::FONT_FACE },
    { Sid::tupletFontSize,                     Pid::FONT_SIZE },
    { Sid::tupletLineSpacing,                  Pid::TEXT_LINE_SPACING },
    { Sid::tupletFontSpatiumDependent,         Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::tupletFontStyle,                    Pid::FONT_STYLE },
    { Sid::tupletColor,                        Pid::COLOR },
    { Sid::tupletAlign,                        Pid::ALIGN },
    { Sid::tupletOffset,                       Pid::OFFSET },
    { Sid::tupletFrameType,                    Pid::FRAME_TYPE },
    { Sid::tupletFramePadding,                 Pid::FRAME_PADDING },
    { Sid::tupletFrameWidth,                   Pid::FRAME_WIDTH },
    { Sid::tupletFrameRound,                   Pid::FRAME_ROUND },
    { Sid::tupletFrameFgColor,                 Pid::FRAME_FG_COLOR },
    { Sid::tupletFrameBgColor,                 Pid::FRAME_BG_COLOR },
} };

const TextStyle systemTextStyle { {
    { Sid::systemTextFontFace,                 Pid::FONT_FACE },
    { Sid::systemTextFontSize,                 Pid::FONT_SIZE },
    { Sid::systemTextLineSpacing,              Pid::TEXT_LINE_SPACING },
    { Sid::systemTextFontSpatiumDependent,     Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::systemTextFontStyle,                Pid::FONT_STYLE },
    { Sid::systemTextColor,                    Pid::COLOR },
    { Sid::systemTextAlign,                    Pid::ALIGN },
    { Sid::systemTextPosAbove,                 Pid::OFFSET },
    { Sid::systemTextFrameType,                Pid::FRAME_TYPE },
    { Sid::systemTextFramePadding,             Pid::FRAME_PADDING },
    { Sid::systemTextFrameWidth,               Pid::FRAME_WIDTH },
    { Sid::systemTextFrameRound,               Pid::FRAME_ROUND },
    { Sid::systemTextFrameFgColor,             Pid::FRAME_FG_COLOR },
    { Sid::systemTextFrameBgColor,             Pid::FRAME_BG_COLOR },
} };

const TextStyle staffTextStyle { {
    { Sid::staffTextFontFace,                  Pid::FONT_FACE },
    { Sid::staffTextFontSize,                  Pid::FONT_SIZE },
    { Sid::staffTextLineSpacing,               Pid::TEXT_LINE_SPACING },
    { Sid::staffTextFontSpatiumDependent,      Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::staffTextFontStyle,                 Pid::FONT_STYLE },
    { Sid::staffTextColor,                     Pid::COLOR },
    { Sid::staffTextAlign,                     Pid::ALIGN },
    { Sid::staffTextPosAbove,                  Pid::OFFSET },
    { Sid::staffTextFrameType,                 Pid::FRAME_TYPE },
    { Sid::staffTextFramePadding,              Pid::FRAME_PADDING },
    { Sid::staffTextFrameWidth,                Pid::FRAME_WIDTH },
    { Sid::staffTextFrameRound,                Pid::FRAME_ROUND },
    { Sid::staffTextFrameFgColor,              Pid::FRAME_FG_COLOR },
    { Sid::staffTextFrameBgColor,              Pid::FRAME_BG_COLOR },
} };

const TextStyle chordSymbolTextStyleA { {
    { Sid::chordSymbolAFontFace,               Pid::FONT_FACE },
    { Sid::chordSymbolAFontSize,               Pid::FONT_SIZE },
    { Sid::chordSymbolALineSpacing,            Pid::TEXT_LINE_SPACING },
    { Sid::chordSymbolAFontSpatiumDependent,   Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::chordSymbolAFontStyle,              Pid::FONT_STYLE },
    { Sid::chordSymbolAColor,                  Pid::COLOR },
    { Sid::chordSymbolAAlign,                  Pid::ALIGN },
    { Sid::chordSymbolAPosAbove,               Pid::OFFSET },
    { Sid::chordSymbolAFrameType,              Pid::FRAME_TYPE },
    { Sid::chordSymbolAFramePadding,           Pid::FRAME_PADDING },
    { Sid::chordSymbolAFrameWidth,             Pid::FRAME_WIDTH },
    { Sid::chordSymbolAFrameRound,             Pid::FRAME_ROUND },
    { Sid::chordSymbolAFrameFgColor,           Pid::FRAME_FG_COLOR },
    { Sid::chordSymbolAFrameBgColor,           Pid::FRAME_BG_COLOR },
} };

const TextStyle chordSymbolTextStyleB { {
    { Sid::chordSymbolBFontFace,               Pid::FONT_FACE },
    { Sid::chordSymbolBFontSize,               Pid::FONT_SIZE },
    { Sid::chordSymbolBLineSpacing,            Pid::TEXT_LINE_SPACING },
    { Sid::chordSymbolBFontSpatiumDependent,   Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::chordSymbolBFontStyle,              Pid::FONT_STYLE },
    { Sid::chordSymbolBColor,                  Pid::COLOR },
    { Sid::chordSymbolBAlign,                  Pid::ALIGN },
    { Sid::chordSymbolBPosAbove,               Pid::OFFSET },
    { Sid::chordSymbolBFrameType,              Pid::FRAME_TYPE },
    { Sid::chordSymbolBFramePadding,           Pid::FRAME_PADDING },
    { Sid::chordSymbolBFrameWidth,             Pid::FRAME_WIDTH },
    { Sid::chordSymbolBFrameRound,             Pid::FRAME_ROUND },
    { Sid::chordSymbolBFrameFgColor,           Pid::FRAME_FG_COLOR },
    { Sid::chordSymbolBFrameBgColor,           Pid::FRAME_BG_COLOR },
} };

const TextStyle romanNumeralTextStyle { {
    { Sid::romanNumeralFontFace,               Pid::FONT_FACE },
    { Sid::romanNumeralFontSize,               Pid::FONT_SIZE },
    { Sid::romanNumeralLineSpacing,            Pid::TEXT_LINE_SPACING },
    { Sid::romanNumeralFontSpatiumDependent,   Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::romanNumeralFontStyle,              Pid::FONT_STYLE },
    { Sid::romanNumeralColor,                  Pid::COLOR },
    { Sid::romanNumeralAlign,                  Pid::ALIGN },
    { Sid::romanNumeralPosAbove,               Pid::OFFSET },
    { Sid::romanNumeralFrameType,              Pid::FRAME_TYPE },
    { Sid::romanNumeralFramePadding,           Pid::FRAME_PADDING },
    { Sid::romanNumeralFrameWidth,             Pid::FRAME_WIDTH },
    { Sid::romanNumeralFrameRound,             Pid::FRAME_ROUND },
    { Sid::romanNumeralFrameFgColor,           Pid::FRAME_FG_COLOR },
    { Sid::romanNumeralFrameBgColor,           Pid::FRAME_BG_COLOR },
} };

const TextStyle nashvilleNumberTextStyle { {
    { Sid::nashvilleNumberFontFace,               Pid::FONT_FACE },
    { Sid::nashvilleNumberFontSize,               Pid::FONT_SIZE },
    { Sid::nashvilleNumberLineSpacing,            Pid::TEXT_LINE_SPACING },
    { Sid::nashvilleNumberFontSpatiumDependent,   Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::nashvilleNumberFontStyle,              Pid::FONT_STYLE },
    { Sid::nashvilleNumberColor,                  Pid::COLOR },
    { Sid::nashvilleNumberAlign,                  Pid::ALIGN },
    { Sid::nashvilleNumberPosAbove,               Pid::OFFSET },
    { Sid::nashvilleNumberFrameType,              Pid::FRAME_TYPE },
    { Sid::nashvilleNumberFramePadding,           Pid::FRAME_PADDING },
    { Sid::nashvilleNumberFrameWidth,             Pid::FRAME_WIDTH },
    { Sid::nashvilleNumberFrameRound,             Pid::FRAME_ROUND },
    { Sid::nashvilleNumberFrameFgColor,           Pid::FRAME_FG_COLOR },
    { Sid::nashvilleNumberFrameBgColor,           Pid::FRAME_BG_COLOR },
} };

const TextStyle rehearsalMarkTextStyle { {
    { Sid::rehearsalMarkFontFace,              Pid::FONT_FACE },
    { Sid::rehearsalMarkFontSize,              Pid::FONT_SIZE },
    { Sid::rehearsalMarkLineSpacing,           Pid::TEXT_LINE_SPACING },
    { Sid::rehearsalMarkFontSpatiumDependent,  Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::rehearsalMarkFontStyle,             Pid::FONT_STYLE },
    { Sid::rehearsalMarkColor,                 Pid::COLOR },
    { Sid::rehearsalMarkAlign,                 Pid::ALIGN },
    { Sid::rehearsalMarkPosAbove,              Pid::OFFSET },
    { Sid::rehearsalMarkFrameType,             Pid::FRAME_TYPE },
    { Sid::rehearsalMarkFramePadding,          Pid::FRAME_PADDING },
    { Sid::rehearsalMarkFrameWidth,            Pid::FRAME_WIDTH },
    { Sid::rehearsalMarkFrameRound,            Pid::FRAME_ROUND },
    { Sid::rehearsalMarkFrameFgColor,          Pid::FRAME_FG_COLOR },
    { Sid::rehearsalMarkFrameBgColor,          Pid::FRAME_BG_COLOR },
} };

const TextStyle repeatLeftTextStyle { {
    { Sid::repeatLeftFontFace,                 Pid::FONT_FACE },
    { Sid::repeatLeftFontSize,                 Pid::FONT_SIZE },
    { Sid::repeatLeftLineSpacing,              Pid::TEXT_LINE_SPACING },
    { Sid::repeatLeftFontSpatiumDependent,     Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::repeatLeftFontStyle,                Pid::FONT_STYLE },
    { Sid::repeatLeftColor,                    Pid::COLOR },
    { Sid::repeatLeftAlign,                    Pid::ALIGN },
    { Sid::markerPosAbove,                     Pid::OFFSET },
    { Sid::repeatLeftFrameType,                Pid::FRAME_TYPE },
    { Sid::repeatLeftFramePadding,             Pid::FRAME_PADDING },
    { Sid::repeatLeftFrameWidth,               Pid::FRAME_WIDTH },
    { Sid::repeatLeftFrameRound,               Pid::FRAME_ROUND },
    { Sid::repeatLeftFrameFgColor,             Pid::FRAME_FG_COLOR },
    { Sid::repeatLeftFrameBgColor,             Pid::FRAME_BG_COLOR },
} };

const TextStyle repeatRightTextStyle { {
    { Sid::repeatRightFontFace,                Pid::FONT_FACE },
    { Sid::repeatRightFontSize,                Pid::FONT_SIZE },
    { Sid::repeatRightLineSpacing,             Pid::TEXT_LINE_SPACING },
    { Sid::repeatRightFontSpatiumDependent,    Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::repeatRightFontStyle,               Pid::FONT_STYLE },
    { Sid::repeatRightColor,                   Pid::COLOR },
    { Sid::repeatRightAlign,                   Pid::ALIGN },
    { Sid::jumpPosAbove,                       Pid::OFFSET },
    { Sid::repeatRightFrameType,               Pid::FRAME_TYPE },
    { Sid::repeatRightFramePadding,            Pid::FRAME_PADDING },
    { Sid::repeatRightFrameWidth,              Pid::FRAME_WIDTH },
    { Sid::repeatRightFrameRound,              Pid::FRAME_ROUND },
    { Sid::repeatRightFrameFgColor,            Pid::FRAME_FG_COLOR },
    { Sid::repeatRightFrameBgColor,            Pid::FRAME_BG_COLOR },
} };

const TextStyle frameTextStyle { {
    { Sid::frameFontFace,                      Pid::FONT_FACE },
    { Sid::frameFontSize,                      Pid::FONT_SIZE },
    { Sid::frameLineSpacing,                   Pid::TEXT_LINE_SPACING },
    { Sid::frameFontSpatiumDependent,          Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::frameFontStyle,                     Pid::FONT_STYLE },
    { Sid::frameColor,                         Pid::COLOR },
    { Sid::frameAlign,                         Pid::ALIGN },
    { Sid::frameOffset,                        Pid::OFFSET },
    { Sid::frameFrameType,                     Pid::FRAME_TYPE },
    { Sid::frameFramePadding,                  Pid::FRAME_PADDING },
    { Sid::frameFrameWidth,                    Pid::FRAME_WIDTH },
    { Sid::frameFrameRound,                    Pid::FRAME_ROUND },
    { Sid::frameFrameFgColor,                  Pid::FRAME_FG_COLOR },
    { Sid::frameFrameBgColor,                  Pid::FRAME_BG_COLOR },
} };

const TextStyle textLineTextStyle { {
    { Sid::textLineFontFace,                   Pid::BEGIN_FONT_FACE },
    { Sid::textLineFontSize,                   Pid::BEGIN_FONT_SIZE },
    { Sid::textLineLineSpacing,                Pid::TEXT_LINE_SPACING },
    { Sid::textLineFontSpatiumDependent,       Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::textLineFontStyle,                  Pid::BEGIN_FONT_STYLE },
    { Sid::textLineColor,                      Pid::COLOR },
    { Sid::textLineTextAlign,                  Pid::ALIGN },
    { Sid::textLinePosAbove,                   Pid::OFFSET },
    { Sid::textLineFrameType,                  Pid::FRAME_TYPE },
    { Sid::textLineFramePadding,               Pid::FRAME_PADDING },
    { Sid::textLineFrameWidth,                 Pid::FRAME_WIDTH },
    { Sid::textLineFrameRound,                 Pid::FRAME_ROUND },
    { Sid::textLineFrameFgColor,               Pid::FRAME_FG_COLOR },
    { Sid::textLineFrameBgColor,               Pid::FRAME_BG_COLOR },
} };

const TextStyle glissandoTextStyle { {
    { Sid::glissandoFontFace,                  Pid::FONT_FACE },
    { Sid::glissandoFontSize,                  Pid::FONT_SIZE },
    { Sid::glissandoLineSpacing,               Pid::TEXT_LINE_SPACING },
    { Sid::glissandoFontSpatiumDependent,      Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::glissandoFontStyle,                 Pid::FONT_STYLE },
    { Sid::glissandoColor,                     Pid::COLOR },
    { Sid::glissandoAlign,                     Pid::ALIGN },
    { Sid::glissandoOffset,                    Pid::OFFSET },
    { Sid::glissandoFrameType,                 Pid::FRAME_TYPE },
    { Sid::glissandoFramePadding,              Pid::FRAME_PADDING },
    { Sid::glissandoFrameWidth,                Pid::FRAME_WIDTH },
    { Sid::glissandoFrameRound,                Pid::FRAME_ROUND },
    { Sid::glissandoFrameFgColor,              Pid::FRAME_FG_COLOR },
    { Sid::glissandoFrameBgColor,              Pid::FRAME_BG_COLOR },
} };

const TextStyle ottavaTextStyle { {
    { Sid::ottavaFontFace,                     Pid::BEGIN_FONT_FACE },
    { Sid::ottavaFontSize,                     Pid::BEGIN_FONT_SIZE },
    { Sid::ottavaLineSpacing,                  Pid::TEXT_LINE_SPACING },
    { Sid::ottavaFontSpatiumDependent,         Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::ottavaFontStyle,                    Pid::BEGIN_FONT_STYLE },
    { Sid::ottavaColor,                        Pid::COLOR },
    { Sid::ottavaTextAlign,                    Pid::BEGIN_TEXT_ALIGN },
    { Sid::ottavaPosAbove,                     Pid::OFFSET },
    { Sid::ottavaFrameType,                    Pid::FRAME_TYPE },
    { Sid::ottavaFramePadding,                 Pid::FRAME_PADDING },
    { Sid::ottavaFrameWidth,                   Pid::FRAME_WIDTH },
    { Sid::ottavaFrameRound,                   Pid::FRAME_ROUND },
    { Sid::ottavaFrameFgColor,                 Pid::FRAME_FG_COLOR },
    { Sid::ottavaFrameBgColor,                 Pid::FRAME_BG_COLOR },
} };

const TextStyle voltaTextStyle { {
    { Sid::voltaFontFace,                      Pid::BEGIN_FONT_FACE },
    { Sid::voltaFontSize,                      Pid::BEGIN_FONT_SIZE },
    { Sid::voltaLineSpacing,                   Pid::TEXT_LINE_SPACING },
    { Sid::voltaFontSpatiumDependent,          Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::voltaFontStyle,                     Pid::BEGIN_FONT_STYLE },
    { Sid::voltaColor,                         Pid::COLOR },
    { Sid::voltaAlign,                         Pid::BEGIN_TEXT_ALIGN },
    { Sid::voltaOffset,                        Pid::BEGIN_TEXT_OFFSET },
    { Sid::voltaFrameType,                     Pid::FRAME_TYPE },
    { Sid::voltaFramePadding,                  Pid::FRAME_PADDING },
    { Sid::voltaFrameWidth,                    Pid::FRAME_WIDTH },
    { Sid::voltaFrameRound,                    Pid::FRAME_ROUND },
    { Sid::voltaFrameFgColor,                  Pid::FRAME_FG_COLOR },
    { Sid::voltaFrameBgColor,                  Pid::FRAME_BG_COLOR },
} };

const TextStyle pedalTextStyle { {
    { Sid::pedalFontFace,                      Pid::BEGIN_FONT_FACE },
    { Sid::pedalFontSize,                      Pid::BEGIN_FONT_SIZE },
    { Sid::pedalLineSpacing,                   Pid::TEXT_LINE_SPACING },
    { Sid::pedalFontSpatiumDependent,          Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::pedalFontStyle,                     Pid::BEGIN_FONT_STYLE },
    { Sid::pedalColor,                         Pid::COLOR },
    { Sid::pedalTextAlign,                     Pid::BEGIN_TEXT_ALIGN },
    { Sid::pedalPosAbove,                      Pid::BEGIN_TEXT_OFFSET },
    { Sid::pedalFrameType,                     Pid::FRAME_TYPE },
    { Sid::pedalFramePadding,                  Pid::FRAME_PADDING },
    { Sid::pedalFrameWidth,                    Pid::FRAME_WIDTH },
    { Sid::pedalFrameRound,                    Pid::FRAME_ROUND },
    { Sid::pedalFrameFgColor,                  Pid::FRAME_FG_COLOR },
    { Sid::pedalFrameBgColor,                  Pid::FRAME_BG_COLOR },
} };

const TextStyle letRingTextStyle { {
    { Sid::letRingFontFace,                    Pid::BEGIN_FONT_FACE },
    { Sid::letRingFontSize,                    Pid::BEGIN_FONT_SIZE },
    { Sid::letRingLineSpacing,                 Pid::TEXT_LINE_SPACING },
    { Sid::letRingFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::letRingFontStyle,                   Pid::BEGIN_FONT_STYLE },
    { Sid::letRingColor,                       Pid::COLOR },
    { Sid::letRingTextAlign,                   Pid::BEGIN_TEXT_ALIGN },
    { Sid::letRingPosAbove,                    Pid::BEGIN_TEXT_OFFSET },
    { Sid::letRingFrameType,                   Pid::FRAME_TYPE },
    { Sid::letRingFramePadding,                Pid::FRAME_PADDING },
    { Sid::letRingFrameWidth,                  Pid::FRAME_WIDTH },
    { Sid::letRingFrameRound,                  Pid::FRAME_ROUND },
    { Sid::letRingFrameFgColor,                Pid::FRAME_FG_COLOR },
    { Sid::letRingFrameBgColor,                Pid::FRAME_BG_COLOR },
} };

const TextStyle palmMuteTextStyle { {
    { Sid::palmMuteFontFace,                   Pid::BEGIN_FONT_FACE },
    { Sid::palmMuteFontSize,                   Pid::BEGIN_FONT_SIZE },
    { Sid::palmMuteLineSpacing,                Pid::TEXT_LINE_SPACING },
    { Sid::palmMuteFontSpatiumDependent,       Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::palmMuteFontStyle,                  Pid::BEGIN_FONT_STYLE },
    { Sid::palmMuteColor,                      Pid::COLOR },
    { Sid::palmMuteTextAlign,                  Pid::BEGIN_TEXT_ALIGN },
    { Sid::palmMutePosAbove,                   Pid::BEGIN_TEXT_OFFSET },
    { Sid::palmMuteFrameType,                  Pid::FRAME_TYPE },
    { Sid::palmMuteFramePadding,               Pid::FRAME_PADDING },
    { Sid::palmMuteFrameWidth,                 Pid::FRAME_WIDTH },
    { Sid::palmMuteFrameRound,                 Pid::FRAME_ROUND },
    { Sid::palmMuteFrameFgColor,               Pid::FRAME_FG_COLOR },
    { Sid::palmMuteFrameBgColor,               Pid::FRAME_BG_COLOR },
} };

const TextStyle hairpinTextStyle { {
    { Sid::hairpinFontFace,                    Pid::BEGIN_FONT_FACE },
    { Sid::hairpinFontSize,                    Pid::BEGIN_FONT_SIZE },
    { Sid::hairpinLineSpacing,                 Pid::TEXT_LINE_SPACING },
    { Sid::hairpinFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::hairpinFontStyle,                   Pid::BEGIN_FONT_STYLE },
    { Sid::hairpinColor,                       Pid::COLOR },
    { Sid::hairpinTextAlign,                   Pid::BEGIN_TEXT_ALIGN },
    { Sid::hairpinPosAbove,                    Pid::BEGIN_TEXT_OFFSET },
    { Sid::hairpinFrameType,                   Pid::FRAME_TYPE },
    { Sid::hairpinFramePadding,                Pid::FRAME_PADDING },
    { Sid::hairpinFrameWidth,                  Pid::FRAME_WIDTH },
    { Sid::hairpinFrameRound,                  Pid::FRAME_ROUND },
    { Sid::hairpinFrameFgColor,                Pid::FRAME_FG_COLOR },
    { Sid::hairpinFrameBgColor,                Pid::FRAME_BG_COLOR },
} };

const TextStyle bendTextStyle { {
    { Sid::bendFontFace,                       Pid::FONT_FACE },
    { Sid::bendFontSize,                       Pid::FONT_SIZE },
    { Sid::bendLineSpacing,                    Pid::TEXT_LINE_SPACING },
    { Sid::bendFontSpatiumDependent,           Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::bendFontStyle,                      Pid::FONT_STYLE },
    { Sid::bendColor,                          Pid::COLOR },
    { Sid::bendAlign,                          Pid::BEGIN_TEXT_ALIGN },
    { Sid::bendOffset,                         Pid::BEGIN_TEXT_OFFSET },
    { Sid::bendFrameType,                      Pid::FRAME_TYPE },
    { Sid::bendFramePadding,                   Pid::FRAME_PADDING },
    { Sid::bendFrameWidth,                     Pid::FRAME_WIDTH },
    { Sid::bendFrameRound,                     Pid::FRAME_ROUND },
    { Sid::bendFrameFgColor,                   Pid::FRAME_FG_COLOR },
    { Sid::bendFrameBgColor,                   Pid::FRAME_BG_COLOR },
} };

const TextStyle headerTextStyle { {
    { Sid::headerFontFace,                     Pid::FONT_FACE },
    { Sid::headerFontSize,                     Pid::FONT_SIZE },
    { Sid::headerLineSpacing,                  Pid::TEXT_LINE_SPACING },
    { Sid::headerFontSpatiumDependent,         Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::headerFontStyle,                    Pid::FONT_STYLE },
    { Sid::headerColor,                        Pid::COLOR },
    { Sid::headerAlign,                        Pid::ALIGN },
    { Sid::headerOffset,                       Pid::OFFSET },
    { Sid::headerFrameType,                    Pid::FRAME_TYPE },
    { Sid::headerFramePadding,                 Pid::FRAME_PADDING },
    { Sid::headerFrameWidth,                   Pid::FRAME_WIDTH },
    { Sid::headerFrameRound,                   Pid::FRAME_ROUND },
    { Sid::headerFrameFgColor,                 Pid::FRAME_FG_COLOR },
    { Sid::headerFrameBgColor,                 Pid::FRAME_BG_COLOR },
} };

const TextStyle footerTextStyle { {
    { Sid::footerFontFace,                     Pid::FONT_FACE },
    { Sid::footerFontSize,                     Pid::FONT_SIZE },
    { Sid::footerLineSpacing,                  Pid::TEXT_LINE_SPACING },
    { Sid::footerFontSpatiumDependent,         Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::footerFontStyle,                    Pid::FONT_STYLE },
    { Sid::footerColor,                        Pid::COLOR },
    { Sid::footerAlign,                        Pid::ALIGN },
    { Sid::footerOffset,                       Pid::OFFSET },
    { Sid::footerFrameType,                    Pid::FRAME_TYPE },
    { Sid::footerFramePadding,                 Pid::FRAME_PADDING },
    { Sid::footerFrameWidth,                   Pid::FRAME_WIDTH },
    { Sid::footerFrameRound,                   Pid::FRAME_ROUND },
    { Sid::footerFrameFgColor,                 Pid::FRAME_FG_COLOR },
    { Sid::footerFrameBgColor,                 Pid::FRAME_BG_COLOR },
} };

const TextStyle instrumentChangeTextStyle { {
    { Sid::instrumentChangeFontFace,             Pid::FONT_FACE },
    { Sid::instrumentChangeFontSize,             Pid::FONT_SIZE },
    { Sid::instrumentChangeLineSpacing,          Pid::TEXT_LINE_SPACING },
    { Sid::instrumentChangeFontSpatiumDependent, Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::instrumentChangeFontStyle,            Pid::FONT_STYLE },
    { Sid::instrumentChangeColor,                Pid::COLOR },
    { Sid::instrumentChangeAlign,                Pid::ALIGN },
    { Sid::instrumentChangeOffset,               Pid::OFFSET },
    { Sid::instrumentChangeFrameType,            Pid::FRAME_TYPE },
    { Sid::instrumentChangeFramePadding,         Pid::FRAME_PADDING },
    { Sid::instrumentChangeFrameWidth,           Pid::FRAME_WIDTH },
    { Sid::instrumentChangeFrameRound,           Pid::FRAME_ROUND },
    { Sid::instrumentChangeFrameFgColor,         Pid::FRAME_FG_COLOR },
    { Sid::instrumentChangeFrameBgColor,         Pid::FRAME_BG_COLOR },
} };

const TextStyle stickingTextStyle { {
    { Sid::stickingFontFace,                   Pid::FONT_FACE },
    { Sid::stickingFontSize,                   Pid::FONT_SIZE },
    { Sid::stickingLineSpacing,                Pid::TEXT_LINE_SPACING },
    { Sid::stickingFontSpatiumDependent,       Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::stickingFontStyle,                  Pid::FONT_STYLE },
    { Sid::stickingColor,                      Pid::COLOR },
    { Sid::stickingAlign,                      Pid::ALIGN },
    { Sid::stickingOffset,                     Pid::OFFSET },
    { Sid::stickingFrameType,                  Pid::FRAME_TYPE },
    { Sid::stickingFramePadding,               Pid::FRAME_PADDING },
    { Sid::stickingFrameWidth,                 Pid::FRAME_WIDTH },
    { Sid::stickingFrameRound,                 Pid::FRAME_ROUND },
    { Sid::stickingFrameFgColor,               Pid::FRAME_FG_COLOR },
    { Sid::stickingFrameBgColor,               Pid::FRAME_BG_COLOR },
} };

const TextStyle user1TextStyle { {
    { Sid::user1FontFace,                      Pid::FONT_FACE },
    { Sid::user1FontSize,                      Pid::FONT_SIZE },
    { Sid::user1LineSpacing,                   Pid::TEXT_LINE_SPACING },
    { Sid::user1FontSpatiumDependent,          Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::user1FontStyle,                     Pid::FONT_STYLE },
    { Sid::user1Color,                         Pid::COLOR },
    { Sid::user1Align,                         Pid::ALIGN },
    { Sid::user1Offset,                        Pid::OFFSET },
    { Sid::user1FrameType,                     Pid::FRAME_TYPE },
    { Sid::user1FramePadding,                  Pid::FRAME_PADDING },
    { Sid::user1FrameWidth,                    Pid::FRAME_WIDTH },
    { Sid::user1FrameRound,                    Pid::FRAME_ROUND },
    { Sid::user1FrameFgColor,                  Pid::FRAME_FG_COLOR },
    { Sid::user1FrameBgColor,                  Pid::FRAME_BG_COLOR },
} };

const TextStyle user2TextStyle { {
    { Sid::user2FontFace,                      Pid::FONT_FACE },
    { Sid::user2FontSize,                      Pid::FONT_SIZE },
    { Sid::user2LineSpacing,                   Pid::TEXT_LINE_SPACING },
    { Sid::user2FontSpatiumDependent,          Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::user2FontStyle,                     Pid::FONT_STYLE },
    { Sid::user2Color,                         Pid::COLOR },
    { Sid::user2Align,                         Pid::ALIGN },
    { Sid::user2Offset,                        Pid::OFFSET },
    { Sid::user2FrameType,                     Pid::FRAME_TYPE },
    { Sid::user2FramePadding,                  Pid::FRAME_PADDING },
    { Sid::user2FrameWidth,                    Pid::FRAME_WIDTH },
    { Sid::user2FrameRound,                    Pid::FRAME_ROUND },
    { Sid::user2FrameFgColor,                  Pid::FRAME_FG_COLOR },
    { Sid::user2FrameBgColor,                  Pid::FRAME_BG_COLOR },
} };

const TextStyle user3TextStyle { {
    { Sid::user3FontFace,                      Pid::FONT_FACE },
    { Sid::user3FontSize,                      Pid::FONT_SIZE },
    { Sid::user3LineSpacing,                   Pid::TEXT_LINE_SPACING },
    { Sid::user3FontSpatiumDependent,          Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::user3FontStyle,                     Pid::FONT_STYLE },
    { Sid::user3Color,                         Pid::COLOR },
    { Sid::user3Align,                         Pid::ALIGN },
    { Sid::user3Offset,                        Pid::OFFSET },
    { Sid::user3FrameType,                     Pid::FRAME_TYPE },
    { Sid::user3FramePadding,                  Pid::FRAME_PADDING },
    { Sid::user3FrameWidth,                    Pid::FRAME_WIDTH },
    { Sid::user3FrameRound,                    Pid::FRAME_ROUND },
    { Sid::user3FrameFgColor,                  Pid::FRAME_FG_COLOR },
    { Sid::user3FrameBgColor,                  Pid::FRAME_BG_COLOR },
} };

const TextStyle user4TextStyle { {
    { Sid::user4FontFace,                      Pid::FONT_FACE },
    { Sid::user4FontSize,                      Pid::FONT_SIZE },
    { Sid::user4LineSpacing,                   Pid::TEXT_LINE_SPACING },
    { Sid::user4FontSpatiumDependent,          Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::user4FontStyle,                     Pid::FONT_STYLE },
    { Sid::user4Color,                         Pid::COLOR },
    { Sid::user4Align,                         Pid::ALIGN },
    { Sid::user4Offset,                        Pid::OFFSET },
    { Sid::user4FrameType,                     Pid::FRAME_TYPE },
    { Sid::user4FramePadding,                  Pid::FRAME_PADDING },
    { Sid::user4FrameWidth,                    Pid::FRAME_WIDTH },
    { Sid::user4FrameRound,                    Pid::FRAME_ROUND },
    { Sid::user4FrameFgColor,                  Pid::FRAME_FG_COLOR },
    { Sid::user4FrameBgColor,                  Pid::FRAME_BG_COLOR },
} };

const TextStyle user5TextStyle { {
    { Sid::user5FontFace,                      Pid::FONT_FACE },
    { Sid::user5FontSize,                      Pid::FONT_SIZE },
    { Sid::user5LineSpacing,                   Pid::TEXT_LINE_SPACING },
    { Sid::user5FontSpatiumDependent,          Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::user5FontStyle,                     Pid::FONT_STYLE },
    { Sid::user5Color,                         Pid::COLOR },
    { Sid::user5Align,                         Pid::ALIGN },
    { Sid::user5Offset,                        Pid::OFFSET },
    { Sid::user5FrameType,                     Pid::FRAME_TYPE },
    { Sid::user5FramePadding,                  Pid::FRAME_PADDING },
    { Sid::user5FrameWidth,                    Pid::FRAME_WIDTH },
    { Sid::user5FrameRound,                    Pid::FRAME_ROUND },
    { Sid::user5FrameFgColor,                  Pid::FRAME_FG_COLOR },
    { Sid::user5FrameBgColor,                  Pid::FRAME_BG_COLOR },
} };

const TextStyle user6TextStyle { {
    { Sid::user6FontFace,                      Pid::FONT_FACE },
    { Sid::user6FontSize,                      Pid::FONT_SIZE },
    { Sid::user6LineSpacing,                   Pid::TEXT_LINE_SPACING },
    { Sid::user6FontSpatiumDependent,          Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::user6FontStyle,                     Pid::FONT_STYLE },
    { Sid::user6Color,                         Pid::COLOR },
    { Sid::user6Align,                         Pid::ALIGN },
    { Sid::user6Offset,                        Pid::OFFSET },
    { Sid::user6FrameType,                     Pid::FRAME_TYPE },
    { Sid::user6FramePadding,                  Pid::FRAME_PADDING },
    { Sid::user6FrameWidth,                    Pid::FRAME_WIDTH },
    { Sid::user6FrameRound,                    Pid::FRAME_ROUND },
    { Sid::user6FrameFgColor,                  Pid::FRAME_FG_COLOR },
    { Sid::user6FrameBgColor,                  Pid::FRAME_BG_COLOR },
} };

const TextStyle user7TextStyle { {
    { Sid::user7FontFace,                      Pid::FONT_FACE },
    { Sid::user7FontSize,                      Pid::FONT_SIZE },
    { Sid::user7LineSpacing,                   Pid::TEXT_LINE_SPACING },
    { Sid::user7FontSpatiumDependent,          Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::user7FontStyle,                     Pid::FONT_STYLE },
    { Sid::user7Color,                         Pid::COLOR },
    { Sid::user7Align,                         Pid::ALIGN },
    { Sid::user7Offset,                        Pid::OFFSET },
    { Sid::user7FrameType,                     Pid::FRAME_TYPE },
    { Sid::user7FramePadding,                  Pid::FRAME_PADDING },
    { Sid::user7FrameWidth,                    Pid::FRAME_WIDTH },
    { Sid::user7FrameRound,                    Pid::FRAME_ROUND },
    { Sid::user7FrameFgColor,                  Pid::FRAME_FG_COLOR },
    { Sid::user7FrameBgColor,                  Pid::FRAME_BG_COLOR },
} };

const TextStyle user8TextStyle { {
    { Sid::user8FontFace,                      Pid::FONT_FACE },
    { Sid::user8FontSize,                      Pid::FONT_SIZE },
    { Sid::user8LineSpacing,                   Pid::TEXT_LINE_SPACING },
    { Sid::user8FontSpatiumDependent,          Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::user8FontStyle,                     Pid::FONT_STYLE },
    { Sid::user8Color,                         Pid::COLOR },
    { Sid::user8Align,                         Pid::ALIGN },
    { Sid::user8Offset,                        Pid::OFFSET },
    { Sid::user8FrameType,                     Pid::FRAME_TYPE },
    { Sid::user8FramePadding,                  Pid::FRAME_PADDING },
    { Sid::user8FrameWidth,                    Pid::FRAME_WIDTH },
    { Sid::user8FrameRound,                    Pid::FRAME_ROUND },
    { Sid::user8FrameFgColor,                  Pid::FRAME_FG_COLOR },
    { Sid::user8FrameBgColor,                  Pid::FRAME_BG_COLOR },
} };

const TextStyle user9TextStyle { {
    { Sid::user9FontFace,                      Pid::FONT_FACE },
    { Sid::user9FontSize,                      Pid::FONT_SIZE },
    { Sid::user9LineSpacing,                   Pid::TEXT_LINE_SPACING },
    { Sid::user9FontSpatiumDependent,          Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::user9FontStyle,                     Pid::FONT_STYLE },
    { Sid::user9Color,                         Pid::COLOR },
    { Sid::user9Align,                         Pid::ALIGN },
    { Sid::user9Offset,                        Pid::OFFSET },
    { Sid::user9FrameType,                     Pid::FRAME_TYPE },
    { Sid::user9FramePadding,                  Pid::FRAME_PADDING },
    { Sid::user9FrameWidth,                    Pid::FRAME_WIDTH },
    { Sid::user9FrameRound,                    Pid::FRAME_ROUND },
    { Sid::user9FrameFgColor,                  Pid::FRAME_FG_COLOR },
    { Sid::user9FrameBgColor,                  Pid::FRAME_BG_COLOR },
} };

const TextStyle user10TextStyle { {
    { Sid::user10FontFace,                     Pid::FONT_FACE },
    { Sid::user10FontSize,                     Pid::FONT_SIZE },
    { Sid::user10LineSpacing,                  Pid::TEXT_LINE_SPACING },
    { Sid::user10FontSpatiumDependent,         Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::user10FontStyle,                    Pid::FONT_STYLE },
    { Sid::user10Color,                        Pid::COLOR },
    { Sid::user10Align,                        Pid::ALIGN },
    { Sid::user10Offset,                       Pid::OFFSET },
    { Sid::user10FrameType,                    Pid::FRAME_TYPE },
    { Sid::user10FramePadding,                 Pid::FRAME_PADDING },
    { Sid::user10FrameWidth,                   Pid::FRAME_WIDTH },
    { Sid::user10FrameRound,                   Pid::FRAME_ROUND },
    { Sid::user10FrameFgColor,                 Pid::FRAME_FG_COLOR },
    { Sid::user10FrameBgColor,                 Pid::FRAME_BG_COLOR },
} };

const TextStyle user11TextStyle { {
    { Sid::user11FontFace,                     Pid::FONT_FACE },
    { Sid::user11FontSize,                     Pid::FONT_SIZE },
    { Sid::user11LineSpacing,                  Pid::TEXT_LINE_SPACING },
    { Sid::user11FontSpatiumDependent,         Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::user11FontStyle,                    Pid::FONT_STYLE },
    { Sid::user11Color,                        Pid::COLOR },
    { Sid::user11Align,                        Pid::ALIGN },
    { Sid::user11Offset,                       Pid::OFFSET },
    { Sid::user11FrameType,                    Pid::FRAME_TYPE },
    { Sid::user11FramePadding,                 Pid::FRAME_PADDING },
    { Sid::user11FrameWidth,                   Pid::FRAME_WIDTH },
    { Sid::user11FrameRound,                   Pid::FRAME_ROUND },
    { Sid::user11FrameFgColor,                 Pid::FRAME_FG_COLOR },
    { Sid::user11FrameBgColor,                 Pid::FRAME_BG_COLOR },
} };

const TextStyle user12TextStyle { {
    { Sid::user12FontFace,                     Pid::FONT_FACE },
    { Sid::user12FontSize,                     Pid::FONT_SIZE },
    { Sid::user12LineSpacing,                  Pid::TEXT_LINE_SPACING },
    { Sid::user12FontSpatiumDependent,         Pid::SIZE_SPATIUM_DEPENDENT },
    { Sid::user12FontStyle,                    Pid::FONT_STYLE },
    { Sid::user12Color,                        Pid::COLOR },
    { Sid::user12Align,                        Pid::ALIGN },
    { Sid::user12Offset,                       Pid::OFFSET },
    { Sid::user12FrameType,                    Pid::FRAME_TYPE },
    { Sid::user12FramePadding,                 Pid::FRAME_PADDING },
    { Sid::user12FrameWidth,                   Pid::FRAME_WIDTH },
    { Sid::user12FrameRound,                   Pid::FRAME_ROUND },
    { Sid::user12FrameFgColor,                 Pid::FRAME_FG_COLOR },
    { Sid::user12FrameBgColor,                 Pid::FRAME_BG_COLOR },
} };

//---------------------------------------------------------
//   TextStyleName
//---------------------------------------------------------

struct TextStyleName {
    const char* name;
    const TextStyle* ts;
    Tid tid;
};

// Must be in sync with Tid enum (in types.h)

static constexpr std::array<TextStyleName, int(Tid::TEXT_STYLES)> textStyles { {
    { QT_TRANSLATE_NOOP("TextStyle", "Default"),                 &defaultTextStyle,           Tid::DEFAULT },
// Page-orientde styles
    { QT_TRANSLATE_NOOP("TextStyle", "Title"),                   &titleTextStyle,             Tid::TITLE },
    { QT_TRANSLATE_NOOP("TextStyle", "Subtitle"),                &subTitleTextStyle,          Tid::SUBTITLE },
    { QT_TRANSLATE_NOOP("TextStyle", "Composer"),                &composerTextStyle,          Tid::COMPOSER },
    { QT_TRANSLATE_NOOP("TextStyle", "Lyricist"),                &lyricistTextStyle,          Tid::POET },
    { QT_TRANSLATE_NOOP("TextStyle", "Translator"),              &translatorTextStyle,        Tid::TRANSLATOR },
    { QT_TRANSLATE_NOOP("TextStyle", "Frame"),                   &frameTextStyle,             Tid::FRAME },
    { QT_TRANSLATE_NOOP("TextStyle", "Instrument Name (Part)"),  &partInstrumentTextStyle,    Tid::INSTRUMENT_EXCERPT },
    { QT_TRANSLATE_NOOP("TextStyle", "Instrument Name (Long)"),  &longInstrumentTextStyle,    Tid::INSTRUMENT_LONG },
    { QT_TRANSLATE_NOOP("TextStyle", "Instrument Name (Short)"), &shortInstrumentTextStyle,   Tid::INSTRUMENT_SHORT },
    { QT_TRANSLATE_NOOP("TextStyle", "Instrument Change"),       &instrumentChangeTextStyle,  Tid::INSTRUMENT_CHANGE },
    { QT_TRANSLATE_NOOP("TextStyle", "Header"),                  &headerTextStyle,            Tid::HEADER },
    { QT_TRANSLATE_NOOP("TextStyle", "Footer"),                  &footerTextStyle,            Tid::FOOTER },
// Measure-oriented styles
    { QT_TRANSLATE_NOOP("TextStyle", "Measure Number"),          &measureNumberTextStyle,     Tid::MEASURE_NUMBER },
    { QT_TRANSLATE_NOOP("TextStyle", "Multimeasure Rest Range"), &mmRestRangeTextStyle,       Tid::MMREST_RANGE },
// Sytem-level styles
    { QT_TRANSLATE_NOOP("TextStyle", "Tempo"),                   &tempoTextStyle,             Tid::TEMPO },
    { QT_TRANSLATE_NOOP("TextStyle", "Metronome"),               &metronomeTextStyle,         Tid::METRONOME },
    { QT_TRANSLATE_NOOP("TextStyle", "Repeat Text Left"),        &repeatLeftTextStyle,        Tid::REPEAT_LEFT },
    { QT_TRANSLATE_NOOP("TextStyle", "Repeat Text Right"),       &repeatRightTextStyle,       Tid::REPEAT_RIGHT },
    { QT_TRANSLATE_NOOP("TextStyle", "Rehearsal Mark"),          &rehearsalMarkTextStyle,     Tid::REHEARSAL_MARK },
    { QT_TRANSLATE_NOOP("TextStyle", "System"),                  &systemTextStyle,            Tid::SYSTEM },
// Staff oriented styles
    { QT_TRANSLATE_NOOP("TextStyle", "Staff"),                   &staffTextStyle,             Tid::STAFF },
    { QT_TRANSLATE_NOOP("TextStyle", "Expression"),              &expressionTextStyle,        Tid::EXPRESSION },
    { QT_TRANSLATE_NOOP("TextStyle", "Dynamics"),                &dynamicsTextStyle,          Tid::DYNAMICS },
    { QT_TRANSLATE_NOOP("TextStyle", "Hairpin"),                 &hairpinTextStyle,           Tid::HAIRPIN },
    { QT_TRANSLATE_NOOP("TextStyle", "Lyrics Odd Lines"),        &lyricsOddTextStyle,         Tid::LYRICS_ODD },
    { QT_TRANSLATE_NOOP("TextStyle", "Lyrics Even Lines"),       &lyricsEvenTextStyle,        Tid::LYRICS_EVEN },
    { QT_TRANSLATE_NOOP("TextStyle", "Chord Symbol"),            &chordSymbolTextStyleA,      Tid::HARMONY_A },
    { QT_TRANSLATE_NOOP("TextStyle", "Chord Symbol (Alternate)"), &chordSymbolTextStyleB,      Tid::HARMONY_B },
    { QT_TRANSLATE_NOOP("TextStyle", "Roman Numeral Analysis"),  &romanNumeralTextStyle,      Tid::HARMONY_ROMAN },
    { QT_TRANSLATE_NOOP("TextStyle", "Nashville Number"),        &nashvilleNumberTextStyle,   Tid::HARMONY_NASHVILLE },
// Note oriented styles
    { QT_TRANSLATE_NOOP("TextStyle", "Tuplet"),                  &tupletTextStyle,            Tid::TUPLET },
    { QT_TRANSLATE_NOOP("TextStyle", "Sticking"),                &stickingTextStyle,          Tid::STICKING },
    { QT_TRANSLATE_NOOP("TextStyle", "Fingering"),               &fingeringTextStyle,         Tid::FINGERING },
    { QT_TRANSLATE_NOOP("TextStyle", "LH Guitar Fingering"),     &lhGuitarFingeringTextStyle, Tid::LH_GUITAR_FINGERING },
    { QT_TRANSLATE_NOOP("TextStyle", "RH Guitar Fingering"),     &rhGuitarFingeringTextStyle, Tid::RH_GUITAR_FINGERING },
    { QT_TRANSLATE_NOOP("TextStyle", "String Number"),           &stringNumberTextStyle,      Tid::STRING_NUMBER },
// Line-oriented styles
    { QT_TRANSLATE_NOOP("TextStyle", "Text Line"),               &textLineTextStyle,          Tid::TEXTLINE },
    { QT_TRANSLATE_NOOP("TextStyle", "Volta"),                   &voltaTextStyle,             Tid::VOLTA },
    { QT_TRANSLATE_NOOP("TextStyle", "Ottava"),                  &ottavaTextStyle,            Tid::OTTAVA },
    { QT_TRANSLATE_NOOP("TextStyle", "Glissando"),               &glissandoTextStyle,         Tid::GLISSANDO },
    { QT_TRANSLATE_NOOP("TextStyle", "Pedal"),                   &pedalTextStyle,             Tid::PEDAL },
    { QT_TRANSLATE_NOOP("TextStyle", "Bend"),                    &bendTextStyle,              Tid::BEND },
    { QT_TRANSLATE_NOOP("TextStyle", "Let Ring"),                &letRingTextStyle,           Tid::LET_RING },
    { QT_TRANSLATE_NOOP("TextStyle", "Palm Mute"),               &palmMuteTextStyle,          Tid::PALM_MUTE },
// User styles
    { QT_TRANSLATE_NOOP("TextStyle", "User-1"),                  &user1TextStyle,             Tid::USER1 },
    { QT_TRANSLATE_NOOP("TextStyle", "User-2"),                  &user2TextStyle,             Tid::USER2 },
    { QT_TRANSLATE_NOOP("TextStyle", "User-3"),                  &user3TextStyle,             Tid::USER3 },
    { QT_TRANSLATE_NOOP("TextStyle", "User-4"),                  &user4TextStyle,             Tid::USER4 },
    { QT_TRANSLATE_NOOP("TextStyle", "User-5"),                  &user5TextStyle,             Tid::USER5 },
    { QT_TRANSLATE_NOOP("TextStyle", "User-6"),                  &user6TextStyle,             Tid::USER6 },
    { QT_TRANSLATE_NOOP("TextStyle", "User-7"),                  &user7TextStyle,             Tid::USER7 },
    { QT_TRANSLATE_NOOP("TextStyle", "User-8"),                  &user8TextStyle,             Tid::USER8 },
    { QT_TRANSLATE_NOOP("TextStyle", "User-9"),                  &user9TextStyle,             Tid::USER9 },
    { QT_TRANSLATE_NOOP("TextStyle", "User-10"),                 &user10TextStyle,            Tid::USER10 },
    { QT_TRANSLATE_NOOP("TextStyle", "User-11"),                 &user11TextStyle,            Tid::USER11 },
    { QT_TRANSLATE_NOOP("TextStyle", "User-12"),                 &user12TextStyle,            Tid::USER12 },
} };

//---------------------------------------------------------
//   textStyle
//---------------------------------------------------------

const TextStyle* textStyle(const char* name)
{
    for (const auto& s : textStyles) {
        if (strcmp(s.name, name) == 0) {
            return s.ts;
        }
    }
    qDebug("textStyle <%s> not known", name);
    return textStyles[0].ts;
}

const TextStyle* textStyle(Tid idx)
{
    Q_ASSERT(idx == textStyles[int(idx)].tid);
    return textStyles[int(idx)].ts;
}

//---------------------------------------------------------
//   TextStyleFromName
//---------------------------------------------------------

Tid textStyleFromName(const QString& name)
{
    for (const auto& s : textStyles) {
        if (s.name == name) {
            return s.tid;
        }
    }
    if (name == "Technique") {                  // compatibility
        return Tid::EXPRESSION;
    }

    qWarning("text style <%s> not known", qPrintable(name));
    return Tid::DEFAULT;
}

//---------------------------------------------------------
//   textStyleName
//---------------------------------------------------------

const char* textStyleName(Tid idx)
{
    Q_ASSERT(idx == textStyles[int(idx)].tid);
    return textStyles[int(idx)].name;
}

//---------------------------------------------------------
//   textStyleUserName
//---------------------------------------------------------

QString textStyleUserName(Tid idx)
{
    Q_ASSERT(idx == textStyles[int(idx)].tid);
    return qtrc("TextStyle", textStyleName(idx));
}

static std::vector<Tid> _allTextStyles;

static const std::vector<Tid> _primaryTextStyles = {
    Tid::TITLE,
    Tid::SUBTITLE,
    Tid::COMPOSER,
    Tid::POET,
    Tid::TRANSLATOR,
    Tid::FRAME,
    Tid::INSTRUMENT_EXCERPT,
    Tid::INSTRUMENT_CHANGE,
    Tid::HEADER,
    Tid::FOOTER,
    Tid::MEASURE_NUMBER,
    Tid::MMREST_RANGE,
    Tid::TEMPO,
    Tid::REPEAT_LEFT,
    Tid::REPEAT_RIGHT,
    Tid::REHEARSAL_MARK,
    Tid::SYSTEM,
    Tid::STAFF,
    Tid::EXPRESSION,
    Tid::DYNAMICS,
    Tid::HAIRPIN,
    Tid::LYRICS_ODD,
    Tid::LYRICS_EVEN,
    Tid::HARMONY_A,
    Tid::HARMONY_B,
    Tid::HARMONY_ROMAN,
    Tid::HARMONY_NASHVILLE,
    Tid::STICKING,
    Tid::USER1,
    Tid::USER2,
    Tid::USER3,
    Tid::USER4,
    Tid::USER5,
    Tid::USER6,
    Tid::USER7,
    Tid::USER8,
    Tid::USER9,
    Tid::USER10,
    Tid::USER11,
    Tid::USER12,
};

//---------------------------------------------------------
//   allTextStyles
//---------------------------------------------------------

const std::vector<Tid>& allTextStyles()
{
    if (_allTextStyles.empty()) {
        _allTextStyles.reserve(int(Tid::TEXT_STYLES));
        for (const auto& s : textStyles) {
            if (s.tid == Tid::DEFAULT) {
                continue;
            }
            _allTextStyles.push_back(s.tid);
        }
    }
    return _allTextStyles;
}

//---------------------------------------------------------
//   primaryTextStyles
//---------------------------------------------------------

const std::vector<Tid>& primaryTextStyles()
{
    return _primaryTextStyles;
}

QSet<Sid> pageStyles()
{
    static const QSet<Sid> styles {
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
        Sid::spatium
    };

    return styles;
}

//---------------------------------------------------------
//   valueType
//---------------------------------------------------------

const char* MStyle::valueType(const Sid i)
{
    return StyleDef::styleTypes[int(i)].valueType();
}

//---------------------------------------------------------
//   value
//---------------------------------------------------------

const QVariant& MStyle::value(Sid idx) const
{
    const QVariant& val = _values[int(idx)];
    if (!val.isValid()) {
        qDebug("invalid style value %d %s", int(idx), MStyle::valueName(idx));
        static QVariant emptyVal;
        return emptyVal;
    }
    return val;
}

//---------------------------------------------------------
//   valueName
//---------------------------------------------------------

const char* MStyle::valueName(const Sid i)
{
    if (i == Sid::NOSTYLE) {
        return "no style";
    }
    return StyleDef::styleTypes[int(i)].name();
}

//---------------------------------------------------------
//   styleIdx
//---------------------------------------------------------

Sid MStyle::styleIdx(const QString& name)
{
    for (StyleDef::StyleType st : StyleDef::styleTypes) {
        if (st.name() == name) {
            return st.styleIdx();
        }
    }
    return Sid::NOSTYLE;
}

MStyle* MStyle::resolveStyleDefaults(const int defaultsVersion)
{
    switch (defaultsVersion) {
    case LEGACY_MSC_VERSION_V3:
        return styleDefaults301();
    case LEGACY_MSC_VERSION_V2:
        return styleDefaults206();
    case LEGACY_MSC_VERSION_V1:
        return styleDefaults114();
    default:
        return &MScore::baseStyle();
    }
}

//---------------------------------------------------------
//   Style
//---------------------------------------------------------

MStyle::MStyle()
{
    _defaultStyleVersion = MSCVERSION;
    _customChordList = false;
    for (const StyleDef::StyleType& t : StyleDef::styleTypes) {
        _values[t.idx()] = t.defaultValue();
    }
}

//---------------------------------------------------------
//   precomputeValues
//---------------------------------------------------------

void MStyle::precomputeValues()
{
    qreal _spatium = value(Sid::spatium).toDouble();
    for (const StyleDef::StyleType& t : StyleDef::styleTypes) {
        if (!strcmp(t.valueType(), "Ms::Spatium")) {
            _precomputedValues[t.idx()] = _values[t.idx()].value<Spatium>().val() * _spatium;
        }
    }
}

//---------------------------------------------------------
//   isDefault
//    caution: custom types need to register comparison operator
//          to make this work
//---------------------------------------------------------

bool MStyle::isDefault(Sid idx) const
{
    return value(idx) == resolveStyleDefaults(_defaultStyleVersion)->value(idx);
}

void MStyle::setDefaultStyleVersion(const int defaultsVersion)
{
    _defaultStyleVersion = defaultsVersion;
}

//---------------------------------------------------------
//   chordDescription
//---------------------------------------------------------

const ChordDescription* MStyle::chordDescription(int id) const
{
    if (!_chordList.contains(id)) {
        return 0;
    }
    return &*_chordList.find(id);
}

//---------------------------------------------------------
//   checkChordList
//---------------------------------------------------------

void MStyle::checkChordList()
{
    // make sure we have a chordlist
    if (!_chordList.loaded()) {
        qreal emag = value(Sid::chordExtensionMag).toDouble();
        qreal eadjust = value(Sid::chordExtensionAdjust).toDouble();
        qreal mmag = value(Sid::chordModifierMag).toDouble();
        qreal madjust = value(Sid::chordModifierAdjust).toDouble();
        _chordList.configureAutoAdjust(emag, eadjust, mmag, madjust);
        if (value(Sid::chordsXmlFile).toBool()) {
            _chordList.read("chords.xml");
        }
        _chordList.read(value(Sid::chordDescriptionFile).toString());
    }
}

//---------------------------------------------------------
//   setChordList
//---------------------------------------------------------

void MStyle::setChordList(ChordList* cl, bool custom)
{
    _chordList       = *cl;
    _customChordList = custom;
}

//---------------------------------------------------------
//   set
//---------------------------------------------------------
void MStyle::set(Sid idx, const mu::PointF& v)
{
    set(idx, QVariant::fromValue(v));
}

void MStyle::set(const Sid t, const QVariant& val)
{
    const int idx = int(t);
    _values[idx] = val;
    if (t == Sid::spatium) {
        precomputeValues();
    } else {
        if (!strcmp(StyleDef::styleTypes[idx].valueType(), "Ms::Spatium")) {
            qreal _spatium = value(Sid::spatium).toDouble();
            _precomputedValues[idx] = _values[idx].value<Spatium>().val() * _spatium;
        }
    }
}

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool MStyle::readProperties(XmlReader& e)
{
    const QStringRef& tag(e.name());

    for (const StyleDef::StyleType& t : StyleDef::styleTypes) {
        Sid idx = t.styleIdx();
        if (t.name() == tag) {
            const char* type = t.valueType();
            if (!strcmp("Ms::Spatium", type)) {
                set(idx, Spatium(e.readElementText().toDouble()));
            } else if (!strcmp("double", type)) {
                set(idx, QVariant(e.readElementText().toDouble()));
            } else if (!strcmp("bool", type)) {
                set(idx, QVariant(bool(e.readElementText().toInt())));
            } else if (!strcmp("int", type)) {
                set(idx, QVariant(e.readElementText().toInt()));
            } else if (!strcmp("Ms::Direction", type)) {
                set(idx, QVariant::fromValue(Direction(e.readElementText().toInt())));
            } else if (!strcmp("QString", type)) {
                set(idx, QVariant(e.readElementText()));
            } else if (!strcmp("Ms::Align", type)) {
                QStringList sl = e.readElementText().split(',');
                if (sl.size() != 2) {
                    qDebug("bad align text <%s>", qPrintable(e.readElementText()));
                    return true;
                }
                Align align = Align::LEFT;
                if (sl[0] == "center") {
                    align = align | Align::HCENTER;
                } else if (sl[0] == "right") {
                    align = align | Align::RIGHT;
                } else if (sl[0] == "left") {
                } else {
                    qDebug("bad align text <%s>", qPrintable(sl[0]));
                    return true;
                }
                if (sl[1] == "center") {
                    align = align | Align::VCENTER;
                } else if (sl[1] == "bottom") {
                    align = align | Align::BOTTOM;
                } else if (sl[1] == "baseline") {
                    align = align | Align::BASELINE;
                } else if (sl[1] == "top") {
                } else {
                    qDebug("bad align text <%s>", qPrintable(sl[1]));
                    return true;
                }
                set(idx, QVariant::fromValue(align));
            } else if (!strcmp("QPointF", type)) {
                qreal x = e.doubleAttribute("x", 0.0);
                qreal y = e.doubleAttribute("y", 0.0);
                set(idx, PointF(x, y));
                e.readElementText();
            } else if (!strcmp("mu::PointF", type)) {
                qreal x = e.doubleAttribute("x", 0.0);
                qreal y = e.doubleAttribute("y", 0.0);
                set(idx, PointF(x, y));
                e.readElementText();
            } else if (!strcmp("QSizeF", type)) {
                qreal x = e.doubleAttribute("w", 0.0);
                qreal y = e.doubleAttribute("h", 0.0);
                set(idx, SizeF(x, y));
                e.readElementText();
            } else if (!strcmp("mu::SizeF", type)) {
                qreal x = e.doubleAttribute("w", 0.0);
                qreal y = e.doubleAttribute("h", 0.0);
                set(idx, SizeF(x, y));
                e.readElementText();
            } else if (!strcmp("QColor", type)) {
                QColor c;
                c.setRed(e.intAttribute("r"));
                c.setGreen(e.intAttribute("g"));
                c.setBlue(e.intAttribute("b"));
                c.setAlpha(e.intAttribute("a", 255));
                set(idx, c);
                e.readElementText();
            } else {
                qFatal("unhandled type %s", type);
            }
            return true;
        }
    }
    if (readStyleValCompat(e)) {
        return true;
    }
    return false;
}

//---------------------------------------------------------
//   readStyleValCompat
//    Read obsolete style values which may appear in files
//    produced by older versions of MuseScore.
//---------------------------------------------------------

bool MStyle::readStyleValCompat(XmlReader& e)
{
    const QStringRef tag(e.name());
    if (tag == "tempoOffset") {   // pre-3.0-beta
        const qreal x = e.doubleAttribute("x", 0.0);
        const qreal y = e.doubleAttribute("y", 0.0);
        const PointF val(x, y);
        set(Sid::tempoPosAbove, val);
        set(Sid::tempoPosBelow, val);
        e.readElementText();
        return true;
    }
    if (readTextStyleValCompat(e)) {
        return true;
    }
    return false;
}

//---------------------------------------------------------
//   readTextStyleValCompat
//    Handle transition from separate bold, underline and
//    italic style properties to the single *FontStyle
//    property set.
//---------------------------------------------------------

bool MStyle::readTextStyleValCompat(XmlReader& e)
{
    static const std::array<std::pair<const char*, FontStyle>, 3> styleNamesEndings { {
        { "FontBold",      FontStyle::Bold },
        { "FontItalic",    FontStyle::Italic },
        { "FontUnderline", FontStyle::Underline }
    } };

    const QStringRef tag(e.name());
    FontStyle readFontStyle = FontStyle::Normal;
    QStringRef typeName;
    for (auto& fontStyle : styleNamesEndings) {
        if (tag.endsWith(fontStyle.first)) {
            readFontStyle = fontStyle.second;
            typeName = tag.mid(0, tag.length() - int(strlen(fontStyle.first)));
            break;
        }
    }
    if (readFontStyle == FontStyle::Normal) {
        return false;
    }

    const QString newFontStyleName = typeName.toString() + "FontStyle";
    const Sid sid = MStyle::styleIdx(newFontStyleName);
    if (sid == Sid::NOSTYLE) {
        qWarning() << "readFontStyleValCompat: couldn't read text readFontStyle value:" << tag;
        return false;
    }

    const bool readVal = bool(e.readElementText().toInt());
    const QVariant val = value(sid);
    FontStyle newFontStyle = (val == QVariant()) ? FontStyle::Normal : FontStyle(val.toInt());
    if (readVal) {
        newFontStyle = newFontStyle + readFontStyle;
    } else {
        newFontStyle = newFontStyle - readFontStyle;
    }

    set(sid, int(newFontStyle));
    return true;
}

//---------------------------------------------------------
//   load
//---------------------------------------------------------

bool MStyle::load(QFile* qf, bool ign)
{
    XmlReader e(qf);
    while (e.readNextStartElement()) {
        if (e.name() == "museScore") {
            QString version = e.attribute("version");
            QStringList sl  = version.split('.');
            int mscVersion  = sl[0].toInt() * 100 + sl[1].toInt();
            if (mscVersion != MSCVERSION && !ign) {
                return false;
            }
            while (e.readNextStartElement()) {
                if (e.name() == "Style") {
                    load(e);
                } else {
                    e.unknown();
                }
            }
        }
    }
    return true;
}

extern void readPageFormat(MStyle* style, XmlReader& e);

void MStyle::load(XmlReader& e)
{
    QString oldChordDescriptionFile = value(Sid::chordDescriptionFile).toString();
    bool chordListTag = false;
    while (e.readNextStartElement()) {
        const QStringRef& tag(e.name());

        if (tag == "TextStyle") {
            //readTextStyle206(this, e);        // obsolete
            e.readElementText();
        } else if (tag == "ottavaHook") {             // obsolete, for 3.0dev bw. compatibility, should be removed in final release
            qreal y = qAbs(e.readDouble());
            set(Sid::ottavaHookAbove, y);
            set(Sid::ottavaHookBelow, -y);
        } else if (tag == "Spatium") {
            set(Sid::spatium, e.readDouble() * DPMM);
        } else if (tag == "page-layout") {      // obsolete
            readPageFormat(this, e);            // from read206.cpp
        } else if (tag == "displayInConcertPitch") {
            set(Sid::concertPitch, QVariant(bool(e.readInt())));
        } else if (tag == "ChordList") {
            _chordList.unload();
            _chordList.read(e);
            _customChordList = true;
            chordListTag = true;
        } else if (tag == "lyricsDashMaxLegth") { // pre-3.6 typo
            set(Sid::lyricsDashMaxLength, e.readDouble());
        } else if (tag == "dontHidStavesInFirstSystm") { // pre-3.6.3/4.0 typo
            set(Sid::dontHideStavesInFirstSystem, e.readBool());
        } else if (!readProperties(e)) {
            e.unknown();
        }
    }

    // if we just specified a new chord description file
    // and didn't encounter a ChordList tag
    // then load the chord description file

    QString newChordDescriptionFile = value(Sid::chordDescriptionFile).toString();
    if (newChordDescriptionFile != oldChordDescriptionFile && !chordListTag) {
        if (!newChordDescriptionFile.startsWith("chords_") && value(Sid::chordStyle).toString() == "std") {
            // should not normally happen,
            // but treat as "old" (114) score just in case
            set(Sid::chordStyle, QVariant(QString("custom")));
            set(Sid::chordsXmlFile, QVariant(true));
            qDebug("StyleData::load: custom chord description file %s with chordStyle == std", qPrintable(newChordDescriptionFile));
        }
        if (value(Sid::chordStyle).toString() == "custom") {
            _customChordList = true;
        } else {
            _customChordList = false;
        }
        _chordList.unload();
    }

    if (!chordListTag) {
        checkChordList();
    }
}

void MStyle::applyNewDefaults(const MStyle& other, const int defaultsVersion)
{
    _defaultStyleVersion = defaultsVersion;

    for (StyleDef::StyleType st : StyleDef::styleTypes) {
        if (isDefault(st.styleIdx())) {
            st._defaultValue = other.value(st.styleIdx());
            _values.at(st.idx()) = other.value(st.styleIdx());
        }
    }
}

//---------------------------------------------------------
//   save
//---------------------------------------------------------

void MStyle::save(XmlWriter& xml, bool optimize)
{
    xml.stag("Style");

    for (const StyleDef::StyleType& st : StyleDef::styleTypes) {
        Sid idx = st.styleIdx();
        if (idx == Sid::spatium) {         // special handling for spatium
            continue;
        }
        if (optimize && isDefault(idx)) {
            continue;
        }
        const char* type = st.valueType();
        if (!strcmp("Ms::Spatium", type)) {
            xml.tag(st.name(), value(idx).value<Spatium>().val());
        } else if (!strcmp("Ms::Direction", type)) {
            xml.tag(st.name(), value(idx).toInt());
        } else if (!strcmp("Ms::Align", type)) {
            Align a = Align(value(idx).toInt());
            // Don't write if it's the default value
            if (optimize && a == Align(st.defaultValue().toInt())) {
                continue;
            }
            QString horizontal = "left";
            QString vertical = "top";
            if (a & Align::HCENTER) {
                horizontal = "center";
            } else if (a & Align::RIGHT) {
                horizontal = "right";
            }

            if (a & Align::VCENTER) {
                vertical = "center";
            } else if (a & Align::BOTTOM) {
                vertical = "bottom";
            } else if (a & Align::BASELINE) {
                vertical = "baseline";
            }

            xml.tag(st.name(), horizontal + "," + vertical);
        } else {
            xml.tag(st.name(), value(idx));
        }
    }
    if (_customChordList && !_chordList.empty()) {
        xml.stag("ChordList");
        _chordList.write(xml);
        xml.etag();
    }
    xml.tag("Spatium", value(Sid::spatium).toDouble() / DPMM);
    xml.etag();
}

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void MStyle::resetAllStyles(Score* score, const QSet<Sid>& ignoredStyles)
{
    for (const StyleDef::StyleType& st : StyleDef::styleTypes) {
        if (ignoredStyles.isEmpty() || !ignoredStyles.contains(st.styleIdx())) {
            score->undo(new ChangeStyleVal(score, st.styleIdx(), MScore::defaultStyle().value(st.styleIdx())));
        }
    }
}

void MStyle::resetStyles(Score* score, const QSet<Sid>& stylesToReset)
{
    for (const StyleDef::StyleType& st : StyleDef::styleTypes) {
        if (stylesToReset.contains(st.styleIdx())) {
            score->undo(new ChangeStyleVal(score, st.styleIdx(), MScore::defaultStyle().value(st.styleIdx())));
        }
    }
}
}
