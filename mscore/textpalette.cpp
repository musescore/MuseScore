//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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

#include "palette.h"
#include "textpalette.h"
#include "icons.h"
#include "libmscore/text.h"
#include "libmscore/sym.h"
#include "libmscore/symbol.h"
#include "libmscore/style.h"
#include "libmscore/clef.h"
#include "libmscore/score.h"
#include "musescore.h"

namespace Ms {

extern QMap<QString, QStringList>* smuflRanges();

//const int buttonSize = 40;
//const int iconSize   = 20;
//const int fontSize   = 20;


const QPoint rangeInfo[241] = {
      { 0x0020, 0x007F },
      { 0x0080, 0x00FF },
      { 0x0100, 0x017F },
      { 0x0180, 0x024F },
      { 0x0250, 0x02AF },
      { 0x02B0, 0x02FF },
      { 0x0300, 0x036F },
      { 0x0370, 0x03FF },
      { 0x0400, 0x04FF },
      { 0x0500, 0x052F },
      { 0x0530, 0x058F },
      { 0x0590, 0x05FF },
      { 0x0600, 0x06FF },
      { 0x0700, 0x074F },
      { 0x0750, 0x077F },
      { 0x0780, 0x07BF },
      { 0x07C0, 0x07FF },
      { 0x0800, 0x083F },
      { 0x0840, 0x085F },
      { 0x08A0, 0x08FF },
      { 0x0900, 0x097F },
      { 0x0980, 0x09FF },
      { 0x0A00, 0x0A7F },
      { 0x0A80, 0x0AFF },
      { 0x0B00, 0x0B7F },
      { 0x0B80, 0x0BFF },
      { 0x0C00, 0x0C7F },
      { 0x0C80, 0x0CFF },
      { 0x0D00, 0x0D7F },
      { 0x0D80, 0x0DFF },
      { 0x0E00, 0x0E7F },
      { 0x0E80, 0x0EFF },
      { 0x0F00, 0x0FFF },
      { 0x1000, 0x109F },
      { 0x10A0, 0x10FF },
      { 0x1100, 0x11FF },
      { 0x1200, 0x137F },
      { 0x1380, 0x139F },
      { 0x13A0, 0x13FF },
      { 0x1400, 0x167F },
      { 0x1680, 0x169F },
      { 0x16A0, 0x16FF },
      { 0x1700, 0x171F },
      { 0x1720, 0x173F },
      { 0x1740, 0x175F },
      { 0x1760, 0x177F },
      { 0x1780, 0x17FF },
      { 0x1800, 0x18AF },
      { 0x18B0, 0x18FF },
      { 0x1900, 0x194F },
      { 0x1950, 0x197F },
      { 0x1980, 0x19DF },
      { 0x19E0, 0x19FF },
      { 0x1A00, 0x1A1F },
      { 0x1A20, 0x1AAF },
      { 0x1AB0, 0x1AFF },
      { 0x1B00, 0x1B7F },
      { 0x1B80, 0x1BBF },
      { 0x1BC0, 0x1BFF },
      { 0x1C00, 0x1C4F },
      { 0x1C50, 0x1C7F },
      { 0x1CC0, 0x1CCF },
      { 0x1CD0, 0x1CFF },
      { 0x1D00, 0x1D7F },
      { 0x1D80, 0x1DBF },
      { 0x1DC0, 0x1DFF },
      { 0x1E00, 0x1EFF },
      { 0x1F00, 0x1FFF },
      { 0x2000, 0x206F },
      { 0x2070, 0x209F },
      { 0x20A0, 0x20CF },
      { 0x20D0, 0x20FF },
      { 0x2100, 0x214F },
      { 0x2150, 0x218F },
      { 0x2190, 0x21FF },
      { 0x2200, 0x22FF },
      { 0x2300, 0x23FF },
      { 0x2400, 0x243F },
      { 0x2440, 0x245F },
      { 0x2460, 0x24FF },
      { 0x2500, 0x257F },
      { 0x2580, 0x259F },
      { 0x25A0, 0x25FF },
      { 0x2600, 0x26FF },
      { 0x2700, 0x27BF },
      { 0x27C0, 0x27EF },
      { 0x27F0, 0x27FF },
      { 0x2800, 0x28FF },
      { 0x2900, 0x297F },
      { 0x2980, 0x29FF },
      { 0x2A00, 0x2AFF },
      { 0x2B00, 0x2BFF },
      { 0x2C00, 0x2C5F },
      { 0x2C60, 0x2C7F },
      { 0x2C80, 0x2CFF },
      { 0x2D00, 0x2D2F },
      { 0x2D30, 0x2D7F },
      { 0x2D80, 0x2DDF },
      { 0x2DE0, 0x2DFF },
      { 0x2E00, 0x2E7F },
      { 0x2E80, 0x2EFF },
      { 0x2F00, 0x2FDF },
      { 0x2FF0, 0x2FFF },
      { 0x3000, 0x303F },
      { 0x3040, 0x309F },
      { 0x30A0, 0x30FF },
      { 0x3100, 0x312F },
      { 0x3130, 0x318F },
      { 0x3190, 0x319F },
      { 0x31A0, 0x31BF },
      { 0x31C0, 0x31EF },
      { 0x31F0, 0x31FF },
      { 0x3200, 0x32FF },
      { 0x3300, 0x33FF },
      { 0x3400, 0x4DB5 },
      { 0x4DC0, 0x4DFF },
      { 0x4E00, 0x9FCC },
      { 0xA000, 0xA48F },
      { 0xA490, 0xA4CF },
      { 0xA4D0, 0xA4FF },
      { 0xA500, 0xA63F },
      { 0xA640, 0xA69F },
      { 0xA6A0, 0xA6FF },
      { 0xA700, 0xA71F },
      { 0xA720, 0xA7FF },
      { 0xA800, 0xA82F },
      { 0xA830, 0xA83F },
      { 0xA840, 0xA87F },
      { 0xA880, 0xA8DF },
      { 0xA8E0, 0xA8FF },
      { 0xA900, 0xA92F },
      { 0xA930, 0xA95F },
      { 0xA960, 0xA97F },
      { 0xA980, 0xA9DF },
      { 0xA9E0, 0xA9FF },
      { 0xAA00, 0xAA5F },
      { 0xAA60, 0xAA7F },
      { 0xAA80, 0xAADF },
      { 0xAAE0, 0xAAFF },
      { 0xAB00, 0xAB2F },
      { 0xAB30, 0xAB6F },
      { 0xABC0, 0xABFF },
      { 0xAC00, 0xD7A3 },
      { 0xD7B0, 0xD7FF },
      //{ 0xD800, 0xDB7F },
      //{ 0xDB80, 0xDBFF },
      //{ 0xDC00, 0xDFFF },
      { 0xE000, 0xF8FF },
      { 0xF900, 0xFAFF },
      { 0xFB00, 0xFB4F },
      { 0xFB50, 0xFDFF },
      { 0xFE00, 0xFE0F },
      { 0xFE10, 0xFE1F },
      { 0xFE20, 0xFE2F },
      { 0xFE30, 0xFE4F },
      { 0xFE50, 0xFE6F },
      { 0xFE70, 0xFEFF },
      { 0xFF00, 0xFFEF },
      { 0xFFF0, 0xFFFF },
      { 0x10000, 0x1007F },
      { 0x10080, 0x100FF },
      { 0x10100, 0x1013F },
      { 0x10140, 0x1018F },
      { 0x10190, 0x101CF },
      { 0x101D0, 0x101FF },
      { 0x10280, 0x1029F },
      { 0x102A0, 0x102DF },
      { 0x102E0, 0x102FF },
      { 0x10300, 0x1032F },
      { 0x10330, 0x1034F },
      { 0x10350, 0x1037F },
      { 0x10380, 0x1039F },
      { 0x103A0, 0x103DF },
      { 0x10400, 0x1044F },
      { 0x10450, 0x1047F },
      { 0x10480, 0x104AF },
      { 0x10500, 0x1052F },
      { 0x10530, 0x1056F },
      { 0x10600, 0x1077F },
      { 0x10800, 0x1083F },
      { 0x10840, 0x1085F },
      { 0x10860, 0x1087F },
      { 0x10880, 0x108AF },
      { 0x10900, 0x1091F },
      { 0x10920, 0x1093F },
      { 0x10980, 0x1099F },
      { 0x109A0, 0x109FF },
      { 0x10A00, 0x10A5F },
      { 0x10A60, 0x10A7F },
      { 0x10A80, 0x10A9F },
      { 0x10AC0, 0x10AFF },
      { 0x10B00, 0x10B3F },
      { 0x10B40, 0x10B5F },
      { 0x10B60, 0x10B7F },
      { 0x10B80, 0x10BAF },
      { 0x10C00, 0x10C4F },
      { 0x10E60, 0x10E7F },
      { 0x11000, 0x1107F },
      { 0x11080, 0x110CF },
      { 0x110D0, 0x110FF },
      { 0x11100, 0x1114F },
      { 0x11150, 0x1117F },
      { 0x11180, 0x111DF },
      { 0x111E0, 0x111FF },
      { 0x11200, 0x1124F },
      { 0x112B0, 0x112FF },
      { 0x11300, 0x1137F },
      { 0x11480, 0x114DF },
      { 0x11580, 0x115FF },
      { 0x11600, 0x1165F },
      { 0x11680, 0x116CF },
      { 0x118A0, 0x118FF },
      { 0x11AC0, 0x11AFF },
      { 0x12000, 0x123FF },
      { 0x12400, 0x1247F },
      { 0x13000, 0x1342F },
      { 0x16800, 0x16A3F },
      { 0x16A40, 0x16A6F },
      { 0x16AD0, 0x16AFF },
      { 0x16B00, 0x16B8F },
      { 0x16F00, 0x16F9F },
      { 0x1B000, 0x1B0FF },
      { 0x1BC00, 0x1BC9F },
      { 0x1BCA0, 0x1BCAF },
      { 0x1D000, 0x1D0FF },
      { 0x1D100, 0x1D1FF },
      { 0x1D200, 0x1D24F },
      { 0x1D300, 0x1D35F },
      { 0x1D360, 0x1D37F },
      { 0x1D400, 0x1D7FF },
      { 0x1E800, 0x1E8DF },
      { 0x1EE00, 0x1EEFF },
      { 0x1F000, 0x1F02F },
      { 0x1F030, 0x1F09F },
      { 0x1F0A0, 0x1F0FF },
      { 0x1F100, 0x1F1FF },
      { 0x1F200, 0x1F2FF },
      { 0x1F300, 0x1F5FF },
      { 0x1F600, 0x1F64F },
      { 0x1F650, 0x1F67F },
      { 0x1F680, 0x1F6FF },
      { 0x1F700, 0x1F77F },
      { 0x1F780, 0x1F7FF },
      { 0x1F800, 0x1F8FF }
      };

const QStringList unicodeRangeNames = {
      QT_TRANSLATE_NOOP("textpalette", "Basic Latin"),
      QT_TRANSLATE_NOOP("textpalette", "Latin-1 Supplement"),
      QT_TRANSLATE_NOOP("textpalette", "Latin Extended-A"),
      QT_TRANSLATE_NOOP("textpalette", "Latin Extended-B"),
      QT_TRANSLATE_NOOP("textpalette", "IPA Extensions"),
      QT_TRANSLATE_NOOP("textpalette", "Spacing Modifier Letters"),
      QT_TRANSLATE_NOOP("textpalette", "Combining Diacritical Marks"),
      QT_TRANSLATE_NOOP("textpalette", "Greek and Coptic"),
      QT_TRANSLATE_NOOP("textpalette", "Cyrillic"),
      QT_TRANSLATE_NOOP("textpalette", "Cyrillic Supplement"),
      QT_TRANSLATE_NOOP("textpalette", "Armenian"),
      QT_TRANSLATE_NOOP("textpalette", "Hebrew"),
      QT_TRANSLATE_NOOP("textpalette", "Arabic"),
      QT_TRANSLATE_NOOP("textpalette", "Syriac"),
      QT_TRANSLATE_NOOP("textpalette", "Arabic Supplement"),
      QT_TRANSLATE_NOOP("textpalette", "Thaana"),
      QT_TRANSLATE_NOOP("textpalette", "NKo"),
      QT_TRANSLATE_NOOP("textpalette", "Samaritan"),
      QT_TRANSLATE_NOOP("textpalette", "Mandaic"),
      QT_TRANSLATE_NOOP("textpalette", "Arabic Extended-A"),
      QT_TRANSLATE_NOOP("textpalette", "Devanagari"),
      QT_TRANSLATE_NOOP("textpalette", "Bengali"),
      QT_TRANSLATE_NOOP("textpalette", "Gurmukhi"),
      QT_TRANSLATE_NOOP("textpalette", "Gujarati"),
      QT_TRANSLATE_NOOP("textpalette", "Oriya"),
      QT_TRANSLATE_NOOP("textpalette", "Tamil"),
      QT_TRANSLATE_NOOP("textpalette", "Telugu"),
      QT_TRANSLATE_NOOP("textpalette", "Kannada"),
      QT_TRANSLATE_NOOP("textpalette", "Malayalam"),
      QT_TRANSLATE_NOOP("textpalette", "Sinhala"),
      QT_TRANSLATE_NOOP("textpalette", "Thai"),
      QT_TRANSLATE_NOOP("textpalette", "Lao"),
      QT_TRANSLATE_NOOP("textpalette", "Tibetan"),
      QT_TRANSLATE_NOOP("textpalette", "Myanmar"),
      QT_TRANSLATE_NOOP("textpalette", "Georgian"),
      QT_TRANSLATE_NOOP("textpalette", "Hangul Jamo"),
      QT_TRANSLATE_NOOP("textpalette", "Ethiopic"),
      QT_TRANSLATE_NOOP("textpalette", "Ethiopic Supplement"),
      QT_TRANSLATE_NOOP("textpalette", "Cherokee"),
      QT_TRANSLATE_NOOP("textpalette", "Unified Canadian Aboriginal Syllabics Extended"),
      QT_TRANSLATE_NOOP("textpalette", "Ogham"),
      QT_TRANSLATE_NOOP("textpalette", "Runic"),
      QT_TRANSLATE_NOOP("textpalette", "Tagalog"),
      QT_TRANSLATE_NOOP("textpalette", "Hanunoo"),
      QT_TRANSLATE_NOOP("textpalette", "Buhid"),
      QT_TRANSLATE_NOOP("textpalette", "Tagbanwa"),
      QT_TRANSLATE_NOOP("textpalette", "Khmer"),
      QT_TRANSLATE_NOOP("textpalette", "Mongolian"),
      QT_TRANSLATE_NOOP("textpalette", "Unified Canadian Aboriginal Syllabics"),
      QT_TRANSLATE_NOOP("textpalette", "Limbu"),
      QT_TRANSLATE_NOOP("textpalette", "Tai Le"),
      QT_TRANSLATE_NOOP("textpalette", "New Tai Lue"),
      QT_TRANSLATE_NOOP("textpalette", "Khmer Symbols"),
      QT_TRANSLATE_NOOP("textpalette", "Buginese"),
      QT_TRANSLATE_NOOP("textpalette", "Tai Tham"),
      QT_TRANSLATE_NOOP("textpalette", "Combining Diacritical Marks Extended"),
      QT_TRANSLATE_NOOP("textpalette", "Balinese"),
      QT_TRANSLATE_NOOP("textpalette", "Sundanese"),
      QT_TRANSLATE_NOOP("textpalette", "Batak"),
      QT_TRANSLATE_NOOP("textpalette", "Lepcha"),
      QT_TRANSLATE_NOOP("textpalette", "Ol Chiki"),
      QT_TRANSLATE_NOOP("textpalette", "Sundanese Supplement"),
      QT_TRANSLATE_NOOP("textpalette", "Vedic Extensions"),
      QT_TRANSLATE_NOOP("textpalette", "Phonetic Extensions"),
      QT_TRANSLATE_NOOP("textpalette", "Phonetic Extensions Supplement"),
      QT_TRANSLATE_NOOP("textpalette", "Combining Diacritical Marks Supplement"),
      QT_TRANSLATE_NOOP("textpalette", "Latin Extended Additional"),
      QT_TRANSLATE_NOOP("textpalette", "Greek Extended"),
      QT_TRANSLATE_NOOP("textpalette", "General Punctuation"),
      QT_TRANSLATE_NOOP("textpalette", "Superscripts and Subscripts"),
      QT_TRANSLATE_NOOP("textpalette", "Currency Symbols"),
      QT_TRANSLATE_NOOP("textpalette", "Combining Diacritical Marks for Symbol"),
      QT_TRANSLATE_NOOP("textpalette", "Letterlike Symbols"),
      QT_TRANSLATE_NOOP("textpalette", "Number Forms"),
      QT_TRANSLATE_NOOP("textpalette", "Arrows"),
      QT_TRANSLATE_NOOP("textpalette", "Mathematical Operators"),
      QT_TRANSLATE_NOOP("textpalette", "Miscellaneous Technical"),
      QT_TRANSLATE_NOOP("textpalette", "Control Pictures"),
      QT_TRANSLATE_NOOP("textpalette", "Optical Character Recognition"),
      QT_TRANSLATE_NOOP("textpalette", "Enclosed Alphanumerics"),
      QT_TRANSLATE_NOOP("textpalette", "Box Drawing"),
      QT_TRANSLATE_NOOP("textpalette", "Block Elements"),
      QT_TRANSLATE_NOOP("textpalette", "Geometric Shapes"),
      QT_TRANSLATE_NOOP("textpalette", "Miscellaneous Symbols"),
      QT_TRANSLATE_NOOP("textpalette", "Dingbats"),
      QT_TRANSLATE_NOOP("textpalette", "Miscellaneous Mathematical Symbols-A"),
      QT_TRANSLATE_NOOP("textpalette", "Supplemental Arrows-A"),
      QT_TRANSLATE_NOOP("textpalette", "Braille Patterns"),
      QT_TRANSLATE_NOOP("textpalette", "Supplemental Arrows-B"),
      QT_TRANSLATE_NOOP("textpalette", "Miscellaneous Mathematical Symbols-B"),
      QT_TRANSLATE_NOOP("textpalette", "Supplemental Mathematical Operators"),
      QT_TRANSLATE_NOOP("textpalette", "Miscellaneous Symbols and Arrows"),
      QT_TRANSLATE_NOOP("textpalette", "Glagolitic"),
      QT_TRANSLATE_NOOP("textpalette", "Latin Extended-C"),
      QT_TRANSLATE_NOOP("textpalette", "Coptic"),
      QT_TRANSLATE_NOOP("textpalette", "Georgian Supplement"),
      QT_TRANSLATE_NOOP("textpalette", "Tifinagh"),
      QT_TRANSLATE_NOOP("textpalette", "Ethiopic Extended"),
      QT_TRANSLATE_NOOP("textpalette", "Cyrillic Extended-A"),
      QT_TRANSLATE_NOOP("textpalette", "Supplemental Punctuation"),
      QT_TRANSLATE_NOOP("textpalette", "CJK Radicals Supplement"),
      QT_TRANSLATE_NOOP("textpalette", "Kangxi Radicals"),
      QT_TRANSLATE_NOOP("textpalette", "Ideographic Description Characters"),
      QT_TRANSLATE_NOOP("textpalette", "CJK Symbols and Punctuation"),
      QT_TRANSLATE_NOOP("textpalette", "Hiragana"),
      QT_TRANSLATE_NOOP("textpalette", "Katakana"),
      QT_TRANSLATE_NOOP("textpalette", "Bopomofo"),
      QT_TRANSLATE_NOOP("textpalette", "Hangul Compatibility Jamo"),
      QT_TRANSLATE_NOOP("textpalette", "Kanbun"),
      QT_TRANSLATE_NOOP("textpalette", "Bopomofo Extended"),
      QT_TRANSLATE_NOOP("textpalette", "CJK Strokes"),
      QT_TRANSLATE_NOOP("textpalette", "Katakana Phonetic Extensions"),
      QT_TRANSLATE_NOOP("textpalette", "Enclosed CJK Letters and Months"),
      QT_TRANSLATE_NOOP("textpalette", "CJK Compatibility"),
      QT_TRANSLATE_NOOP("textpalette", "CJK Unified Ideographs Extension A"),
      QT_TRANSLATE_NOOP("textpalette", "Yijing Hexagram Symbols"),
      QT_TRANSLATE_NOOP("textpalette", "CJK Unified Ideographs"),
      QT_TRANSLATE_NOOP("textpalette", "Yi Syllables"),
      QT_TRANSLATE_NOOP("textpalette", "Yi Radicals"),
      QT_TRANSLATE_NOOP("textpalette", "Lisu"),
      QT_TRANSLATE_NOOP("textpalette", "Vai"),
      QT_TRANSLATE_NOOP("textpalette", "Cyrillic Extended-B"),
      QT_TRANSLATE_NOOP("textpalette", "Bamum"),
      QT_TRANSLATE_NOOP("textpalette", "Modifier Tone Letters"),
      QT_TRANSLATE_NOOP("textpalette", "Latin Extended-D"),
      QT_TRANSLATE_NOOP("textpalette", "Syloti Nagri"),
      QT_TRANSLATE_NOOP("textpalette", "Common Indic Number Forms"),
      QT_TRANSLATE_NOOP("textpalette", "Phags-pa"),
      QT_TRANSLATE_NOOP("textpalette", "Saurashtra"),
      QT_TRANSLATE_NOOP("textpalette", "Devanagari Extended"),
      QT_TRANSLATE_NOOP("textpalette", "Kayah Li"),
      QT_TRANSLATE_NOOP("textpalette", "Rejang"),
      QT_TRANSLATE_NOOP("textpalette", "Hangul Jamo Extended-A"),
      QT_TRANSLATE_NOOP("textpalette", "Javanese"),
      QT_TRANSLATE_NOOP("textpalette", "Myanmar Extended-B"),
      QT_TRANSLATE_NOOP("textpalette", "Cham"),
      QT_TRANSLATE_NOOP("textpalette", "Myanmar Extended-A"),
      QT_TRANSLATE_NOOP("textpalette", "Tai Viet"),
      QT_TRANSLATE_NOOP("textpalette", "Meetei Mayek Extensions"),
      QT_TRANSLATE_NOOP("textpalette", "Ethiopic Extended-A"),
      QT_TRANSLATE_NOOP("textpalette", "Latin Extended-E"),
      QT_TRANSLATE_NOOP("textpalette", "Meetei Mayek"),
      QT_TRANSLATE_NOOP("textpalette", "Hangul Syllables"),
      QT_TRANSLATE_NOOP("textpalette", "Hangul Jamo Extended-B"),
      //QT_TRANSLATE_NOOP("textpalette", "High Surrogates"),
      //QT_TRANSLATE_NOOP("textpalette", "High Private Use Surrogates"),
      //QT_TRANSLATE_NOOP("textpalette", "Low Surrogates"),
      QT_TRANSLATE_NOOP("textpalette", "Private Use Area"),
      QT_TRANSLATE_NOOP("textpalette", "CJK Compatibility Ideographs"),
      QT_TRANSLATE_NOOP("textpalette", "Alphabetic Presentation Forms"),
      QT_TRANSLATE_NOOP("textpalette", "Arabic Presentation Forms-A"),
      QT_TRANSLATE_NOOP("textpalette", "Variation Selectors"),
      QT_TRANSLATE_NOOP("textpalette", "Vertical Forms"),
      QT_TRANSLATE_NOOP("textpalette", "Combining Half Marks"),
      QT_TRANSLATE_NOOP("textpalette", "CJK Compatibility Forms"),
      QT_TRANSLATE_NOOP("textpalette", "Small Form Variants"),
      QT_TRANSLATE_NOOP("textpalette", "Arabic Presentation Forms-B"),
      QT_TRANSLATE_NOOP("textpalette", "Halfwidth and Fullwidth Forms"),
      QT_TRANSLATE_NOOP("textpalette", "Specials"),
      QT_TRANSLATE_NOOP("textpalette", "Linear B Syllabary"),
      QT_TRANSLATE_NOOP("textpalette", "Linear B Ideograms"),
      QT_TRANSLATE_NOOP("textpalette", "Aegean Numbers"),
      QT_TRANSLATE_NOOP("textpalette", "Ancient Greek Numbers"),
      QT_TRANSLATE_NOOP("textpalette", "Ancient Symbols"),
      QT_TRANSLATE_NOOP("textpalette", "Phaistos Disc"),
      QT_TRANSLATE_NOOP("textpalette", "Lycian"),
      QT_TRANSLATE_NOOP("textpalette", "Carian"),
      QT_TRANSLATE_NOOP("textpalette", "Coptic Epact Numbers"),
      QT_TRANSLATE_NOOP("textpalette", "Old Italic"),
      QT_TRANSLATE_NOOP("textpalette", "Gothic"),
      QT_TRANSLATE_NOOP("textpalette", "Old Permic"),
      QT_TRANSLATE_NOOP("textpalette", "Ugaritic"),
      QT_TRANSLATE_NOOP("textpalette", "Old Persian"),
      QT_TRANSLATE_NOOP("textpalette", "Deseret"),
      QT_TRANSLATE_NOOP("textpalette", "Shavian"),
      QT_TRANSLATE_NOOP("textpalette", "Osmanya"),
      QT_TRANSLATE_NOOP("textpalette", "Elbasan"),
      QT_TRANSLATE_NOOP("textpalette", "Caucasian Albanian"),
      QT_TRANSLATE_NOOP("textpalette", "Linear A"),
      QT_TRANSLATE_NOOP("textpalette", "Cypriot Syllabary"),
      QT_TRANSLATE_NOOP("textpalette", "Imperial Aramaic"),
      QT_TRANSLATE_NOOP("textpalette", "Palmyrene"),
      QT_TRANSLATE_NOOP("textpalette", "Nabataean"),
      QT_TRANSLATE_NOOP("textpalette", "Phoenician"),
      QT_TRANSLATE_NOOP("textpalette", "Lydian"),
      QT_TRANSLATE_NOOP("textpalette", "Meroitic Hieroglyphs"),
      QT_TRANSLATE_NOOP("textpalette", "Meroitic Cursive"),
      QT_TRANSLATE_NOOP("textpalette", "Kharoshthi"),
      QT_TRANSLATE_NOOP("textpalette", "Old South Arabian"),
      QT_TRANSLATE_NOOP("textpalette", "Old North Arabian"),
      QT_TRANSLATE_NOOP("textpalette", "Manichaean"),
      QT_TRANSLATE_NOOP("textpalette", "Avestan"),
      QT_TRANSLATE_NOOP("textpalette", "Inscriptional Parthian"),
      QT_TRANSLATE_NOOP("textpalette", "Inscriptional Pahlavi"),
      QT_TRANSLATE_NOOP("textpalette", "Psalter Pahlavi"),
      QT_TRANSLATE_NOOP("textpalette", "Old Turkic"),
      QT_TRANSLATE_NOOP("textpalette", "Rumi Numeral Symbols"),
      QT_TRANSLATE_NOOP("textpalette", "Brahmi"),
      QT_TRANSLATE_NOOP("textpalette", "Kaithi"),
      QT_TRANSLATE_NOOP("textpalette", "Sora Sompeng"),
      QT_TRANSLATE_NOOP("textpalette", "Chakma"),
      QT_TRANSLATE_NOOP("textpalette", "Mahajani"),
      QT_TRANSLATE_NOOP("textpalette", "Sharada"),
      QT_TRANSLATE_NOOP("textpalette", "Sinhala Archaic Numbers"),
      QT_TRANSLATE_NOOP("textpalette", "Khojki"),
      QT_TRANSLATE_NOOP("textpalette", "Khudawadi"),
      QT_TRANSLATE_NOOP("textpalette", "Grantha"),
      QT_TRANSLATE_NOOP("textpalette", "Tirhuta"),
      QT_TRANSLATE_NOOP("textpalette", "Siddham"),
      QT_TRANSLATE_NOOP("textpalette", "Modi"),
      QT_TRANSLATE_NOOP("textpalette", "Takri"),
      QT_TRANSLATE_NOOP("textpalette", "Warang Citi"),
      QT_TRANSLATE_NOOP("textpalette", "Pau Cin Hau"),
      QT_TRANSLATE_NOOP("textpalette", "Cuneiform"),
      QT_TRANSLATE_NOOP("textpalette", "Cuneiform Numbers and Punctuation"),
      QT_TRANSLATE_NOOP("textpalette", "Egyptian Hieroglyphs"),
      QT_TRANSLATE_NOOP("textpalette", "Bamum Supplement"),
      QT_TRANSLATE_NOOP("textpalette", "Mro"),
      QT_TRANSLATE_NOOP("textpalette", "Bassa Vah"),
      QT_TRANSLATE_NOOP("textpalette", "Pahawh Hmong"),
      QT_TRANSLATE_NOOP("textpalette", "Miao"),
      QT_TRANSLATE_NOOP("textpalette", "Kana Supplement"),
      QT_TRANSLATE_NOOP("textpalette", "Duployan"),
      QT_TRANSLATE_NOOP("textpalette", "Shorthand Format Controls"),
      QT_TRANSLATE_NOOP("textpalette", "Byzantine Musical Symbols"),
      QT_TRANSLATE_NOOP("textpalette", "Musical Symbols"),
      QT_TRANSLATE_NOOP("textpalette", "Ancient Greek Musical Notation"),
      QT_TRANSLATE_NOOP("textpalette", "Tai Xuan Jing Symbols"),
      QT_TRANSLATE_NOOP("textpalette", "Counting Rod Numerals"),
      QT_TRANSLATE_NOOP("textpalette", "Mathematical Alphanumeric Symbols"),
      QT_TRANSLATE_NOOP("textpalette", "Mende Kikakui"),
      QT_TRANSLATE_NOOP("textpalette", "Arabic Mathematical Alphabetic Symbols"),
      QT_TRANSLATE_NOOP("textpalette", "Mahjong Tiles"),
      QT_TRANSLATE_NOOP("textpalette", "Domino Tiles"),
      QT_TRANSLATE_NOOP("textpalette", "Playing Cards"),
      QT_TRANSLATE_NOOP("textpalette", "Enclosed Alphanumeric Supplement"),
      QT_TRANSLATE_NOOP("textpalette", "Enclosed Ideographic Supplement"),
      QT_TRANSLATE_NOOP("textpalette", "Miscellaneous Symbols and Pictographs"),
      QT_TRANSLATE_NOOP("textpalette", "Emoticons (Emoji)"),
      QT_TRANSLATE_NOOP("textpalette", "Ornamental Dingbats"),
      QT_TRANSLATE_NOOP("textpalette", "Transport and Map Symbols"),
      QT_TRANSLATE_NOOP("textpalette", "Alchemical Symbols"),
      QT_TRANSLATE_NOOP("textpalette", "Geometric Shapes Extended"),
      QT_TRANSLATE_NOOP("textpalette", "Supplemental Arrows-C")
      };


//---------------------------------------------------------
//   TextPalette
//---------------------------------------------------------

TextPalette::TextPalette(QWidget* parent)
   : QWidget(parent)
      {
      setObjectName("TextPalette");
      setWindowFlags(Qt::Tool);
      setupUi(this);

      pCommon = new Palette;
      pCommon->setMag(0.8);
      pCommon->setGrid(33, 60);
      pCommon->setReadOnly(true);

      pSmufl = new Palette;
      pSmufl->setMag(0.8);
      pSmufl->setGrid(33, 60);
      pSmufl->setReadOnly(true);

      pUnicode = new Palette;
      pUnicode->setMag(0.8);
      pUnicode->setGrid(33, 60);
      pUnicode->setReadOnly(true);

      PaletteScrollArea* psa = new PaletteScrollArea(pCommon);
      psa->setRestrictHeight(false);

      tabWidget->clear();
      tabWidget->addTab(psa, tr("Common Symbols"));

      psa = new PaletteScrollArea(pSmufl);
      psa->setRestrictHeight(false);

      QSplitter* ws = new QSplitter;
      lws = new QListWidget;

      lws->addItems(smuflRanges()->keys());
      lws->setCurrentRow(0);

      ws->addWidget(lws);
      ws->addWidget(psa);

      tabWidget->addTab(ws, tr("Musical Symbols"));

      psa = new PaletteScrollArea(pUnicode);
      psa->setRestrictHeight(false);

      QSplitter* wu = new QSplitter;
      lwu = new QListWidget;
      lwu->setSortingEnabled(true);
      for (int i = 0; i < unicodeRangeNames.length(); i++) {
            QListWidgetItem* newItem = new QListWidgetItem(qApp->translate("accidental", unicodeRangeNames.at(i).toUtf8().constData()));
            newItem->setData(Qt::UserRole, i);
            lwu->addItem(newItem);
            if (i == 0)
                  lwu->setCurrentItem(newItem);
            }

      wu->addWidget(lwu);
      wu->addWidget(psa);

      tabWidget->addTab(wu, tr("Unicode Symbols"));

      connect(lws, SIGNAL(currentRowChanged(int)), SLOT(populateSmufl()));
      connect(lwu, SIGNAL(currentRowChanged(int)), SLOT(populateUnicode()));

	// others are done in setFont
      populateSmufl();

      setFocusPolicy(Qt::NoFocus);
      MuseScore::restoreGeometry(this);
      }

//---------------------------------------------------------
//   populateCommon
//---------------------------------------------------------

int unicodeAccidentals[] = { //better size and alignment, so put these first
      0x266d,    // flat
      0x266e,    // natural
      0x266f     // sharp
      // not supported in most fonts
      // 0x1d12b,   // double flat
      // 0x1d12a    // double sharp
      };

SymId commonScoreSymbols[] = {
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

int commonTextSymbols[] = {
      0x00a9,    // &copy;

      // upper case ligatures
      0x00c0,    // &Agrave;
      0x00c1,    // &Aacute;
      0x00c2,    // &Acirc;
      0x00c3,    // &Atilde;
      0x00c4,    // &Auml;
      0x00c5,    // &Aring;
      0x00c6,    // &AElig;
      0x00c7,    // &Ccedil;
      0x00c8,    // &Egrave;
      0x00c9,    // &Eacute;
      0x00ca,    // &Ecirc;
      0x00cb,    // &Euml;
      0x00cc,    // %Igrave;
      0x00cd,    // &Iacute;
      0x00ce,    // &Icirc;
      0x00cf,    // &Iuml

      0x00d0,    // &ETH;
      0x00d1,    // &Ntilde;
      0x00d2,    // &Ograve;
      0x00d3,    // &Oacute;
      0x00d4,    // &Ocirc;
      0x00d5,    // &Otilde;
      0x00d6,    // &Ouml;
      0x00d7,    // &times;
      0x00d8,    // &Oslash;
      0x00d9,    // &Ugrave;
      0x00da,    // &Uacute;
      0x00db,    // &Ucirc;
      0x00dc,    // &Uuml;
      0x00dd,    // &Yacute;
      0x00de,    // &THORN;
      0x00df,    // &szlig;

      // upper case letters esperanto
      0x0108,    // &Ccirc; ?
      0x011c,    // &Gcirc; ?
      0x0124,    // &Hcirc; ?
      0x0134,    // &Jcirc; ?
      0x0152,    // &OElig;
      0x015c,    // &Scirc; ?
      0x0160,    // &Scaron;
      0x016c,    // &Ucaron; ?
      0x0178,    // &Yuml;

      // lower case ligatures
      0x00e0,    // &agrave;
      0x00e1,    // &aacute;
      0x00e2,    // &acirc;
      0x00e3,    // &atilde;
      0x00e4,    // &auml;
      0x00e5,    // &aring;
      0x00e6,    // &aelig;
      0x00e7,    // &ccdil;
      0x00e8,    // &egrave;
      0x00e9,    // &eacute;
      0x00ea,    // &ecirc;
      0x00eb,    // &euml;
      0x00ec,    // igrave;
      0x00ed,    // &iacute;
      0x00ee,    // &icirc;
      0x00ef,    // &iuml;

      0x00f0,    // &eth;
      0x00f1,    // &ntilde;
      0x00f2,    // &ograve;
      0x00f3,    // &oacute;
      0x00f4,    // &ocirc;
      0x00f5,    // &otilde;
      0x00f6,    // &ouml;
      0x00f7,    // &divide;
      0x00f8,    // &oslash;
      0x00f9,    // &ugrave;
      0x00fa,    // &uacute;
      0x00fb,    // &ucirc;
      0x00fc,    // &uuml;
      0x00fd,    // &yacute;
      0x00fe,    // &thorn;
      0x00ff,    // &yuml;

      // lower case letters esperanto
      0x0109,    // &ccirc; ?
      0x011d,    // &gcirc; ?
      0x0125,    // &hcirc; ?
      0x0135,    // &jcirc; ?
      0x0153,    // &oelig;
      0x015d,    // &scirc; ?
      0x0161,    // &scaron;
      0x016d,    // &ucaron; ?

      0x00BC,    // &frac14;
      0x00BD,    // &frac12;
      0x00BE,    // &frac34;
      0x2153,    // &frac13; ?
      0x2154,    // &frac23; ?
      0x2155,    // &frac15; ?
      0x2156,    // &frac25; ?
      0x2157,    // &frac35; ?
      0x2158,    // &frac45; ?
      0x2159,    // &frac16; ?
      0x215A,    // &frac56; ?
      0x215B,    // &frac18; ?
      0x215C,    // &frac38; ?
      0x215D,    // &frac58; ?
      0x215E,    // &frac78; ?

      // 0x203F,    // curved ligature to connect two syllables
      0x035c,    // curved ligature to connect two syllables
      0x0361     // curved ligature (top)
      };

void TextPalette::populateCommon()
      {
      pCommon->clear();

      for (auto id : unicodeAccidentals) {
            FSymbol* fs = new FSymbol(gscore);
            fs->setCode(id);
            fs->setFont(_font);
            pCommon->append(fs, QString(id));
            }

      for (auto id : commonScoreSymbols) {
            Symbol* s = new Symbol(gscore);
            s->setSym(id, gscore->scoreFont());
            pCommon->append(s, Sym::id2userName(id));
            }

      for (auto id : commonTextSymbols) {
            FSymbol* fs = new FSymbol(gscore);
            fs->setCode(id);
            fs->setFont(_font);
            pCommon->append(fs, QString(id));
            }
      }

//---------------------------------------------------------
//   populateSmufl
//---------------------------------------------------------

void TextPalette::populateSmufl()
      {
      int row = lws->currentRow();
      QString key = smuflRanges()->keys().at(row);
      QStringList smuflNames = (*smuflRanges())[key];

      pSmufl->clear();
      for (QString name : smuflNames) {
            Symbol* s = new Symbol(gscore);
            s->setSym(Sym::name2id(name), gscore->scoreFont());
            pSmufl->append(s, Sym::id2userName(Sym::name2id(name)));
            }
      }

//---------------------------------------------------------
//   populateUnicode
//---------------------------------------------------------

void TextPalette::populateUnicode()
      {
      int row = lwu->currentItem()->data(Qt::UserRole).toInt();
      QPoint p = rangeInfo[row];
      pUnicode->clear();
      for (int code = p.x(); code <= p.y(); ++code) {
            FSymbol* fs = new FSymbol(gscore);
            fs->setCode(code);
            fs->setFont(_font);
            pUnicode->append(fs, QString("0x%1").arg(code, 5, 16, QLatin1Char('0')));
            }
      }

//---------------------------------------------------------
//   setText
//---------------------------------------------------------

void TextPalette::setText(TextBase* te)
      {
      _textElement = te;
      }

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void TextPalette::hideEvent(QHideEvent* ev)
      {
      MuseScore::saveGeometry(this);
      QWidget::hideEvent(ev);
      }

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void TextPalette::closeEvent(QCloseEvent* ev)
      {
      QWidget::closeEvent(ev);
      getAction("show-keys")->setChecked(false);
      }

//---------------------------------------------------------
//   setFont
//---------------------------------------------------------

void TextPalette::setFont(const QFont& font)
      {
      _font = font;
      _font.setPointSize(20);
      populateUnicode();
      populateCommon();
      update();
      }
}

