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
#include "textstyle.h"

#include "dom/property.h"

#include "log.h"

using namespace mu;
namespace mu::engraving {
const TextStyle defaultTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::defaultFontFace,                        Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::defaultFontSize,                        Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::defaultLineSpacing,                     Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::defaultFontSpatiumDependent,            Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::defaultFontStyle,                       Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::defaultColor,                           Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::defaultAlign,                           Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::defaultOffset,                          Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::defaultFrameType,                       Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::defaultFramePadding,                    Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::defaultFrameWidth,                      Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::defaultFrameRound,                      Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::defaultFrameFgColor,                    Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::defaultFrameBgColor,                    Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle titleTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::titleFontFace,                          Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::titleFontSize,                          Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::titleLineSpacing,                       Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::titleFontSpatiumDependent,              Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::titleFontStyle,                         Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::titleColor,                             Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::titleAlign,                             Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::titleOffset,                            Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::titleFrameType,                         Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::titleFramePadding,                      Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::titleFrameWidth,                        Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::titleFrameRound,                        Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::titleFrameFgColor,                      Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::titleFrameBgColor,                      Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle subTitleTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::subTitleFontFace,                       Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::subTitleFontSize,                       Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::subTitleLineSpacing,                    Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::subTitleFontSpatiumDependent,           Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::subTitleFontStyle,                      Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::subTitleColor,                          Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::subTitleAlign,                          Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::subTitleOffset,                         Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::subTitleFrameType,                      Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::subTitleFramePadding,                   Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::subTitleFrameWidth,                     Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::subTitleFrameRound,                     Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::subTitleFrameFgColor,                   Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::subTitleFrameBgColor,                   Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle composerTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::composerFontFace,                       Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::composerFontSize,                       Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::composerLineSpacing,                    Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::composerFontSpatiumDependent,           Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::composerFontStyle,                      Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::composerColor,                          Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::composerAlign,                          Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::composerOffset,                         Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::composerFrameType,                      Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::composerFramePadding,                   Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::composerFrameWidth,                     Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::composerFrameRound,                     Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::composerFrameFgColor,                   Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::composerFrameBgColor,                   Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle lyricistTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::lyricistFontFace,                       Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::lyricistFontSize,                       Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::lyricistLineSpacing,                    Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::lyricistFontSpatiumDependent,           Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::lyricistFontStyle,                      Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::lyricistColor,                          Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::lyricistAlign,                          Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::lyricistOffset,                         Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::lyricistFrameType,                      Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::lyricistFramePadding,                   Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::lyricistFrameWidth,                     Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::lyricistFrameRound,                     Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::lyricistFrameFgColor,                   Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::lyricistFrameBgColor,                   Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle lyricsEvenTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::lyricsEvenFontFace,                     Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::lyricsEvenFontSize,                     Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::lyricsEvenLineSpacing,                  Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::lyricsEvenFontSpatiumDependent,         Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::lyricsEvenFontStyle,                    Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::lyricsEvenColor,                        Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::lyricsEvenAlign,                        Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::lyricsPosBelow,                         Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::lyricsEvenFrameType,                    Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::lyricsEvenFramePadding,                 Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::lyricsEvenFrameWidth,                   Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::lyricsEvenFrameRound,                   Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::lyricsEvenFrameFgColor,                 Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::lyricsEvenFrameBgColor,                 Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle lyricsOddTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::lyricsOddFontFace,                      Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::lyricsOddFontSize,                      Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::lyricsOddLineSpacing,                   Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::lyricsOddFontSpatiumDependent,          Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::lyricsOddFontStyle,                     Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::lyricsOddColor,                         Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::lyricsOddAlign,                         Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::lyricsPosBelow,                         Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::lyricsOddFrameType,                     Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::lyricsOddFramePadding,                  Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::lyricsOddFrameWidth,                    Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::lyricsOddFrameRound,                    Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::lyricsOddFrameFgColor,                  Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::lyricsOddFrameBgColor,                  Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle fingeringTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::fingeringFontFace,                      Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::fingeringFontSize,                      Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::fingeringLineSpacing,                   Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::fingeringFontSpatiumDependent,          Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::fingeringFontStyle,                     Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::fingeringColor,                         Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::fingeringAlign,                         Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::fingeringOffset,                        Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::fingeringFrameType,                     Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::fingeringFramePadding,                  Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::fingeringFrameWidth,                    Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::fingeringFrameRound,                    Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::fingeringFrameFgColor,                  Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::fingeringFrameBgColor,                  Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle tabFretNumberTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::tabFretNumberFontFace,                Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::tabFretNumberFontSize,                Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::tabFretNumberLineSpacing,             Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::tabFretNumberFontSpatiumDependent,    Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::tabFretNumberFontStyle,               Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::tabFretNumberColor,                   Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::tabFretNumberAlign,                   Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::tabFretNumberOffset,                  Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::tabFretNumberFrameType,               Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::tabFretNumberFramePadding,            Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::tabFretNumberFrameWidth,              Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::tabFretNumberFrameRound,              Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::tabFretNumberFrameFgColor,            Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::tabFretNumberFrameBgColor,            Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,             Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle lhGuitarFingeringTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::lhGuitarFingeringFontFace,              Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::lhGuitarFingeringFontSize,              Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::lhGuitarFingeringLineSpacing,           Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::lhGuitarFingeringFontSpatiumDependent,  Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::lhGuitarFingeringFontStyle,             Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::lhGuitarFingeringColor,                 Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::lhGuitarFingeringAlign,                 Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::lhGuitarFingeringOffset,                Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::lhGuitarFingeringFrameType,             Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::lhGuitarFingeringFramePadding,          Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::lhGuitarFingeringFrameWidth,            Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::lhGuitarFingeringFrameRound,            Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::lhGuitarFingeringFrameFgColor,          Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::lhGuitarFingeringFrameBgColor,          Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle rhGuitarFingeringTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::rhGuitarFingeringFontFace,              Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::rhGuitarFingeringFontSize,              Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::rhGuitarFingeringLineSpacing,           Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::rhGuitarFingeringFontSpatiumDependent,  Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::rhGuitarFingeringFontStyle,             Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::rhGuitarFingeringColor,                 Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::rhGuitarFingeringAlign,                 Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::rhGuitarFingeringOffset,                Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::rhGuitarFingeringFrameType,             Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::rhGuitarFingeringFramePadding,          Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::rhGuitarFingeringFrameWidth,            Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::rhGuitarFingeringFrameRound,            Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::rhGuitarFingeringFrameFgColor,          Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::rhGuitarFingeringFrameBgColor,          Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle hammerOnPullOffTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::hammerOnPullOffTappingFontFace,                      Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::hammerOnPullOffTappingFontSize,                      Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::hammerOnPullOffTappingLineSpacing,                   Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::hammerOnPullOffTappingFontSpatiumDependent,          Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::hammerOnPullOffTappingFontStyle,                     Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::hammerOnPullOffTappingColor,                         Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::hammerOnPullOffTappingAlign,                         Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::hammerOnPullOffTappingOffset,                        Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::hammerOnPullOffTappingFrameType,                     Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::hammerOnPullOffTappingFramePadding,                  Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::hammerOnPullOffTappingFrameWidth,                    Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::hammerOnPullOffTappingFrameRound,                    Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::hammerOnPullOffTappingFrameFgColor,                  Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::hammerOnPullOffTappingFrameBgColor,                  Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle stringNumberTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::stringNumberFontFace,                   Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::stringNumberFontSize,                   Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::stringNumberLineSpacing,                Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::stringNumberFontSpatiumDependent,       Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::stringNumberFontStyle,                  Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::stringNumberColor,                      Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::stringNumberAlign,                      Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::stringNumberOffset,                     Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::stringNumberFrameType,                  Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::stringNumberFramePadding,               Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::stringNumberFrameWidth,                 Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::stringNumberFrameRound,                 Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::stringNumberFrameFgColor,               Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::stringNumberFrameBgColor,               Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle stringTuningsStyle { { // identical to staffText except for fontSize
    { TextStylePropertyType::FontFace,             Sid::staffTextFontFace,                      Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::stringTuningsFontSize,                  Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::staffTextLineSpacing,                   Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::staffTextFontSpatiumDependent,          Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::staffTextFontStyle,                     Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::staffTextColor,                         Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::staffTextAlign,                         Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::staffTextPosAbove,                      Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::staffTextFrameType,                     Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::staffTextFramePadding,                  Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::staffTextFrameWidth,                    Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::staffTextFrameRound,                    Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::staffTextFrameFgColor,                  Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::staffTextFrameBgColor,                  Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle fretDiagramFingeringStyle { {
    { TextStylePropertyType::FontFace,             Sid::fretDiagramFingeringFontFace,                      Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::fretDiagramFingeringFontSize,                      Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::fretDiagramFingeringLineSpacing,                   Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::fretDiagramFingeringFontSpatiumDependent,          Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::fretDiagramFingeringFontStyle,                     Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::fretDiagramFingeringColor,                         Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::fretDiagramFingeringAlign,                         Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::fretDiagramFingeringPosAbove,                      Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::fretDiagramFingeringFrameType,                     Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::fretDiagramFingeringFramePadding,                  Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::fretDiagramFingeringFrameWidth,                    Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::fretDiagramFingeringFrameRound,                    Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::fretDiagramFingeringFrameFgColor,                  Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::fretDiagramFingeringFrameBgColor,                  Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,                          Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle fretDiagramFretNumberStyle { {
    { TextStylePropertyType::FontFace,             Sid::fretDiagramFretNumberFontFace,                      Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::fretDiagramFretNumberFontSize,                          Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::fretDiagramFretNumberLineSpacing,                   Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::fretDiagramFretNumberFontSpatiumDependent,          Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::fretDiagramFretNumberFontStyle,                     Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::fretDiagramFretNumberColor,                         Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::fretDiagramFretNumberAlign,                          Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::fretDiagramFretNumberPosAbove,                      Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::fretDiagramFretNumberFrameType,                     Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::fretDiagramFretNumberFramePadding,                  Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::fretDiagramFretNumberFrameWidth,                    Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::fretDiagramFretNumberFrameRound,                    Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::fretDiagramFretNumberFrameFgColor,                  Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::fretDiagramFretNumberFrameBgColor,                  Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,                           Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle harpPedalDiagramTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::harpPedalDiagramFontFace,               Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::harpPedalDiagramFontSize,               Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::harpPedalDiagramLineSpacing,            Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::harpPedalDiagramFontSpatiumDependent,   Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::harpPedalDiagramFontStyle,              Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::harpPedalDiagramColor,                  Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::harpPedalDiagramAlign,                  Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::harpPedalDiagramOffset,                 Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::harpPedalDiagramFrameType,              Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::harpPedalDiagramFramePadding,           Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::harpPedalDiagramFrameWidth,             Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::harpPedalDiagramFrameRound,             Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::harpPedalDiagramFrameFgColor,           Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::harpPedalDiagramFrameBgColor,           Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::harpPedalDiagramMusicalSymbolsScale,    Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle harpPedalTextDiagramTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::harpPedalTextDiagramFontFace,               Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::harpPedalTextDiagramFontSize,               Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::harpPedalTextDiagramLineSpacing,            Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::harpPedalTextDiagramFontSpatiumDependent,   Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::harpPedalTextDiagramFontStyle,              Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::harpPedalTextDiagramColor,                  Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::harpPedalTextDiagramAlign,                  Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::harpPedalTextDiagramOffset,                 Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::harpPedalTextDiagramFrameType,              Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::harpPedalTextDiagramFramePadding,           Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::harpPedalTextDiagramFrameWidth,             Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::harpPedalTextDiagramFrameRound,             Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::harpPedalTextDiagramFrameFgColor,           Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::harpPedalTextDiagramFrameBgColor,           Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle longInstrumentTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::longInstrumentFontFace,                 Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::longInstrumentFontSize,                 Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::longInstrumentLineSpacing,              Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::longInstrumentFontSpatiumDependent,     Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::longInstrumentFontStyle,                Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::longInstrumentColor,                    Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::longInstrumentAlign,                    Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::longInstrumentOffset,                   Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::longInstrumentFrameType,                Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::longInstrumentFramePadding,             Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::longInstrumentFrameWidth,               Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::longInstrumentFrameRound,               Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::longInstrumentFrameFgColor,             Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::longInstrumentFrameBgColor,             Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle shortInstrumentTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::shortInstrumentFontFace,                Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::shortInstrumentFontSize,                Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::shortInstrumentLineSpacing,             Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::shortInstrumentFontSpatiumDependent,    Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::shortInstrumentFontStyle,               Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::shortInstrumentColor,                   Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::shortInstrumentAlign,                   Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::shortInstrumentOffset,                  Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::shortInstrumentFrameType,               Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::shortInstrumentFramePadding,            Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::shortInstrumentFrameWidth,              Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::shortInstrumentFrameRound,              Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::shortInstrumentFrameFgColor,            Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::shortInstrumentFrameBgColor,            Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle partInstrumentTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::partInstrumentFontFace,                 Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::partInstrumentFontSize,                 Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::partInstrumentLineSpacing,              Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::partInstrumentFontSpatiumDependent,     Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::partInstrumentFontStyle,                Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::partInstrumentColor,                    Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::partInstrumentAlign,                    Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::partInstrumentOffset,                   Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::partInstrumentFrameType,                Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::partInstrumentFramePadding,             Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::partInstrumentFrameWidth,               Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::partInstrumentFrameRound,               Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::partInstrumentFrameFgColor,             Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::partInstrumentFrameBgColor,             Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

// With the single exception of text alignment, after 4.1 Dynamics use Expression text style
const TextStyle dynamicsTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::expressionFontFace,                     Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::expressionFontSize,                     Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::expressionLineSpacing,                  Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::expressionFontSpatiumDependent,         Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::expressionFontStyle,                    Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::expressionColor,                        Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::dynamicsAlign,                          Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::expressionOffset,                       Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::expressionFrameType,                    Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::expressionFramePadding,                 Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::expressionFrameWidth,                   Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::expressionFrameRound,                   Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::expressionFrameFgColor,                 Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::expressionFrameBgColor,                 Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle expressionTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::expressionFontFace,                     Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::expressionFontSize,                     Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::expressionLineSpacing,                  Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::expressionFontSpatiumDependent,         Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::expressionFontStyle,                    Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::expressionColor,                        Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::expressionAlign,                        Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::expressionOffset,                       Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::expressionFrameType,                    Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::expressionFramePadding,                 Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::expressionFrameWidth,                   Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::expressionFrameRound,                   Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::expressionFrameFgColor,                 Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::expressionFrameBgColor,                 Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle tempoTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::tempoFontFace,                          Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::tempoFontSize,                          Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::tempoLineSpacing,                       Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::tempoFontSpatiumDependent,              Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::tempoFontStyle,                         Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::tempoColor,                             Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::tempoAlign,                             Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::tempoPosAbove,                          Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::tempoFrameType,                         Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::tempoFramePadding,                      Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::tempoFrameWidth,                        Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::tempoFrameRound,                        Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::tempoFrameFgColor,                      Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::tempoFrameBgColor,                      Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle tempoChangeTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::tempoChangeFontFace,                          Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::tempoChangeFontSize,                          Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::tempoChangeLineSpacing,                       Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::tempoChangeFontSpatiumDependent,              Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::tempoChangeFontStyle,                         Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::tempoChangeColor,                             Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::tempoChangeAlign,                             Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::tempoChangePosAbove,                          Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::tempoChangeFrameType,                         Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::tempoChangeFramePadding,                      Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::tempoChangeFrameWidth,                        Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::tempoChangeFrameRound,                        Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::tempoChangeFrameFgColor,                      Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::tempoChangeFrameBgColor,                      Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle metronomeTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::metronomeFontFace,                      Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::metronomeFontSize,                      Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::metronomeLineSpacing,                   Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::metronomeFontSpatiumDependent,          Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::metronomeFontStyle,                     Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::metronomeColor,                         Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::metronomeAlign,                         Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::metronomeOffset,                        Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::metronomeFrameType,                     Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::metronomeFramePadding,                  Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::metronomeFrameWidth,                    Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::metronomeFrameRound,                    Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::metronomeFrameFgColor,                  Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::metronomeFrameBgColor,                  Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle measureNumberTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::measureNumberFontFace,                  Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::measureNumberFontSize,                  Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::measureNumberLineSpacing,               Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::measureNumberFontSpatiumDependent,      Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::measureNumberFontStyle,                 Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::measureNumberColor,                     Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::measureNumberAlign,                     Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::measureNumberPosAbove,                  Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::measureNumberFrameType,                 Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::measureNumberFramePadding,              Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::measureNumberFrameWidth,                Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::measureNumberFrameRound,                Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::measureNumberFrameFgColor,              Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::measureNumberFrameBgColor,              Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle mmRestRangeTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::mmRestRangeFontFace,                    Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::mmRestRangeFontSize,                    Pid::FONT_SIZE },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::mmRestRangeFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::mmRestRangeFontStyle,                   Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::mmRestRangeColor,                       Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::mmRestRangeAlign,                       Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::mmRestRangePosAbove,                    Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::mmRestRangeFrameType,                   Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::mmRestRangeFramePadding,                Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::mmRestRangeFrameWidth,                  Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::mmRestRangeFrameRound,                  Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::mmRestRangeFrameFgColor,                Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::mmRestRangeFrameBgColor,                Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle translatorTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::translatorFontFace,                     Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::translatorFontSize,                     Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::translatorLineSpacing,                  Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::translatorFontSpatiumDependent,         Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::translatorFontStyle,                    Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::translatorColor,                        Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::translatorAlign,                        Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::translatorOffset,                       Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::translatorFrameType,                    Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::translatorFramePadding,                 Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::translatorFrameWidth,                   Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::translatorFrameRound,                   Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::translatorFrameFgColor,                 Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::translatorFrameBgColor,                 Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle tupletTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::tupletFontFace,                         Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::tupletFontSize,                         Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::tupletLineSpacing,                      Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::tupletFontSpatiumDependent,             Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::tupletFontStyle,                        Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::tupletColor,                            Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::tupletAlign,                            Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::tupletOffset,                           Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::tupletFrameType,                        Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::tupletFramePadding,                     Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::tupletFrameWidth,                       Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::tupletFrameRound,                       Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::tupletFrameFgColor,                     Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::tupletFrameBgColor,                     Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::tupletMusicalSymbolsScale,              Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle systemTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::systemTextFontFace,                     Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::systemTextFontSize,                     Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::systemTextLineSpacing,                  Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::systemTextFontSpatiumDependent,         Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::systemTextFontStyle,                    Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::systemTextColor,                        Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::systemTextAlign,                        Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::systemTextPosAbove,                     Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::systemTextFrameType,                    Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::systemTextFramePadding,                 Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::systemTextFrameWidth,                   Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::systemTextFrameRound,                   Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::systemTextFrameFgColor,                 Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::systemTextFrameBgColor,                 Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle staffTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::staffTextFontFace,                      Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::staffTextFontSize,                      Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::staffTextLineSpacing,                   Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::staffTextFontSpatiumDependent,          Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::staffTextFontStyle,                     Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::staffTextColor,                         Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::staffTextAlign,                         Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::staffTextPosAbove,                      Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::staffTextFrameType,                     Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::staffTextFramePadding,                  Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::staffTextFrameWidth,                    Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::staffTextFrameRound,                    Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::staffTextFrameFgColor,                  Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::staffTextFrameBgColor,                  Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle chordSymbolTextStyleA { {
    { TextStylePropertyType::FontFace,             Sid::chordSymbolAFontFace,                   Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::chordSymbolAFontSize,                   Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::chordSymbolALineSpacing,                Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::chordSymbolAFontSpatiumDependent,       Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::chordSymbolAFontStyle,                  Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::chordSymbolAColor,                      Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::chordSymbolAAlign,                      Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::chordSymbolAPosAbove,                   Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::chordSymbolAFrameType,                  Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::chordSymbolAFramePadding,               Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::chordSymbolAFrameWidth,                 Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::chordSymbolAFrameRound,                 Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::chordSymbolAFrameFgColor,               Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::chordSymbolAFrameBgColor,               Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle chordSymbolTextStyleB { {
    { TextStylePropertyType::FontFace,             Sid::chordSymbolBFontFace,                   Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::chordSymbolBFontSize,                   Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::chordSymbolBLineSpacing,                Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::chordSymbolBFontSpatiumDependent,       Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::chordSymbolBFontStyle,                  Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::chordSymbolBColor,                      Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::chordSymbolBAlign,                      Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::chordSymbolBPosAbove,                   Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::chordSymbolBFrameType,                  Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::chordSymbolBFramePadding,               Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::chordSymbolBFrameWidth,                 Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::chordSymbolBFrameRound,                 Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::chordSymbolBFrameFgColor,               Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::chordSymbolBFrameBgColor,               Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle romanNumeralTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::romanNumeralFontFace,                   Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::romanNumeralFontSize,                   Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::romanNumeralLineSpacing,                Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::romanNumeralFontSpatiumDependent,       Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::romanNumeralFontStyle,                  Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::romanNumeralColor,                      Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::romanNumeralAlign,                      Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::romanNumeralPosAbove,                   Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::romanNumeralFrameType,                  Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::romanNumeralFramePadding,               Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::romanNumeralFrameWidth,                 Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::romanNumeralFrameRound,                 Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::romanNumeralFrameFgColor,               Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::romanNumeralFrameBgColor,               Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle nashvilleNumberTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::nashvilleNumberFontFace,                Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::nashvilleNumberFontSize,                Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::nashvilleNumberLineSpacing,             Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::nashvilleNumberFontSpatiumDependent,    Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::nashvilleNumberFontStyle,               Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::nashvilleNumberColor,                   Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::nashvilleNumberAlign,                   Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::nashvilleNumberPosAbove,                Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::nashvilleNumberFrameType,               Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::nashvilleNumberFramePadding,            Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::nashvilleNumberFrameWidth,              Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::nashvilleNumberFrameRound,              Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::nashvilleNumberFrameFgColor,            Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::nashvilleNumberFrameBgColor,            Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle rehearsalMarkTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::rehearsalMarkFontFace,                  Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::rehearsalMarkFontSize,                  Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::rehearsalMarkLineSpacing,               Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::rehearsalMarkFontSpatiumDependent,      Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::rehearsalMarkFontStyle,                 Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::rehearsalMarkColor,                     Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::rehearsalMarkAlign,                     Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::rehearsalMarkPosAbove,                  Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::rehearsalMarkFrameType,                 Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::rehearsalMarkFramePadding,              Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::rehearsalMarkFrameWidth,                Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::rehearsalMarkFrameRound,                Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::rehearsalMarkFrameFgColor,              Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::rehearsalMarkFrameBgColor,              Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle repeatLeftTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::repeatLeftFontFace,                     Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::repeatLeftFontSize,                     Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::repeatLeftLineSpacing,                  Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::repeatLeftFontSpatiumDependent,         Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::repeatLeftFontStyle,                    Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::repeatLeftColor,                        Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::repeatLeftAlign,                        Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::markerPosAbove,                         Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::repeatLeftFrameType,                    Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::repeatLeftFramePadding,                 Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::repeatLeftFrameWidth,                   Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::repeatLeftFrameRound,                   Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::repeatLeftFrameFgColor,                 Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::repeatLeftFrameBgColor,                 Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle repeatRightTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::repeatRightFontFace,                    Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::repeatRightFontSize,                    Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::repeatRightLineSpacing,                 Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::repeatRightFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::repeatRightFontStyle,                   Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::repeatRightColor,                       Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::repeatRightAlign,                       Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::jumpPosAbove,                           Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::repeatRightFrameType,                   Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::repeatRightFramePadding,                Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::repeatRightFrameWidth,                  Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::repeatRightFrameRound,                  Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::repeatRightFrameFgColor,                Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::repeatRightFrameBgColor,                Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle frameTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::frameFontFace,                          Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::frameFontSize,                          Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::frameLineSpacing,                       Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::frameFontSpatiumDependent,              Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::frameFontStyle,                         Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::frameColor,                             Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::frameAlign,                             Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::frameOffset,                            Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::frameFrameType,                         Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::frameFramePadding,                      Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::frameFrameWidth,                        Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::frameFrameRound,                        Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::frameFrameFgColor,                      Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::frameFrameBgColor,                      Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle textLineTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::textLineFontFace,                       Pid::BEGIN_FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::textLineFontSize,                       Pid::BEGIN_FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::textLineLineSpacing,                    Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::textLineFontSpatiumDependent,           Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::textLineFontStyle,                      Pid::BEGIN_FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::textLineColor,                          Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::textLineTextAlign,                      Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::textLinePosAbove,                       Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::textLineFrameType,                      Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::textLineFramePadding,                   Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::textLineFrameWidth,                     Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::textLineFrameRound,                     Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::textLineFrameFgColor,                   Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::textLineFrameBgColor,                   Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle noteLineTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::noteLineFontFace,                      Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::noteLineFontSize,                      Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::noteLineLineSpacing,                   Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::noteLineFontSpatiumDependent,          Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::noteLineFontStyle,                     Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::noteLineColor,                         Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::noteLineAlign,                         Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::noteLineOffset,                        Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::noteLineFrameType,                     Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::noteLineFramePadding,                  Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::noteLineFrameWidth,                    Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::noteLineFrameRound,                    Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::noteLineFrameFgColor,                  Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::noteLineFrameBgColor,                  Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,              Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle glissandoTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::glissandoFontFace,                      Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::glissandoFontSize,                      Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::glissandoLineSpacing,                   Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::glissandoFontSpatiumDependent,          Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::glissandoFontStyle,                     Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::glissandoColor,                         Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::glissandoAlign,                         Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::glissandoOffset,                        Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::glissandoFrameType,                     Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::glissandoFramePadding,                  Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::glissandoFrameWidth,                    Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::glissandoFrameRound,                    Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::glissandoFrameFgColor,                  Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::glissandoFrameBgColor,                  Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle ottavaTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::ottavaFontFace,                         Pid::BEGIN_FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::ottavaFontSize,                         Pid::BEGIN_FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::ottavaLineSpacing,                      Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::ottavaFontSpatiumDependent,             Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::ottavaFontStyle,                        Pid::BEGIN_FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::ottavaColor,                            Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::ottavaTextAlignAbove,                   Pid::BEGIN_TEXT_ALIGN },
    { TextStylePropertyType::Offset,               Sid::ottavaPosAbove,                         Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::ottavaFrameType,                        Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::ottavaFramePadding,                     Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::ottavaFrameWidth,                       Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::ottavaFrameRound,                       Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::ottavaFrameFgColor,                     Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::ottavaFrameBgColor,                     Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::ottavaMusicalSymbolsScale,              Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle voltaTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::voltaFontFace,                          Pid::BEGIN_FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::voltaFontSize,                          Pid::BEGIN_FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::voltaLineSpacing,                       Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::voltaFontSpatiumDependent,              Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::voltaFontStyle,                         Pid::BEGIN_FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::voltaColor,                             Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::voltaAlign,                             Pid::BEGIN_TEXT_ALIGN },
    { TextStylePropertyType::Offset,               Sid::voltaOffset,                            Pid::BEGIN_TEXT_OFFSET },
    { TextStylePropertyType::FrameType,            Sid::voltaFrameType,                         Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::voltaFramePadding,                      Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::voltaFrameWidth,                        Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::voltaFrameRound,                        Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::voltaFrameFgColor,                      Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::voltaFrameBgColor,                      Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle pedalTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::pedalFontFace,                          Pid::BEGIN_FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::pedalFontSize,                          Pid::BEGIN_FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::pedalLineSpacing,                       Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::pedalFontSpatiumDependent,              Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::pedalFontStyle,                         Pid::BEGIN_FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::pedalColor,                             Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::pedalTextAlign,                         Pid::BEGIN_TEXT_ALIGN },
    { TextStylePropertyType::Offset,               Sid::pedalPosAbove,                          Pid::BEGIN_TEXT_OFFSET },
    { TextStylePropertyType::FrameType,            Sid::pedalFrameType,                         Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::pedalFramePadding,                      Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::pedalFrameWidth,                        Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::pedalFrameRound,                        Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::pedalFrameFgColor,                      Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::pedalFrameBgColor,                      Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::pedalMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle letRingTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::letRingFontFace,                        Pid::BEGIN_FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::letRingFontSize,                        Pid::BEGIN_FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::letRingLineSpacing,                     Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::letRingFontSpatiumDependent,            Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::letRingFontStyle,                       Pid::BEGIN_FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::letRingColor,                           Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::letRingTextAlign,                       Pid::BEGIN_TEXT_ALIGN },
    { TextStylePropertyType::Offset,               Sid::letRingPosAbove,                        Pid::BEGIN_TEXT_OFFSET },
    { TextStylePropertyType::FrameType,            Sid::letRingFrameType,                       Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::letRingFramePadding,                    Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::letRingFrameWidth,                      Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::letRingFrameRound,                      Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::letRingFrameFgColor,                    Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::letRingFrameBgColor,                    Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle palmMuteTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::palmMuteFontFace,                       Pid::BEGIN_FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::palmMuteFontSize,                       Pid::BEGIN_FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::palmMuteLineSpacing,                    Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::palmMuteFontSpatiumDependent,           Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::palmMuteFontStyle,                      Pid::BEGIN_FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::palmMuteColor,                          Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::palmMuteTextAlign,                      Pid::BEGIN_TEXT_ALIGN },
    { TextStylePropertyType::Offset,               Sid::palmMutePosAbove,                       Pid::BEGIN_TEXT_OFFSET },
    { TextStylePropertyType::FrameType,            Sid::palmMuteFrameType,                      Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::palmMuteFramePadding,                   Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::palmMuteFrameWidth,                     Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::palmMuteFrameRound,                     Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::palmMuteFrameFgColor,                   Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::palmMuteFrameBgColor,                   Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle hairpinTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::hairpinFontFace,                        Pid::BEGIN_FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::hairpinFontSize,                        Pid::BEGIN_FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::hairpinLineSpacing,                     Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::hairpinFontSpatiumDependent,            Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::hairpinFontStyle,                       Pid::BEGIN_FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::hairpinColor,                           Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::hairpinTextAlign,                       Pid::BEGIN_TEXT_ALIGN },
    { TextStylePropertyType::Offset,               Sid::hairpinPosAbove,                        Pid::BEGIN_TEXT_OFFSET },
    { TextStylePropertyType::FrameType,            Sid::hairpinFrameType,                       Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::hairpinFramePadding,                    Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::hairpinFrameWidth,                      Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::hairpinFrameRound,                      Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::hairpinFrameFgColor,                    Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::hairpinFrameBgColor,                    Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle bendTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::bendFontFace,                           Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::bendFontSize,                           Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::bendLineSpacing,                        Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::bendFontSpatiumDependent,               Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::bendFontStyle,                          Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::bendColor,                              Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::bendAlign,                              Pid::BEGIN_TEXT_ALIGN },
    { TextStylePropertyType::Offset,               Sid::bendOffset,                             Pid::BEGIN_TEXT_OFFSET },
    { TextStylePropertyType::FrameType,            Sid::bendFrameType,                          Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::bendFramePadding,                       Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::bendFrameWidth,                         Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::bendFrameRound,                         Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::bendFrameFgColor,                       Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::bendFrameBgColor,                       Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle headerTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::headerFontFace,                         Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::headerFontSize,                         Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::headerLineSpacing,                      Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::headerFontSpatiumDependent,             Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::headerFontStyle,                        Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::headerColor,                            Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::headerAlign,                            Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::headerOffset,                           Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::headerFrameType,                        Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::headerFramePadding,                     Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::headerFrameWidth,                       Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::headerFrameRound,                       Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::headerFrameFgColor,                     Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::headerFrameBgColor,                     Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle footerTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::footerFontFace,                         Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::footerFontSize,                         Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::footerLineSpacing,                      Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::footerFontSpatiumDependent,             Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::footerFontStyle,                        Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::footerColor,                            Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::footerAlign,                            Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::footerOffset,                           Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::footerFrameType,                        Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::footerFramePadding,                     Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::footerFrameWidth,                       Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::footerFrameRound,                       Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::footerFrameFgColor,                     Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::footerFrameBgColor,                     Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle copyrightTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::copyrightFontFace,                      Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::copyrightFontSize,                      Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::copyrightLineSpacing,                   Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::copyrightFontSpatiumDependent,          Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::copyrightFontStyle,                     Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::copyrightColor,                         Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::copyrightAlign,                         Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::copyrightOffset,                        Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::copyrightFrameType,                     Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::copyrightFramePadding,                  Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::copyrightFrameWidth,                    Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::copyrightFrameRound,                    Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::copyrightFrameFgColor,                  Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::copyrightFrameBgColor,                  Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle pageNumberTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::pageNumberFontFace,                     Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::pageNumberFontSize,                     Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::pageNumberLineSpacing,                  Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::pageNumberFontSpatiumDependent,         Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::pageNumberFontStyle,                    Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::pageNumberColor,                        Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::pageNumberAlign,                        Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::pageNumberOffset,                       Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::pageNumberFrameType,                    Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::pageNumberFramePadding,                 Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::pageNumberFrameWidth,                   Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::pageNumberFrameRound,                   Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::pageNumberFrameFgColor,                 Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::pageNumberFrameBgColor,                 Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle instrumentChangeTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::instrumentChangeFontFace,               Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::instrumentChangeFontSize,               Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::instrumentChangeLineSpacing,            Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::instrumentChangeFontSpatiumDependent,   Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::instrumentChangeFontStyle,              Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::instrumentChangeColor,                  Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::instrumentChangeAlign,                  Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::instrumentChangeOffset,                 Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::instrumentChangeFrameType,              Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::instrumentChangeFramePadding,           Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::instrumentChangeFrameWidth,             Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::instrumentChangeFrameRound,             Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::instrumentChangeFrameFgColor,           Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::instrumentChangeFrameBgColor,           Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle stickingTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::stickingFontFace,                       Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::stickingFontSize,                       Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::stickingLineSpacing,                    Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::stickingFontSpatiumDependent,           Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::stickingFontStyle,                      Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::stickingColor,                          Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::stickingAlign,                          Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::stickingOffset,                         Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::stickingFrameType,                      Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::stickingFramePadding,                   Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::stickingFrameWidth,                     Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::stickingFrameRound,                     Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::stickingFrameFgColor,                   Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::stickingFrameBgColor,                   Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle user1TextStyle { {
    { TextStylePropertyType::FontFace,             Sid::user1FontFace,                          Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::user1FontSize,                          Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::user1LineSpacing,                       Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::user1FontSpatiumDependent,              Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::user1FontStyle,                         Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::user1Color,                             Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::user1Align,                             Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::user1Offset,                            Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::user1FrameType,                         Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::user1FramePadding,                      Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::user1FrameWidth,                        Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::user1FrameRound,                        Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::user1FrameFgColor,                      Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::user1FrameBgColor,                      Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle user2TextStyle { {
    { TextStylePropertyType::FontFace,             Sid::user2FontFace,                          Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::user2FontSize,                          Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::user2LineSpacing,                       Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::user2FontSpatiumDependent,              Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::user2FontStyle,                         Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::user2Color,                             Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::user2Align,                             Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::user2Offset,                            Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::user2FrameType,                         Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::user2FramePadding,                      Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::user2FrameWidth,                        Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::user2FrameRound,                        Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::user2FrameFgColor,                      Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::user2FrameBgColor,                      Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle user3TextStyle { {
    { TextStylePropertyType::FontFace,             Sid::user3FontFace,                          Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::user3FontSize,                          Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::user3LineSpacing,                       Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::user3FontSpatiumDependent,              Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::user3FontStyle,                         Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::user3Color,                             Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::user3Align,                             Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::user3Offset,                            Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::user3FrameType,                         Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::user3FramePadding,                      Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::user3FrameWidth,                        Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::user3FrameRound,                        Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::user3FrameFgColor,                      Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::user3FrameBgColor,                      Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle user4TextStyle { {
    { TextStylePropertyType::FontFace,             Sid::user4FontFace,                          Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::user4FontSize,                          Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::user4LineSpacing,                       Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::user4FontSpatiumDependent,              Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::user4FontStyle,                         Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::user4Color,                             Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::user4Align,                             Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::user4Offset,                            Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::user4FrameType,                         Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::user4FramePadding,                      Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::user4FrameWidth,                        Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::user4FrameRound,                        Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::user4FrameFgColor,                      Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::user4FrameBgColor,                      Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle user5TextStyle { {
    { TextStylePropertyType::FontFace,             Sid::user5FontFace,                          Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::user5FontSize,                          Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::user5LineSpacing,                       Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::user5FontSpatiumDependent,              Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::user5FontStyle,                         Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::user5Color,                             Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::user5Align,                             Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::user5Offset,                            Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::user5FrameType,                         Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::user5FramePadding,                      Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::user5FrameWidth,                        Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::user5FrameRound,                        Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::user5FrameFgColor,                      Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::user5FrameBgColor,                      Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle user6TextStyle { {
    { TextStylePropertyType::FontFace,             Sid::user6FontFace,                          Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::user6FontSize,                          Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::user6LineSpacing,                       Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::user6FontSpatiumDependent,              Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::user6FontStyle,                         Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::user6Color,                             Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::user6Align,                             Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::user6Offset,                            Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::user6FrameType,                         Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::user6FramePadding,                      Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::user6FrameWidth,                        Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::user6FrameRound,                        Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::user6FrameFgColor,                      Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::user6FrameBgColor,                      Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle user7TextStyle { {
    { TextStylePropertyType::FontFace,             Sid::user7FontFace,                          Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::user7FontSize,                          Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::user7LineSpacing,                       Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::user7FontSpatiumDependent,              Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::user7FontStyle,                         Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::user7Color,                             Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::user7Align,                             Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::user7Offset,                            Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::user7FrameType,                         Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::user7FramePadding,                      Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::user7FrameWidth,                        Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::user7FrameRound,                        Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::user7FrameFgColor,                      Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::user7FrameBgColor,                      Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle user8TextStyle { {
    { TextStylePropertyType::FontFace,             Sid::user8FontFace,                          Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::user8FontSize,                          Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::user8LineSpacing,                       Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::user8FontSpatiumDependent,              Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::user8FontStyle,                         Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::user8Color,                             Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::user8Align,                             Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::user8Offset,                            Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::user8FrameType,                         Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::user8FramePadding,                      Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::user8FrameWidth,                        Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::user8FrameRound,                        Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::user8FrameFgColor,                      Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::user8FrameBgColor,                      Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle user9TextStyle { {
    { TextStylePropertyType::FontFace,             Sid::user9FontFace,                          Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::user9FontSize,                          Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::user9LineSpacing,                       Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::user9FontSpatiumDependent,              Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::user9FontStyle,                         Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::user9Color,                             Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::user9Align,                             Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::user9Offset,                            Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::user9FrameType,                         Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::user9FramePadding,                      Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::user9FrameWidth,                        Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::user9FrameRound,                        Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::user9FrameFgColor,                      Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::user9FrameBgColor,                      Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle user10TextStyle { {
    { TextStylePropertyType::FontFace,             Sid::user10FontFace,                         Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::user10FontSize,                         Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::user10LineSpacing,                      Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::user10FontSpatiumDependent,             Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::user10FontStyle,                        Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::user10Color,                            Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::user10Align,                            Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::user10Offset,                           Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::user10FrameType,                        Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::user10FramePadding,                     Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::user10FrameWidth,                       Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::user10FrameRound,                       Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::user10FrameFgColor,                     Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::user10FrameBgColor,                     Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle user11TextStyle { {
    { TextStylePropertyType::FontFace,             Sid::user11FontFace,                         Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::user11FontSize,                         Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::user11LineSpacing,                      Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::user11FontSpatiumDependent,             Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::user11FontStyle,                        Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::user11Color,                            Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::user11Align,                            Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::user11Offset,                           Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::user11FrameType,                        Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::user11FramePadding,                     Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::user11FrameWidth,                       Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::user11FrameRound,                       Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::user11FrameFgColor,                     Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::user11FrameBgColor,                     Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle user12TextStyle { {
    { TextStylePropertyType::FontFace,             Sid::user12FontFace,                         Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::user12FontSize,                         Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::user12LineSpacing,                      Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::user12FontSpatiumDependent,             Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::user12FontStyle,                        Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::user12Color,                            Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::user12Align,                            Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::user12Offset,                           Pid::OFFSET },
    { TextStylePropertyType::FrameType,            Sid::user12FrameType,                        Pid::FRAME_TYPE },
    { TextStylePropertyType::FramePadding,         Sid::user12FramePadding,                     Pid::FRAME_PADDING },
    { TextStylePropertyType::FrameWidth,           Sid::user12FrameWidth,                       Pid::FRAME_WIDTH },
    { TextStylePropertyType::FrameRound,           Sid::user12FrameRound,                       Pid::FRAME_ROUND },
    { TextStylePropertyType::FrameBorderColor,     Sid::user12FrameFgColor,                     Pid::FRAME_FG_COLOR },
    { TextStylePropertyType::FrameFillColor,       Sid::user12FrameBgColor,                     Pid::FRAME_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
} };

const TextStyle* textStyle(TextStyleType idx)
{
    switch (idx) {
    case TextStyleType::DEFAULT: return &defaultTextStyle;

    case TextStyleType::TITLE: return &titleTextStyle;
    case TextStyleType::SUBTITLE: return &subTitleTextStyle;
    case TextStyleType::COMPOSER: return &composerTextStyle;
    case TextStyleType::LYRICIST: return &lyricistTextStyle;
    case TextStyleType::TRANSLATOR: return &translatorTextStyle;
    case TextStyleType::FRAME: return &frameTextStyle;
    case TextStyleType::INSTRUMENT_EXCERPT: return &partInstrumentTextStyle;
    case TextStyleType::INSTRUMENT_LONG: return &longInstrumentTextStyle;
    case TextStyleType::INSTRUMENT_SHORT: return &shortInstrumentTextStyle;
    case TextStyleType::INSTRUMENT_CHANGE: return &instrumentChangeTextStyle;
    case TextStyleType::HEADER: return &headerTextStyle;
    case TextStyleType::FOOTER: return &footerTextStyle;
    case TextStyleType::COPYRIGHT: return &copyrightTextStyle;
    case TextStyleType::PAGE_NUMBER: return &pageNumberTextStyle;

    case TextStyleType::MEASURE_NUMBER: return &measureNumberTextStyle;
    case TextStyleType::MMREST_RANGE: return &mmRestRangeTextStyle;

    case TextStyleType::TEMPO: return &tempoTextStyle;
    case TextStyleType::TEMPO_CHANGE: return &tempoChangeTextStyle;
    case TextStyleType::METRONOME: return &metronomeTextStyle;
    case TextStyleType::REPEAT_LEFT: return &repeatLeftTextStyle;
    case TextStyleType::REPEAT_RIGHT: return &repeatRightTextStyle;
    case TextStyleType::REHEARSAL_MARK: return &rehearsalMarkTextStyle;
    case TextStyleType::SYSTEM: return &systemTextStyle;

    case TextStyleType::STAFF: return &staffTextStyle;
    case TextStyleType::EXPRESSION: return &expressionTextStyle;
    case TextStyleType::DYNAMICS: return &dynamicsTextStyle;
    case TextStyleType::HAIRPIN: return &hairpinTextStyle;
    case TextStyleType::LYRICS_ODD: return &lyricsOddTextStyle;
    case TextStyleType::LYRICS_EVEN: return &lyricsEvenTextStyle;
    case TextStyleType::HARMONY_A: return &chordSymbolTextStyleA;
    case TextStyleType::HARMONY_B: return &chordSymbolTextStyleB;
    case TextStyleType::HARMONY_ROMAN: return &romanNumeralTextStyle;
    case TextStyleType::HARMONY_NASHVILLE: return &nashvilleNumberTextStyle;

    case TextStyleType::TUPLET: return &tupletTextStyle;
    case TextStyleType::STICKING: return &stickingTextStyle;
    case TextStyleType::FINGERING: return &fingeringTextStyle;
    case TextStyleType::TAB_FRET_NUMBER: return &tabFretNumberTextStyle;
    case TextStyleType::LH_GUITAR_FINGERING: return &lhGuitarFingeringTextStyle;
    case TextStyleType::RH_GUITAR_FINGERING: return &rhGuitarFingeringTextStyle;
    case TextStyleType::HAMMER_ON_PULL_OFF: return &hammerOnPullOffTextStyle;
    case TextStyleType::STRING_NUMBER: return &stringNumberTextStyle;
    case TextStyleType::STRING_TUNINGS: return &stringTuningsStyle; // todo
    case TextStyleType::FRET_DIAGRAM_FINGERING: return &fretDiagramFingeringStyle;
    case TextStyleType::FRET_DIAGRAM_FRET_NUMBER: return &fretDiagramFretNumberStyle;
    case TextStyleType::HARP_PEDAL_DIAGRAM: return &harpPedalDiagramTextStyle;
    case TextStyleType::HARP_PEDAL_TEXT_DIAGRAM: return &harpPedalTextDiagramTextStyle;

    case TextStyleType::TEXTLINE: return &textLineTextStyle;
    case TextStyleType::NOTELINE: return &noteLineTextStyle;
    case TextStyleType::VOLTA: return &voltaTextStyle;
    case TextStyleType::OTTAVA: return &ottavaTextStyle;
    case TextStyleType::GLISSANDO: return &glissandoTextStyle;
    case TextStyleType::PEDAL: return &pedalTextStyle;
    case TextStyleType::BEND: return &bendTextStyle;
    case TextStyleType::LET_RING: return &letRingTextStyle;
    case TextStyleType::PALM_MUTE: return &palmMuteTextStyle;

    case TextStyleType::USER1: return &user1TextStyle;
    case TextStyleType::USER2: return &user2TextStyle;
    case TextStyleType::USER3: return &user3TextStyle;
    case TextStyleType::USER4: return &user4TextStyle;
    case TextStyleType::USER5: return &user5TextStyle;
    case TextStyleType::USER6: return &user6TextStyle;
    case TextStyleType::USER7: return &user7TextStyle;
    case TextStyleType::USER8: return &user8TextStyle;
    case TextStyleType::USER9: return &user9TextStyle;
    case TextStyleType::USER10: return &user10TextStyle;
    case TextStyleType::USER11: return &user11TextStyle;
    case TextStyleType::USER12: return &user12TextStyle;

    case TextStyleType::TEXT_TYPES: return nullptr;
    case TextStyleType::IGNORED_TYPES: return nullptr;
    }

    UNREACHABLE;
    return nullptr;
}

static std::vector<TextStyleType> _allTextStyles;

static const std::vector<TextStyleType> _primaryTextStyles = {
    TextStyleType::TITLE,
    TextStyleType::SUBTITLE,
    TextStyleType::COMPOSER,
    TextStyleType::LYRICIST,
    TextStyleType::TRANSLATOR,
    TextStyleType::FRAME,
    TextStyleType::INSTRUMENT_EXCERPT,
    TextStyleType::INSTRUMENT_CHANGE,
    TextStyleType::HEADER,
    TextStyleType::FOOTER,
    TextStyleType::COPYRIGHT,
    TextStyleType::PAGE_NUMBER,
    TextStyleType::MEASURE_NUMBER,
    TextStyleType::MMREST_RANGE,
    TextStyleType::TEMPO,
    TextStyleType::REPEAT_LEFT,
    TextStyleType::REPEAT_RIGHT,
    TextStyleType::REHEARSAL_MARK,
    TextStyleType::SYSTEM,
    TextStyleType::STAFF,
    TextStyleType::EXPRESSION,
    TextStyleType::DYNAMICS,
    TextStyleType::HAIRPIN,
    TextStyleType::LYRICS_ODD,
    TextStyleType::LYRICS_EVEN,
    TextStyleType::HARMONY_A,
    TextStyleType::HARMONY_B,
    TextStyleType::HARMONY_ROMAN,
    TextStyleType::HARMONY_NASHVILLE,
    TextStyleType::STICKING,
    TextStyleType::USER1,
    TextStyleType::USER2,
    TextStyleType::USER3,
    TextStyleType::USER4,
    TextStyleType::USER5,
    TextStyleType::USER6,
    TextStyleType::USER7,
    TextStyleType::USER8,
    TextStyleType::USER9,
    TextStyleType::USER10,
    TextStyleType::USER11,
    TextStyleType::USER12,
};

//---------------------------------------------------------
//   allTextStyles
//---------------------------------------------------------

const std::vector<TextStyleType>& allTextStyles()
{
    if (_allTextStyles.empty()) {
        _allTextStyles.reserve(int(TextStyleType::TEXT_TYPES));
        for (int t = int(TextStyleType::DEFAULT) + 1; t < int(TextStyleType::TEXT_TYPES); ++t) {
            _allTextStyles.push_back(TextStyleType(t));
        }
    }
    return _allTextStyles;
}

//---------------------------------------------------------
//   primaryTextStyles
//---------------------------------------------------------

const std::vector<TextStyleType>& primaryTextStyles()
{
    return _primaryTextStyles;
}

static std::vector<TextStyleType> _editableTextStyles;

const std::vector<TextStyleType>& editableTextStyles()
{
    if (_editableTextStyles.empty()) {
        _editableTextStyles = allTextStyles();
        muse::remove(_editableTextStyles, TextStyleType::DYNAMICS);
    }
    return _editableTextStyles;
}
}
