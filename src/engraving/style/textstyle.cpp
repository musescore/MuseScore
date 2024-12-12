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
    { TextStylePropertyType::BorderType,            Sid::defaultBorderType,                       Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::defaultBorderPadding,                    Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::defaultBorderWidth,                      Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::defaultBorderRound,                      Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,           Sid::defaultBorderFgColor,                    Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::defaultBorderBgColor,                    Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::defaultPosition,                        Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::titleBorderType,                         Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::titleBorderPadding,                      Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::titleBorderWidth,                        Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::titleBorderRound,                        Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,           Sid::titleBorderFgColor,                      Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::titleBorderBgColor,                      Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::titlePosition,                          Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::subTitleBorderType,                      Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::subTitleBorderPadding,                   Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::subTitleBorderWidth,                     Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::subTitleBorderRound,                     Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,           Sid::subTitleBorderFgColor,                   Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::subTitleBorderBgColor,                   Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::subTitlePosition,                       Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::composerBorderType,                      Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::composerBorderPadding,                   Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::composerBorderWidth,                     Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::composerBorderRound,                     Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,           Sid::composerBorderFgColor,                   Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::composerBorderBgColor,                   Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::composerPosition,                       Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::lyricistBorderType,                      Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::lyricistBorderPadding,                   Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::lyricistBorderWidth,                     Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::lyricistBorderRound,                     Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,           Sid::lyricistBorderFgColor,                   Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::lyricistBorderBgColor,                   Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::lyricistPosition,                       Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::lyricsEvenBorderType,                    Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::lyricsEvenBorderPadding,                 Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::lyricsEvenBorderWidth,                   Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::lyricsEvenBorderRound,                   Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,           Sid::lyricsEvenBorderFgColor,                 Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::lyricsEvenBorderBgColor,                 Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::lyricsEvenPosition,                     Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::lyricsOddBorderType,                     Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::lyricsOddBorderPadding,                  Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::lyricsOddBorderWidth,                    Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::lyricsOddBorderRound,                    Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,           Sid::lyricsOddBorderFgColor,                  Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::lyricsOddBorderBgColor,                  Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::lyricsOddPosition,                      Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::fingeringBorderType,                     Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::fingeringBorderPadding,                  Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::fingeringBorderWidth,                    Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::fingeringBorderRound,                    Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,           Sid::fingeringBorderFgColor,                  Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::fingeringBorderBgColor,                  Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::fingeringPosition,                      Pid::POSITION },
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
    { TextStylePropertyType::BorderType,           Sid::tabFretNumberFrameType,               Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,        Sid::tabFretNumberFramePadding,            Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,          Sid::tabFretNumberFrameWidth,              Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,          Sid::tabFretNumberFrameRound,              Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,          Sid::tabFretNumberFrameFgColor,            Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,      Sid::tabFretNumberFrameBgColor,            Pid::BORDER_BG_COLOR },
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
    { TextStylePropertyType::BorderType,            Sid::lhGuitarFingeringBorderType,             Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::lhGuitarFingeringBorderPadding,          Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::lhGuitarFingeringBorderWidth,            Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::lhGuitarFingeringBorderRound,            Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,           Sid::lhGuitarFingeringBorderFgColor,          Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::lhGuitarFingeringBorderBgColor,          Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::lhGuitarFingeringPosition,              Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::rhGuitarFingeringBorderType,             Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::rhGuitarFingeringBorderPadding,          Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::rhGuitarFingeringBorderWidth,            Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::rhGuitarFingeringBorderRound,            Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,           Sid::rhGuitarFingeringBorderFgColor,          Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::rhGuitarFingeringBorderBgColor,          Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::rhGuitarFingeringPosition,                      Pid::POSITION },
} };

const TextStyle hammerOnPullOffTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::hammerOnPullOffTappingFontFace,             Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::hammerOnPullOffTappingFontSize,             Pid::FONT_SIZE },
    { TextStylePropertyType::LineSpacing,          Sid::hammerOnPullOffTappingLineSpacing,          Pid::TEXT_LINE_SPACING },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::hammerOnPullOffTappingFontSpatiumDependent, Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::hammerOnPullOffTappingFontStyle,            Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::hammerOnPullOffTappingColor,                Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::hammerOnPullOffTappingAlign,                Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::hammerOnPullOffTappingOffset,               Pid::OFFSET },
    { TextStylePropertyType::BorderType,           Sid::hammerOnPullOffTappingFrameType,            Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,        Sid::hammerOnPullOffTappingFramePadding,         Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,          Sid::hammerOnPullOffTappingFrameWidth,           Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,          Sid::hammerOnPullOffTappingFrameRound,           Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,          Sid::hammerOnPullOffTappingFrameFgColor,         Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,      Sid::hammerOnPullOffTappingFrameBgColor,         Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,                   Pid::MUSICAL_SYMBOLS_SCALE },
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
    { TextStylePropertyType::BorderType,            Sid::stringNumberBorderType,                  Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::stringNumberBorderPadding,               Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::stringNumberBorderWidth,                 Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::stringNumberBorderRound,                 Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,           Sid::stringNumberBorderFgColor,               Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::stringNumberBorderBgColor,               Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::stringNumberPosition,                   Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::staffTextBorderType,                     Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::staffTextBorderPadding,                  Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::staffTextBorderWidth,                    Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::staffTextBorderRound,                    Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,           Sid::staffTextBorderFgColor,                  Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::staffTextBorderBgColor,                  Pid::BORDER_BG_COLOR },
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
    { TextStylePropertyType::BorderType,            Sid::fretDiagramFingeringBorderType,                     Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::fretDiagramFingeringBorderPadding,                  Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::fretDiagramFingeringBorderWidth,                    Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::fretDiagramFingeringBorderRound,                    Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,           Sid::fretDiagramFingeringBorderFgColor,                  Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::fretDiagramFingeringBorderBgColor,                  Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,                          Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::fretDiagramFingeringPosition,                      Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::fretDiagramFretNumberBorderType,                     Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::fretDiagramFretNumberBorderPadding,                  Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::fretDiagramFretNumberBorderWidth,                    Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::fretDiagramFretNumberBorderRound,                    Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,           Sid::fretDiagramFretNumberBorderFgColor,                  Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::fretDiagramFretNumberBorderBgColor,                  Pid::BORDER_BG_COLOR },
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
    { TextStylePropertyType::BorderType,           Sid::harpPedalDiagramBorderType,             Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,        Sid::harpPedalDiagramBorderPadding,          Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,          Sid::harpPedalDiagramBorderWidth,            Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,          Sid::harpPedalDiagramBorderRound,            Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,          Sid::harpPedalDiagramBorderFgColor,          Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,      Sid::harpPedalDiagramBorderBgColor,          Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::harpPedalDiagramMusicalSymbolsScale,    Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::harpPedalDiagramPosition,               Pid::POSITION },
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
    { TextStylePropertyType::BorderType,             Sid::harpPedalTextDiagramBorderType,      Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::harpPedalTextDiagramBorderPadding,   Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::harpPedalTextDiagramBorderWidth,     Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::harpPedalTextDiagramBorderRound,     Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,           Sid::harpPedalTextDiagramBorderFgColor,   Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::harpPedalTextDiagramBorderBgColor,   Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,                   Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::harpPedalTextDiagramPosition,               Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::longInstrumentBorderType,                Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::longInstrumentBorderPadding,             Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::longInstrumentBorderWidth,               Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::longInstrumentBorderRound,               Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::longInstrumentBorderFgColor,             Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::longInstrumentBorderBgColor,             Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::longInstrumentPosition,                 Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::shortInstrumentBorderType,               Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::shortInstrumentBorderPadding,            Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::shortInstrumentBorderWidth,              Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::shortInstrumentBorderRound,              Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::shortInstrumentBorderFgColor,            Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::shortInstrumentBorderBgColor,            Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::shortInstrumentPosition,            Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::partInstrumentBorderType,                Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::partInstrumentBorderPadding,             Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::partInstrumentBorderWidth,               Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::partInstrumentBorderRound,               Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::partInstrumentBorderFgColor,             Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::partInstrumentBorderBgColor,             Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::partInstrumentPosition,                      Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::expressionBorderType,                    Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::expressionBorderPadding,                 Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::expressionBorderWidth,                   Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::expressionBorderRound,                   Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::expressionBorderFgColor,                 Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::expressionBorderBgColor,                 Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::dynamicsPosition,                       Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::expressionBorderType,                    Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::expressionBorderPadding,                 Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::expressionBorderWidth,                   Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::expressionBorderRound,                   Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::expressionBorderFgColor,                 Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::expressionBorderBgColor,                 Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::expressionPosition,                     Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::tempoBorderType,                         Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::tempoBorderPadding,                      Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::tempoBorderWidth,                        Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::tempoBorderRound,                        Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::tempoBorderFgColor,                      Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::tempoBorderBgColor,                      Pid::BORDER_BG_COLOR },
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
    { TextStylePropertyType::BorderType,            Sid::tempoChangeBorderType,                         Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::tempoChangeBorderPadding,                      Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::tempoChangeBorderWidth,                        Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::tempoChangeBorderRound,                        Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::tempoChangeBorderFgColor,                      Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::tempoChangeBorderBgColor,                      Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,                     Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::tempoChangePosition,                      Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::metronomeBorderType,                     Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::metronomeBorderPadding,                  Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::metronomeBorderWidth,                    Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::metronomeBorderRound,                    Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::metronomeBorderFgColor,                  Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::metronomeBorderBgColor,                  Pid::BORDER_BG_COLOR },
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
    { TextStylePropertyType::BorderType,            Sid::measureNumberBorderType,                 Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::measureNumberBorderPadding,              Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::measureNumberBorderWidth,                Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::measureNumberBorderRound,                Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::measureNumberBorderFgColor,              Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::measureNumberBorderBgColor,              Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::measureNumberPosition,                  Pid::POSITION },
} };

const TextStyle mmRestRangeTextStyle { {
    { TextStylePropertyType::FontFace,             Sid::mmRestRangeFontFace,                    Pid::FONT_FACE },
    { TextStylePropertyType::FontSize,             Sid::mmRestRangeFontSize,                    Pid::FONT_SIZE },
    { TextStylePropertyType::SizeSpatiumDependent, Sid::mmRestRangeFontSpatiumDependent,        Pid::SIZE_SPATIUM_DEPENDENT },
    { TextStylePropertyType::FontStyle,            Sid::mmRestRangeFontStyle,                   Pid::FONT_STYLE },
    { TextStylePropertyType::Color,                Sid::mmRestRangeColor,                       Pid::COLOR },
    { TextStylePropertyType::TextAlign,            Sid::mmRestRangeAlign,                       Pid::ALIGN },
    { TextStylePropertyType::Offset,               Sid::mmRestRangePosAbove,                    Pid::OFFSET },
    { TextStylePropertyType::BorderType,            Sid::mmRestRangeBorderType,                   Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::mmRestRangeBorderPadding,                Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::mmRestRangeBorderWidth,                  Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::mmRestRangeBorderRound,                  Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::mmRestRangeBorderFgColor,                Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::mmRestRangeBorderBgColor,                Pid::BORDER_BG_COLOR },
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
    { TextStylePropertyType::BorderType,            Sid::translatorBorderType,                    Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::translatorBorderPadding,                 Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::translatorBorderWidth,                   Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::translatorBorderRound,                   Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::translatorBorderFgColor,                 Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::translatorBorderBgColor,                 Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::translatorPosition,                     Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::tupletBorderType,                        Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::tupletBorderPadding,                     Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::tupletBorderWidth,                       Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::tupletBorderRound,                       Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::tupletBorderFgColor,                     Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::tupletBorderBgColor,                     Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::tupletMusicalSymbolsScale,              Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::tupletPosition,                         Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::systemTextBorderType,                    Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::systemTextBorderPadding,                 Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::systemTextBorderWidth,                   Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::systemTextBorderRound,                   Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::systemTextBorderFgColor,                 Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::systemTextBorderBgColor,                 Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::systemTextPosition,                     Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::staffTextBorderType,                     Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::staffTextBorderPadding,                  Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::staffTextBorderWidth,                    Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::staffTextBorderRound,                    Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::staffTextBorderFgColor,                  Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::staffTextBorderBgColor,                  Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::staffTextPosition,                      Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::chordSymbolABorderType,                  Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::chordSymbolABorderPadding,               Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::chordSymbolABorderWidth,                 Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::chordSymbolABorderRound,                 Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::chordSymbolABorderFgColor,               Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::chordSymbolABorderBgColor,               Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::chordSymPosition,                       Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::chordSymbolBBorderType,                  Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::chordSymbolBBorderPadding,               Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::chordSymbolBBorderWidth,                 Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::chordSymbolBBorderRound,                 Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::chordSymbolBBorderFgColor,               Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::chordSymbolBBorderBgColor,               Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::chordSymPosition,                       Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::romanNumeralBorderType,                  Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::romanNumeralBorderPadding,               Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::romanNumeralBorderWidth,                 Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::romanNumeralBorderRound,                 Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::romanNumeralBorderFgColor,               Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::romanNumeralBorderBgColor,               Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::chordSymPosition,                       Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::nashvilleNumberBorderType,               Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::nashvilleNumberBorderPadding,            Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::nashvilleNumberBorderWidth,              Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::nashvilleNumberBorderRound,              Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::nashvilleNumberBorderFgColor,            Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::nashvilleNumberBorderBgColor,            Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::chordSymPosition,                       Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::rehearsalMarkBorderType,                 Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::rehearsalMarkBorderPadding,              Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::rehearsalMarkBorderWidth,                Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::rehearsalMarkBorderRound,                Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::rehearsalMarkBorderFgColor,              Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::rehearsalMarkBorderBgColor,              Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::rehearsalMarkPosition,                  Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::repeatLeftBorderType,                    Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::repeatLeftBorderPadding,                 Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::repeatLeftBorderWidth,                   Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::repeatLeftBorderRound,                   Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::repeatLeftBorderFgColor,                 Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::repeatLeftBorderBgColor,                 Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::repeatLeftPosition,                     Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::repeatRightBorderType,                   Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::repeatRightBorderPadding,                Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::repeatRightBorderWidth,                  Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::repeatRightBorderRound,                  Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::repeatRightBorderFgColor,                Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::repeatRightBorderBgColor,                Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::repeatRightPosition,                    Pid::POSITION },
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
    { TextStylePropertyType::BorderType,           Sid::frameBorderType,                        Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,        Sid::frameBorderPadding,                     Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,          Sid::frameBorderWidth,                       Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,          Sid::frameBorderRound,                       Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,          Sid::frameBorderFgColor,                     Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,      Sid::frameBorderBgColor,                     Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::framePosition,                          Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::textLineBorderType,                      Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::textLineBorderPadding,                   Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::textLineBorderWidth,                     Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::textLineBorderRound,                     Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::textLineBorderFgColor,                   Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::textLineBorderBgColor,                   Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::textLinePosition,                       Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::noteLineBorderType,                     Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::noteLineBorderPadding,                  Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::noteLineBorderWidth,                    Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::noteLineBorderRound,                    Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::noteLineBorderFgColor,                  Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::noteLineBorderBgColor,                  Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,              Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::noteLinePosition,                      Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::glissandoBorderType,                     Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::glissandoBorderPadding,                  Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::glissandoBorderWidth,                    Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::glissandoBorderRound,                    Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::glissandoBorderFgColor,                  Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::glissandoBorderBgColor,                  Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::glissandoPosition,                      Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::ottavaBorderType,                        Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::ottavaBorderPadding,                     Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::ottavaBorderWidth,                       Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::ottavaBorderRound,                       Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::ottavaBorderFgColor,                     Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::ottavaBorderBgColor,                     Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::ottavaMusicalSymbolsScale,              Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::ottavaPosition,                         Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::voltaBorderType,                         Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::voltaBorderPadding,                      Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::voltaBorderWidth,                        Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::voltaBorderRound,                        Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::voltaBorderFgColor,                      Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::voltaBorderBgColor,                      Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::voltaPosition,                          Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::pedalBorderType,                         Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::pedalBorderPadding,                      Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::pedalBorderWidth,                        Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::pedalBorderRound,                        Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::pedalBorderFgColor,                      Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::pedalBorderBgColor,                      Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::pedalMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::pedalPosition,                      Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::letRingBorderType,                       Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::letRingBorderPadding,                    Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::letRingBorderWidth,                      Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::letRingBorderRound,                      Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::letRingBorderFgColor,                    Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::letRingBorderBgColor,                    Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::letRingPosition,                        Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::palmMuteBorderType,                      Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::palmMuteBorderPadding,                   Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::palmMuteBorderWidth,                     Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::palmMuteBorderRound,                     Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::palmMuteBorderFgColor,                   Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::palmMuteBorderBgColor,                   Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::palmMutePosition,                       Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::hairpinBorderType,                       Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::hairpinBorderPadding,                    Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::hairpinBorderWidth,                      Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::hairpinBorderRound,                      Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::hairpinBorderFgColor,                    Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::hairpinBorderBgColor,                    Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::hairpinPosition,                        Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::bendBorderType,                          Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::bendBorderPadding,                       Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::bendBorderWidth,                         Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::bendBorderRound,                         Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::bendBorderFgColor,                       Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::bendBorderBgColor,                       Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::bendPosition,                           Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::headerBorderType,                        Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::headerBorderPadding,                     Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::headerBorderWidth,                       Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::headerBorderRound,                       Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::headerBorderFgColor,                     Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::headerBorderBgColor,                     Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::headerPosition,                         Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::footerBorderType,                        Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::footerBorderPadding,                     Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::footerBorderWidth,                       Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::footerBorderRound,                       Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::footerBorderFgColor,                     Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::footerBorderBgColor,                     Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::footerPosition,                         Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::copyrightBorderType,                     Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::copyrightBorderPadding,                  Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::copyrightBorderWidth,                    Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::copyrightBorderRound,                    Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::copyrightBorderFgColor,                  Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::copyrightBorderBgColor,                  Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::copyrightPosition,                      Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::pageNumberBorderType,                    Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::pageNumberBorderPadding,                 Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::pageNumberBorderWidth,                   Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::pageNumberBorderRound,                   Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::pageNumberBorderFgColor,                 Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::pageNumberBorderBgColor,                 Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::pageNumberPosition,                     Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::instrumentChangeBorderType,              Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::instrumentChangeBorderPadding,           Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::instrumentChangeBorderWidth,             Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::instrumentChangeBorderRound,             Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::instrumentChangeBorderFgColor,           Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::instrumentChangeBorderBgColor,           Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::instrumentChangePosition,               Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::stickingBorderType,                      Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::stickingBorderPadding,                   Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::stickingBorderWidth,                     Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::stickingBorderRound,                     Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::stickingBorderFgColor,                   Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::stickingBorderBgColor,                   Pid::BORDER_BG_COLOR },
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
    { TextStylePropertyType::BorderType,            Sid::user1BorderType,                         Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::user1BorderPadding,                      Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::user1BorderWidth,                        Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::user1BorderRound,                        Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::user1BorderFgColor,                      Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::user1BorderBgColor,                      Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::user1Position,                          Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::user2BorderType,                         Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::user2BorderPadding,                      Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::user2BorderWidth,                        Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::user2BorderRound,                        Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::user2BorderFgColor,                      Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::user2BorderBgColor,                      Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::user2Position,                          Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::user3BorderType,                         Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::user3BorderPadding,                      Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::user3BorderWidth,                        Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::user3BorderRound,                        Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::user3BorderFgColor,                      Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::user3BorderBgColor,                      Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::user3Position,                          Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::user4BorderType,                         Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::user4BorderPadding,                      Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::user4BorderWidth,                        Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::user4BorderRound,                        Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::user4BorderFgColor,                      Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::user4BorderBgColor,                      Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::user4Position,                          Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::user5BorderType,                         Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::user5BorderPadding,                      Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::user5BorderWidth,                        Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::user5BorderRound,                        Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::user5BorderFgColor,                      Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::user5BorderBgColor,                      Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::user5Position,                          Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::user6BorderType,                         Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::user6BorderPadding,                      Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::user6BorderWidth,                        Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::user6BorderRound,                        Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::user6BorderFgColor,                      Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::user6BorderBgColor,                      Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::user6Position,                          Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::user7BorderType,                         Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::user7BorderPadding,                      Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::user7BorderWidth,                        Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::user7BorderRound,                        Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::user7BorderFgColor,                      Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::user7BorderBgColor,                      Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::user7Position,                          Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::user8BorderType,                         Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::user8BorderPadding,                      Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::user8BorderWidth,                        Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::user8BorderRound,                        Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::user8BorderFgColor,                      Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::user8BorderBgColor,                      Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::user8Position,                          Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::user9BorderType,                         Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::user9BorderPadding,                      Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::user9BorderWidth,                        Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::user9BorderRound,                        Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::user9BorderFgColor,                      Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::user9BorderBgColor,                      Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::user9Position,                          Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::user10BorderType,                        Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::user10BorderPadding,                     Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::user10BorderWidth,                       Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::user10BorderRound,                       Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::user10BorderFgColor,                     Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::user10BorderBgColor,                     Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::user10Position,                         Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::user11BorderType,                        Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::user11BorderPadding,                     Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::user11BorderWidth,                       Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::user11BorderRound,                       Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::user11BorderFgColor,                     Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::user11BorderBgColor,                     Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::user11Position,                         Pid::POSITION },
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
    { TextStylePropertyType::BorderType,            Sid::user12BorderType,                        Pid::BORDER_TYPE },
    { TextStylePropertyType::BorderPadding,         Sid::user12BorderPadding,                     Pid::BORDER_PADDING },
    { TextStylePropertyType::BorderWidth,           Sid::user12BorderWidth,                       Pid::BORDER_WIDTH },
    { TextStylePropertyType::BorderRound,           Sid::user12BorderRound,                       Pid::BORDER_ROUND },
    { TextStylePropertyType::BorderColor,     Sid::user12BorderFgColor,                     Pid::BORDER_FG_COLOR },
    { TextStylePropertyType::BorderFillColor,       Sid::user12BorderBgColor,                     Pid::BORDER_BG_COLOR },
    { TextStylePropertyType::MusicalSymbolsScale,  Sid::dummyMusicalSymbolsScale,               Pid::MUSICAL_SYMBOLS_SCALE },
    { TextStylePropertyType::Position,             Sid::user12Position,                         Pid::POSITION },
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
