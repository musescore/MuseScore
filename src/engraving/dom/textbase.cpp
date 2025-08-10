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

#include <cmath>
#include <stack>

#include "draw/fontmetrics.h"
#include "draw/types/pen.h"
#include "draw/types/brush.h"

#include "iengravingfont.h"

#include "style/textstyle.h"

#include "rw/xmlreader.h"
#include "rw/xmlwriter.h"

#include "types/symnames.h"
#include "types/translatablestring.h"
#include "types/typesconv.h"

#ifndef ENGRAVING_NO_ACCESSIBILITY
#include "accessibility/accessibleitem.h"
#endif

#include "anchors.h"
#include "barline.h"
#include "box.h"
#include "dynamic.h"
#include "instrumentname.h"
#include "measure.h"
#include "mscore.h"
#include "page.h"
#include "score.h"
#include "textedit.h"
#include "textline.h"
#include "undo.h"

#include "log.h"

using namespace mu;
using namespace muse::draw;
using namespace mu::engraving;

namespace mu::engraving {
static constexpr double subScriptSize     = 0.6;
static constexpr double subScriptOffset   = 0.5; // of x-height
static constexpr double superScriptOffset = -0.9; // of x-height

static const char* FALLBACK_SYMBOL_FONT = "Bravura";
static const char* FALLBACK_SYMBOLTEXT_FONT = "Bravura Text";

//---------------------------------------------------------
//   isSorted
/// return true if (r1,c1) is at or before (r2,c2)
//---------------------------------------------------------

bool TextBase::isSorted(size_t r1, size_t c1, size_t r2, size_t c2)
{
    if (r1 < r2) {
        return true;
    }

    if ((r1 == r2) && (c1 <= c2)) {
        return true;
    }

    return false;
}

//---------------------------------------------------------
//   swap
/// swap (r1,c1) with (r2,c2)
//---------------------------------------------------------

void TextBase::swap(size_t& r1, size_t& c1, size_t& r2, size_t& c2)
{
    std::swap(r1, r2);
    std::swap(c1, c2);
}

//---------------------------------------------------------
//   sort
/// swap (r1,c1) with (r2,c2) if they are not sorted
//---------------------------------------------------------

void TextBase::sort(size_t& r1, size_t& c1, size_t& r2, size_t& c2)
{
    if (!isSorted(r1, c1, r2, c2)) {
        swap(r1, c1, r2, c2);
    }
}

const String TextBase::UNDEFINED_FONT_FAMILY = String(u"Undefined");
const double TextBase::UNDEFINED_FONT_SIZE = -1.0;

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool CharFormat::operator==(const CharFormat& cf) const
{
    return cf.style() == style()
           && cf.valign() == valign()
           && cf.fontSize() == fontSize()
           && cf.fontFamily() == fontFamily();
}

//---------------------------------------------------------
//   operator=
//---------------------------------------------------------

CharFormat& CharFormat::operator=(const CharFormat& cf)
{
    setStyle(cf.style());
    setValign(cf.valign());
    setFontSize(cf.fontSize());
    setFontFamily(cf.fontFamily());

    return *this;
}

//---------------------------------------------------------
//   clearSelection
//---------------------------------------------------------

void TextCursor::clearSelection()
{
    m_selectLine   = m_row;
    m_selectColumn = m_column;
}

void TextCursor::startEdit()
{
    setRow(0);
    setColumn(0);
    clearSelection();
    m_editing = true;
}

void TextCursor::endEdit()
{
    setRow(0);
    setColumn(0);
    clearSelection();
    m_editing = false;
}

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void TextCursor::init()
{
    PropertyValue family = m_text->propertyDefault(Pid::FONT_FACE);
    m_format.setFontFamily(family.value<String>());

    PropertyValue size = m_text->propertyDefault(Pid::FONT_SIZE);
    m_format.setFontSize(size.toReal());

    PropertyValue style = m_text->propertyDefault(Pid::FONT_STYLE);
    m_format.setStyle(static_cast<FontStyle>(style.toInt()));

    PropertyValue verticalAlign = m_text->propertyDefault(Pid::TEXT_SCRIPT_ALIGN);
    m_format.setValign(static_cast<VerticalAlignment>(verticalAlign.toInt()));
}

std::pair<size_t, size_t> TextCursor::positionToLocalCoord(int position) const
{
    const TextBase::LayoutData* ldata = m_text->ldata();
    IF_ASSERT_FAILED(ldata) {
        return { muse::nidx, muse::nidx };
    }

    int currentPosition = 0;
    for (size_t i = 0; i < ldata->blocks.size(); ++i) {
        const TextBlock& t = ldata->blocks.at(i);
        for (size_t j = 0; j < t.columns(); ++j) {
            if (currentPosition == position) {
                return { i, j };
            }

            currentPosition++;
        }
    }

    return { muse::nidx, muse::nidx };
}

int TextCursor::currentPosition() const
{
    return position(static_cast<int>(row()), static_cast<int>(column()));
}

TextCursor::Range TextCursor::selectionRange() const
{
    int cursorPosition = currentPosition();
    int selectionPosition = position(static_cast<int>(selectLine()), static_cast<int>(selectColumn()));

    if (cursorPosition > selectionPosition) {
        return range(selectionPosition, cursorPosition);
    } else {
        return range(cursorPosition, selectionPosition);
    }
}

//---------------------------------------------------------
//   columns
//---------------------------------------------------------

size_t TextCursor::columns() const
{
    return m_text->ldata()->textBlock(static_cast<int>(m_row)).columns();
}

//---------------------------------------------------------
//   currentCharacter
//---------------------------------------------------------

Char TextCursor::currentCharacter() const
{
    const TextBase::LayoutData* ldata = m_text->ldata();
    IF_ASSERT_FAILED(ldata) {
        return Char();
    }

    const TextBlock& t = ldata->blocks.at(row());
    String s = t.text(static_cast<int>(column()), 1);
    if (s.isEmpty()) {
        return Char();
    }
    return s.at(0);
}

//---------------------------------------------------------
//   updateCursorFormat
//---------------------------------------------------------

void TextCursor::updateCursorFormat()
{
    const TextBase::LayoutData* ldata = m_text->ldata();
    IF_ASSERT_FAILED(ldata) {
        return;
    }
    const TextBlock& block = ldata->blocks.at(m_row);

    size_t col = hasSelection() ? selectColumn() : column();
    // Get format at the LEFT of the cursor position
    const CharFormat* format = block.formatAt(std::max(static_cast<int>(col) - 1, 0));
    if (!format) {
        init();
    } else {
        setFormat(*format);
    }
}

//---------------------------------------------------------
//   cursorRect
//---------------------------------------------------------

RectF TextCursor::cursorRect() const
{
    const TextBlock& tline       = curLine();
    const TextFragment* fragment = tline.fragment(static_cast<int>(column()));

    Font _font  = fragment ? fragment->font(m_text) : m_text->font();
    if (fragment && _font.type() == Font::Type::MusicSymbol) {
        // Ensure the cursor height matches that of the associated text font
        String textFontId(_font.family().id() + String(u" Text"));
        _font.setFamily(textFontId, Font::Type::MusicSymbolText);
        _font.setPointSizeF(fragment->format.fontSize());
    }

    double ascent = FontMetrics::ascent(_font);
    double h = ascent;
    double x = tline.xpos(column(), m_text);
    double y = tline.y() - ascent * .9;
    return RectF(x, y, 4.0, h);
}

//---------------------------------------------------------
//   curLine
//    return the current text line in edit mode
//---------------------------------------------------------

const TextBlock& TextCursor::curLine() const
{
    const TextBase::LayoutData* ldata = m_text->ldata();
    IF_ASSERT_FAILED(ldata) {
        static TextBlock dummy;
        return dummy;
    }

    return ldata->blocks.at(m_row);
}

TextBlock& TextCursor::curLine()
{
    TextBase::LayoutData* ldata = m_text->mutldata();
    return ldata->blocks[m_row];
}

//---------------------------------------------------------
//   changeSelectionFormat
//---------------------------------------------------------

void TextCursor::changeSelectionFormat(FormatId id, const FormatValue& val)
{
    TextBase::LayoutData* ldata = m_text->mutldata();

    size_t r1 = selectLine();
    size_t r2 = row();
    size_t c1 = selectColumn();
    size_t c2 = column();

    TextBase::sort(r1, c1, r2, c2);

    for (size_t row = 0; row < ldata->blocks.size(); ++row) {
        TextBlock& t = ldata->blocks[row];
        if (row < r1) {
            continue;
        }
        if (row > r2) {
            break;
        }
        if (row == r1 && r1 == r2) {
            t.changeFormat(id, val, static_cast<int>(c1), static_cast<int>(c2 - c1));
        } else if (row == r1) {
            t.changeFormat(id, val, static_cast<int>(c1), static_cast<int>(t.columns() - c1));
        } else if (row == r2) {
            t.changeFormat(id, val, 0, static_cast<int>(c2));
        } else {
            t.changeFormat(id, val, 0, static_cast<int>(t.columns()));
        }
    }

    m_text->renderer()->layoutText1(m_text);
}

const CharFormat TextCursor::selectedFragmentsFormat() const
{
    if (!m_text || (!hasSelection() && editing())) {
        return m_format;
    }

    const TextBase::LayoutData* ldata = m_text->ldata();
    IF_ASSERT_FAILED(ldata) {
        return CharFormat();
    }

    if (ldata->blocks.empty()) {
        return m_format;
    }

    size_t startColumn = hasSelection() ? std::min(selectColumn(), m_column) : 0;
    size_t startRow = hasSelection() ? std::min(selectLine(), m_row) : 0;

    size_t endSelectionRow = hasSelection() ? std::max(selectLine(), m_row) : ldata->blocks.size() - 1;

    const TextFragment* tf = ldata->textBlock(static_cast<int>(startRow)).fragment(static_cast<int>(startColumn));
    CharFormat resultFormat = tf ? tf->format : CharFormat();

    for (size_t row = startRow; row <= endSelectionRow; ++row) {
        const TextBlock& block = ldata->blocks.at(row);

        if (block.fragments().empty()) {
            continue;
        }

        size_t endSelectionColumn = hasSelection() ? std::max(selectColumn(), m_column) : block.columns();

        for (size_t column = startColumn; column < endSelectionColumn; column++) {
            const TextFragment* fragment = block.fragment(static_cast<int>(column));
            CharFormat format = fragment ? fragment->format : CharFormat();

            // proper bitwise 'and' to ensure Bold/Italic/Underline/Strike only true if true for all fragments
            resultFormat.setStyle(static_cast<FontStyle>(static_cast<int>(resultFormat.style()) & static_cast<int>(format.style())));

            if (resultFormat.fontFamily() == "ScoreText") {
                resultFormat.setFontFamily(format.fontFamily());
            }
            if (format.fontFamily() != "ScoreText" && resultFormat.fontFamily() != format.fontFamily()) {
                resultFormat.setFontFamily(TextBase::UNDEFINED_FONT_FAMILY);
            }

            if (resultFormat.fontSize() != format.fontSize()) {
                resultFormat.setFontSize(TextBase::UNDEFINED_FONT_SIZE);
            }

            if (resultFormat.valign() != format.valign()) {
                resultFormat.setValign(VerticalAlignment::AlignUndefined);
            }
        }
    }

    return resultFormat;
}

//---------------------------------------------------------
//   PointF
//---------------------------------------------------------

void TextCursor::setFormat(FormatId id, FormatValue val)
{
    if (!hasSelection()) {
        if (!editing()) {
            m_text->selectAll(this);
        } else if (format()->formatValue(id) == val) {
            return;
        }
    }
    format()->setFormatValue(id, val);
    changeSelectionFormat(id, val);
    if (hasSelection()) {
        text()->setTextInvalid();
    }
    if (!editing()) {
        clearSelection();
    }
}

//---------------------------------------------------------
//   movePosition
//---------------------------------------------------------

bool TextCursor::movePosition(TextCursor::MoveOperation op, TextCursor::MoveMode mode, int count)
{
    for (int i = 0; i < count; i++) {
        switch (op) {
        case TextCursor::MoveOperation::Left:
            if (hasSelection() && mode == TextCursor::MoveMode::MoveAnchor) {
                size_t r1 = m_selectLine;
                size_t r2 = m_row;
                size_t c1 = m_selectColumn;
                size_t c2 = m_column;

                TextBase::sort(r1, c1, r2, c2);
                clearSelection();
                m_row    = r1;
                m_column = c1;
            } else if (m_column == 0) {
                if (m_row == 0) {
                    return false;
                }
                --m_row;
                m_column = curLine().columns();
            } else {
                --m_column;
            }
            break;

        case TextCursor::MoveOperation::Right:
            if (hasSelection() && mode == TextCursor::MoveMode::MoveAnchor) {
                size_t r1 = m_selectLine;
                size_t r2 = m_row;
                size_t c1 = m_selectColumn;
                size_t c2 = m_column;

                TextBase::sort(r1, c1, r2, c2);
                clearSelection();
                m_row    = r2;
                m_column = c2;
            } else if (column() >= curLine().columns()) {
                if (m_row >= m_text->ldata()->rows() - 1) {
                    return false;
                }
                ++m_row;
                m_column = 0;
            } else {
                ++m_column;
            }
            break;

        case TextCursor::MoveOperation::Up:
            if (m_row == 0) {
                return false;
            }
            --m_row;
            if (m_column > curLine().columns()) {
                m_column = curLine().columns();
            }

            break;

        case TextCursor::MoveOperation::Down:
            if (m_row >= m_text->ldata()->rows() - 1) {
                return false;
            }
            ++m_row;
            if (m_column > curLine().columns()) {
                m_column = curLine().columns();
            }

            break;

        case TextCursor::MoveOperation::Start:
            m_row    = 0;
            m_column = 0;

            break;

        case TextCursor::MoveOperation::End:
            m_row    = m_text->ldata()->rows() - 1;
            m_column = curLine().columns();

            break;

        case TextCursor::MoveOperation::StartOfLine:
            m_column = 0;

            break;

        case TextCursor::MoveOperation::EndOfLine:
            m_column = curLine().columns();

            break;

        case TextCursor::MoveOperation::WordLeft:
            if (m_column > 0) {
                --m_column;
                while (m_column > 0 && currentCharacter().isSpace()) {
                    --m_column;
                }
                while (m_column > 0 && !currentCharacter().isSpace()) {
                    --m_column;
                }
                if (currentCharacter().isSpace()) {
                    ++m_column;
                }
            }
            break;

        case TextCursor::MoveOperation::NextWord: {
            size_t cols =  columns();
            if (m_column < cols) {
                ++m_column;
                while (m_column < cols && !currentCharacter().isSpace()) {
                    ++m_column;
                }
                while (m_column < cols && currentCharacter().isSpace()) {
                    ++m_column;
                }
            }
        }
        break;

        default:
            LOGD("Text::movePosition: not implemented");
            return false;
        }
        if (mode == TextCursor::MoveMode::MoveAnchor) {
            clearSelection();
        }
    }

    updateCursorFormat();
    m_text->score()->addRefresh(m_text->canvasBoundingRect());

    return true;
}

//---------------------------------------------------------
//   doubleClickSelect
//---------------------------------------------------------

void TextCursor::selectWord()
{
    clearSelection();

    // if clicked on a space, select surrounding spaces
    // otherwise select surround non-spaces
    const bool selectSpaces = currentCharacter().isSpace();

    //handle double-clicking inside a word
    size_t startPosition = m_column;

    while (m_column > 0 && currentCharacter().isSpace() == selectSpaces) {
        --m_column;
    }

    if (currentCharacter().isSpace() != selectSpaces) {
        ++m_column;
    }

    m_selectColumn = m_column;

    m_column = startPosition;
    while (m_column < curLine().columns() && currentCharacter().isSpace() == selectSpaces) {
        ++m_column;
    }

    updateCursorFormat();
    m_text->score()->addRefresh(m_text->canvasBoundingRect());
}

//---------------------------------------------------------
//   set
//---------------------------------------------------------

bool TextCursor::set(const PointF& p, TextCursor::MoveMode mode)
{
    PointF pt  = p - m_text->canvasPos();
    if (!m_text->ldata()->bbox().contains(pt)) {
        return false;
    }

//      if (_text->_layout.empty())
//            _text->_layout.append(TextBlock());
    m_row = 0;

    const TextBase::LayoutData* ldata = m_text->ldata();
    IF_ASSERT_FAILED(ldata) {
        return false;
    }

    for (size_t row = 0; row < ldata->blocks.size(); ++row) {
        const TextBlock& l = ldata->blocks.at(row);
        if (l.y() > pt.y()) {
            m_row = row;
            break;
        }
    }
    m_column = curLine().column(pt.x(), m_text);

    m_text->score()->setUpdateAll();
    if (mode == TextCursor::MoveMode::MoveAnchor) {
        clearSelection();
    }
    updateCursorFormat();

    return true;
}

//---------------------------------------------------------
//   selectedText
//    return current selection
//---------------------------------------------------------

String TextCursor::selectedText(bool withFormat) const
{
    size_t r1 = selectLine();
    size_t r2 = m_row;
    size_t c1 = selectColumn();
    size_t c2 = column();
    TextBase::sort(r1, c1, r2, c2);
    return extractText(static_cast<int>(r1), static_cast<int>(c1), static_cast<int>(r2), static_cast<int>(c2), withFormat);
}

//---------------------------------------------------------
//   extractText
//    return text between (r1,c1) and (r2,c2).
//---------------------------------------------------------

String TextCursor::extractText(int r1, int c1, int r2, int c2, bool withFormat) const
{
    const TextBase::LayoutData* ldata = m_text->ldata();
    IF_ASSERT_FAILED(ldata) {
        return String();
    }

    assert(TextBase::isSorted(r1, c1, r2, c2));
    const std::vector<TextBlock>& tb = ldata->blocks;

    if (r1 == r2) {
        return tb.at(r1).text(c1, c2 - c1, withFormat);
    }

    String str = tb.at(r1).text(c1, -1, withFormat) + u"\n";

    for (int r = r1 + 1; r < r2; ++r) {
        str += tb.at(r).text(0, -1, withFormat) + u"\n";
    }

    str += tb.at(r2).text(0, c2, withFormat);
    return str;
}

TextCursor::Range TextCursor::range(int start, int end) const
{
    const TextBase::LayoutData* ldata = m_text->ldata();
    IF_ASSERT_FAILED(ldata) {
        return Range();
    }

    String result;
    int pos = 0;
    for (size_t i = 0; i < ldata->blocks.size(); ++i) {
        const TextBlock& t = ldata->blocks.at(i);

        for (size_t j = 0; j < t.columns(); ++j) {
            if (pos > end) {
                return { start, end, result };
            }

            if (start < pos) {
                result += t.text(static_cast<int>(j), 1);
            }

            pos++;
        }
    }

    return { start, end, result };
}

int TextCursor::position(int row, int column) const
{
    const TextBase::LayoutData* ldata = m_text->ldata();
    IF_ASSERT_FAILED(ldata) {
        return 0;
    }

    int result = 0;

    for (int i = 0; i < row; ++i) {
        const TextBlock& t = ldata->blocks.at(i);
        result += static_cast<int>(t.columns());
    }

    result += column;

    return result;
}

//---------------------------------------------------------
//   TextFragment
//---------------------------------------------------------
TextFragment::TextFragment(const String& s)
{
    text = s;
}

TextFragment::TextFragment(TextCursor* cursor, const String& s)
    : TextFragment(s)
{
    format = *cursor->format();
}

TextFragment::TextFragment(const TextFragment& f)
{
    text = f.text;
    format = f.format;
    pos = f.pos;
}

TextFragment& TextFragment::operator =(const TextFragment& f)
{
    text = f.text;
    format = f.format;
    pos = f.pos;
    return *this;
}

//---------------------------------------------------------
//   split
//---------------------------------------------------------

TextFragment TextFragment::split(int column)
{
    size_t idx = 0;
    int col = 0;
    TextFragment f;
    f.format = format;

    for (size_t i = 0; i < text.size(); ++i) {
        const Char& c = text.at(i);
        if (col == column) {
            if (idx) {
                if (idx < text.size()) {
                    f.text = text.mid(idx);
                    text   = text.left(idx);
                }
            }
            return f;
        }
        ++idx;
        if (c.isHighSurrogate()) {
            continue;
        }
        ++col;
    }
    return f;
}

//---------------------------------------------------------
//   columns
//---------------------------------------------------------

int TextFragment::columns() const
{
    int col = 0;
    for (size_t i = 0; i < text.size(); ++i) {
        if (text.at(i).isHighSurrogate()) {
            continue;
        }
        ++col;
    }
    return col;
}

//---------------------------------------------------------
//   operator ==
//---------------------------------------------------------

bool TextFragment::operator ==(const TextFragment& f) const
{
    return format == f.format && text == f.text;
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void TextFragment::draw(Painter* p, const TextBase* t) const
{
    Font f(font(t));
    f.setPointSizeF(f.pointSizeF() * MScore::pixelRatio);
#ifndef Q_OS_MACOS
    TextBase::drawTextWorkaround(p, f, pos, text);
#else
    p->setFont(f);
    p->drawText(pos, text);
#endif
}

//---------------------------------------------------------
//   drawTextWorkaround
//---------------------------------------------------------

void TextBase::drawTextWorkaround(Painter* p, Font& f, const PointF& pos, const String& text)
{
    double mm = p->worldTransform().m11();
    if (!(MScore::pdfPrinting) && (mm < 1.0) && f.bold() && !(f.underline() || f.strike())) {
        p->drawTextWorkaround(f, pos, text);
    } else {
        p->setFont(f);
        p->drawText(pos, text);
    }
}

//---------------------------------------------------------
//   font
//---------------------------------------------------------

Font TextFragment::font(const TextBase* t) const
{
    Font font;

    double m = format.fontSize();
    double spatiumScaling = 0.0;

    if (t->isInstrumentName()) {
        spatiumScaling = toInstrumentName(t)->largestStaffSpatium() / SPATIUM20;
    } else {
        spatiumScaling = t->spatium() / SPATIUM20;
    }

    if (t->sizeIsSpatiumDependent()) {
        m *= spatiumScaling;
    }
    if (format.valign() != VerticalAlignment::AlignNormal) {
        m *= subScriptSize;
    }

    String family;
    Font::Type fontType = Font::Type::Unknown;
    if (format.fontFamily() == "ScoreText") {
        if (t->isDynamic()
            || t->isStringTunings()
            || t->isPlayTechAnnotation()
            || t->textStyleType() == TextStyleType::OTTAVA
            || t->textStyleType() == TextStyleType::HARP_PEDAL_DIAGRAM
            || t->textStyleType() == TextStyleType::TUPLET
            || t->textStyleType() == TextStyleType::PEDAL
            ) {
            std::string fontName = engravingFonts()->fontByName(t->style().styleSt(Sid::musicalSymbolFont).toStdString())->family();
            family = String::fromStdString(fontName);
            fontType = Font::Type::MusicSymbol;
            if (!t->isStringTunings()) {
                m = MUSICAL_SYMBOLS_DEFAULT_FONT_SIZE;
                if (t->isDynamic()) {
                    m *= t->getProperty(Pid::DYNAMICS_SIZE).toDouble() * spatiumScaling;
                    if (t->style().styleB(Sid::dynamicsOverrideFont)) {
                        std::string fontName2 = engravingFonts()->fontByName(t->style().styleSt(Sid::dynamicsFont).toStdString())->family();
                        family = String::fromStdString(fontName2);
                    }
                } else {
                    for (const auto& a : *textStyle(t->textStyleType())) {
                        if (a.type == TextStylePropertyType::MusicalSymbolsScale) {
                            m *= t->style().styleD(a.sid);
                            if (t->sizeIsSpatiumDependent()) {
                                m *= spatiumScaling;
                            }
                            break;
                        }
                    }
                }
            }
            // We use a default font size of 10pt for historical reasons,
            // but Smufl standard is 20pt so multiply x2 here.
            m *= 2;
        } else if (t->isTempoText()) {
            family = t->style().styleSt(Sid::musicalTextFont);
            fontType = Font::Type::MusicSymbolText;
            // to keep desired size ratio (based on 20pt symbol size to 12pt text size)
            m *= 5.0 / 3.0;
        } else if (t->isMarker()) {
            family = t->style().styleSt(Sid::musicalTextFont);
            fontType = Font::Type::MusicSymbolText;
            m = t->getProperty(Pid::MARKER_SYMBOL_SIZE).toDouble();
        } else {
            family = t->style().styleSt(Sid::musicalTextFont);
            fontType = Font::Type::MusicSymbolText;
        }
        // check if all symbols are available
        font.setFamily(family, fontType);
        font.setNoFontMerging(true);
        FontMetrics fm(font);

        bool fail = false;
        for (size_t i = 0; i < text.size(); ++i) {
            const Char& c = text.at(i);
            if (c.isHighSurrogate()) {
                if (i + 1 == text.size()) {
                    ASSERT_X("bad string");
                }
                const Char& c2 = text.at(i + 1);
                ++i;
                char32_t v = Char::surrogateToUcs4(c, c2);
                if (!fm.inFontUcs4(v)) {
                    fail = true;
                    break;
                }
            } else {
                if (!fm.inFont(c)) {
                    fail = true;
                    break;
                }
            }
        }
        if (fail) {
            if (fontType == Font::Type::MusicSymbol) {
                family = String::fromUtf8(FALLBACK_SYMBOL_FONT);
            } else {
                family = String::fromUtf8(FALLBACK_SYMBOLTEXT_FONT);
            }
        }
    } else {
        family = format.fontFamily();
        fontType = Font::Type::Unknown;
        font.setBold(format.bold());
        font.setItalic(format.italic());
        font.setUnderline(format.underline());
        font.setStrike(format.strike());
    }

    font.setFamily(family, fontType);
    assert(m > 0.0);

    font.setPointSizeF(m * t->mag());
    return font;
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void TextBlock::draw(Painter* p, const TextBase* t) const
{
    p->translate(0.0, m_y);
    for (const TextFragment& f : m_fragments) {
        f.draw(p, t);
    }
    p->translate(0.0, -m_y);
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TextBlock::layout(const TextBase* t)
{
    m_shape.clear();
    double x      = 0.0;
    m_lineSpacing = 0.0;
    double lm     = 0.0;

    double layoutWidth = 0;
    EngravingItem* e = t->parentItem();
    // TODO - remove when position is implemented for all text items
    if (e && t->layoutToParentWidth()) {
        layoutWidth = e->width();
        switch (e->type()) {
        case ElementType::HBOX:
        case ElementType::VBOX:
        case ElementType::TBOX: {
            Box* b = toBox(e);
            layoutWidth -= ((b->leftMargin() + b->rightMargin()) * DPMM);
            lm = b->leftMargin() * DPMM;
        }
        break;
        case ElementType::PAGE: {
            Page* p = toPage(e);
            layoutWidth -= (p->lm() + p->rm());
            lm = p->lm();
        }
        break;
        case ElementType::MEASURE: {
            // ignore courtesy keysig, timesig, but fall back if needed
            Measure* m = toMeasure(e);
            const BarLine* bl = m->endBarLine();
            layoutWidth = bl ? bl->segment()->x() + bl->ldata()->bbox().width() : m->width();
        }
        break;
        default:
            break;
        }
    }

    if (m_fragments.empty()) {
        FontMetrics fm = t->fontMetrics();
        m_shape.add(RectF(0.0, -fm.ascent(), 1.0, fm.descent()), t);
        m_lineSpacing = fm.lineSpacing();
    } else if (m_fragments.size() == 1 && m_fragments.front().text.isEmpty()) {
        auto fi = m_fragments.begin();
        TextFragment& f = *fi;
        f.pos.setX(x);
        FontMetrics fm(f.font(t));
        if (f.format.valign() != VerticalAlignment::AlignNormal) {
            double voffset = fm.xHeight() / subScriptSize;   // use original height
            if (f.format.valign() == VerticalAlignment::AlignSubScript) {
                voffset *= subScriptOffset;
            } else {
                voffset *= superScriptOffset;
            }

            f.pos.setY(voffset);
        } else {
            f.pos.setY(0.0);
        }

        RectF temp(0.0, -fm.ascent(), 1.0, fm.descent());
        m_shape.add(temp, t);
        m_lineSpacing = std::max(m_lineSpacing, fm.lineSpacing());
    } else {
        const auto fiLast = --m_fragments.end();
        for (auto fi = m_fragments.begin(); fi != m_fragments.end(); ++fi) {
            TextFragment& f = *fi;
            f.pos.setX(x);
            Font fragmentFont = f.font(t);
            FontMetrics fm(fragmentFont);
            if (f.format.valign() != VerticalAlignment::AlignNormal) {
                double voffset = fm.xHeight() / subScriptSize;           // use original height
                if (f.format.valign() == VerticalAlignment::AlignSubScript) {
                    voffset *= subScriptOffset;
                } else {
                    voffset *= superScriptOffset;
                }
                f.pos.setY(voffset);
            } else {
                f.pos.setY(0.0);
            }

            // Optimization: don't calculate character position
            // for the next fragment if there is no next fragment
            if (fi != fiLast) {
                const double w  = fm.width(f.text);
                x += w;
            }

            double yOffset = musicSymbolBaseLineAdjust(t, f, fi);
            f.pos.ry() -= yOffset;

            RectF textBRect = fm.tightBoundingRect(f.text).translated(f.pos);
            bool useDynamicSymShape = fragmentFont.type() == Font::Type::MusicSymbol && t->isDynamic();
            if (useDynamicSymShape) {
                const Dynamic* dyn = toDynamic(t);
                SymId symId = TConv::symId(dyn->dynamicType());
                if (symId != SymId::noSym) {
                    m_shape.add(dyn->symShapeWithCutouts(symId).translated(f.pos));
                } else {
                    m_shape.add(textBRect, t);
                }
            } else {
                m_shape.add(textBRect, t);
            }
            if (fragmentFont.type() == Font::Type::MusicSymbol || fragmentFont.type() == Font::Type::MusicSymbolText) {
                // SEMI-HACK: Music fonts can have huge linespacing because of tall symbols, so instead of using the
                // font linespacing value we just use the height of the individual fragment with some added margin

                m_lineSpacing = std::max(m_lineSpacing, 1.25 * (m_shape.bbox().height() - m_shape.bbox().bottom()) + yOffset);
            } else {
                m_lineSpacing = std::max(m_lineSpacing, fm.lineSpacing());
            }
        }
    }

    // Apply style/custom line spacing
    m_lineSpacing *= t->textLineSpacing();

    // OLD ALIGN TEXT
    // TODO - remove when position is implemented for all text items
    if (!t->positionSeparateFromAlignment()) {
        double rx = 0;
        AlignH alignH = t->align().horizontal;
        bool dynamicAlwaysCentered = t->isDynamic() && t->getProperty(Pid::CENTER_ON_NOTEHEAD).toBool();

        RectF bbox = m_shape.bbox();
        if (alignH == AlignH::HCENTER || dynamicAlwaysCentered) {
            rx = (layoutWidth - (bbox.left() + bbox.right())) * .5;
        } else if (alignH == AlignH::LEFT) {
            rx = -bbox.left();
        } else if (alignH == AlignH::RIGHT) {
            rx = layoutWidth - bbox.right();
        }

        rx += lm;

        for (TextFragment& f : m_fragments) {
            f.pos.rx() += rx;
        }
        m_shape.translate(PointF(rx, 0.0));
    }
}

//---------------------------------------------------------
//   fragmentsWithoutEmpty
//---------------------------------------------------------

std::list<TextFragment> TextBlock::fragmentsWithoutEmpty()
{
    std::list<TextFragment> list;
    for (const auto& x : m_fragments) {
        if (!x.text.isEmpty()) {
            list.push_back(x);
        }
    }

    return list;
}

//---------------------------------------------------------
//   xpos
//---------------------------------------------------------

double TextBlock::xpos(size_t column, const TextBase* t) const
{
    size_t col = 0;
    for (const TextFragment& f : m_fragments) {
        if (column == col) {
            return f.pos.x();
        }
        FontMetrics fm(f.font(t));
        size_t idx = 0;
        for (size_t i = 0; i < f.text.size(); ++i) {
            ++idx;
            if (f.text.at(i).isHighSurrogate()) {
                continue;
            }
            ++col;
            if (column == col) {
                return f.pos.x() + fm.width(f.text.left(idx));
            }
        }
    }
    return m_shape.bbox().x();
}

//---------------------------------------------------------
//   fragment
//---------------------------------------------------------

const TextFragment* TextBlock::fragment(int column) const
{
    if (m_fragments.empty()) {
        return nullptr;
    }
    int col = 0;
    auto f = m_fragments.begin();
    for (; f != m_fragments.end(); ++f) {
        for (size_t i = 0; i < f->text.size(); ++i) {
            if (f->text.at(i).isHighSurrogate()) {
                continue;
            }
            if (column == col) {
                return &*f;
            }
            ++col;
        }
    }
    if (column == col) {
        return &*(std::prev(f));
    }
    return 0;
}

//---------------------------------------------------------
//   formatAt
//---------------------------------------------------------

const CharFormat* TextBlock::formatAt(int column) const
{
    const TextFragment* f = fragment(column);
    if (f) {
        return &(f->format);
    }
    return 0;
}

//---------------------------------------------------------
//   boundingRect
//---------------------------------------------------------

RectF TextBlock::boundingRect(int col1, int col2, const TextBase* t) const
{
    double x1 = xpos(col1, t);
    double x2 = xpos(col2, t);
    const RectF& bbox = m_shape.bbox();
    return RectF(x1, bbox.y(), x2 - x1, bbox.height());
}

//---------------------------------------------------------
//   columns
//---------------------------------------------------------

size_t TextBlock::columns() const
{
    size_t col = 0;
    for (const TextFragment& f : m_fragments) {
        for (size_t i = 0; i < f.text.size(); ++i) {
            if (!f.text.at(i).isHighSurrogate()) {
                ++col;
            }
        }
    }
    return col;
}

//---------------------------------------------------------
//   column
//    Return nearest column for position x. X is in
//    Text coordinate system
//---------------------------------------------------------

int TextBlock::column(double x, TextBase* t) const
{
    int col = 0;
    for (const TextFragment& f : m_fragments) {
        int idx = 0;
        if (x <= f.pos.x()) {
            return col;
        }
        double px = 0.0;
        for (size_t i = 0; i < f.text.size(); ++i) {
            ++idx;
            if (f.text.at(i).isHighSurrogate()) {
                continue;
            }
            FontMetrics fm(f.font(t));
            double xo = fm.width(f.text.left(idx));
            if (x <= f.pos.x() + px + (xo - px) * .5) {
                return col;
            }
            ++col;
            px = xo;
        }
    }
    return static_cast<int>(this->columns());
}

//---------------------------------------------------------
//   insert
//---------------------------------------------------------

void TextBlock::insert(TextCursor* cursor, const String& s)
{
    int rcol, ridx;
    removeEmptyFragment();   // since we are going to write text, we don't need an empty fragment to hold format info. if such exists, delete it
    auto i = fragment(static_cast<int>(cursor->column()), &rcol, &ridx);
    if (i != m_fragments.end()) {
        if (!(i->format == *cursor->format())) {
            if (rcol == 0) {
                m_fragments.insert(i, TextFragment(cursor, s));
            } else {
                TextFragment f2 = i->split(rcol);
                i = m_fragments.insert(std::next(i), TextFragment(cursor, s));
                m_fragments.insert(std::next(i), f2);
            }
        } else {
            i->text.insert(ridx, s);
        }
    } else {
        if (!m_fragments.empty() && m_fragments.back().format == *cursor->format()) {
            m_fragments.back().text.append(s);
        } else {
            m_fragments.push_back(TextFragment(cursor, s));
        }
    }
}

//---------------------------------------------------------
//
//   insertEmptyFragmentIfNeeded
//   used to insert an empty TextFragment in TextBlocks that have none
//   that way, the formatting information (most importantly the font size) of the line is preserved
//
//---------------------------------------------------------

void TextBlock::insertEmptyFragmentIfNeeded(TextCursor* cursor)
{
    if (m_fragments.size() == 0 || m_fragments.front().text.isEmpty()) {
        m_fragments.insert(m_fragments.begin(), TextFragment(cursor, u""));
    }
}

//---------------------------------------------------------
//   removeEmptyFragment
//---------------------------------------------------------

void TextBlock::removeEmptyFragment()
{
    if (m_fragments.size() > 0 && m_fragments.front().text.isEmpty()) {
        m_fragments.pop_back();
    }
}

//---------------------------------------------------------
//   fragment
//    inputs:
//      column is the column relative to the start of the TextBlock.
//    outputs:
//      rcol will be the column relative to the start of the TextFragment that the input column is in.
//      ridx will be the QChar index into TextFragment's text String relative to the start of that TextFragment.
//
//---------------------------------------------------------

std::list<TextFragment>::iterator TextBlock::fragment(int column, int* rcol, int* ridx)
{
    int col = 0;
    for (auto it = m_fragments.begin(); it != m_fragments.end(); ++it) {
        *rcol = 0;
        *ridx = 0;
        for (size_t i = 0; i < it->text.size(); ++i) {
            if (col == column) {
                return it;
            }
            ++*ridx;
            if (it->text.at(i).isHighSurrogate()) {
                continue;
            }
            ++col;
            ++*rcol;
        }
    }
    return m_fragments.end();
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

String TextBlock::remove(int column, TextCursor* cursor)
{
    int col = 0;
    String s;
    for (auto it = m_fragments.begin(); it != m_fragments.end(); ++it) {
        size_t idx = 0;

        for (size_t i = 0; i < it->text.size(); ++i) {
            if (col == column) {
                if (it->text.at(i).isSurrogate()) {
                    s = it->text.mid(idx, 2);
                    it->text.remove(idx, 2);
                } else {
                    s = it->text.mid(idx, 1);
                    it->text.remove(idx, 1);
                }
                if (it->text.isEmpty()) {
                    m_fragments.erase(it);
                }
                simplify();
                insertEmptyFragmentIfNeeded(cursor);         // without this, cursorRect can't calculate the y position of the cursor correctly
                return s;
            }
            ++idx;
            if (it->text.at(i).isHighSurrogate()) {
                continue;
            }
            ++col;
        }
    }
    insertEmptyFragmentIfNeeded(cursor);   // without this, cursorRect can't calculate the y position of the cursor correctly
    return s;
//      LOGD("TextBlock::remove: column %d not found", column);
}

//---------------------------------------------------------
//   simplify
//---------------------------------------------------------

void TextBlock::simplify()
{
    if (m_fragments.size() < 2) {
        return;
    }
    auto i = m_fragments.begin();
    TextFragment* f = &*i;
    ++i;
    for (; i != m_fragments.end(); ++i) {
        while (i != m_fragments.end() && (i->format == f->format)) {
            f->text.append(i->text);
            i = m_fragments.erase(i);
        }
        if (i == m_fragments.end()) {
            break;
        }
        f = &*i;
    }
}

double TextBlock::musicSymbolBaseLineAdjust(const TextBase* t, const TextFragment& f, const std::list<TextFragment>::iterator fi)
{
    Font fragmentFont = f.font(t);
    FontMetrics fm(fragmentFont);
    const bool adjustSymbol = fragmentFont.type() == Font::Type::MusicSymbolText && t->isMarker();
    if (!adjustSymbol) {
        return 0.0;
    }

    // Align the x-height of the coda symbol to half the x-height of the surrounding text
    Font refFont;
    if (m_fragments.size() == 1) {
        refFont = t->font();
    } else {
        TextFragment& refFragment = fi != m_fragments.begin() ? *(std::prev(fi)) : *(std::next(fi));
        refFont = refFragment.font(t);
    }
    FontMetrics refFm(refFont);

    const double middle = (fm.tightBoundingRect(f.text).height() / 2) - fm.tightBoundingRect(f.text).bottom();
    const double refXHeight = refFm.capHeight() / 2;
    return refXHeight - middle;
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

String TextBlock::remove(int start, int n, TextCursor* cursor)
{
    if (n == 0) {
        return String();
    }
    int col = 0;
    String s;
    for (auto i = m_fragments.begin(); i != m_fragments.end();) {
        bool inc = true;
        for (size_t idx = 0; idx < i->text.size();) {
            Char c = i->text.at(idx);
            if (col == start) {
                if (c.isHighSurrogate()) {
                    s += c;
                    i->text.remove(idx, 1);
                    c = i->text.at(idx);
                }
                s += c;
                i->text.remove(idx, 1);
                if (i->text.isEmpty() && (m_fragments.size() > 1)) {
                    i = m_fragments.erase(i);
                    inc = false;
                }
                --n;
                if (n == 0) {
                    insertEmptyFragmentIfNeeded(cursor);           // without this, cursorRect can't calculate the y position of the cursor correctly
                    return s;
                }
                continue;
            }
            ++idx;
            if (c.isHighSurrogate()) {
                continue;
            }
            ++col;
        }
        if (inc) {
            ++i;
        }
    }
    insertEmptyFragmentIfNeeded(cursor);   // without this, cursorRect can't calculate the y position of the cursor correctly
    return s;
}

//---------------------------------------------------------
//   changeFormat
//---------------------------------------------------------

void TextBlock::changeFormat(FormatId id, const FormatValue& data, int start, int n)
{
    int col = 0;
    for (auto i = m_fragments.begin(); i != m_fragments.end(); ++i) {
        int columns = i->columns();
        if (start + n <= col) {
            break;
        }
        if (start >= col + columns) {
            col += i->columns();
            continue;
        }
        int endCol = col + columns;

        if ((start <= col) && (start < endCol) && ((start + n) < endCol)) {
            // left
            TextFragment f = i->split(start + n - col);
            i->changeFormat(id, data);
            i = m_fragments.insert(std::next(i), f);
        } else if (start > col && ((start + n) < endCol)) {
            // middle
            TextFragment lf = i->split(start + n - col);
            TextFragment mf = i->split(start - col);
            mf.changeFormat(id, data);
            i = m_fragments.insert(std::next(i), mf);
            i = m_fragments.insert(std::next(i), lf);
        } else if (start > col) {
            // right
            TextFragment f = i->split(start - col);
            f.changeFormat(id, data);
            i = m_fragments.insert(std::next(i), f);
        } else {
            if (id == FormatId::FontFamily && i->format.fontFamily() == "ScoreText") {
                void(0);// do nothing, we need to leave that as is
            } else {
                // complete fragment
                i->changeFormat(id, data);
            }
        }
        col = endCol;
    }
}

//---------------------------------------------------------
//   formatValue
//---------------------------------------------------------
FormatValue CharFormat::formatValue(FormatId id) const
{
    switch (id) {
    case FormatId::Bold: return bold();
    case FormatId::Italic: return italic();
    case FormatId::Underline: return underline();
    case FormatId::Strike: return strike();
    case FormatId::Valign: return static_cast<int>(valign());
    case FormatId::FontSize: return fontSize();
    case FormatId::FontFamily: return fontFamily();
    }

    return FormatValue();
}

//---------------------------------------------------------
//   setFormatValue
//---------------------------------------------------------

void CharFormat::setFormatValue(FormatId id, const FormatValue& val)
{
    switch (id) {
    case FormatId::Bold:
        setBold(std::get<bool>(val));
        break;
    case FormatId::Italic:
        setItalic(std::get<bool>(val));
        break;
    case FormatId::Underline:
        setUnderline(std::get<bool>(val));
        break;
    case FormatId::Strike:
        setStrike(std::get<bool>(val));
        break;
    case FormatId::Valign:
        m_valign = static_cast<VerticalAlignment>(std::get<int>(val));
        break;
    case FormatId::FontSize:
        m_fontSize = std::get<double>(val);
        break;
    case FormatId::FontFamily:
        m_fontFamily = std::get<String>(val);
        break;
    }
}

//---------------------------------------------------------
//   changeFormat
//---------------------------------------------------------

void TextFragment::changeFormat(FormatId id, const FormatValue& data)
{
    format.setFormatValue(id, data);
}

//---------------------------------------------------------
//   split
//---------------------------------------------------------

TextBlock TextBlock::split(int column, TextCursor* cursor)
{
    TextBlock tl;

    int col = 0;
    for (auto it = m_fragments.begin(); it != m_fragments.end(); ++it) {
        size_t idx = 0;
        for (size_t i = 0; i < it->text.size(); ++i) {
            if (col == column) {
                if (idx) {
                    if (idx < it->text.size()) {
                        TextFragment tf(it->text.mid(idx));
                        tf.format = it->format;
                        tl.m_fragments.push_back(tf);
                        it->text = it->text.left(idx);
                        ++it;
                    }
                }
                for (; it != m_fragments.end(); it = m_fragments.erase(it)) {
                    tl.m_fragments.push_back(*it);
                }

                if (m_fragments.size() == 0) {
                    insertEmptyFragmentIfNeeded(cursor);
                }
                return tl;
            }
            ++idx;
            if (it->text.at(i).isHighSurrogate()) {
                continue;
            }
            ++col;
        }
    }

    TextFragment tf(u"");
    if (m_fragments.size() > 0) {
        tf.format = m_fragments.back().format;
    } else if (m_fragments.size() == 0) {
        insertEmptyFragmentIfNeeded(cursor);
    }

    tl.m_fragments.push_back(tf);
    return tl;
}

static String toSymbolXml(Char c)
{
    static std::shared_ptr<IEngravingFontsProvider> provider = muse::modularity::globalIoc()->resolve<IEngravingFontsProvider>("engraving");

    SymId symId = provider->fallbackFont()->fromCode(c.unicode());
    return u"<sym>" + String::fromAscii(SymNames::nameForSymId(symId).ascii()) + u"</sym>";
}

//---------------------------------------------------------
//   text
//    extract text, symbols are marked with <sym>xxx</sym>
//---------------------------------------------------------

String TextBlock::text(int col1, int len, bool withFormat) const
{
    String s;
    int col = 0;
    double size;
    String family;
    for (const auto& f : m_fragments) {
        if (f.text.isEmpty()) {
            continue;
        }
        if (withFormat) {
            s += TextBase::getHtmlStartTag(f.format.fontSize(), size, f.format.fontFamily(), family, f.format.style(), f.format.valign());
        }

        for (size_t i = 0; i < f.text.size(); ++i) {
            Char c = f.text.at(i);
            if (col >= col1 && (len < 0 || ((col - col1) < len))) {
                if (f.format.fontFamily() == "ScoreText" && withFormat) {
                    s += toSymbolXml(c);
                } else {
                    s += String::toXmlEscaped(c.unicode());
                }
            }
            if (!c.isHighSurrogate()) {
                ++col;
            }
        }
        if (withFormat) {
            s += TextBase::getHtmlEndTag(f.format.style(), f.format.valign());
        }
    }
    return s;
}

//---------------------------------------------------------
//   Text
//---------------------------------------------------------

TextBase::TextBase(const ElementType& type, EngravingItem* parent, TextStyleType tid, ElementFlags f)
    : EngravingItem(type, parent, f | ElementFlag::MOVABLE)
{
    m_textLineSpacing        = 1.0;
    m_textStyleType          = tid;
    m_bgColor                = Color::transparent;
    m_frameColor             = Color::BLACK;
    m_align                  = { AlignH::LEFT, AlignV::TOP };
    m_frameType              = FrameType::NO_FRAME;
    m_frameWidth             = Spatium(0.1);
    m_paddingWidth           = Spatium(0.2);
    m_frameRound             = 0;

    m_cursor                 = new TextCursor(this);
    m_cursor->init();
}

TextBase::TextBase(const ElementType& type, EngravingItem* parent, ElementFlags f)
    : TextBase(type, parent, TextStyleType::DEFAULT, f)
{
}

TextBase::TextBase(const TextBase& st)
    : EngravingItem(st)
{
    m_cursor                      = new TextCursor(this);
    m_cursor->setFormat(*(st.cursor()->format()));
    m_text                        = st.m_text;
    m_textInvalid                  = st.m_textInvalid;
    m_layoutToParentWidth         = st.m_layoutToParentWidth;
    m_hexState                     = -1;

    m_textStyleType               = st.m_textStyleType;
    m_textLineSpacing             = st.m_textLineSpacing;
    m_bgColor                     = st.m_bgColor;
    m_frameColor                  = st.m_frameColor;
    m_align                       = st.m_align;
    m_frameType                   = st.m_frameType;
    m_frameWidth                  = st.m_frameWidth;
    m_paddingWidth                = st.m_paddingWidth;
    m_frameRound                  = st.m_frameRound;

    m_voiceAssignment = st.m_voiceAssignment;
    m_direction = st.m_direction;
    m_centerBetweenStaves = st.m_centerBetweenStaves;
    m_anchorToEndOfPrevious = st.m_anchorToEndOfPrevious;

    size_t n = m_elementStyle->size() + TEXT_STYLE_SIZE;
    delete[] m_propertyFlagsList;
    m_propertyFlagsList = new PropertyFlags[n];
    for (size_t i = 0; i < n; ++i) {
        m_propertyFlagsList[i] = st.m_propertyFlagsList[i];
    }
    m_links = 0;
}

TextBase::~TextBase()
{
    delete m_cursor;
}

//---------------------------------------------------------
//   textColor
//---------------------------------------------------------

Color TextBase::textColor() const
{
    return curColor();
}

//---------------------------------------------------------
//   insert
//    insert character
//---------------------------------------------------------

void TextBase::insert(TextCursor* cursor, char32_t code, LayoutData* ldata) const
{
    if (cursor->row() >= ldata->blocks.size()) {
        ldata->blocks.push_back(TextBlock());
    }
    if (code == '\t') {
        code = ' ';
    }

    if (cursor->row() < ldata->blocks.size()) {
        ldata->blocks[cursor->row()].insert(cursor, String::fromUcs4(code));
    }

    cursor->setColumn(cursor->column() + 1);
    cursor->clearSelection();
}

//---------------------------------------------------------
//   parseStringProperty
//---------------------------------------------------------

static String parseStringProperty(const String& s)
{
    String rs;
    for (size_t i = 0; i < s.size(); ++i) {
        Char c = s.at(i);
        if (c == '"') {
            break;
        }
        rs += c;
    }
    return rs;
}

//---------------------------------------------------------
//   parseNumProperty
//---------------------------------------------------------

static double parseNumProperty(const String& s)
{
    return parseStringProperty(s).toDouble();
}

//---------------------------------------------------------
//   createLayout
//    create layout from text
//---------------------------------------------------------
void TextBase::createBlocks()
{
    createBlocks(mutldata());
}

void TextBase::createBlocks(LayoutData* ldata) const
{
    // reset all previous formatting information
    ldata->blocks.clear();
    TextCursor cursor = *m_cursor;
    cursor.setRow(0);
    cursor.setColumn(0);

    int state = 0;
    String token;
    String sym;
    bool symState = false;
    for (size_t i = 0; i < m_text.size(); i++) {
        const Char& c = m_text.at(i);
        if (state == 0) {
            if (c == '<') {
                state = 1;
                token.clear();
            } else if (c == '&') {
                state = 2;
                token.clear();
            } else if (c == '\n') {
                if (ldata->rows() <= cursor.row()) {
                    ldata->blocks.push_back(TextBlock());
                }

                if (cursor.row() < ldata->rows()) {
                    if (ldata->blocks.at(cursor.row()).fragments().size() == 0) {
                        ldata->blocks[cursor.row()].insertEmptyFragmentIfNeeded(&cursor);           // used to preserve the Font size of the line (font info is held in TextFragments, see PR #5881)
                    }

                    ldata->blocks[cursor.row()].setEol(true);
                }

                cursor.setRow(cursor.row() + 1);
                cursor.setColumn(0);
                if (ldata->rows() <= cursor.row()) {
                    ldata->blocks.push_back(TextBlock());
                }

                if (cursor.row() < ldata->rows()) {
                    if (ldata->blocks.at(cursor.row()).fragments().size() == 0) {
                        ldata->blocks[cursor.row()].insertEmptyFragmentIfNeeded(&cursor); // an empty fragment may be needed on either side of the newline
                    }
                }
            } else {
                if (symState) {
                    sym += c;
                } else {
                    if (c.isHighSurrogate()) {
                        i++;
                        assert(i < m_text.size());
                        insert(&cursor, Char::surrogateToUcs4(c, m_text.at(i)), ldata);
                    } else {
                        insert(&cursor, c.unicode(), ldata);
                    }
                }
            }
        } else if (state == 1) {
            if (c == '>') {
                state = 0;
                const_cast<TextBase*>(this)->prepareFormat(token, cursor);
                if (token == "sym") {
                    symState = true;
                    sym.clear();
                } else if (token == "/sym") {
                    symState = false;
                    SymId id = SymNames::symIdByName(sym);
                    if (id != SymId::noSym) {
                        CharFormat fmt = *cursor.format(); // save format

                        //char32_t code = score()->scoreFont()->symCode(id);
                        char32_t code = id
                                        == SymId::space ? static_cast<char32_t>(' ') : score()->engravingFonts()->fallbackFont()->symCode(id);
                        cursor.format()->setFontFamily(u"ScoreText");
                        insert(&cursor, code, ldata);
                        cursor.setFormat(fmt); // restore format
                    } else {
                        LOGD("unknown symbol <%s>", muPrintable(sym));
                    }
                }
            } else {
                token += c;
            }
        } else if (state == 2) {
            if (c == ';') {
                state = 0;
                if (token == "lt") {
                    insert(&cursor, '<', ldata);
                } else if (token == "gt") {
                    insert(&cursor, '>', ldata);
                } else if (token == "amp") {
                    insert(&cursor, '&', ldata);
                } else if (token == "quot") {
                    insert(&cursor, '"', ldata);
                } else {
                    // TODO insert(&cursor, SymNames::symIdByName(token));
                }
            } else {
                token += c;
            }
        }
    }
    if (ldata->blocks.empty()) {
        ldata->blocks.push_back(TextBlock());
    }
    ldata->layoutInvalid = false;
}

//---------------------------------------------------------
//   prepareFormat - used when reading from XML and when pasting from clipboard
//---------------------------------------------------------
bool TextBase::prepareFormat(const String& token, CharFormat& format)
{
    if (token == "b") {
        format.setBold(true);
        return true;
    } else if (token == "/b") {
        format.setBold(false);
    } else if (token == "i") {
        format.setItalic(true);
        return true;
    } else if (token == "/i") {
        format.setItalic(false);
    } else if (token == "u") {
        format.setUnderline(true);
        return true;
    } else if (token == "/u") {
        format.setUnderline(false);
    } else if (token == "s") {
        format.setStrike(true);
        return true;
    } else if (token == "/s") {
        format.setStrike(false);
    } else if (token == "sub") {
        format.setValign(VerticalAlignment::AlignSubScript);
    } else if (token == "/sub") {
        format.setValign(VerticalAlignment::AlignNormal);
    } else if (token == "sup") {
        format.setValign(VerticalAlignment::AlignSuperScript);
    } else if (token == "/sup") {
        format.setValign(VerticalAlignment::AlignNormal);
    } else if (token.startsWith(u"font ")) {
        String remainder = token.mid(5);
        if (remainder.startsWith(u"size=\"")) {
            format.setFontSize(parseNumProperty(remainder.mid(6)));
            return true;
        } else if (remainder.startsWith(u"face=\"")) {
            String face = parseStringProperty(remainder.mid(6));
            face = unEscape(face);
            format.setFontFamily(face);
            return true;
        } else {
            LOGD("cannot parse html property <%s> in text <%s>", muPrintable(token), muPrintable(m_text));
        }
    }
    return false;
}

//---------------------------------------------------------
//   prepareFormat - used when reading from XML
//---------------------------------------------------------
void TextBase::prepareFormat(const String& token, TextCursor& cursor)
{
    if (prepareFormat(token, *cursor.format()) && cursor.format()->fontFamily() != propertyDefault(Pid::FONT_FACE).value<String>()) {
        setPropertyFlags(Pid::FONT_FACE, PropertyFlags::UNSTYLED);
    }
}

//---------------------------------------------------------
//   layoutFrame
//---------------------------------------------------------

void TextBase::layoutFrame()
{
    layoutFrame(mutldata());
}

void TextBase::layoutFrame(LayoutData* ldata) const
{
//      if (empty()) {    // or bbox.width() <= 1.0
    if (ldata->bbox().width() <= 1.0 || ldata->bbox().height() < 1.0) {      // or bbox.width() <= 1.0
        // this does not work for Harmony:
        FontMetrics fm(font());
        double ch = fm.ascent();
        double cw = fm.width('n');
        ldata->frame = RectF(0.0, -ch, cw, ch);
    } else {
        ldata->frame = ldata->bbox();
    }

    if (square()) {
        // make sure width >= height
        if (ldata->frame.height() > ldata->frame.width()) {
            double w = ldata->frame.height() - ldata->frame.width();
            ldata->frame.adjust(-w * .5, 0.0, w * .5, 0.0);
        }
    } else if (circle()) {
        if (ldata->frame.width() > ldata->frame.height()) {
            ldata->frame.setTop(ldata->frame.y() + (ldata->frame.width() - ldata->frame.height()) * -.5);
            ldata->frame.setHeight(ldata->frame.width());
        } else {
            ldata->frame.setLeft(ldata->frame.x() + (ldata->frame.height() - ldata->frame.width()) * -.5);
            ldata->frame.setWidth(ldata->frame.height());
        }
    }
    double _spatium = spatium();
    double w = (paddingWidth() + frameWidth() * .5f).val() * _spatium;
    ldata->frame.adjust(-w, -w, w, w);
    w = 0.5 * frameWidth().val() * _spatium;
    ldata->setBbox(ldata->frame.adjusted(-w, -w, w, w));
}

//---------------------------------------------------------
//   lineSpacing
//---------------------------------------------------------

double TextBase::lineSpacing() const
{
    return fontMetrics().lineSpacing();
}

//---------------------------------------------------------
//   lineHeight
//---------------------------------------------------------

double TextBase::lineHeight() const
{
    return fontMetrics().height();
}

//---------------------------------------------------------
//   baseLine
//---------------------------------------------------------

double TextBase::baseLine() const
{
    return fontMetrics().ascent();
}

FontStyle TextBase::fontStyle() const
{
    return m_cursor->format()->style();
}

String TextBase::family() const
{
    return m_cursor->format()->fontFamily();
}

double TextBase::size() const
{
    return m_cursor->format()->fontSize();
}

void TextBase::setFontStyle(const FontStyle& val)
{
    m_cursor->setFormat(FormatId::Bold, val & FontStyle::Bold);
    m_cursor->setFormat(FormatId::Italic, val & FontStyle::Italic);
    m_cursor->setFormat(FormatId::Underline, val & FontStyle::Underline);
    m_cursor->setFormat(FormatId::Strike, val & FontStyle::Strike);
}

void TextBase::setFamily(const String& val)
{
    m_cursor->setFormat(FormatId::FontFamily, val);
}

void TextBase::setSize(const double& val)
{
    m_cursor->setFormat(FormatId::FontSize, val);
}

//---------------------------------------------------------
//   XmlNesting
//---------------------------------------------------------

class XmlNesting : public std::stack<String>
{
    OBJECT_ALLOCATOR(engraving, XmlNesting)

    String* _s;

public:
    XmlNesting(String* s) { _s = s; }
    void pushToken(const String& t)
    {
        *_s += u"<";
        *_s += t;
        *_s += u">";
        push(t);
    }

    void pushB() { pushToken(u"b"); }
    void pushI() { pushToken(u"i"); }
    void pushU() { pushToken(u"u"); }
    void pushS() { pushToken(u"s"); }

    String popToken()
    {
        String s = top();
        pop();
        *_s += u"</";
        *_s += s;
        *_s += u">";
        return s;
    }

    void popToken(const char* t)
    {
        StringList ps;
        for (;;) {
            String s = popToken();
            if (s == t) {
                break;
            }
            ps << s;
        }
        for (const String& s : ps) {
            pushToken(s);
        }
    }

    void popB() { popToken("b"); }
    void popI() { popToken("i"); }
    void popU() { popToken("u"); }
    void popS() { popToken("s"); }
};

//---------------------------------------------------------
//   genText
//---------------------------------------------------------
String TextBase::genText(const LayoutData* ldata) const
{
    if (!ldata) {
        return String();
    }

    String text;
    XmlNesting xmlNesting(&text);

    CharFormat fmt;
    fmt.setFontFamily(propertyDefault(Pid::FONT_FACE).value<String>());
    fmt.setFontSize(propertyDefault(Pid::FONT_SIZE).toReal());
    fmt.setStyle(static_cast<FontStyle>(propertyDefault(Pid::FONT_STYLE).toInt()));

    // Prepare the initial style tags (if any).
    // We only need those if there is any fragment in the blocks with a different style than the default one.
    bool bold_      = false;
    bool italic_    = false;
    bool underline_ = false;
    bool strike_    = false;
    for (const TextBlock& block : ldata->blocks) {
        for (const TextFragment& f : block.fragments()) {
            if (!f.format.bold() && fmt.bold()) {
                bold_ = true;
            }
            if (!f.format.italic() && fmt.italic()) {
                italic_ = true;
            }
            if (!f.format.underline() && fmt.underline()) {
                underline_ = true;
            }
            if (!f.format.strike() && fmt.strike()) {
                strike_ = true;
            }
        }
    }
    if (bold_) {
        xmlNesting.pushB();
    }
    if (italic_) {
        xmlNesting.pushI();
    }
    if (underline_) {
        xmlNesting.pushU();
    }
    if (strike_) {
        xmlNesting.pushS();
    }

    // And here for the actual formatting of the text.
    for (const TextBlock& block : ldata->blocks) {
        for (const TextFragment& f : block.fragments()) {
            // don't skip, empty text fragments hold information for empty lines
            //    if (f.text.isEmpty())                   // skip empty fragments, not to
            //        continue;                           // insert extra HTML formatting

            // Push or Pop XML tags according to the current format changes
            const CharFormat& format = f.format;
            if (fmt.bold() != format.bold()) {
                if (format.bold()) {
                    xmlNesting.pushB();
                } else {
                    xmlNesting.popB();
                }
            }
            if (fmt.italic() != format.italic()) {
                if (format.italic()) {
                    xmlNesting.pushI();
                } else {
                    xmlNesting.popI();
                }
            }
            if (fmt.underline() != format.underline()) {
                if (format.underline()) {
                    xmlNesting.pushU();
                } else {
                    xmlNesting.popU();
                }
            }
            if (fmt.strike() != format.strike()) {
                if (format.strike()) {
                    xmlNesting.pushS();
                } else {
                    xmlNesting.popS();
                }
            }

            if (format.fontSize() != fmt.fontSize()) {
                text += String(u"<font size=\"%1\"/>").arg(format.fontSize());
            }
            if (format.fontFamily() != "ScoreText" && format.fontFamily() != fmt.fontFamily()) {
                text += String(u"<font face=\"%1\"/>").arg(TextBase::escape(format.fontFamily()));
            }

            VerticalAlignment va = format.valign();
            VerticalAlignment cva = fmt.valign();
            if (cva != va) {
                switch (va) {
                case VerticalAlignment::AlignNormal:
                    xmlNesting.popToken(cva == VerticalAlignment::AlignSuperScript ? "sup" : "sub");
                    break;
                case VerticalAlignment::AlignSuperScript:
                    xmlNesting.pushToken(u"sup");
                    break;
                case VerticalAlignment::AlignSubScript:
                    xmlNesting.pushToken(u"sub");
                    break;
                case VerticalAlignment::AlignUndefined:
                    break;
                }
            }
            if (format.fontFamily() == u"ScoreText") {
                for (size_t i = 0; i < f.text.size(); ++i) {
                    text += toSymbolXml(f.text.at(i));
                }
            } else {
                text += XmlWriter::xmlString(f.text);
            }
            fmt = format;
        }
        if (block.eol()) {
            text += Char::LineFeed;
        }
    }
    while (!xmlNesting.empty()) {
        xmlNesting.popToken();
    }

    return text;
}

void TextBase::genText()
{
    m_text = genText(ldata());
    m_textInvalid = false;
}

//---------------------------------------------------------
//   selectAll
//---------------------------------------------------------

void TextBase::selectAll(TextCursor* cursor)
{
    const LayoutData* ldata = this->ldata();
    if (!ldata || ldata->blocks.empty()) {
        return;
    }

    cursor->setSelectColumn(0);
    cursor->setSelectLine(0);
    cursor->setRow(ldata->rows() - 1);
    cursor->setColumn(cursor->curLine().columns());
}

void TextBase::select(EditData& editData, SelectTextType type)
{
    switch (type) {
    case SelectTextType::Word:
        cursorFromEditData(editData)->selectWord();
        break;
    case SelectTextType::All:
        selectAll(cursorFromEditData(editData));
        break;
    }
}

//---------------------------------------------------------
//   pageRectangle
//---------------------------------------------------------

RectF TextBase::pageRectangle() const
{
    if (explicitParent() && (explicitParent()->isHBox() || explicitParent()->isVBox() || explicitParent()->isTBox())) {
        Box* box = toBox(explicitParent());
        RectF r = box->pageBoundingRect();
        double x = r.x() + box->leftMargin() * DPMM;
        double y = r.y() + box->topMargin() * DPMM;
        double h = r.height() - (box->topMargin() + box->bottomMargin()) * DPMM;
        double w = r.width() - (box->leftMargin() + box->rightMargin()) * DPMM;

        // SizeF ps = _doc->pageSize();
        // return RectF(x, y, ps.width(), ps.height());

        return RectF(x, y, w, h);
    }
    if (explicitParent() && explicitParent()->isPage()) {
        Page* box  = toPage(explicitParent());
        RectF r = box->pageBoundingRect();
        double x = r.x() + box->lm();
        double y = r.y() + box->tm();
        double h = r.height() - box->tm() - box->bm();
        double w = r.width() - box->lm() - box->rm();
        return RectF(x, y, w, h);
    }
    return pageBoundingRect();
}

//---------------------------------------------------------
//   dragTo
//---------------------------------------------------------

void TextBase::dragTo(EditData& ed)
{
    TextEditData* ted = static_cast<TextEditData*>(ed.getData(this).get());
    TextCursor* cursor = ted->cursor();
    cursor->set(ed.pos, TextCursor::MoveMode::KeepAnchor);
    score()->setUpdateAll();
    score()->update();
}

//---------------------------------------------------------
//   dragAnchorLines
//---------------------------------------------------------

std::vector<LineF> TextBase::dragAnchorLines() const
{
    std::vector<LineF> result(genericDragAnchorLines());

    if (layoutToParentWidth() && !result.empty()) {
        LineF& line = result[0];
        line.setP2(line.p2() + ldata()->bbox().topLeft());
    }

    return result;
}

//---------------------------------------------------------
//   mousePress
//    set text cursor
//---------------------------------------------------------

bool TextBase::mousePress(EditData& ed)
{
    bool shift = ed.modifiers & ShiftModifier;
    TextEditData* ted = static_cast<TextEditData*>(ed.getData(this).get());
    if (!ted->cursor()->set(ed.startMove, shift ? TextCursor::MoveMode::KeepAnchor : TextCursor::MoveMode::MoveAnchor)) {
        return false;
    }

    score()->setUpdateAll();
    return true;
}

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool TextBase::acceptDrop(EditData& data) const
{
    // do not accept the drop if this text element is not being edited
    ElementEditDataPtr eed = data.getData(this);
    if (!eed || eed->type() != EditDataType::TextEditData) {
        return false;
    }
    ElementType type = data.dropElement->type();
    return type == ElementType::SYMBOL || type == ElementType::FSYMBOL;
}

//--------------------------------------------------------
//   setXmlText
//---------------------------------------------------------

void TextBase::setXmlText(const String& s)
{
    m_text = s;
    m_textInvalid = false;
    mutldata()->layoutInvalid = true;
}

void TextBase::checkCustomFormatting(const String& s)
{
    if (s.contains(u"<font size")) {
        setPropertyFlags(Pid::FONT_SIZE, PropertyFlags::UNSTYLED);
    }
    if (s.contains(u"<b>") || s.contains(u"<i>") || s.contains(u"<u>") || s.contains(u"<s>")) {
        setPropertyFlags(Pid::FONT_STYLE, PropertyFlags::UNSTYLED);
    }
}

void TextBase::resetFormatting()
{
    // reset any formatting properties that can be changed per-character (doesn't change existing text)
    cursor()->format()->setFontFamily(propertyDefault(Pid::FONT_FACE).value<String>());
    cursor()->format()->setFontSize(propertyDefault(Pid::FONT_SIZE).toReal());
    cursor()->format()->setStyle(static_cast<FontStyle>(propertyDefault(Pid::FONT_STYLE).toInt()));
    cursor()->format()->setValign(VerticalAlignment::AlignNormal);
}

//---------------------------------------------------------
//   fragmentList
//---------------------------------------------------------

/*
 Return the text as a single list of TextFragment
 Used by the MusicXML formatted export to avoid parsing the xml text format
 */

std::list<TextFragment> TextBase::fragmentList() const
{
    std::list<TextFragment> res;

    const TextBase* text = this;
    std::unique_ptr<TextBase> tmpText;
    const LayoutData* ldata = this->ldata();
    if (!ldata || ldata->layoutInvalid) {
        // Create temporary text object to avoid side effects
        // of createLayout() call.
        tmpText.reset(toTextBase(this->clone()));
        tmpText->createBlocks();
        text = tmpText.get();
    }

    const LayoutData* tmlldata = text->ldata();
    for (const TextBlock& block : tmlldata->blocks) {
        for (const TextFragment& f : block.fragments()) {
            /* TODO TBD
            if (f.text.empty())                     // skip empty fragments, not to
                  continue;                           // insert extra HTML formatting
             */
            res.push_back(f);
            if (block.eol()) {
                // simply append a newline
                res.back().text += u"\n";
            }
        }
    }
    return res;
}

//---------------------------------------------------------
//   plainText
//    return plain text with symbols
//---------------------------------------------------------

String TextBase::plainText() const
{
    String s;

    const TextBase* text = this;
    std::unique_ptr<TextBase> tmpText;
    const LayoutData* ldata = this->ldata();
    if (!ldata || ldata->layoutInvalid) {
        // Create temporary text object to avoid side effects
        // of createLayout() call.
        tmpText.reset(toTextBase(this->clone()));
        tmpText->createBlocks();
        text = tmpText.get();
    }

    const LayoutData* tmlldata = text->ldata();
    for (const TextBlock& block : tmlldata->blocks) {
        for (const TextFragment& f : block.fragments()) {
            s += f.text;
        }
        if (block.eol()) {
            s += Char::LineFeed;
        }
    }
    return s;
}

//---------------------------------------------------------
//   xmlText
//---------------------------------------------------------

String TextBase::xmlText() const
{
    if (!m_textInvalid) {
        return m_text;
    }

    String text = genText(ldata());
    return text;
}

//---------------------------------------------------------
//   unEscape
//---------------------------------------------------------

String TextBase::unEscape(String s)
{
    s.replace(u"&lt;", u"<");
    s.replace(u"&gt;", u">");
    s.replace(u"&amp;", u"&");
    s.replace(u"&quot;", u"\"");
    return s;
}

//---------------------------------------------------------
//   escape
//---------------------------------------------------------

String TextBase::escape(String s)
{
    s.replace(u"<", u"&lt;");
    s.replace(u">", u"&gt;");
    s.replace(u"&", u"&amp;");
    s.replace(u"\"", u"&quot;");
    return s;
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String TextBase::accessibleInfo() const
{
    String rez;
    switch (textStyleType()) {
    case TextStyleType::TITLE:
    case TextStyleType::SUBTITLE:
    case TextStyleType::COMPOSER:
    case TextStyleType::LYRICIST:
    case TextStyleType::TRANSLATOR:
    case TextStyleType::MEASURE_NUMBER:
    case TextStyleType::MMREST_RANGE:
        rez = translatedSubtypeUserName();
        break;
    default:
        rez = EngravingItem::accessibleInfo();
        break;
    }
    String s = plainText().simplified();
    if (s.size() > 20) {
        s.truncate(20);
        s += u"…";
    }
    return String(u"%1: %2").arg(rez, s);
}

//---------------------------------------------------------
//   screenReaderInfo
//---------------------------------------------------------

String TextBase::screenReaderInfo() const
{
    String rez;

    switch (textStyleType()) {
    case TextStyleType::TITLE:
    case TextStyleType::SUBTITLE:
    case TextStyleType::COMPOSER:
    case TextStyleType::LYRICIST:
    case TextStyleType::TRANSLATOR:
    case TextStyleType::MEASURE_NUMBER:
    case TextStyleType::MMREST_RANGE:
        rez = translatedSubtypeUserName();
        break;
    default:
        rez = EngravingItem::accessibleInfo();
        break;
    }
    String s = plainText().simplified();
    return String(u"%1: %2").arg(rez, s);
}

//---------------------------------------------------------
//   subtype
//---------------------------------------------------------

int TextBase::subtype() const
{
    return int(textStyleType());
}

//---------------------------------------------------------
//   subtypeUserName
//---------------------------------------------------------

TranslatableString TextBase::subtypeUserName() const
{
    return score() ? score()->getTextStyleUserName(textStyleType()) : TConv::userName(textStyleType());
}

//---------------------------------------------------------
//   validateText
//    check if s is a valid musescore xml text string
//    - simple bugs are automatically adjusted
//   return true if text is valid or could be fixed
//  (this is incomplete/experimental)
//---------------------------------------------------------

bool TextBase::validateText(String& s)
{
    String d;
    for (size_t i = 0; i < s.size(); ++i) {
        Char c = s.at(i);
        if (c == u'&') {
            const char16_t* ok[] { u"amp;", u"lt;", u"gt;", u"quot;" };
            String t = s.mid(i + 1);
            bool found = false;
            for (auto k : ok) {
                if (t.startsWith(k)) {
                    d.append(c);
                    d.append(k);
                    i += int(std::u16string_view(k).size());
                    found = true;
                    break;
                }
            }
            if (!found) {
                d.append(u"&amp;");
            }
        } else if (c == u'<') {
            const char16_t* ok[] { u"b>", u"/b>", u"i>", u"/i>", u"u>", u"/u", u"s>", u"/s>", u"font ", u"/font>", u"sym>", u"/sym>",
                                   u"sub>", u"/sub>", u"sup>", u"/sup>" };
            String t = s.mid(i + 1);
            bool found = false;
            for (auto k : ok) {
                if (t.startsWith(k)) {
                    d.append(c);
                    d.append(k);
                    i += int(std::u16string_view(k).size());
                    found = true;
                    break;
                }
            }
            if (!found) {
                d.append(u"&lt;");
            }
        } else {
            d.append(c);
        }
    }

    String ss = u"<data>" + d + u"</data>\n";
    muse::ByteArray ba = ss.toUtf8();
    XmlReader xml(ba);
    while (xml.readNextStartElement()) {
        // LOGD("  token %d <%s>", int(xml.tokenType()), muPrintable(xml.name().toString()));
    }
    if (xml.error() == XmlReader::NoError) {
        s = d;
        return true;
    }
    LOGD() << "xml error at line " << xml.lineNumber() << " column " << xml.columnNumber()
           << ": " << xml.errorString();
    LOGD() << "text: |" << ss << "|";
    return false;
}

//---------------------------------------------------------
//   font
//---------------------------------------------------------

Font TextBase::font() const
{
    double m = size();
    if (sizeIsSpatiumDependent()) {
        m *= spatium() / SPATIUM20;
    }
    Font f(family(), Font::Type::Unknown);
    f.setPointSizeF(m);
    f.setBold(bold());
    f.setItalic(italic());
    if (underline()) {
        f.setUnderline(underline());
    }
    if (strike()) {
        f.setStrike(strike());
    }

    return f;
}

//---------------------------------------------------------
//   fontMetrics
//---------------------------------------------------------

FontMetrics TextBase::fontMetrics() const
{
    return FontMetrics(font());
}

bool TextBase::isPropertyLinkedToMaster(Pid id) const
{
    if (propertyGroup(id) == PropertyGroup::TEXT) {
        return isTextLinkedToMaster();
    }

    return EngravingItem::isPropertyLinkedToMaster(id);
}

bool TextBase::isUnlinkedFromMaster() const
{
    EngravingItem* parent = parentItem();
    if (parent && parent->isUnlinkedFromMaster()) {
        return true;
    }

    return !getProperty(Pid::TEXT_LINKED_TO_MASTER).toBool() || EngravingItem::isUnlinkedFromMaster();
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue TextBase::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::TEXT_STYLE:
        return textStyleType();
    case Pid::FONT_FACE:
        return m_cursor->selectedFragmentsFormat().fontFamily();
    case Pid::FONT_SIZE:
        return m_cursor->selectedFragmentsFormat().fontSize();
    case Pid::FONT_STYLE:
        return static_cast<int>(m_cursor->selectedFragmentsFormat().style());
    case Pid::TEXT_LINE_SPACING:
        return textLineSpacing();
    case Pid::FRAME_TYPE:
        return static_cast<int>(frameType());
    case Pid::FRAME_WIDTH:
        return frameWidth();
    case Pid::FRAME_PADDING:
        return paddingWidth();
    case Pid::FRAME_ROUND:
        return frameRound();
    case Pid::FRAME_FG_COLOR:
        return PropertyValue::fromValue(frameColor());
    case Pid::FRAME_BG_COLOR:
        return PropertyValue::fromValue(bgColor());
    case Pid::ALIGN:
        return PropertyValue::fromValue(align());
    case Pid::POSITION:
        return PropertyValue::fromValue(position());
    case Pid::TEXT_SCRIPT_ALIGN:
        return static_cast<int>(m_cursor->selectedFragmentsFormat().valign());
    case Pid::TEXT:
        return xmlText();
    case Pid::TEXT_LINKED_TO_MASTER:
        return isTextLinkedToMaster();
    case Pid::DIRECTION:
        return direction();
    case Pid::CENTER_BETWEEN_STAVES:
        return centerBetweenStaves();
    case Pid::VOICE_ASSIGNMENT:
        return voiceAssignment();
    default:
        return EngravingItem::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool TextBase::setProperty(Pid pid, const PropertyValue& v)
{
    if (m_textInvalid && ldata() && ldata()->isValid()) {
        genText();
    }

    bool rv = true;
    switch (pid) {
    case Pid::COLOR:
        if (color() == frameColor()) {
            setFrameColor(v.value<Color>());
        }
        EngravingItem::setProperty(pid, v);
        break;
    case Pid::TEXT_STYLE:
        initTextStyleType(v.value<TextStyleType>());
        break;
    case Pid::FONT_FACE:
        setFamily(v.value<String>());
        break;
    case Pid::FONT_SIZE:
        setSize(v.toReal());
        break;
    case Pid::FONT_STYLE:
        setFontStyle(FontStyle(v.toInt()));
        break;
    case Pid::TEXT_LINE_SPACING:
        setTextLineSpacing(v.toReal());
        break;
    case Pid::FRAME_TYPE:
        setFrameType(FrameType(v.toInt()));
        break;
    case Pid::FRAME_WIDTH:
        setFrameWidth(v.value<Spatium>());
        break;
    case Pid::FRAME_PADDING:
        setPaddingWidth(v.value<Spatium>());
        break;
    case Pid::FRAME_ROUND:
        setFrameRound(v.toInt());
        break;
    case Pid::FRAME_FG_COLOR:
        setFrameColor(v.value<Color>());
        break;
    case Pid::FRAME_BG_COLOR:
        setBgColor(v.value<Color>());
        break;
    case Pid::TEXT:
        setXmlText(v.value<String>());
        break;
    case Pid::ALIGN:
        setAlign(v.value<Align>());
        break;
    case Pid::POSITION:
        setPosition(v.value<AlignH>());
        break;
    case Pid::TEXT_SCRIPT_ALIGN:
        m_cursor->setFormat(FormatId::Valign, v.toInt());
        break;
    case Pid::TEXT_LINKED_TO_MASTER:
        if (isTextLinkedToMaster() == v.toBool()) {
            break;
        }
        if (!isTextLinkedToMaster()) {
            relinkPropertiesToMaster(PropertyGroup::TEXT);
        }
        setTextLinkedToMaster(v.toBool());
        break;
    case Pid::DIRECTION:
        setDirection(v.value<DirectionV>());
        break;
    case Pid::CENTER_BETWEEN_STAVES:
        setCenterBetweenStaves(v.value<AutoOnOff>());
        break;
    case Pid::VOICE_ASSIGNMENT:
        setVoiceAssignment(v.value<VoiceAssignment>());
        break;
    default:
        rv = EngravingItem::setProperty(pid, v);
        break;
    }

    triggerLayout();

    return rv;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue TextBase::propertyDefault(Pid id) const
{
    if (id == Pid::Z) {
        return EngravingItem::propertyDefault(id);
    }

    if (composition()) {
        PropertyValue v = explicitParent()->propertyDefault(id);
        if (v.isValid()) {
            return v;
        }
    }

    Sid sid = getPropertyStyle(id);
    if (sid != Sid::NOSTYLE) {
        return styleValue(id, sid);
    }

    switch (id) {
    case Pid::TEXT_STYLE:
        return TextStyleType::DEFAULT;
    case Pid::TEXT:
        return String();
    case Pid::TEXT_SCRIPT_ALIGN:
        return static_cast<int>(VerticalAlignment::AlignNormal);
    case Pid::TEXT_LINKED_TO_MASTER:
        return true;
    case Pid::DIRECTION:
        return DirectionV::AUTO;
    case Pid::CENTER_BETWEEN_STAVES:
        return AutoOnOff::AUTO;
    case Pid::VOICE_ASSIGNMENT:
        return VoiceAssignment::ALL_VOICE_IN_INSTRUMENT;
    default:
        for (const auto& p : *textStyle(TextStyleType::DEFAULT)) {
            if (p.pid == id) {
                return styleValue(id, p.sid);
            }
        }
    }

    return EngravingItem::propertyDefault(id);
}

//---------------------------------------------------------
//   getPropertyFlagsIdx
//---------------------------------------------------------

int TextBase::getPropertyFlagsIdx(Pid id) const
{
    int i = 0;
    for (const StyledProperty& p : *m_elementStyle) {
        if (p.pid == id) {
            return i;
        }
        ++i;
    }
    for (const auto& p : *textStyle(textStyleType())) {
        if (p.pid == id) {
            return i;
        }
        ++i;
    }
    return -1;
}

//---------------------------------------------------------
//   offsetSid
//---------------------------------------------------------

Sid TextBase::offsetSid() const
{
    TextStyleType defaultTid = propertyDefault(Pid::TEXT_STYLE).value<TextStyleType>();
    if (textStyleType() != defaultTid) {
        return Sid::NOSTYLE;
    }
    bool above = placeAbove();
    switch (textStyleType()) {
    case TextStyleType::DYNAMICS:
        return above ? Sid::dynamicsPosAbove : Sid::dynamicsPosBelow;
    case TextStyleType::EXPRESSION:
        return above ? Sid::expressionPosAbove : Sid::expressionPosBelow;
    case TextStyleType::LYRICS_ODD:
    case TextStyleType::LYRICS_EVEN:
        return above ? Sid::lyricsPosAbove : Sid::lyricsPosBelow;
    case TextStyleType::REHEARSAL_MARK:
        return above ? Sid::rehearsalMarkPosAbove : Sid::rehearsalMarkPosBelow;
    case TextStyleType::STAFF:
        return above ? Sid::staffTextPosAbove : Sid::staffTextPosBelow;
    case TextStyleType::STICKING:
        return above ? Sid::stickingPosAbove : Sid::stickingPosBelow;
    case TextStyleType::SYSTEM:
        return above ? Sid::systemTextPosAbove : Sid::systemTextPosBelow;
    case TextStyleType::TEMPO:
        return above ? Sid::tempoPosAbove : Sid::tempoPosBelow;
    case TextStyleType::MEASURE_NUMBER:
        return above ? Sid::measureNumberPosAbove : Sid::measureNumberPosBelow;
    case TextStyleType::MMREST_RANGE:
        return above ? Sid::mmRestRangePosAbove : Sid::mmRestRangePosBelow;
    default:
        break;
    }
    return Sid::NOSTYLE;
}

//---------------------------------------------------------
//   getHtmlStartTag - helper function for extractText with withFormat = true
//---------------------------------------------------------
String TextBase::getHtmlStartTag(double newSize, double& curSize, const String& newFamily, String& curFamily, FontStyle style,
                                 VerticalAlignment vAlign)
{
    String s;
    if (std::fabs(newSize - curSize) > 0.1) {
        curSize = newSize;
        s += String(u"<font size=\"%1\"/>").arg(newSize);
    }
    if (newFamily != curFamily && newFamily != "ScoreText") {
        curFamily = newFamily;
        s += String(u"<font face=\"%1\"/>").arg(newFamily);
    }
    if (style & FontStyle::Bold) {
        s += u"<b>";
    }
    if (style & FontStyle::Italic) {
        s += u"<i>";
    }
    if (style & FontStyle::Underline) {
        s += u"<u>";
    }
    if (style & mu::engraving::FontStyle::Strike) {
        s += u"<s>";
    }
    if (vAlign == VerticalAlignment::AlignSubScript) {
        s += u"<sub>";
    } else if (vAlign == VerticalAlignment::AlignSuperScript) {
        s += u"<sup>";
    }
    return s;
}

//---------------------------------------------------------
//   getHtmlEndTag - helper function for extractText with withFormat = true
//---------------------------------------------------------
String TextBase::getHtmlEndTag(FontStyle style, VerticalAlignment vAlign)
{
    String s;
    if (vAlign == VerticalAlignment::AlignSubScript) {
        s += u"</sub>";
    } else if (vAlign == VerticalAlignment::AlignSuperScript) {
        s += u"</sup>";
    }
    if (style & FontStyle::Strike) {
        s += u"</s>";
    }
    if (style & FontStyle::Underline) {
        s += u"</u>";
    }
    if (style & FontStyle::Italic) {
        s += u"</i>";
    }
    if (style & FontStyle::Bold) {
        s += u"</b>";
    }
    return s;
}

#ifndef ENGRAVING_NO_ACCESSIBILITY
AccessibleItemPtr TextBase::createAccessible()
{
    return std::make_shared<AccessibleItem>(this, AccessibleItem::EditableText);
}

#endif

void TextBase::notifyAboutTextCursorChanged()
{
#ifndef ENGRAVING_NO_ACCESSIBILITY
    using namespace muse::accessibility;
    if (accessible()) {
        accessible()->accessiblePropertyChanged().send(IAccessible::Property::TextCursor, muse::Val());
    }
#endif
}

void TextBase::notifyAboutTextInserted(int startPosition, int endPosition, const String& text)
{
#ifndef ENGRAVING_NO_ACCESSIBILITY
    using namespace muse::accessibility;
    if (accessible()) {
        auto range = IAccessible::TextRange(startPosition, endPosition, text);
        accessible()->accessiblePropertyChanged().send(IAccessible::Property::TextInsert, muse::Val::fromQVariant(range.toMap()));
    }
#else
    UNUSED(startPosition);
    UNUSED(endPosition);
    UNUSED(text);
#endif
}

void TextBase::notifyAboutTextRemoved(int startPosition, int endPosition, const String& text)
{
#ifndef ENGRAVING_NO_ACCESSIBILITY
    using namespace muse::accessibility;
    if (accessible()) {
        auto range = IAccessible::TextRange(startPosition, endPosition, text);
        accessible()->accessiblePropertyChanged().send(IAccessible::Property::TextRemove, muse::Val::fromQVariant(range.toMap()));
    }
#else
    UNUSED(startPosition);
    UNUSED(endPosition);
    UNUSED(text);
#endif
}

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

Sid TextBase::getPropertyStyle(Pid id) const
{
    if (id == Pid::OFFSET) {
        Sid sid = offsetSid();
        if (sid != Sid::NOSTYLE) {
            return sid;
        }
    }
    for (const StyledProperty& p : *m_elementStyle) {
        if (p.pid == id) {
            return p.sid;
        }
    }
    for (const auto& p : *textStyle(textStyleType())) {
        if (p.pid == id) {
            return p.sid;
        }
    }
    return Sid::NOSTYLE;
}

//---------------------------------------------------------
//   styleChanged
//---------------------------------------------------------

void TextBase::styleChanged()
{
    if (!styledProperties()) {
        LOGD("no styled properties");
        return;
    }
    int i = 0;
    for (const StyledProperty& spp : *m_elementStyle) {
        PropertyFlags f = m_propertyFlagsList[i];
        if (f == PropertyFlags::STYLED) {
            setProperty(spp.pid, styleValue(spp.pid, getPropertyStyle(spp.pid)));
        }
        ++i;
    }
    for (const auto& spp : *textStyle(textStyleType())) {
        PropertyFlags f = m_propertyFlagsList[i];
        if (f == PropertyFlags::STYLED) {
            setProperty(spp.pid, styleValue(spp.pid, getPropertyStyle(spp.pid)));
        }
        ++i;
    }
}

//---------------------------------------------------------
//   initElementStyle
//---------------------------------------------------------

void TextBase::initElementStyle(const ElementStyle* ss)
{
    m_elementStyle = ss;
    size_t n      = ss->size() + TEXT_STYLE_SIZE;

    delete[] m_propertyFlagsList;
    m_propertyFlagsList = new PropertyFlags[n];
    for (size_t i = 0; i < n; ++i) {
        m_propertyFlagsList[i] = PropertyFlags::STYLED;
    }
    for (const StyledProperty& p : *m_elementStyle) {
        setProperty(p.pid, styleValue(p.pid, p.sid));
    }
    for (const auto& p : *textStyle(textStyleType())) {
        setProperty(p.pid, styleValue(p.pid, p.sid));
    }
}

//---------------------------------------------------------
//   initTid
//---------------------------------------------------------

void TextBase::initTextStyleType(TextStyleType tid, bool preserveDifferent)
{
    if (!preserveDifferent) {
        initTextStyleType(tid);
        return;
    }

    const LayoutData* ldata = this->ldata();
    if (!ldata || ldata->layoutInvalid) {
        createBlocks();
    }

    // Before setting the new style - check if any fragments contain custom formatting. If they do, preserve
    // the old values for face, size, and style...
    const bool hadCustomFragments = hasCustomFormatting();

    setTextStyleType(tid);

    for (const auto& p : *textStyle(tid)) {
        const bool isFragmentStyle = p.pid == Pid::FONT_FACE || p.pid == Pid::FONT_SIZE || p.pid == Pid::FONT_STYLE;
        if (isFragmentStyle && hadCustomFragments) {
            continue;
        }
        if (getProperty(p.pid) == propertyDefault(p.pid)) {
            setProperty(p.pid, styleValue(p.pid, p.sid));
        }
    }
}

void TextBase::initTextStyleType(TextStyleType tid)
{
    setTextStyleType(tid);
    for (const auto& p : *textStyle(tid)) {
        setProperty(p.pid, styleValue(p.pid, p.sid));
    }
}

RectF TextBase::drag(EditData& ed)
{
    RectF result = EngravingItem::drag(ed);

    MoveElementAnchors::moveElementAnchorsOnDrag(this, ed);

    return result;
}

void TextBase::endDrag(EditData& ed)
{
    if (m_cursor->editing()) {
        return;
    }
    EngravingItem::endDrag(ed);
}

//---------------------------------------------------------
//   editCut
//---------------------------------------------------------

void TextBase::editCut(EditData& ed)
{
    TextEditData* ted = static_cast<TextEditData*>(ed.getData(this).get());
    TextCursor* cursor = ted->cursor();
    String s = cursor->selectedText(true);

    if (!s.isEmpty()) {
        ted->selectedText = cursor->selectedText(true);
        ed.curGrip = Grip::START;
        ed.key     = Key_Delete;
        ed.s       = String();
        edit(ed);
    }
}

//---------------------------------------------------------
//   editCopy
//---------------------------------------------------------

void TextBase::editCopy(EditData& ed)
{
    //
    // store selection as rich and plain text
    //
    TextEditData* ted = static_cast<TextEditData*>(ed.getData(this).get());
    TextCursor* cursor = ted->cursor();
    ted->selectedText = cursor->selectedText(true);
    ted->selectedPlainText = cursor->selectedText(false);
}

bool TextBase::nudge(const EditData& ed)
{
    bool ctrlMod = ed.modifiers & ControlModifier;
    double step = spatium() * (ctrlMod ? MScore::nudgeStep10 : MScore::nudgeStep);
    PointF addOffset = PointF();
    switch (ed.key) {
    case Key_Up:
        addOffset = PointF(0.0, -step);
        break;
    case Key_Down:
        addOffset = PointF(0.0, step);
        break;
    case Key_Left:
        addOffset = PointF(-step, 0.0);
        break;
    case Key_Right:
        addOffset = PointF(step, 0.0);
        break;
    default:
        return false;
    }
    undoChangeProperty(Pid::OFFSET, offset() + addOffset, PropertyFlags::UNSTYLED);
    return true;
}

//---------------------------------------------------------
//   cursor
//---------------------------------------------------------

TextCursor* TextBase::cursorFromEditData(const EditData& ed)
{
    TextEditData* ted = static_cast<TextEditData*>(ed.getData(this).get());
    assert(ted);
    return ted->cursor();
}

//---------------------------------------------------------
//   hasCustomFormatting
//---------------------------------------------------------

bool TextBase::hasCustomFormatting() const
{
    const LayoutData* ldata = this->ldata();
    IF_ASSERT_FAILED(ldata) {
        return false;
    }

    CharFormat fmt;
    fmt.setFontFamily(family());
    fmt.setFontSize(size());
    fmt.setStyle(fontStyle());
    fmt.setValign(VerticalAlignment::AlignNormal);

    for (const TextBlock& block : ldata->blocks) {
        for (const TextFragment& f : block.fragments()) {
            if (f.text.isEmpty()) {                         // skip empty fragments, not to
                continue;                                   // insert extra HTML formatting
            }
            const CharFormat& format = f.format;
            if (fmt.style() != format.style()) {
                return true;
            }
            if (format.fontSize() != fmt.fontSize()) {
                return true;
            }
            if (format.fontFamily() != fmt.fontFamily()) {
                return true;
            }

            VerticalAlignment va = format.valign();
            VerticalAlignment cva = fmt.valign();
            if (cva != va) {
                return true;
            }
        }
    }
    return false;
}

//---------------------------------------------------------
//   stripText
//    remove some custom text formatting and return
//    result as xml string
//---------------------------------------------------------

String TextBase::stripText(bool removeStyle, bool removeSize, bool removeFace) const
{
    const LayoutData* ldata = this->ldata();
    IF_ASSERT_FAILED(ldata) {
        return String();
    }

    String _txt;
    bool bold_      = false;
    bool italic_    = false;
    bool underline_ = false;
    bool strike_    = false;

    for (const TextBlock& block : ldata->blocks) {
        for (const TextFragment& f : block.fragments()) {
            if (!f.format.bold() && bold()) {
                bold_ = true;
            }
            if (!f.format.italic() && italic()) {
                italic_ = true;
            }
            if (!f.format.underline() && underline()) {
                underline_ = true;
            }
            if (!f.format.strike() && strike()) {
                strike_ = true;
            }
        }
    }
    CharFormat fmt;
    fmt.setFontFamily(family());
    fmt.setFontSize(size());
    fmt.setStyle(fontStyle());
    fmt.setValign(VerticalAlignment::AlignNormal);

    XmlNesting xmlNesting(&_txt);
    if (!removeStyle) {
        if (bold_) {
            xmlNesting.pushB();
        }
        if (italic_) {
            xmlNesting.pushI();
        }
        if (underline_) {
            xmlNesting.pushU();
        }
        if (strike_) {
            xmlNesting.pushS();
        }
    }

    for (const TextBlock& block : ldata->blocks) {
        for (const TextFragment& f : block.fragments()) {
            if (f.text.isEmpty()) {                         // skip empty fragments, not to
                continue;                                   // insert extra HTML formatting
            }
            const CharFormat& format = f.format;
            if (!removeStyle) {
                if (fmt.bold() != format.bold()) {
                    if (format.bold()) {
                        xmlNesting.pushB();
                    } else {
                        xmlNesting.popB();
                    }
                }
                if (fmt.italic() != format.italic()) {
                    if (format.italic()) {
                        xmlNesting.pushI();
                    } else {
                        xmlNesting.popI();
                    }
                }
                if (fmt.underline() != format.underline()) {
                    if (format.underline()) {
                        xmlNesting.pushU();
                    } else {
                        xmlNesting.popU();
                    }
                }
                if (fmt.strike() != format.strike()) {
                    if (format.strike()) {
                        xmlNesting.pushS();
                    } else {
                        xmlNesting.popS();
                    }
                }
            }

            if (!removeSize && (format.fontSize() != fmt.fontSize())) {
                _txt += String(u"<font size=\"%1\"/>").arg(format.fontSize());
            }
            if (!removeFace && (format.fontFamily() != fmt.fontFamily())) {
                _txt += String(u"<font face=\"%1\"/>").arg(TextBase::escape(format.fontFamily()));
            }

            VerticalAlignment va = format.valign();
            VerticalAlignment cva = fmt.valign();
            if (cva != va) {
                switch (va) {
                case VerticalAlignment::AlignNormal:
                    xmlNesting.popToken(cva == VerticalAlignment::AlignSuperScript ? "sup" : "sub");
                    break;
                case VerticalAlignment::AlignSuperScript:
                    xmlNesting.pushToken(u"sup");
                    break;
                case VerticalAlignment::AlignSubScript:
                    xmlNesting.pushToken(u"sub");
                    break;
                case VerticalAlignment::AlignUndefined:
                    break;
                }
            }
            _txt += XmlWriter::xmlString(f.text);
            fmt = format;
        }
        if (block.eol()) {
            _txt += Char::LineFeed;
        }
    }
    while (!xmlNesting.empty()) {
        xmlNesting.popToken();
    }
    return _txt;
}

//---------------------------------------------------------
//   undoChangeProperty
//---------------------------------------------------------

void TextBase::undoChangeProperty(Pid id, const PropertyValue& v, PropertyFlags ps)
{
    if (ps == PropertyFlags::STYLED && v == propertyDefault(id)) {
        // this is a reset
        // remove some custom formatting
        if (id == Pid::FONT_STYLE) {
            undoChangeProperty(Pid::TEXT, stripText(true, false, false), propertyFlags(id));
        } else if (id == Pid::FONT_SIZE) {
            undoChangeProperty(Pid::TEXT, stripText(false, true, false), propertyFlags(id));
        } else if (id == Pid::FONT_FACE) {
            undoChangeProperty(Pid::TEXT, stripText(false, false, true), propertyFlags(id));
        }
    }

    static const PropertyIdSet CHARACTER_SPECIFIC_PROPERTIES {
        Pid::FONT_STYLE,
        Pid::FONT_FACE,
        Pid::FONT_SIZE,
        Pid::TEXT_SCRIPT_ALIGN
    };

    if (!muse::contains(CHARACTER_SPECIFIC_PROPERTIES, id)) {
        EngravingItem::undoChangeProperty(id, v, ps);
        return;
    }

    score()->undo(new ChangeTextProperties(m_cursor, id, v, ps));

    if (m_cursor->editing()) {
        // If we're in edit mode, changes will be propagated later when
        // propagating the Pid::TEXT property, so don't propagate now.
        return;
    }

    const std::list<EngravingObject*> linkedObjects = linkListForPropertyPropagation();
    for (EngravingObject* linkedObject : linkedObjects) {
        TextBase* linkedText = toTextBase(linkedObject);
        if (linkedText == this) {
            continue;
        }

        Score* linkedScore = linkedText->score();
        TextCursor* linkedCursor = linkedText->cursor();
        PropertyPropagation propertyPropagate = propertyPropagation(linkedText, id);

        switch (propertyPropagate) {
        case PropertyPropagation::PROPAGATE:
            linkedScore->undo(new ChangeTextProperties(linkedCursor, id, v, ps));
            break;
        case PropertyPropagation::UNLINK:
            unlinkPropertyFromMaster(id);
            break;
        default:
            break;
        }
    }
}
}
