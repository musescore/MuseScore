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
#include "specialcharactersdialog.h"

#include <QListWidget>
#include <QSplitter>

#include "palettewidget.h"

#include "translation.h"
#include "smuflranges.h"

#include "engraving/libmscore/text.h"
#include "engraving/libmscore/symbol.h"
#include "engraving/libmscore/clef.h"
#include "engraving/libmscore/score.h"
#include "engraving/style/style.h"
#include "engraving/types/symnames.h"

#include "ui/view/widgetstatestore.h"

static const QString SPECIAL_CHARACTERS_DIALOG_NAME("SpecialCharactersDialog");

using namespace mu::engraving;
using namespace mu::notation;
using namespace mu::palette;
using namespace mu::ui;

namespace Ms {
static constexpr SymId commonScoreSymbols[] = {
    SymId::accidentalFlat,
    SymId::accidentalNatural,
    SymId::accidentalSharp,
    SymId::accidentalDoubleFlat,
    SymId::accidentalDoubleSharp,
    SymId::metNoteWhole,
    SymId::metNoteHalfUp,
    SymId::metNoteQuarterUp,
    SymId::metNote8thUp,
    SymId::metNote16thUp,
    SymId::metNote32ndUp,
    SymId::metNote64thUp,
    SymId::metNote128thUp,
    SymId::metAugmentationDot,
    SymId::restWholeLegerLine,
    SymId::restHalfLegerLine,
    SymId::restQuarter,
    SymId::rest8th,
    SymId::rest16th,
    SymId::rest32nd,
    SymId::rest64th,
    SymId::rest128th,
    SymId::segno,
    SymId::coda,
    SymId::segnoSerpent1,
    SymId::codaSquare,
    SymId::repeat1Bar,
    SymId::repeat2Bars,
    SymId::repeat4Bars,
    SymId::gClef,
    SymId::fClef,
    SymId::cClef,
    SymId::lyricsElisionNarrow,
    SymId::lyricsElision,
    SymId::lyricsElisionWide,
    SymId::dynamicPiano,
    SymId::dynamicMezzo,
    SymId::dynamicForte,
    SymId::dynamicNiente,
    SymId::dynamicRinforzando,
    SymId::dynamicSforzando,
    SymId::dynamicZ,
    SymId::space
};

struct UnicodeRange {
    int first;
    int last;
    const char* name;
};
static constexpr UnicodeRange unicodeRanges[] = {
    { 0x0020, 0x007F, QT_TRANSLATE_NOOP("textpalette", "Basic Latin") }, // leaving out the first 32 non-printable characters
    { 0x0080, 0x00FF, QT_TRANSLATE_NOOP("textpalette", "Latin-1 Supplement") },
    { 0x0100, 0x017F, QT_TRANSLATE_NOOP("textpalette", "Latin Extended-A") },
    { 0x0180, 0x024F, QT_TRANSLATE_NOOP("textpalette", "Latin Extended-B") },
    { 0x0250, 0x02AF, QT_TRANSLATE_NOOP("textpalette", "IPA Extensions") },
    { 0x02B0, 0x02FF, QT_TRANSLATE_NOOP("textpalette", "Spacing Modifier Letters") },
    { 0x0300, 0x036F, QT_TRANSLATE_NOOP("textpalette", "Combining Diacritical Marks") },
    { 0x0370, 0x03FF, QT_TRANSLATE_NOOP("textpalette", "Greek and Coptic") },
    { 0x0400, 0x04FF, QT_TRANSLATE_NOOP("textpalette", "Cyrillic") },
    { 0x0500, 0x052F, QT_TRANSLATE_NOOP("textpalette", "Cyrillic Supplement") },
    { 0x0530, 0x058F, QT_TRANSLATE_NOOP("textpalette", "Armenian") },
    { 0x0590, 0x05FF, QT_TRANSLATE_NOOP("textpalette", "Hebrew") },
    { 0x0600, 0x06FF, QT_TRANSLATE_NOOP("textpalette", "Arabic") },
    { 0x0700, 0x074F, QT_TRANSLATE_NOOP("textpalette", "Syriac") },
    { 0x0750, 0x077F, QT_TRANSLATE_NOOP("textpalette", "Arabic Supplement") },
    { 0x0780, 0x07BF, QT_TRANSLATE_NOOP("textpalette", "Thaana") },
    { 0x07C0, 0x07FF, QT_TRANSLATE_NOOP("textpalette", "NKo") },
    { 0x0800, 0x083F, QT_TRANSLATE_NOOP("textpalette", "Samaritan") },
    { 0x0840, 0x085F, QT_TRANSLATE_NOOP("textpalette", "Mandaic") },
    { 0x0860, 0x086F, QT_TRANSLATE_NOOP("textpalette", "Syriac Supplement") },
    { 0x08A0, 0x08FF, QT_TRANSLATE_NOOP("textpalette", "Arabic Extended-A") },
    { 0x0900, 0x097F, QT_TRANSLATE_NOOP("textpalette", "Devanagari") },
    { 0x0980, 0x09FF, QT_TRANSLATE_NOOP("textpalette", "Bengali") },
    { 0x0A00, 0x0A7F, QT_TRANSLATE_NOOP("textpalette", "Gurmukhi") },
    { 0x0A80, 0x0AFF, QT_TRANSLATE_NOOP("textpalette", "Gujarati") },
    { 0x0B00, 0x0B7F, QT_TRANSLATE_NOOP("textpalette", "Oriya") },
    { 0x0B80, 0x0BFF, QT_TRANSLATE_NOOP("textpalette", "Tamil") },
    { 0x0C00, 0x0C7F, QT_TRANSLATE_NOOP("textpalette", "Telugu") },
    { 0x0C80, 0x0CFF, QT_TRANSLATE_NOOP("textpalette", "Kannada") },
    { 0x0D00, 0x0D7F, QT_TRANSLATE_NOOP("textpalette", "Malayalam") },
    { 0x0D80, 0x0DFF, QT_TRANSLATE_NOOP("textpalette", "Sinhala") },
    { 0x0E00, 0x0E7F, QT_TRANSLATE_NOOP("textpalette", "Thai") },
    { 0x0E80, 0x0EFF, QT_TRANSLATE_NOOP("textpalette", "Lao") },
    { 0x0F00, 0x0FFF, QT_TRANSLATE_NOOP("textpalette", "Tibetan") },
    { 0x1000, 0x109F, QT_TRANSLATE_NOOP("textpalette", "Myanmar") },
    { 0x10A0, 0x10FF, QT_TRANSLATE_NOOP("textpalette", "Georgian") },
    { 0x1100, 0x11FF, QT_TRANSLATE_NOOP("textpalette", "Hangul Jamo") },
    { 0x1200, 0x137F, QT_TRANSLATE_NOOP("textpalette", "Ethiopic") },
    { 0x1380, 0x139F, QT_TRANSLATE_NOOP("textpalette", "Ethiopic Supplement") },
    { 0x13A0, 0x13FF, QT_TRANSLATE_NOOP("textpalette", "Cherokee") },
    { 0x1400, 0x167F, QT_TRANSLATE_NOOP("textpalette", "Unified Canadian Aboriginal Syllabics") },
    { 0x1680, 0x169F, QT_TRANSLATE_NOOP("textpalette", "Ogham") },
    { 0x16A0, 0x16FF, QT_TRANSLATE_NOOP("textpalette", "Runic") },
    { 0x1700, 0x171F, QT_TRANSLATE_NOOP("textpalette", "Tagalog") },
    { 0x1720, 0x173F, QT_TRANSLATE_NOOP("textpalette", "Hanunoo") },
    { 0x1740, 0x175F, QT_TRANSLATE_NOOP("textpalette", "Buhid") },
    { 0x1760, 0x177F, QT_TRANSLATE_NOOP("textpalette", "Tagbanwa") },
    { 0x1780, 0x17FF, QT_TRANSLATE_NOOP("textpalette", "Khmer") },
    { 0x1800, 0x18AF, QT_TRANSLATE_NOOP("textpalette", "Mongolian") },
    { 0x18B0, 0x18FF, QT_TRANSLATE_NOOP("textpalette", "Unified Canadian Aboriginal Syllabics Extended") },
    { 0x1900, 0x194F, QT_TRANSLATE_NOOP("textpalette", "Limbu") },
    { 0x1950, 0x197F, QT_TRANSLATE_NOOP("textpalette", "Tai Le") },
    { 0x1980, 0x19DF, QT_TRANSLATE_NOOP("textpalette", "New Tai Lue") },
    { 0x19E0, 0x19FF, QT_TRANSLATE_NOOP("textpalette", "Khmer Symbols") },
    { 0x1A00, 0x1A1F, QT_TRANSLATE_NOOP("textpalette", "Buginese") },
    { 0x1A20, 0x1AAF, QT_TRANSLATE_NOOP("textpalette", "Tai Tham") },
    { 0x1AB0, 0x1AFF, QT_TRANSLATE_NOOP("textpalette", "Combining Diacritical Marks Extended") },
    { 0x1B00, 0x1B7F, QT_TRANSLATE_NOOP("textpalette", "Balinese") },
    { 0x1B80, 0x1BBF, QT_TRANSLATE_NOOP("textpalette", "Sundanese") },
    { 0x1BC0, 0x1BFF, QT_TRANSLATE_NOOP("textpalette", "Batak") },
    { 0x1C00, 0x1C4F, QT_TRANSLATE_NOOP("textpalette", "Lepcha") },
    { 0x1C50, 0x1C7F, QT_TRANSLATE_NOOP("textpalette", "Ol Chiki") },
    { 0x1C80, 0x1C8F, QT_TRANSLATE_NOOP("textpalette", "Cyrillic Extended-C") },
    { 0x1C90, 0x1CBF, QT_TRANSLATE_NOOP("textpalette", "Georgian Extended") },
    { 0x1CC0, 0x1CCF, QT_TRANSLATE_NOOP("textpalette", "Sundanese Supplement") },
    { 0x1CD0, 0x1CFF, QT_TRANSLATE_NOOP("textpalette", "Vedic Extensions") },
    { 0x1D00, 0x1D7F, QT_TRANSLATE_NOOP("textpalette", "Phonetic Extensions") },
    { 0x1D80, 0x1DBF, QT_TRANSLATE_NOOP("textpalette", "Phonetic Extensions Supplement") },
    { 0x1DC0, 0x1DFF, QT_TRANSLATE_NOOP("textpalette", "Combining Diacritical Marks Supplement") },
    { 0x1E00, 0x1EFF, QT_TRANSLATE_NOOP("textpalette", "Latin Extended Additional") },
    { 0x1F00, 0x1FFF, QT_TRANSLATE_NOOP("textpalette", "Greek Extended") },
    { 0x2000, 0x206F, QT_TRANSLATE_NOOP("textpalette", "General Punctuation") },
    { 0x2070, 0x209F, QT_TRANSLATE_NOOP("textpalette", "Superscripts and Subscripts") },
    { 0x20A0, 0x20CF, QT_TRANSLATE_NOOP("textpalette", "Currency Symbols") },
    { 0x20D0, 0x20FF, QT_TRANSLATE_NOOP("textpalette", "Combining Diacritical Marks for Symbols") },
    { 0x2100, 0x214F, QT_TRANSLATE_NOOP("textpalette", "Letterlike Symbols") },
    { 0x2150, 0x218F, QT_TRANSLATE_NOOP("textpalette", "Number Forms") },
    { 0x2190, 0x21FF, QT_TRANSLATE_NOOP("textpalette", "Arrows") },
    { 0x2200, 0x22FF, QT_TRANSLATE_NOOP("textpalette", "Mathematical Operators") },
    { 0x2300, 0x23FF, QT_TRANSLATE_NOOP("textpalette", "Miscellaneous Technical") },
    { 0x2400, 0x243F, QT_TRANSLATE_NOOP("textpalette", "Control Pictures") },
    { 0x2440, 0x245F, QT_TRANSLATE_NOOP("textpalette", "Optical Character Recognition") },
    { 0x2460, 0x24FF, QT_TRANSLATE_NOOP("textpalette", "Enclosed Alphanumerics") },
    { 0x2500, 0x257F, QT_TRANSLATE_NOOP("textpalette", "Box Drawing") },
    { 0x2580, 0x259F, QT_TRANSLATE_NOOP("textpalette", "Block Elements") },
    { 0x25A0, 0x25FF, QT_TRANSLATE_NOOP("textpalette", "Geometric Shapes") },
    { 0x2600, 0x26FF, QT_TRANSLATE_NOOP("textpalette", "Miscellaneous Symbols") },
    { 0x2700, 0x27BF, QT_TRANSLATE_NOOP("textpalette", "Dingbats") },
    { 0x27C0, 0x27EF, QT_TRANSLATE_NOOP("textpalette", "Miscellaneous Mathematical Symbols-A") },
    { 0x27F0, 0x27FF, QT_TRANSLATE_NOOP("textpalette", "Supplemental Arrows-A") },
    { 0x2800, 0x28FF, QT_TRANSLATE_NOOP("textpalette", "Braille Patterns") },
    { 0x2900, 0x297F, QT_TRANSLATE_NOOP("textpalette", "Supplemental Arrows-B") },
    { 0x2980, 0x29FF, QT_TRANSLATE_NOOP("textpalette", "Miscellaneous Mathematical Symbols-B") },
    { 0x2A00, 0x2AFF, QT_TRANSLATE_NOOP("textpalette", "Supplemental Mathematical Operators") },
    { 0x2B00, 0x2BFF, QT_TRANSLATE_NOOP("textpalette", "Miscellaneous Symbols and Arrows") },
    { 0x2C00, 0x2C5F, QT_TRANSLATE_NOOP("textpalette", "Glagolitic") },
    { 0x2C60, 0x2C7F, QT_TRANSLATE_NOOP("textpalette", "Latin Extended-C") },
    { 0x2C80, 0x2CFF, QT_TRANSLATE_NOOP("textpalette", "Coptic") },
    { 0x2D00, 0x2D2F, QT_TRANSLATE_NOOP("textpalette", "Georgian Supplement") },
    { 0x2D30, 0x2D7F, QT_TRANSLATE_NOOP("textpalette", "Tifinagh") },
    { 0x2D80, 0x2DDF, QT_TRANSLATE_NOOP("textpalette", "Ethiopic Extended") },
    { 0x2DE0, 0x2DFF, QT_TRANSLATE_NOOP("textpalette", "Cyrillic Extended-A") },
    { 0x2E00, 0x2E7F, QT_TRANSLATE_NOOP("textpalette", "Supplemental Punctuation") },
    { 0x2E80, 0x2EFF, QT_TRANSLATE_NOOP("textpalette", "CJK Radicals Supplement") },
    { 0x2F00, 0x2FDF, QT_TRANSLATE_NOOP("textpalette", "Kangxi Radicals") },
    { 0x2FF0, 0x2FFF, QT_TRANSLATE_NOOP("textpalette", "Ideographic Description Characters") },
    { 0x3000, 0x303F, QT_TRANSLATE_NOOP("textpalette", "CJK Symbols and Punctuation") },
    { 0x3040, 0x309F, QT_TRANSLATE_NOOP("textpalette", "Hiragana") },
    { 0x30A0, 0x30FF, QT_TRANSLATE_NOOP("textpalette", "Katakana") },
    { 0x3100, 0x312F, QT_TRANSLATE_NOOP("textpalette", "Bopomofo") },
    { 0x3130, 0x318F, QT_TRANSLATE_NOOP("textpalette", "Hangul Compatibility Jamo") },
    { 0x3190, 0x319F, QT_TRANSLATE_NOOP("textpalette", "Kanbun") },
    { 0x31A0, 0x31BF, QT_TRANSLATE_NOOP("textpalette", "Bopomofo Extended") },
    { 0x31C0, 0x31EF, QT_TRANSLATE_NOOP("textpalette", "CJK Strokes") },
    { 0x31F0, 0x31FF, QT_TRANSLATE_NOOP("textpalette", "Katakana Phonetic Extensions") },
    { 0x3200, 0x32FF, QT_TRANSLATE_NOOP("textpalette", "Enclosed CJK Letters and Months") },
    { 0x3300, 0x33FF, QT_TRANSLATE_NOOP("textpalette", "CJK Compatibility") },
    { 0x3400, 0x4DB5, QT_TRANSLATE_NOOP("textpalette", "CJK Unified Ideographs Extension A") },
    { 0x4DC0, 0x4DFF, QT_TRANSLATE_NOOP("textpalette", "Yijing Hexagram Symbols") },
    { 0x4E00, 0x9FCC, QT_TRANSLATE_NOOP("textpalette", "CJK Unified Ideographs") },
    { 0xA000, 0xA48F, QT_TRANSLATE_NOOP("textpalette", "Yi Syllables") },
    { 0xA490, 0xA4CF, QT_TRANSLATE_NOOP("textpalette", "Yi Radicals") },
    { 0xA4D0, 0xA4FF, QT_TRANSLATE_NOOP("textpalette", "Lisu") },
    { 0xA500, 0xA63F, QT_TRANSLATE_NOOP("textpalette", "Vai") },
    { 0xA640, 0xA69F, QT_TRANSLATE_NOOP("textpalette", "Cyrillic Extended-B") },
    { 0xA6A0, 0xA6FF, QT_TRANSLATE_NOOP("textpalette", "Bamum") },
    { 0xA700, 0xA71F, QT_TRANSLATE_NOOP("textpalette", "Modifier Tone Letters") },
    { 0xA720, 0xA7FF, QT_TRANSLATE_NOOP("textpalette", "Latin Extended-D") },
    { 0xA800, 0xA82F, QT_TRANSLATE_NOOP("textpalette", "Syloti Nagri") },
    { 0xA830, 0xA83F, QT_TRANSLATE_NOOP("textpalette", "Common Indic Number Forms") },
    { 0xA840, 0xA87F, QT_TRANSLATE_NOOP("textpalette", "Phags-pa") },
    { 0xA880, 0xA8DF, QT_TRANSLATE_NOOP("textpalette", "Saurashtra") },
    { 0xA8E0, 0xA8FF, QT_TRANSLATE_NOOP("textpalette", "Devanagari Extended") },
    { 0xA900, 0xA92F, QT_TRANSLATE_NOOP("textpalette", "Kayah Li") },
    { 0xA930, 0xA95F, QT_TRANSLATE_NOOP("textpalette", "Rejang") },
    { 0xA960, 0xA97F, QT_TRANSLATE_NOOP("textpalette", "Hangul Jamo Extended-A") },
    { 0xA980, 0xA9DF, QT_TRANSLATE_NOOP("textpalette", "Javanese") },
    { 0xA9E0, 0xA9FF, QT_TRANSLATE_NOOP("textpalette", "Myanmar Extended-B") },
    { 0xAA00, 0xAA5F, QT_TRANSLATE_NOOP("textpalette", "Cham") },
    { 0xAA60, 0xAA7F, QT_TRANSLATE_NOOP("textpalette", "Myanmar Extended-A") },
    { 0xAA80, 0xAADF, QT_TRANSLATE_NOOP("textpalette", "Tai Viet") },
    { 0xAAE0, 0xAAFF, QT_TRANSLATE_NOOP("textpalette", "Meetei Mayek Extensions") },
    { 0xAB00, 0xAB2F, QT_TRANSLATE_NOOP("textpalette", "Ethiopic Extended-A") },
    { 0xAB30, 0xAB6F, QT_TRANSLATE_NOOP("textpalette", "Latin Extended-E") },
    { 0xAB70, 0xABBF, QT_TRANSLATE_NOOP("textpalette", "Cherokee Supplement") },
    { 0xABC0, 0xABFF, QT_TRANSLATE_NOOP("textpalette", "Meetei Mayek") },
    { 0xAC00, 0xD7A3, QT_TRANSLATE_NOOP("textpalette", "Hangul Syllables") },
    { 0xD7B0, 0xD7FF, QT_TRANSLATE_NOOP("textpalette", "Hangul Jamo Extended-B") },
    { 0xD800, 0xDB7F, QT_TRANSLATE_NOOP("textpalette", "High Surrogates") },
    { 0xDB80, 0xDBFF, QT_TRANSLATE_NOOP("textpalette", "High Private Use Surrogates") },
    { 0xDC00, 0xDFFF, QT_TRANSLATE_NOOP("textpalette", "Low Surrogates") },
    { 0xE000, 0xF8FF, QT_TRANSLATE_NOOP("textpalette", "Private Use Area"), },
    { 0xF900, 0xFAFF, QT_TRANSLATE_NOOP("textpalette", "CJK Compatibility Ideographs"), },
    { 0xFB00, 0xFB4F, QT_TRANSLATE_NOOP("textpalette", "Alphabetic Presentation Forms"), },
    { 0xFB50, 0xFDFF, QT_TRANSLATE_NOOP("textpalette", "Arabic Presentation Forms-A"), },
    { 0xFE00, 0xFE0F, QT_TRANSLATE_NOOP("textpalette", "Variation Selectors"), },
    { 0xFE10, 0xFE1F, QT_TRANSLATE_NOOP("textpalette", "Vertical Forms"), },
    { 0xFE20, 0xFE2F, QT_TRANSLATE_NOOP("textpalette", "Combining Half Marks"), },
    { 0xFE30, 0xFE4F, QT_TRANSLATE_NOOP("textpalette", "CJK Compatibility Forms"), },
    { 0xFE50, 0xFE6F, QT_TRANSLATE_NOOP("textpalette", "Small Form Variants"), },
    { 0xFE70, 0xFEFF, QT_TRANSLATE_NOOP("textpalette", "Arabic Presentation Forms-B"), },
    { 0xFF00, 0xFFEF, QT_TRANSLATE_NOOP("textpalette", "Halfwidth and Fullwidth Forms"), },
    { 0xFFF0, 0xFFFF, QT_TRANSLATE_NOOP("textpalette", "Specials"), },
    { 0x10000, 0x1007F, QT_TRANSLATE_NOOP("textpalette", "Linear B Syllabary") },
    { 0x10080, 0x100FF, QT_TRANSLATE_NOOP("textpalette", "Linear B Ideograms") },
    { 0x10100, 0x1013F, QT_TRANSLATE_NOOP("textpalette", "Aegean Numbers") },
    { 0x10140, 0x1018F, QT_TRANSLATE_NOOP("textpalette", "Ancient Greek Numbers") },
    { 0x10190, 0x101CF, QT_TRANSLATE_NOOP("textpalette", "Ancient Symbols") },
    { 0x101D0, 0x101FF, QT_TRANSLATE_NOOP("textpalette", "Phaistos Disc") },
    { 0x10280, 0x1029F, QT_TRANSLATE_NOOP("textpalette", "Lycian") },
    { 0x102A0, 0x102DF, QT_TRANSLATE_NOOP("textpalette", "Carian") },
    { 0x102E0, 0x102FF, QT_TRANSLATE_NOOP("textpalette", "Coptic Epact Numbers") },
    { 0x10300, 0x1032F, QT_TRANSLATE_NOOP("textpalette", "Old Italic") },
    { 0x10330, 0x1034F, QT_TRANSLATE_NOOP("textpalette", "Gothic") },
    { 0x10350, 0x1037F, QT_TRANSLATE_NOOP("textpalette", "Old Permic") },
    { 0x10380, 0x1039F, QT_TRANSLATE_NOOP("textpalette", "Ugaritic") },
    { 0x103A0, 0x103DF, QT_TRANSLATE_NOOP("textpalette", "Old Persian") },
    { 0x10400, 0x1044F, QT_TRANSLATE_NOOP("textpalette", "Deseret") },
    { 0x10450, 0x1047F, QT_TRANSLATE_NOOP("textpalette", "Shavian") },
    { 0x10480, 0x104AF, QT_TRANSLATE_NOOP("textpalette", "Osmanya") },
    { 0x104B0, 0x104FF, QT_TRANSLATE_NOOP("textpalette", "Osage") },
    { 0x10500, 0x1052F, QT_TRANSLATE_NOOP("textpalette", "Elbasan") },
    { 0x10530, 0x1056F, QT_TRANSLATE_NOOP("textpalette", "Caucasian Albanian") },
    { 0x10600, 0x1077F, QT_TRANSLATE_NOOP("textpalette", "Linear A") },
    { 0x10800, 0x1083F, QT_TRANSLATE_NOOP("textpalette", "Cypriot Syllabary") },
    { 0x10840, 0x1085F, QT_TRANSLATE_NOOP("textpalette", "Imperial Aramaic") },
    { 0x10860, 0x1087F, QT_TRANSLATE_NOOP("textpalette", "Palmyrene") },
    { 0x10880, 0x108AF, QT_TRANSLATE_NOOP("textpalette", "Nabataean") },
    { 0x108E0, 0x108EF, QT_TRANSLATE_NOOP("textpalette", "Hatran") },
    { 0x10900, 0x1091F, QT_TRANSLATE_NOOP("textpalette", "Phoenician") },
    { 0x10920, 0x1093F, QT_TRANSLATE_NOOP("textpalette", "Lydian") },
    { 0x10980, 0x1099F, QT_TRANSLATE_NOOP("textpalette", "Meroitic Hieroglyphs") },
    { 0x109A0, 0x109FF, QT_TRANSLATE_NOOP("textpalette", "Meroitic Cursive") },
    { 0x10A00, 0x10A5F, QT_TRANSLATE_NOOP("textpalette", "Kharoshthi") },
    { 0x10A60, 0x10A7F, QT_TRANSLATE_NOOP("textpalette", "Old South Arabian") },
    { 0x10A80, 0x10A9F, QT_TRANSLATE_NOOP("textpalette", "Old North Arabian") },
    { 0x10AC0, 0x10AFF, QT_TRANSLATE_NOOP("textpalette", "Manichaean") },
    { 0x10B00, 0x10B3F, QT_TRANSLATE_NOOP("textpalette", "Avestan") },
    { 0x10B40, 0x10B5F, QT_TRANSLATE_NOOP("textpalette", "Inscriptional Parthian") },
    { 0x10B60, 0x10B7F, QT_TRANSLATE_NOOP("textpalette", "Inscriptional Pahlavi") },
    { 0x10B80, 0x10BAF, QT_TRANSLATE_NOOP("textpalette", "Psalter Pahlavi") },
    { 0x10C00, 0x10C4F, QT_TRANSLATE_NOOP("textpalette", "Old Turkic") },
    { 0x10C80, 0x10CFF, QT_TRANSLATE_NOOP("textpalette", "Old Hungarian") },
    { 0x10D00, 0x10D3F, QT_TRANSLATE_NOOP("textpalette", "Hanifi Rohingya") },
    { 0x10E60, 0x10E7F, QT_TRANSLATE_NOOP("textpalette", "Rumi Numeral Symbols") },
    { 0x10E80, 0x10EBF, QT_TRANSLATE_NOOP("textpalette", "Yezidi") },
    { 0x10F00, 0x10F2F, QT_TRANSLATE_NOOP("textpalette", "Old Sogdian") },
    { 0x10F30, 0x10F6F, QT_TRANSLATE_NOOP("textpalette", "Sogdian") },
    { 0x10FB0, 0x10FDF, QT_TRANSLATE_NOOP("textpalette", "Chorasmian") },
    { 0x10FE0, 0x10FFF, QT_TRANSLATE_NOOP("textpalette", "Elymaic") },
    { 0x11000, 0x1107F, QT_TRANSLATE_NOOP("textpalette", "Brahmi") },
    { 0x11080, 0x110CF, QT_TRANSLATE_NOOP("textpalette", "Kaithi") },
    { 0x110D0, 0x110FF, QT_TRANSLATE_NOOP("textpalette", "Sora Sompeng") },
    { 0x11100, 0x1114F, QT_TRANSLATE_NOOP("textpalette", "Chakma") },
    { 0x11150, 0x1117F, QT_TRANSLATE_NOOP("textpalette", "Mahajani") },
    { 0x11180, 0x111DF, QT_TRANSLATE_NOOP("textpalette", "Sharada") },
    { 0x111E0, 0x111FF, QT_TRANSLATE_NOOP("textpalette", "Sinhala Archaic Numbers") },
    { 0x11200, 0x1124F, QT_TRANSLATE_NOOP("textpalette", "Khojki") },
    { 0x11280, 0x112AF, QT_TRANSLATE_NOOP("textpalette", "Multani") },
    { 0x112B0, 0x112FF, QT_TRANSLATE_NOOP("textpalette", "Khudawadi") },
    { 0x11300, 0x1137F, QT_TRANSLATE_NOOP("textpalette", "Grantha") },
    { 0x11400, 0x1147F, QT_TRANSLATE_NOOP("textpalette", "Newa") },
    { 0x11480, 0x114DF, QT_TRANSLATE_NOOP("textpalette", "Tirhuta") },
    { 0x11580, 0x115FF, QT_TRANSLATE_NOOP("textpalette", "Siddham") },
    { 0x11600, 0x1165F, QT_TRANSLATE_NOOP("textpalette", "Modi") },
    { 0x11660, 0x1167F, QT_TRANSLATE_NOOP("textpalette", "Mongolian Supplement") },
    { 0x11680, 0x116CF, QT_TRANSLATE_NOOP("textpalette", "Takri") },
    { 0x11700, 0x1173F, QT_TRANSLATE_NOOP("textpalette", "Ahom") },
    { 0x11800, 0x1184F, QT_TRANSLATE_NOOP("textpalette", "Dogra") },
    { 0x118A0, 0x118FF, QT_TRANSLATE_NOOP("textpalette", "Warang Citi") },
    { 0x11900, 0x1195F, QT_TRANSLATE_NOOP("textpalette", "Dives Akuru") },
    { 0x116A0, 0x119FF, QT_TRANSLATE_NOOP("textpalette", "Nandinagari") },
    { 0x11A00, 0x11A4F, QT_TRANSLATE_NOOP("textpalette", "Zanabazer Square") },
    { 0x11A50, 0x11AAF, QT_TRANSLATE_NOOP("textpalette", "Soyombo") },
    { 0x11AC0, 0x11AFF, QT_TRANSLATE_NOOP("textpalette", "Pau Cin Hau") },
    { 0x11C00, 0x11C6F, QT_TRANSLATE_NOOP("textpalette", "Bhaiksuki") },
    { 0x11C70, 0x11CBF, QT_TRANSLATE_NOOP("textpalette", "Marchen") },
    { 0x11D00, 0x11D5F, QT_TRANSLATE_NOOP("textpalette", "Masaram Gondi") },
    { 0x11D60, 0x11DAF, QT_TRANSLATE_NOOP("textpalette", "Gunjala Gondi") },
    { 0x11EE0, 0x11EFF, QT_TRANSLATE_NOOP("textpalette", "Makasar") },
    { 0x11FB0, 0x11FBF, QT_TRANSLATE_NOOP("textpalette", "Lisu Supplement") },
    { 0x11FC0, 0x11FFF, QT_TRANSLATE_NOOP("textpalette", "Tamil Supplement") },
    { 0x12000, 0x123FF, QT_TRANSLATE_NOOP("textpalette", "Cuneiform") },
    { 0x12400, 0x1247F, QT_TRANSLATE_NOOP("textpalette", "Cuneiform Numbers and Punctuation") },
    { 0x12480, 0x1254F, QT_TRANSLATE_NOOP("textpalette", "Early Dynastic Cuneiform") },
    { 0x13000, 0x1342F, QT_TRANSLATE_NOOP("textpalette", "Egyptian Hieroglyphs") },
    { 0x13430, 0x1343F, QT_TRANSLATE_NOOP("textpalette", "Egyptian Hieroglyph Format Controls") },
    { 0x14400, 0x1467F, QT_TRANSLATE_NOOP("textpalette", "Anatolian Hieroglyphs") },
    { 0x16800, 0x16A3F, QT_TRANSLATE_NOOP("textpalette", "Bamum Supplement") },
    { 0x16A40, 0x16A6F, QT_TRANSLATE_NOOP("textpalette", "Mro") },
    { 0x16AD0, 0x16AFF, QT_TRANSLATE_NOOP("textpalette", "Bassa Vah") },
    { 0x16B00, 0x16B8F, QT_TRANSLATE_NOOP("textpalette", "Pahawh Hmong") },
    { 0x16E40, 0x16E9F, QT_TRANSLATE_NOOP("textpalette", "Medefaidrin") },
    { 0x16F00, 0x16F9F, QT_TRANSLATE_NOOP("textpalette", "Miao") },
    { 0x16FE0, 0x16FFF, QT_TRANSLATE_NOOP("textpalette", "Ideographic Symbols and Punctuation") },
    { 0x18800, 0x18AFF, QT_TRANSLATE_NOOP("textpalette", "Tangut Components") },
    { 0x18B00, 0x16CFF, QT_TRANSLATE_NOOP("textpalette", "Khitan Small Script") },
    { 0x18D00, 0x16D8F, QT_TRANSLATE_NOOP("textpalette", "Tangut Supplement") },
    { 0x1B000, 0x1B0FF, QT_TRANSLATE_NOOP("textpalette", "Kana Supplement") },
    { 0x1B100, 0x1B12F, QT_TRANSLATE_NOOP("textpalette", "Kana Extended-A") },
    { 0x1B130, 0x1B16F, QT_TRANSLATE_NOOP("textpalette", "Small Kana Extension") },
    { 0x1B170, 0x1B2FF, QT_TRANSLATE_NOOP("textpalette", "Nushu") },
    { 0x1BC00, 0x1BC9F, QT_TRANSLATE_NOOP("textpalette", "Duployan") },
    { 0x1BCA0, 0x1BCAF, QT_TRANSLATE_NOOP("textpalette", "Shorthand Format Controls") },
    { 0x1D000, 0x1D0FF, QT_TRANSLATE_NOOP("textpalette", "Byzantine Musical Symbols") },
    { 0x1D100, 0x1D1FF, QT_TRANSLATE_NOOP("textpalette", "Musical Symbols") },
    { 0x1D200, 0x1D24F, QT_TRANSLATE_NOOP("textpalette", "Ancient Greek Musical Notation") },
    { 0x1D300, 0x1D35F, QT_TRANSLATE_NOOP("textpalette", "Tai Xuan Jing Symbols") },
    { 0x1D360, 0x1D37F, QT_TRANSLATE_NOOP("textpalette", "Counting Rod Numerals") },
    { 0x1D400, 0x1D7FF, QT_TRANSLATE_NOOP("textpalette", "Mathematical Alphanumeric Symbols") },
    { 0x1D800, 0x1DAAF, QT_TRANSLATE_NOOP("textpalette", "Sutton SignWriting") },
    { 0x1E000, 0x1E02F, QT_TRANSLATE_NOOP("textpalette", "Glagolitic Supplement") },
    { 0x1E100, 0x1E14F, QT_TRANSLATE_NOOP("textpalette", "Nyiakeng Puachue Hmong") },
    { 0x1E2C0, 0x1E2FF, QT_TRANSLATE_NOOP("textpalette", "Wancho") },
    { 0x1E800, 0x1E8DF, QT_TRANSLATE_NOOP("textpalette", "Mende Kikakui") },
    { 0x1E900, 0x1E95F, QT_TRANSLATE_NOOP("textpalette", "Adlam") },
    { 0x1EC70, 0x1ECBF, QT_TRANSLATE_NOOP("textpalette", "Indic Siyaq Numbers") },
    { 0x1ED00, 0x1ED4F, QT_TRANSLATE_NOOP("textpalette", "Ottoman Siyaq Numbers") },
    { 0x1EE00, 0x1EEFF, QT_TRANSLATE_NOOP("textpalette", "Arabic Mathematical Alphabetic Symbols") },
    { 0x1F000, 0x1F02F, QT_TRANSLATE_NOOP("textpalette", "Mahjong Tiles") },
    { 0x1F030, 0x1F09F, QT_TRANSLATE_NOOP("textpalette", "Domino Tiles") },
    { 0x1F0A0, 0x1F0FF, QT_TRANSLATE_NOOP("textpalette", "Playing Cards") },
    { 0x1F100, 0x1F1FF, QT_TRANSLATE_NOOP("textpalette", "Enclosed Alphanumeric Supplement") },
    { 0x1F200, 0x1F2FF, QT_TRANSLATE_NOOP("textpalette", "Enclosed Ideographic Supplement") },
    { 0x1F300, 0x1F5FF, QT_TRANSLATE_NOOP("textpalette", "Miscellaneous Symbols and Pictographs") },
    { 0x1F600, 0x1F64F, QT_TRANSLATE_NOOP("textpalette", "Emoticons (Emoji)") },
    { 0x1F650, 0x1F67F, QT_TRANSLATE_NOOP("textpalette", "Ornamental Dingbats") },
    { 0x1F680, 0x1F6FF, QT_TRANSLATE_NOOP("textpalette", "Transport and Map Symbols") },
    { 0x1F700, 0x1F77F, QT_TRANSLATE_NOOP("textpalette", "Alchemical Symbols") },
    { 0x1F780, 0x1F7FF, QT_TRANSLATE_NOOP("textpalette", "Geometric Shapes Extended") },
    { 0x1F800, 0x1F8FF, QT_TRANSLATE_NOOP("textpalette", "Supplemental Arrows-C") },
    { 0x1F900, 0x1F9FF, QT_TRANSLATE_NOOP("textpalette", "Supplemental Symbols and Pictographs") },
    { 0x1FA00, 0x1FA6F, QT_TRANSLATE_NOOP("textpalette", "Chess Symbols") },
    { 0x1FA70, 0x1FAFF, QT_TRANSLATE_NOOP("textpalette", "Symbols and Pictographs Extended-A") },
    { 0x1FB00, 0x1FBFF, QT_TRANSLATE_NOOP("textpalette", "Symbols for Legacy Computing") },
    { 0x20000, 0x2A6D6, QT_TRANSLATE_NOOP("textpalette", "CJK Unified Ideographs Extension B") },
    { 0x2A700, 0x2B734, QT_TRANSLATE_NOOP("textpalette", "CJK Unified Ideographs Extension C") },
    { 0x2B740, 0x2B81D, QT_TRANSLATE_NOOP("textpalette", "CJK Unified Ideographs Extension D") },
    { 0x2F800, 0x2F81D, QT_TRANSLATE_NOOP("textpalette", "CJK Compatibility Ideographs Supplement") },
    { 0xE0001, 0xE007F, QT_TRANSLATE_NOOP("textpalette", "Tags") },
    { 0xE0100, 0xE01EF, QT_TRANSLATE_NOOP("textpalette", "Variation Selectors Supplement") },
    { 0xF0000, 0xFFFFD, QT_TRANSLATE_NOOP("textpalette", "Supplementary Private Use Area-A") },
    { 0x100000, 0x10FFFD, QT_TRANSLATE_NOOP("textpalette", "Supplementary Private Use Area-B") }
};

//---------------------------------------------------------
//   SpecialCharactersDialog
//---------------------------------------------------------

SpecialCharactersDialog::SpecialCharactersDialog(QWidget* parent)
    : QDialog(parent)
{
    setObjectName(SPECIAL_CHARACTERS_DIALOG_NAME);
    setWindowFlags(Qt::Tool);
    setupUi(this);

    m_pCommon = new PaletteWidget;
    m_pCommon->setMag(0.8);
    m_pCommon->setGridSize(33, 60);
    m_pCommon->setReadOnly(true);

    m_pSmufl = new PaletteWidget;
    m_pSmufl->setMag(0.8);
    m_pSmufl->setGridSize(33, 60);
    m_pSmufl->setReadOnly(true);

    m_pUnicode = new PaletteWidget;
    m_pUnicode->setMag(0.8);
    m_pUnicode->setGridSize(33, 60);
    m_pUnicode->setReadOnly(true);

    PaletteScrollArea* psa = new PaletteScrollArea(m_pCommon);
    psa->setRestrictHeight(false);

    tabWidget->clear();
    tabWidget->addTab(psa, mu::qtrc("palette", "Common Symbols"));

    psa = new PaletteScrollArea(m_pSmufl);
    psa->setRestrictHeight(false);

    QSplitter* ws = new QSplitter;
    m_lws = new QListWidget;

    m_lws->addItems(mu::smuflRanges()->keys());
    m_lws->setCurrentRow(0);

    ws->addWidget(m_lws);
    ws->addWidget(psa);

    tabWidget->addTab(ws, mu::qtrc("palette", "Musical Symbols"));

    psa = new PaletteScrollArea(m_pUnicode);
    psa->setRestrictHeight(false);

    QSplitter* wu = new QSplitter;
    m_lwu = new QListWidget;
    m_lwu->setSortingEnabled(true);

    int index = 0;
    for (const UnicodeRange& range : unicodeRanges) {
        QListWidgetItem* newItem = new QListWidgetItem(mu::qtrc("textpalette", range.name));
        newItem->setData(Qt::UserRole, index);
        m_lwu->addItem(newItem);
        if (index == 0) {
            m_lwu->setCurrentItem(newItem);
        }
        index++;
    }

    wu->addWidget(m_lwu);
    wu->addWidget(psa);

    tabWidget->addTab(wu, mu::qtrc("palette", "Unicode Symbols"));

    connect(m_lws, &QListWidget::currentRowChanged, this, &SpecialCharactersDialog::populateSmufl);
    connect(m_lwu, &QListWidget::currentRowChanged, this, &SpecialCharactersDialog::populateUnicode);

    // others are done in setFont
    populateSmufl();

    setFocusPolicy(Qt::NoFocus);

    INotationPtr notation = globalContext()->currentNotation();
    INotationInteractionPtr interaction = notation ? notation->interaction() : nullptr;

    if (interaction) {
        const TextBase* editedText = interaction->editedText();
        setFont(editedText->font());
    }

    WidgetStateStore::restoreGeometry(this);
}

SpecialCharactersDialog::SpecialCharactersDialog(const SpecialCharactersDialog& other)
    : QDialog(other.parentWidget())
{
}

void SpecialCharactersDialog::hideEvent(QHideEvent* event)
{
    WidgetStateStore::saveGeometry(this);
    QDialog::hideEvent(event);
}

int SpecialCharactersDialog::static_metaTypeId()
{
    return QMetaType::type(SPECIAL_CHARACTERS_DIALOG_NAME.toStdString().c_str());
}

//---------------------------------------------------------
//   populateCommon
//---------------------------------------------------------

static constexpr int unicodeAccidentals[] = { //better size and alignment, so put these first
    0x266d,      // flat
    0x266e,      // natural
    0x266f       // sharp
    // not supported in most fonts
    // 0x1d12b,   // double flat
    // 0x1d12a    // double sharp
};

static constexpr int commonTextSymbols[] = {
    0x00a9,      // &copy;

    // upper case ligatures
    0x00c0,      // &Agrave;
    0x00c1,      // &Aacute;
    0x00c2,      // &Acirc;
    0x00c3,      // &Atilde;
    0x00c4,      // &Auml;
    0x00c5,      // &Aring;
    0x00c6,      // &AElig;
    0x00c7,      // &Ccedil;
    0x00c8,      // &Egrave;
    0x00c9,      // &Eacute;
    0x00ca,      // &Ecirc;
    0x00cb,      // &Euml;
    0x00cc,      // &Igrave;
    0x00cd,      // &Iacute;
    0x00ce,      // &Icirc;
    0x00cf,      // &Iuml

    0x00d0,      // &ETH;
    0x00d1,      // &Ntilde;
    0x00d2,      // &Ograve;
    0x00d3,      // &Oacute;
    0x00d4,      // &Ocirc;
    0x00d5,      // &Otilde;
    0x00d6,      // &Ouml;
    0x00d7,      // &times;
    0x00d8,      // &Oslash;
    0x00d9,      // &Ugrave;
    0x00da,      // &Uacute;
    0x00db,      // &Ucirc;
    0x00dc,      // &Uuml;
    0x00dd,      // &Yacute;
    0x00de,      // &THORN;
    0x00df,      // &szlig;

    // upper case letters esperanto
    0x0108,      // &Ccirc; ?
    0x011c,      // &Gcirc; ?
    0x0124,      // &Hcirc; ?
    0x0134,      // &Jcirc; ?
    0x0152,      // &OElig;
    0x015c,      // &Scirc; ?
    0x0160,      // &Scaron;
    0x016c,      // &Ucaron; ?
    0x0178,      // &Yuml;

    // lower case ligatures
    0x00e0,      // &agrave;
    0x00e1,      // &aacute;
    0x00e2,      // &acirc;
    0x00e3,      // &atilde;
    0x00e4,      // &auml;
    0x00e5,      // &aring;
    0x00e6,      // &aelig;
    0x00e7,      // &ccdil;
    0x00e8,      // &egrave;
    0x00e9,      // &eacute;
    0x00ea,      // &ecirc;
    0x00eb,      // &euml;
    0x00ec,      // igrave;
    0x00ed,      // &iacute;
    0x00ee,      // &icirc;
    0x00ef,      // &iuml;

    0x00f0,      // &eth;
    0x00f1,      // &ntilde;
    0x00f2,      // &ograve;
    0x00f3,      // &oacute;
    0x00f4,      // &ocirc;
    0x00f5,      // &otilde;
    0x00f6,      // &ouml;
    0x00f7,      // &divide;
    0x00f8,      // &oslash;
    0x00f9,      // &ugrave;
    0x00fa,      // &uacute;
    0x00fb,      // &ucirc;
    0x00fc,      // &uuml;
    0x00fd,      // &yacute;
    0x00fe,      // &thorn;
    0x00ff,      // &yuml;

    // lower case letters esperanto
    0x0109,      // &ccirc; ?
    0x011d,      // &gcirc; ?
    0x0125,      // &hcirc; ?
    0x0135,      // &jcirc; ?
    0x0153,      // &oelig;
    0x015d,      // &scirc; ?
    0x0161,      // &scaron;
    0x016d,      // &ucaron; ?

    0x00BC,      // &frac14;
    0x00BD,      // &frac12;
    0x00BE,      // &frac34;
    0x2153,      // &frac13; ?
    0x2154,      // &frac23; ?
    0x2155,      // &frac15; ?
    0x2156,      // &frac25; ?
    0x2157,      // &frac35; ?
    0x2158,      // &frac45; ?
    0x2159,      // &frac16; ?
    0x215A,      // &frac56; ?
    0x215B,      // &frac18; ?
    0x215C,      // &frac38; ?
    0x215D,      // &frac58; ?
    0x215E,      // &frac78; ?

    //0x203F,    // undertie
    0x035c,      // curved ligature to connect two syllables
    //0x2040,    // character tie
    0x0361,      // curved ligature (top)

    0x2013,      // &endash
    0x2014,      // &emdash
    0x2018,      // &rsquo;
    0x2019,      // &lsquo;
    0x201C,      // &ldquo;
    0x201D,      // &rdquo;
    0x2020,      // &dagger;
    0x2021,      // &Dagger;
    0x2022,      // &bull;
    0x2026,      // &mldr;
    0x00A7,      // &sect;
    0x00B0,      // &deg;
    0x00B1,      // &pm;
    0x00B9,      // &sup1;
    0x00B2,      // &sup2;
    0x00B3,      // &sup3;

    0x0024,      // &dollar;
    0x20Ac,      // &euro;
    0x00A2,      // &cent;
    0x00A3,      // &pound;
    0x00A5,      // &yen;
};

void SpecialCharactersDialog::populateCommon()
{
    m_pCommon->clear();

    for (auto id : unicodeAccidentals) {
        std::shared_ptr<FSymbol> fs = std::make_shared<FSymbol>(gpaletteScore->dummy());
        fs->setCode(id);
        fs->setFont(m_font);
        m_pCommon->appendElement(fs, QString(id));
    }

    for (auto id : commonScoreSymbols) {
        std::shared_ptr<Symbol> s = std::make_shared<Symbol>(gpaletteScore->dummy());
        s->setSym(id, gpaletteScore->scoreFont());
        m_pCommon->appendElement(s, SymNames::translatedUserNameForSymId(id));
    }

    for (auto id : commonTextSymbols) {
        std::shared_ptr<FSymbol> fs = std::make_shared<FSymbol>(gpaletteScore->dummy());
        fs->setCode(id);
        fs->setFont(m_font);
        m_pCommon->appendElement(fs, QString(id));
    }
}

//---------------------------------------------------------
//   populateSmufl
//---------------------------------------------------------

void SpecialCharactersDialog::populateSmufl()
{
    int row = m_lws->currentRow();
    QString key = mu::smuflRanges()->keys().at(row);
    QStringList smuflNames = (*mu::smuflRanges())[key];

    m_pSmufl->clear();
    for (const QString& name : smuflNames) {
        auto symId = SymNames::symIdByName(name);
        std::shared_ptr<Symbol> s = std::make_shared<Symbol>(gpaletteScore->dummy());
        s->setSym(symId, gpaletteScore->scoreFont());
        m_pSmufl->appendElement(s, SymNames::translatedUserNameForSymId(symId));
    }
}

//---------------------------------------------------------
//   populateUnicode
//---------------------------------------------------------

void SpecialCharactersDialog::populateUnicode()
{
    int row = m_lwu->currentItem()->data(Qt::UserRole).toInt();
    const UnicodeRange& range = unicodeRanges[row];
    m_pUnicode->clear();
    for (int code = range.first; code <= range.last; ++code) {
        std::shared_ptr<FSymbol> fs = std::make_shared<FSymbol>(gpaletteScore->dummy());
        fs->setCode(code);
        fs->setFont(m_font);
        m_pUnicode->appendElement(fs, QString("0x%1").arg(code, 5, 16, QLatin1Char('0')));
    }
}

void SpecialCharactersDialog::setFont(const mu::draw::Font& font)
{
    m_font = font;
    m_font.setPointSizeF(20);
    populateUnicode();
    populateCommon();
    update();
}
}
