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

#include <cmath>
#include <stack>

#include "draw/fontmetrics.h"
#include "draw/types/pen.h"
#include "draw/types/brush.h"

#include "infrastructure/symbolfonts.h"

#include "rw/xml.h"

#include "style/defaultstyle.h"
#include "style/textstyle.h"

#include "types/symnames.h"
#include "types/translatablestring.h"
#include "types/typesconv.h"

#include "box.h"
#include "measure.h"
#include "mscore.h"
#include "page.h"
#include "score.h"
#include "system.h"
#include "textedit.h"
#include "textframe.h"
#include "undo.h"

#ifndef ENGRAVING_NO_ACCESSIBILITY
#include "accessibility/accessibleitem.h"
#endif

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
static constexpr double subScriptSize     = 0.6;
static constexpr double subScriptOffset   = 0.5; // of x-height
static constexpr double superScriptOffset = -.9; // of x-height

//---------------------------------------------------------
//   isSorted
/// return true if (r1,c1) is at or before (r2,c2)
//---------------------------------------------------------

static bool isSorted(size_t r1, size_t c1, size_t r2, size_t c2)
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

static void swap(size_t& r1, size_t& c1, size_t& r2, size_t& c2)
{
    std::swap(r1, r2);
    std::swap(c1, c2);
}

//---------------------------------------------------------
//   sort
/// swap (r1,c1) with (r2,c2) if they are not sorted
//---------------------------------------------------------

static void sort(size_t& r1, size_t& c1, size_t& r2, size_t& c2)
{
    if (!isSorted(r1, c1, r2, c2)) {
        swap(r1, c1, r2, c2);
    }
}

const String TextBase::UNDEFINED_FONT_FAMILY = String(u"Undefined");
const int TextBase::UNDEFINED_FONT_SIZE = -1;

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
//   clearSelection
//---------------------------------------------------------

void TextCursor::clearSelection()
{
    _selectLine   = _row;
    _selectColumn = _column;
}

void TextCursor::startEdit()
{
    setRow(0);
    setColumn(0);
    clearSelection();
    _editing = true;
}

void TextCursor::endEdit()
{
    setRow(0);
    setColumn(0);
    clearSelection();
    _editing = false;
}

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void TextCursor::init()
{
    PropertyValue family = _text->propertyDefault(Pid::FONT_FACE);
    _format.setFontFamily(family.value<String>());

    PropertyValue size = _text->propertyDefault(Pid::FONT_SIZE);
    _format.setFontSize(size.toReal());

    PropertyValue style = _text->propertyDefault(Pid::FONT_STYLE);
    _format.setStyle(static_cast<FontStyle>(style.toInt()));

    PropertyValue verticalAlign = _text->propertyDefault(Pid::TEXT_SCRIPT_ALIGN);
    _format.setValign(static_cast<VerticalAlignment>(verticalAlign.toInt()));
}

std::pair<size_t, size_t> TextCursor::positionToLocalCoord(int position) const
{
    int currentPosition = 0;
    for (size_t i = 0; i < _text->rows(); ++i) {
        const TextBlock& t = _text->_layout[i];
        for (size_t j = 0; j < t.columns(); ++j) {
            if (currentPosition == position) {
                return { i, j };
            }

            currentPosition++;
        }
    }

    return { mu::nidx, mu::nidx };
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
    return _text->textBlock(static_cast<int>(_row)).columns();
}

//---------------------------------------------------------
//   currentCharacter
//---------------------------------------------------------

Char TextCursor::currentCharacter() const
{
    const TextBlock& t = _text->_layout[row()];
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
    TextBlock* block = &_text->_layout[_row];
    size_t col = hasSelection() ? selectColumn() : column();
    const CharFormat* format = block->formatAt(static_cast<int>(col));
    if (!format) {
        init();
    } else {
        CharFormat updated = *format;
        if (updated.fontFamily() == "ScoreText") {
            updated.setFontFamily(_format.fontFamily());
        }
        setFormat(updated);
    }
}

//---------------------------------------------------------
//   cursorRect
//---------------------------------------------------------

RectF TextCursor::cursorRect() const
{
    const TextBlock& tline       = curLine();
    const TextFragment* fragment = tline.fragment(static_cast<int>(column()));

    mu::draw::Font _font  = fragment ? fragment->font(_text) : _text->font();
    if (_font.family() == _text->score()->styleSt(Sid::MusicalSymbolFont)) {
        _font.setFamily(_text->score()->styleSt(Sid::MusicalTextFont), draw::Font::Type::MusicSymbolText);
        if (fragment) {
            _font.setPointSizeF(fragment->format.fontSize());
        }
    }
    double ascent = mu::draw::FontMetrics::ascent(_font);
    double h = ascent;
    double x = tline.xpos(column(), _text);
    double y = tline.y() - ascent * .9;
    return RectF(x, y, 4.0, h);
}

//---------------------------------------------------------
//   curLine
//    return the current text line in edit mode
//---------------------------------------------------------

TextBlock& TextCursor::curLine() const
{
    assert(!_text->_layout.empty());
    return _text->_layout[_row];
}

//---------------------------------------------------------
//   changeSelectionFormat
//---------------------------------------------------------

void TextCursor::changeSelectionFormat(FormatId id, const FormatValue& val)
{
    size_t r1 = selectLine();
    size_t r2 = row();
    size_t c1 = selectColumn();
    size_t c2 = column();

    sort(r1, c1, r2, c2);
    size_t rows = _text->rows();
    for (size_t row = 0; row < rows; ++row) {
        TextBlock& t = _text->_layout[row];
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
    _text->layout1();
}

const CharFormat TextCursor::selectedFragmentsFormat() const
{
    if (!_text || _text->fragmentList().empty() || (!hasSelection() && editing())) {
        return _format;
    }

    size_t startColumn = hasSelection() ? std::min(selectColumn(), _column) : 0;
    size_t startRow = hasSelection() ? std::min(selectLine(), _row) : 0;

    size_t endSelectionRow = hasSelection() ? std::max(selectLine(), _row) : _text->rows() - 1;

    const TextFragment* tf = _text->textBlock(static_cast<int>(startRow)).fragment(static_cast<int>(startColumn));
    CharFormat resultFormat = tf ? tf->format : CharFormat();

    for (size_t row = startRow; row <= endSelectionRow; ++row) {
        TextBlock* block = &_text->_layout[row];

        if (block->fragments().empty()) {
            continue;
        }

        size_t endSelectionColumn = hasSelection() ? std::max(selectColumn(), _column) : block->columns();

        for (size_t column = startColumn; column < endSelectionColumn; column++) {
            CharFormat format
                = block->fragment(static_cast<int>(column)) ? block->fragment(static_cast<int>(column))->format : CharFormat();

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
            _text->selectAll(this);
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
                size_t r1 = _selectLine;
                size_t r2 = _row;
                size_t c1 = _selectColumn;
                size_t c2 = _column;

                sort(r1, c1, r2, c2);
                clearSelection();
                _row    = r1;
                _column = c1;
            } else if (_column == 0) {
                if (_row == 0) {
                    return false;
                }
                --_row;
                _column = curLine().columns();
            } else {
                --_column;
            }
            break;

        case TextCursor::MoveOperation::Right:
            if (hasSelection() && mode == TextCursor::MoveMode::MoveAnchor) {
                size_t r1 = _selectLine;
                size_t r2 = _row;
                size_t c1 = _selectColumn;
                size_t c2 = _column;

                sort(r1, c1, r2, c2);
                clearSelection();
                _row    = r2;
                _column = c2;
            } else if (column() >= curLine().columns()) {
                if (_row >= _text->rows() - 1) {
                    return false;
                }
                ++_row;
                _column = 0;
            } else {
                ++_column;
            }
            break;

        case TextCursor::MoveOperation::Up:
            if (_row == 0) {
                return false;
            }
            --_row;
            if (_column > curLine().columns()) {
                _column = curLine().columns();
            }

            break;

        case TextCursor::MoveOperation::Down:
            if (_row >= _text->rows() - 1) {
                return false;
            }
            ++_row;
            if (_column > curLine().columns()) {
                _column = curLine().columns();
            }

            break;

        case TextCursor::MoveOperation::Start:
            _row    = 0;
            _column = 0;

            break;

        case TextCursor::MoveOperation::End:
            _row    = _text->rows() - 1;
            _column = curLine().columns();

            break;

        case TextCursor::MoveOperation::StartOfLine:
            _column = 0;

            break;

        case TextCursor::MoveOperation::EndOfLine:
            _column = curLine().columns();

            break;

        case TextCursor::MoveOperation::WordLeft:
            if (_column > 0) {
                --_column;
                while (_column > 0 && currentCharacter().isSpace()) {
                    --_column;
                }
                while (_column > 0 && !currentCharacter().isSpace()) {
                    --_column;
                }
                if (currentCharacter().isSpace()) {
                    ++_column;
                }
            }
            break;

        case TextCursor::MoveOperation::NextWord: {
            size_t cols =  columns();
            if (_column < cols) {
                ++_column;
                while (_column < cols && !currentCharacter().isSpace()) {
                    ++_column;
                }
                while (_column < cols && currentCharacter().isSpace()) {
                    ++_column;
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
    _text->score()->addRefresh(_text->canvasBoundingRect());

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
    size_t startPosition = _column;

    while (_column > 0 && currentCharacter().isSpace() == selectSpaces) {
        --_column;
    }

    if (currentCharacter().isSpace() != selectSpaces) {
        ++_column;
    }

    _selectColumn = _column;

    _column = startPosition;
    while (_column < curLine().columns() && currentCharacter().isSpace() == selectSpaces) {
        ++_column;
    }

    updateCursorFormat();
    _text->score()->addRefresh(_text->canvasBoundingRect());
}

//---------------------------------------------------------
//   set
//---------------------------------------------------------

bool TextCursor::set(const PointF& p, TextCursor::MoveMode mode)
{
    PointF pt  = p - _text->canvasPos();
    if (!_text->bbox().contains(pt)) {
        return false;
    }
    size_t oldRow    = _row;
    size_t oldColumn = _column;

//      if (_text->_layout.empty())
//            _text->_layout.append(TextBlock());
    _row = 0;
    for (size_t row = 0; row < _text->rows(); ++row) {
        const TextBlock& l = _text->_layout.at(row);
        if (l.y() > pt.y()) {
            _row = row;
            break;
        }
    }
    _column = curLine().column(pt.x(), _text);

    if (oldRow != _row || oldColumn != _column) {
        _text->score()->setUpdateAll();
        if (mode == TextCursor::MoveMode::MoveAnchor) {
            clearSelection();
        }
        updateCursorFormat();
    }
    return true;
}

//---------------------------------------------------------
//   selectedText
//    return current selection
//---------------------------------------------------------

String TextCursor::selectedText(bool withFormat) const
{
    size_t r1 = selectLine();
    size_t r2 = _row;
    size_t c1 = selectColumn();
    size_t c2 = column();
    sort(r1, c1, r2, c2);
    return extractText(static_cast<int>(r1), static_cast<int>(c1), static_cast<int>(r2), static_cast<int>(c2), withFormat);
}

//---------------------------------------------------------
//   extractText
//    return text between (r1,c1) and (r2,c2).
//---------------------------------------------------------

String TextCursor::extractText(int r1, int c1, int r2, int c2, bool withFormat) const
{
    assert(isSorted(r1, c1, r2, c2));
    const std::vector<TextBlock>& tb = _text->_layout;

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
    String result;
    int pos = 0;
    for (size_t i = 0; i < _text->rows(); ++i) {
        const TextBlock& t = _text->_layout[i];

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
    int result = 0;

    for (int i = 0; i < row; ++i) {
        const TextBlock& t = _text->_layout[i];
        result += static_cast<int>(t.columns());
    }

    result += column;

    return result;
}

//---------------------------------------------------------
//   TextFragment
//---------------------------------------------------------

TextFragment::TextFragment()
{
}

TextFragment::TextFragment(const String& s)
{
    text = s;
}

TextFragment::TextFragment(TextCursor* cursor, const String& s)
{
    format = *cursor->format();
    text = s;
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

void TextFragment::draw(mu::draw::Painter* p, const TextBase* t) const
{
    mu::draw::Font f(font(t));
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

void TextBase::drawTextWorkaround(mu::draw::Painter* p, mu::draw::Font& f, const mu::PointF& pos, const String& text)
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

mu::draw::Font TextFragment::font(const TextBase* t) const
{
    mu::draw::Font font;

    double m = format.fontSize();

    if (t->sizeIsSpatiumDependent()) {
        m *= t->spatium() / SPATIUM20;
    }
    if (format.valign() != VerticalAlignment::AlignNormal) {
        m *= subScriptSize;
    }

    String family;
    draw::Font::Type fontType = draw::Font::Type::Unknown;
    if (format.fontFamily() == "ScoreText") {
        if (t->isDynamic() || t->textStyleType() == TextStyleType::OTTAVA) {
            family = SymbolFonts::fontByName(t->score()->styleSt(Sid::MusicalSymbolFont))->family();
            fontType = draw::Font::Type::MusicSymbol;
            // to keep desired size ratio (based on 20pt symbol size to 10pt text size)
            m *= 2;
        } else if (t->isTempoText()) {
            family = t->score()->styleSt(Sid::MusicalTextFont);
            fontType = draw::Font::Type::MusicSymbolText;
            // to keep desired size ratio (based on 20pt symbol size to 12pt text size)
            m *= 5.0 / 3.0;
        } else {
            family = t->score()->styleSt(Sid::MusicalTextFont);
            fontType = draw::Font::Type::MusicSymbolText;
        }
        // check if all symbols are available
        font.setFamily(family, fontType);
        mu::draw::FontMetrics fm(font);

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
            family = String::fromUtf8(SymbolFonts::fallbackTextFont());
            fontType = draw::Font::Type::MusicSymbolText;
        }
    } else {
        family = format.fontFamily();
        fontType = draw::Font::Type::Unknown;
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

void TextBlock::draw(mu::draw::Painter* p, const TextBase* t) const
{
    p->translate(0.0, _y);
    for (const TextFragment& f : _fragments) {
        f.draw(p, t);
    }
    p->translate(0.0, -_y);
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TextBlock::layout(TextBase* t)
{
    _bbox        = RectF();
    double x      = 0.0;
    _lineSpacing = 0.0;
    double lm     = 0.0;

    double layoutWidth = 0;
    EngravingItem* e = t->parentItem();
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
            Measure* m = toMeasure(e);
            layoutWidth = m->bbox().width();
        }
        break;
        default:
            break;
        }
    }

    if (_fragments.empty()) {
        mu::draw::FontMetrics fm = t->fontMetrics();
        _bbox.setRect(0.0, -fm.ascent(), 1.0, fm.descent());
        _lineSpacing = fm.lineSpacing();
    } else if (_fragments.size() == 1 && _fragments.front().text.isEmpty()) {
        auto fi = _fragments.begin();
        TextFragment& f = *fi;
        f.pos.setX(x);
        mu::draw::FontMetrics fm(f.font(t));
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
        _bbox |= temp;
        _lineSpacing = std::max(_lineSpacing, fm.lineSpacing());
    } else {
        const auto fiLast = --_fragments.end();
        for (auto fi = _fragments.begin(); fi != _fragments.end(); ++fi) {
            TextFragment& f = *fi;
            f.pos.setX(x);
            mu::draw::FontMetrics fm(f.font(t));
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

            _bbox   |= fm.tightBoundingRect(f.text).translated(f.pos);
            _lineSpacing = std::max(_lineSpacing, fm.lineSpacing());
        }
    }

    // Apply style/custom line spacing
    _lineSpacing *= t->textLineSpacing();

    double rx = 0;
    switch (t->align().horizontal) {
    case AlignH::LEFT:
        rx = -_bbox.left();
        break;
    case AlignH::HCENTER:
        rx = (layoutWidth - (_bbox.left() + _bbox.right())) * .5;
        break;
    case AlignH::RIGHT:
        rx = layoutWidth - _bbox.right();
        break;
    }

    rx += lm;
    for (TextFragment& f : _fragments) {
        f.pos.rx() += rx;
    }
    _bbox.translate(rx, 0.0);
}

//---------------------------------------------------------
//   fragmentsWithoutEmpty
//---------------------------------------------------------

std::list<TextFragment> TextBlock::fragmentsWithoutEmpty()
{
    std::list<TextFragment> list;
    for (const auto& x : _fragments) {
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
    for (const TextFragment& f : _fragments) {
        if (column == col) {
            return f.pos.x();
        }
        mu::draw::FontMetrics fm(f.font(t));
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
    return _bbox.x();
}

//---------------------------------------------------------
//   fragment
//---------------------------------------------------------

const TextFragment* TextBlock::fragment(int column) const
{
    if (_fragments.empty()) {
        return nullptr;
    }
    int col = 0;
    auto f = _fragments.begin();
    for (; f != _fragments.end(); ++f) {
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
    return RectF(x1, _bbox.y(), x2 - x1, _bbox.height());
}

//---------------------------------------------------------
//   columns
//---------------------------------------------------------

size_t TextBlock::columns() const
{
    size_t col = 0;
    for (const TextFragment& f : _fragments) {
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
    for (const TextFragment& f : _fragments) {
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
            mu::draw::FontMetrics fm(f.font(t));
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
    if (i != _fragments.end()) {
        if (!(i->format == *cursor->format())) {
            if (rcol == 0) {
                _fragments.insert(i, TextFragment(cursor, s));
            } else {
                TextFragment f2 = i->split(rcol);
                i = _fragments.insert(std::next(i), TextFragment(cursor, s));
                _fragments.insert(std::next(i), f2);
            }
        } else {
            i->text.insert(ridx, s);
        }
    } else {
        if (!_fragments.empty() && _fragments.back().format == *cursor->format()) {
            _fragments.back().text.append(s);
        } else {
            _fragments.push_back(TextFragment(cursor, s));
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
    if (_fragments.size() == 0 || _fragments.front().text.isEmpty()) {
        _fragments.insert(_fragments.begin(), TextFragment(cursor, u""));
    }
}

//---------------------------------------------------------
//   removeEmptyFragment
//---------------------------------------------------------

void TextBlock::removeEmptyFragment()
{
    if (_fragments.size() > 0 && _fragments.front().text.isEmpty()) {
        _fragments.pop_back();
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
    for (auto it = _fragments.begin(); it != _fragments.end(); ++it) {
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
    return _fragments.end();
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

String TextBlock::remove(int column, TextCursor* cursor)
{
    int col = 0;
    String s;
    for (auto it = _fragments.begin(); it != _fragments.end(); ++it) {
        int idx  = 0;
        int rcol = 0;

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
                    _fragments.erase(it);
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
            ++rcol;
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
    if (_fragments.size() < 2) {
        return;
    }
    auto i = _fragments.begin();
    TextFragment* f = &*i;
    ++i;
    for (; i != _fragments.end(); ++i) {
        while (i != _fragments.end() && (i->format == f->format)) {
            f->text.append(i->text);
            i = _fragments.erase(i);
        }
        if (i == _fragments.end()) {
            break;
        }
        f = &*i;
    }
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
    for (auto i = _fragments.begin(); i != _fragments.end();) {
        int rcol = 0;
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
                if (i->text.isEmpty() && (_fragments.size() > 1)) {
                    i = _fragments.erase(i);
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
            ++rcol;
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
    for (auto i = _fragments.begin(); i != _fragments.end(); ++i) {
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
            i = _fragments.insert(std::next(i), f);
        } else if (start > col && ((start + n) < endCol)) {
            // middle
            TextFragment lf = i->split(start + n - col);
            TextFragment mf = i->split(start - col);
            mf.changeFormat(id, data);
            i = _fragments.insert(std::next(i), mf);
            i = _fragments.insert(std::next(i), lf);
        } else if (start > col) {
            // right
            TextFragment f = i->split(start - col);
            f.changeFormat(id, data);
            i = _fragments.insert(std::next(i), f);
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
        _valign = static_cast<VerticalAlignment>(std::get<int>(val));
        break;
    case FormatId::FontSize:
        _fontSize = std::get<double>(val);
        break;
    case FormatId::FontFamily:
        _fontFamily = std::get<String>(val);
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
    for (auto it = _fragments.begin(); it != _fragments.end(); ++it) {
        size_t idx = 0;
        for (size_t i = 0; i < it->text.size(); ++i) {
            if (col == column) {
                if (idx) {
                    if (idx < it->text.size()) {
                        TextFragment tf(it->text.mid(idx));
                        tf.format = it->format;
                        tl._fragments.push_back(tf);
                        it->text = it->text.left(idx);
                        ++it;
                    }
                }
                for (; it != _fragments.end(); it = _fragments.erase(it)) {
                    tl._fragments.push_back(*it);
                }

                if (_fragments.size() == 0) {
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
    if (_fragments.size() > 0) {
        tf.format = _fragments.back().format;
    } else if (_fragments.size() == 0) {
        insertEmptyFragmentIfNeeded(cursor);
    }

    tl._fragments.push_back(tf);
    return tl;
}

static String toSymbolXml(Char c)
{
    SymId symId = SymbolFonts::fallbackFont()->fromCode(c.unicode());
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
    for (const auto& f : _fragments) {
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
                    s += XmlWriter::escapeSymbol(c.unicode());
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
    _textLineSpacing        = 1.0;
    _textStyleType          = tid;
    _bgColor                = mu::draw::Color::transparent;
    _frameColor             = mu::draw::Color::black;
    _align                  = { AlignH::LEFT, AlignV::TOP };
    _frameType              = FrameType::NO_FRAME;
    _frameWidth             = Spatium(0.1);
    _paddingWidth           = Spatium(0.2);
    _frameRound             = 0;

    _cursor                 = new TextCursor(this);
    _cursor->init();
}

TextBase::TextBase(const ElementType& type, EngravingItem* parent, ElementFlags f)
    : TextBase(type, parent, TextStyleType::DEFAULT, f)
{
}

TextBase::TextBase(const TextBase& st)
    : EngravingItem(st)
{
    _cursor                      = new TextCursor(this);
    _cursor->setFormat(*(st.cursor()->format()));
    _text                        = st._text;
    textInvalid                  = st.textInvalid;
    _layout                      = st._layout;
    layoutInvalid                = st.layoutInvalid;
    frame                        = st.frame;
    _layoutToParentWidth         = st._layoutToParentWidth;
    hexState                     = -1;

    _textStyleType               = st._textStyleType;
    _textLineSpacing             = st._textLineSpacing;
    _bgColor                     = st._bgColor;
    _frameColor                  = st._frameColor;
    _align                       = st._align;
    _frameType                   = st._frameType;
    _frameWidth                  = st._frameWidth;
    _paddingWidth                = st._paddingWidth;
    _frameRound                  = st._frameRound;

    size_t n = _elementStyle->size() + TEXT_STYLE_SIZE;
    delete[] _propertyFlagsList;
    _propertyFlagsList = new PropertyFlags[n];
    for (size_t i = 0; i < n; ++i) {
        _propertyFlagsList[i] = st._propertyFlagsList[i];
    }
    _links = 0;
}

TextBase::~TextBase()
{
    delete _cursor;
}

//---------------------------------------------------------
//   drawSelection
//---------------------------------------------------------

void TextBase::drawSelection(mu::draw::Painter* p, const RectF& r) const
{
    mu::draw::Brush bg(engravingConfiguration()->selectionColor());
    p->setCompositionMode(mu::draw::CompositionMode::HardLight);
    p->setBrush(bg);
    p->setNoPen();
    p->drawRect(r);
    p->setCompositionMode(mu::draw::CompositionMode::SourceOver);
    p->setPen(textColor());
}

//---------------------------------------------------------
//   textColor
//---------------------------------------------------------

mu::draw::Color TextBase::textColor() const
{
    return curColor();
}

//---------------------------------------------------------
//   insert
//    insert character
//---------------------------------------------------------

void TextBase::insert(TextCursor* cursor, char32_t code)
{
    if (cursor->row() >= rows()) {
        _layout.push_back(TextBlock());
    }
    if (code == '\t') {
        code = ' ';
    }

    String s;
    if (Char::requiresSurrogates(code)) {
        s = String(Char(Char::highSurrogate(code))).append(Char(Char::lowSurrogate(code)));
    } else {
        s = String::fromUcs4(&code, 1);
    }

    if (cursor->row() < rows()) {
        _layout[cursor->row()].insert(cursor, s);
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

void TextBase::createLayout()
{
    // reset all previous formatting information
    _layout.clear();
    TextCursor cursor = *_cursor;
    cursor.setRow(0);
    cursor.setColumn(0);

    int state = 0;
    String token;
    String sym;
    bool symState = false;
    for (size_t i = 0; i < _text.size(); i++) {
        const Char& c = _text.at(i);
        if (state == 0) {
            if (c == '<') {
                state = 1;
                token.clear();
            } else if (c == '&') {
                state = 2;
                token.clear();
            } else if (c == '\n') {
                if (rows() <= cursor.row()) {
                    _layout.push_back(TextBlock());
                }

                if (cursor.row() < rows()) {
                    if (_layout[cursor.row()].fragments().size() == 0) {
                        _layout[cursor.row()].insertEmptyFragmentIfNeeded(&cursor);           // used to preserve the Font size of the line (font info is held in TextFragments, see PR #5881)
                    }

                    _layout[cursor.row()].setEol(true);
                }

                cursor.setRow(cursor.row() + 1);
                cursor.setColumn(0);
                if (rows() <= cursor.row()) {
                    _layout.push_back(TextBlock());
                }

                if (cursor.row() < rows()) {
                    if (_layout[cursor.row()].fragments().size() == 0) {
                        _layout[cursor.row()].insertEmptyFragmentIfNeeded(&cursor); // an empty fragment may be needed on either side of the newline
                    }
                }
            } else {
                if (symState) {
                    sym += c;
                } else {
                    if (c.isHighSurrogate()) {
                        i++;
                        assert(i < _text.size());
                        insert(&cursor, Char::surrogateToUcs4(c, _text.at(i)));
                    } else {
                        insert(&cursor, c.unicode());
                    }
                }
            }
        } else if (state == 1) {
            if (c == '>') {
                state = 0;
                prepareFormat(token, cursor);
                if (token == "sym") {
                    symState = true;
                    sym.clear();
                } else if (token == "/sym") {
                    symState = false;
                    SymId id = SymNames::symIdByName(sym);
                    if (id != SymId::noSym) {
                        CharFormat fmt = *cursor.format(); // save format

                        //char32_t code = score()->scoreFont()->symCode(id);
                        char32_t code = id == SymId::space ? static_cast<char32_t>(' ') : SymbolFonts::fallbackFont()->symCode(id);
                        cursor.format()->setFontFamily(u"ScoreText");
                        insert(&cursor, code);
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
                    insert(&cursor, '<');
                } else if (token == "gt") {
                    insert(&cursor, '>');
                } else if (token == "amp") {
                    insert(&cursor, '&');
                } else if (token == "quot") {
                    insert(&cursor, '"');
                } else {
                    // TODO insert(&cursor, SymNames::symIdByName(token));
                }
            } else {
                token += c;
            }
        }
    }
    if (_layout.empty()) {
        _layout.push_back(TextBlock());
    }
    layoutInvalid = false;
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
            LOGD("cannot parse html property <%s> in text <%s>", muPrintable(token), muPrintable(_text));
        }
    }
    return false;
}

//---------------------------------------------------------
//   prepareFormat - used when reading from XML
//---------------------------------------------------------
void TextBase::prepareFormat(const String& token, TextCursor& cursor)
{
    if (prepareFormat(token, *cursor.format())) {
        setPropertyFlags(Pid::FONT_FACE, PropertyFlags::UNSTYLED);
    }
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TextBase::layout()
{
    setPos(PointF());
    if (!explicitParent()) {
        setOffset(0.0, 0.0);
    }
//      else if (isStyled(Pid::OFFSET))                                   // TODO: should be set already
//            setOffset(propertyDefault(Pid::OFFSET).value<PointF>());
    if (placeBelow()) {
        setPosY(staff() ? staff()->height() : 0.0);
    }
    layout1();
}

//---------------------------------------------------------
//   layout1
//---------------------------------------------------------

void TextBase::layout1()
{
    if (layoutInvalid) {
        createLayout();
    }
    if (_layout.empty()) {
        _layout.push_back(TextBlock());
    }
    RectF bb;
    double y = 0;

    // adjust the bounding box for the text item
    for (size_t i = 0; i < rows(); ++i) {
        TextBlock* t = &_layout[i];
        t->layout(this);
        const RectF* r = &t->boundingRect();

        if (r->height() == 0) {
            r = &_layout[i - i].boundingRect();
        }
        y += t->lineSpacing();
        t->setY(y);
        bb |= r->translated(0.0, y);
    }
    double yoff = 0;
    double h    = 0;
    if (explicitParent()) {
        if (layoutToParentWidth()) {
            if (explicitParent()->isTBox()) {
                // hack: vertical alignment is always TOP
                _align = AlignV::TOP;
            } else if (explicitParent()->isBox()) {
                // consider inner margins of frame
                Box* b = toBox(explicitParent());
                yoff = b->topMargin() * DPMM;

                if (b->height() < bb.bottom()) {
                    h = b->height() / 2 + bb.height();
                } else {
                    h  = b->height() - yoff - b->bottomMargin() * DPMM;
                }
            } else if (explicitParent()->isPage()) {
                Page* p = toPage(explicitParent());
                h = p->height() - p->tm() - p->bm();
                yoff = p->tm();
            } else if (explicitParent()->isMeasure()) {
            } else {
                h  = parentItem()->height();
            }
        }
    } else {
        setPos(PointF());
    }

    if (align() == AlignV::BOTTOM) {
        yoff += h - bb.bottom();
    } else if (align() == AlignV::VCENTER) {
        yoff +=  (h - (bb.top() + bb.bottom())) * .5;
    } else if (align() == AlignV::BASELINE) {
        yoff += h * .5 - _layout.front().lineSpacing();
    } else {
        yoff += -bb.top();
    }

    for (TextBlock& t : _layout) {
        t.setY(t.y() + yoff);
    }

    bb.translate(0.0, yoff);

    setbbox(bb);
    if (hasFrame()) {
        layoutFrame();
    }
    score()->addRefresh(canvasBoundingRect());
}

//---------------------------------------------------------
//   layoutFrame
//---------------------------------------------------------

void TextBase::layoutFrame()
{
//      if (empty()) {    // or bbox.width() <= 1.0
    if (bbox().width() <= 1.0 || bbox().height() < 1.0) {      // or bbox.width() <= 1.0
        // this does not work for Harmony:
        mu::draw::FontMetrics fm(font());
        double ch = fm.ascent();
        double cw = fm.width('n');
        frame = RectF(0.0, -ch, cw, ch);
    } else {
        frame = bbox();
    }

    if (square()) {
        // make sure width >= height
        if (frame.height() > frame.width()) {
            double w = frame.height() - frame.width();
            frame.adjust(-w * .5, 0.0, w * .5, 0.0);
        }
    } else if (circle()) {
        if (frame.width() > frame.height()) {
            frame.setTop(frame.y() + (frame.width() - frame.height()) * -.5);
            frame.setHeight(frame.width());
        } else {
            frame.setLeft(frame.x() + (frame.height() - frame.width()) * -.5);
            frame.setWidth(frame.height());
        }
    }
    double _spatium = spatium();
    double w = (paddingWidth() + frameWidth() * .5f).val() * _spatium;
    frame.adjust(-w, -w, w, w);
    w = frameWidth().val() * _spatium;
    setbbox(frame.adjusted(-w, -w, w, w));
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
    return _cursor->format()->style();
}

String TextBase::family() const
{
    return _cursor->format()->fontFamily();
}

double TextBase::size() const
{
    return _cursor->format()->fontSize();
}

void TextBase::setFontStyle(const FontStyle& val)
{
    _cursor->setFormat(FormatId::Bold, val & FontStyle::Bold);
    _cursor->setFormat(FormatId::Italic, val & FontStyle::Italic);
    _cursor->setFormat(FormatId::Underline, val & FontStyle::Underline);
    _cursor->setFormat(FormatId::Strike, val & FontStyle::Strike);
}

void TextBase::setFamily(const String& val)
{
    _cursor->setFormat(FormatId::FontFamily, val);
}

void TextBase::setSize(const double& val)
{
    _cursor->setFormat(FormatId::FontSize, val);
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

void TextBase::genText() const
{
    _text.clear();
    bool bold_      = false;
    bool italic_    = false;
    bool underline_ = false;
    bool strike_    = false;

    CharFormat fmt;
    fmt.setFontFamily(propertyDefault(Pid::FONT_FACE).value<String>());
    fmt.setFontSize(propertyDefault(Pid::FONT_SIZE).toReal());
    fmt.setStyle(static_cast<FontStyle>(propertyDefault(Pid::FONT_STYLE).toInt()));

    for (const TextBlock& block : _layout) {
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

    XmlNesting xmlNesting(&_text);
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

    for (const TextBlock& block : _layout) {
        for (const TextFragment& f : block.fragments()) {
            // don't skip, empty text fragments hold information for empty lines
//                  if (f.text.isEmpty())                     // skip empty fragments, not to
//                        continue;                           // insert extra HTML formatting
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
                _text += String(u"<font size=\"%1\"/>").arg(format.fontSize());
            }
            if (format.fontFamily() != "ScoreText" && format.fontFamily() != fmt.fontFamily()) {
                _text += String(u"<font face=\"%1\"/>").arg(TextBase::escape(format.fontFamily()));
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
                    _text += toSymbolXml(f.text.at(i));
                }
            } else {
                _text += XmlWriter::xmlString(f.text);
            }
            fmt = format;
        }
        if (block.eol()) {
            _text += Char::LineFeed;
        }
    }
    while (!xmlNesting.empty()) {
        xmlNesting.popToken();
    }
    textInvalid = false;
}

//---------------------------------------------------------
//   selectAll
//---------------------------------------------------------

void TextBase::selectAll(TextCursor* cursor)
{
    if (_layout.empty()) {
        return;
    }

    cursor->setSelectColumn(0);
    cursor->setSelectLine(0);
    cursor->setRow(rows() - 1);
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
//   write
//---------------------------------------------------------

void TextBase::write(XmlWriter& xml) const
{
    if (!xml.context()->canWrite(this)) {
        return;
    }
    xml.startElement(this);
    writeProperties(xml, true, true);
    xml.endElement();
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void TextBase::read(XmlReader& e)
{
    while (e.readNextStartElement()) {
        if (!readProperties(e)) {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void TextBase::writeProperties(XmlWriter& xml, bool writeText, bool /*writeStyle*/) const
{
    EngravingItem::writeProperties(xml);
    writeProperty(xml, Pid::TEXT_STYLE);

    for (const StyledProperty& spp : *_elementStyle) {
        if (!isStyled(spp.pid)) {
            writeProperty(xml, spp.pid);
        }
    }
    for (const auto& spp : *textStyle(textStyleType())) {
        if (!isStyled(spp.pid)) {
            writeProperty(xml, spp.pid);
        }
    }
    if (writeText) {
        xml.writeXml(u"text", xmlText());
    }
}

static constexpr std::array<Pid, 18> TextBasePropertyId { {
    Pid::TEXT_STYLE,
    Pid::FONT_FACE,
    Pid::FONT_SIZE,
    Pid::TEXT_LINE_SPACING,
    Pid::FONT_STYLE,
    Pid::COLOR,
    Pid::FRAME_TYPE,
    Pid::FRAME_WIDTH,
    Pid::FRAME_PADDING,
    Pid::FRAME_ROUND,
    Pid::FRAME_FG_COLOR,
    Pid::FRAME_BG_COLOR,
    Pid::ALIGN,
} };

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool TextBase::readProperties(XmlReader& e)
{
    const AsciiStringView tag(e.name());
    for (Pid i : TextBasePropertyId) {
        if (readProperty(tag, e, i)) {
            return true;
        }
    }
    if (tag == "text") {
        setXmlText(e.readXml());
    } else if (tag == "bold") {
        bool val = e.readInt();
        if (val) {
            setFontStyle(fontStyle() + FontStyle::Bold);
        } else {
            setFontStyle(fontStyle() - FontStyle::Bold);
        }
        if (isStyled(Pid::FONT_STYLE)) {
            setPropertyFlags(Pid::FONT_STYLE, PropertyFlags::UNSTYLED);
        }
    } else if (tag == "italic") {
        bool val = e.readInt();
        if (val) {
            setFontStyle(fontStyle() + FontStyle::Italic);
        } else {
            setFontStyle(fontStyle() - FontStyle::Italic);
        }
        if (isStyled(Pid::FONT_STYLE)) {
            setPropertyFlags(Pid::FONT_STYLE, PropertyFlags::UNSTYLED);
        }
    } else if (tag == "underline") {
        bool val = e.readInt();
        if (val) {
            setFontStyle(fontStyle() + FontStyle::Underline);
        } else {
            setFontStyle(fontStyle() - FontStyle::Underline);
        }
        if (isStyled(Pid::FONT_STYLE)) {
            setPropertyFlags(Pid::FONT_STYLE, PropertyFlags::UNSTYLED);
        }
    } else if (tag == "strike") {
        bool val = e.readInt();
        if (val) {
            setFontStyle(fontStyle() + FontStyle::Strike);
        } else {
            setFontStyle(fontStyle() - FontStyle::Strike);
        }
        if (isStyled(Pid::FONT_STYLE)) {
            setPropertyFlags(Pid::FONT_STYLE, PropertyFlags::UNSTYLED);
        }
    } else if (!EngravingItem::readProperties(e)) {
        return false;
    }
    return true;
}

//---------------------------------------------------------
//   pageRectangle
//---------------------------------------------------------

RectF TextBase::pageRectangle() const
{
    if (explicitParent() && (explicitParent()->isHBox() || explicitParent()->isVBox() || explicitParent()->isTBox())) {
        Box* box = toBox(explicitParent());
        RectF r = box->abbox();
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
        RectF r = box->abbox();
        double x = r.x() + box->lm();
        double y = r.y() + box->tm();
        double h = r.height() - box->tm() - box->bm();
        double w = r.width() - box->lm() - box->rm();
        return RectF(x, y, w, h);
    }
    return abbox();
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
        line.setP2(line.p2() + bbox().topLeft());
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
//   layoutEdit
//---------------------------------------------------------

void TextBase::layoutEdit()
{
    layout();
    if (explicitParent() && explicitParent()->type() == ElementType::TBOX) {
        TBox* tbox = toTBox(explicitParent());
        tbox->layout();
        System* system = tbox->system();
        system->setHeight(tbox->height());
        triggerLayout();
    } else {
        static const double w = 2.0;     // 8.0 / view->matrix().m11();
        score()->addRefresh(canvasBoundingRect().adjusted(-w, -w, w, w));
    }
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
    _text = s;
    textInvalid = false;
    layoutInvalid = true;
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
//   plainText
//    return plain text with symbols
//---------------------------------------------------------

String TextBase::plainText() const
{
    String s;

    const TextBase* text = this;
    std::unique_ptr<TextBase> tmpText;
    if (layoutInvalid) {
        // Create temporary text object to avoid side effects
        // of createLayout() call.
        tmpText.reset(toTextBase(this->clone()));
        tmpText->createLayout();
        text = tmpText.get();
    }

    for (const TextBlock& block : text->_layout) {
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
    // this is way too expensive
    // what side effects has genText() ?
    // this method is const by design

    const TextBase* text = this;
    std::unique_ptr<TextBase> tmpText;
    if (textInvalid) {
        // Create temporary text object to avoid side effects
        // of genText() call.
        tmpText.reset(toTextBase(this->clone()));
        tmpText->genText();
        text = tmpText.get();
    }
    return text->_text;
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
    case TextStyleType::POET:
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
    case TextStyleType::POET:
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
//   fragmentList
//---------------------------------------------------------

/*
 Return the text as a single list of TextFragment
 Used by the MusicXML formatted export to avoid parsing the xml text format
 */

std::list<TextFragment> TextBase::fragmentList() const
{
    std::list<TextFragment> res;
    for (const TextBlock& block : _layout) {
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
    ByteArray ba = ss.toUtf8();
    XmlReader xml(ba);
    while (xml.readNextStartElement()) {
        // LOGD("  token %d <%s>", int(xml.tokenType()), muPrintable(xml.name().toString()));
    }
    if (xml.error() == XmlReader::NoError) {
        s = d;
        return true;
    }
    LOGD("xml error at line %lld column %lld: %s", xml.lineNumber(), xml.columnNumber(), muPrintable(xml.errorString()));
    LOGD("text: |%s|", muPrintable(ss));
    return false;
}

//---------------------------------------------------------
//   font
//---------------------------------------------------------

mu::draw::Font TextBase::font() const
{
    double m = size();
    if (sizeIsSpatiumDependent()) {
        m *= spatium() / SPATIUM20;
    }
    mu::draw::Font f(family(), draw::Font::Type::Unknown);
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

mu::draw::FontMetrics TextBase::fontMetrics() const
{
    return mu::draw::FontMetrics(font());
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
        return _cursor->selectedFragmentsFormat().fontFamily();
    case Pid::FONT_SIZE:
        return _cursor->selectedFragmentsFormat().fontSize();
    case Pid::FONT_STYLE:
        return static_cast<int>(_cursor->selectedFragmentsFormat().style());
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
    case Pid::TEXT_SCRIPT_ALIGN:
        return static_cast<int>(_cursor->selectedFragmentsFormat().valign());
    case Pid::TEXT:
        return xmlText();
    default:
        return EngravingItem::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool TextBase::setProperty(Pid pid, const PropertyValue& v)
{
    if (textInvalid) {
        genText();
    }

    bool rv = true;
    switch (pid) {
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
        setFrameColor(v.value<mu::draw::Color>());
        break;
    case Pid::FRAME_BG_COLOR:
        setBgColor(v.value<mu::draw::Color>());
        break;
    case Pid::TEXT:
        setXmlText(v.value<String>());
        break;
    case Pid::ALIGN:
        setAlign(v.value<Align>());
        break;
    case Pid::TEXT_SCRIPT_ALIGN:
        _cursor->setFormat(FormatId::Valign, v.toInt());
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
    for (const StyledProperty& p : *_elementStyle) {
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
    if (fabs(newSize - curSize) > 0.1) {
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
    if (accessible()) {
        accessible()->accessiblePropertyChanged().send(accessibility::IAccessible::Property::TextCursor, Val());
    }
#endif
}

void TextBase::notifyAboutTextInserted(int startPosition, int endPosition, const String& text)
{
#ifndef ENGRAVING_NO_ACCESSIBILITY
    if (accessible()) {
        auto range = accessibility::IAccessible::TextRange(startPosition, endPosition, text);
        accessible()->accessiblePropertyChanged().send(accessibility::IAccessible::Property::TextInsert, Val::fromQVariant(range.toMap()));
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
    if (accessible()) {
        auto range = accessibility::IAccessible::TextRange(startPosition, endPosition, text);
        accessible()->accessiblePropertyChanged().send(accessibility::IAccessible::Property::TextRemove, Val::fromQVariant(range.toMap()));
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
    for (const StyledProperty& p : *_elementStyle) {
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
    for (const StyledProperty& spp : *_elementStyle) {
        PropertyFlags f = _propertyFlagsList[i];
        if (f == PropertyFlags::STYLED) {
            setProperty(spp.pid, styleValue(spp.pid, getPropertyStyle(spp.pid)));
        }
        ++i;
    }
    for (const auto& spp : *textStyle(textStyleType())) {
        PropertyFlags f = _propertyFlagsList[i];
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
    _elementStyle = ss;
    size_t n      = ss->size() + TEXT_STYLE_SIZE;

    delete[] _propertyFlagsList;
    _propertyFlagsList = new PropertyFlags[n];
    for (size_t i = 0; i < n; ++i) {
        _propertyFlagsList[i] = PropertyFlags::STYLED;
    }
    for (const StyledProperty& p : *_elementStyle) {
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
    } else {
        setTextStyleType(tid);
        for (const auto& p : *textStyle(tid)) {
            if (getProperty(p.pid) == propertyDefault(p.pid)) {
                setProperty(p.pid, styleValue(p.pid, p.sid));
            }
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
    // store selection as plain text
    //
    TextEditData* ted = static_cast<TextEditData*>(ed.getData(this).get());
    TextCursor* cursor = ted->cursor();
    ted->selectedText = cursor->selectedText(true);
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
//   draw
//---------------------------------------------------------

void TextBase::draw(mu::draw::Painter* painter) const
{
    TRACE_OBJ_DRAW;
    using namespace mu::draw;
    if (hasFrame()) {
        double baseSpatium = DefaultStyle::baseStyle().value(Sid::spatium).toReal();
        if (frameWidth().val() != 0.0) {
            Color fColor = curColor(visible(), frameColor());
            double frameWidthVal = frameWidth().val() * (sizeIsSpatiumDependent() ? spatium() : baseSpatium);

            Pen pen(fColor, frameWidthVal, PenStyle::SolidLine, PenCapStyle::SquareCap, PenJoinStyle::MiterJoin);
            painter->setPen(pen);
        } else {
            painter->setNoPen();
        }
        Color bg(bgColor());
        painter->setBrush(bg.alpha() ? Brush(bg) : BrushStyle::NoBrush);
        if (circle()) {
            painter->drawEllipse(frame);
        } else {
            double frameRoundFactor = (sizeIsSpatiumDependent() ? (spatium() / baseSpatium) / 2 : 0.5f);

            int r2 = frameRound() * frameRoundFactor;
            if (r2 > 99) {
                r2 = 99;
            }
            painter->drawRoundedRect(frame, frameRound() * frameRoundFactor, r2);
        }
    }
    painter->setBrush(BrushStyle::NoBrush);
    painter->setPen(textColor());
    for (const TextBlock& t : _layout) {
        t.draw(painter, this);
    }
}

//---------------------------------------------------------
//   drawEditMode
//    draw edit mode decorations
//---------------------------------------------------------

void TextBase::drawEditMode(mu::draw::Painter* p, EditData& ed, double currentViewScaling)
{
    using namespace mu::draw;
    PointF pos(canvasPos());
    p->translate(pos);

    TextEditData* ted = static_cast<TextEditData*>(ed.getData(this).get());
    if (!ted) {
        LOGD("ted not found");
        return;
    }
    TextCursor* cursor = ted->cursor();

    if (cursor->hasSelection()) {
        p->setBrush(BrushStyle::NoBrush);
        p->setPen(textColor());
        size_t r1 = cursor->selectLine();
        size_t r2 = cursor->row();
        size_t c1 = cursor->selectColumn();
        size_t c2 = cursor->column();

        sort(r1, c1, r2, c2);
        size_t row = 0;
        for (const TextBlock& t : _layout) {
            t.draw(p, this);
            if (row >= r1 && row <= r2) {
                RectF br;
                if (row == r1 && r1 == r2) {
                    br = t.boundingRect(static_cast<int>(c1), static_cast<int>(c2), this);
                } else if (row == r1) {
                    br = t.boundingRect(static_cast<int>(c1), static_cast<int>(t.columns()), this);
                } else if (row == r2) {
                    br = t.boundingRect(0, static_cast<int>(c2), this);
                } else {
                    br = t.boundingRect();
                }
                br.translate(0.0, t.y());
                drawSelection(p, br);
            }
            ++row;
        }
    }
    p->setBrush(curColor());
    Pen pen(curColor());
    pen.setJoinStyle(PenJoinStyle::MiterJoin);
    p->setPen(pen);

    // Don't draw cursor if there is a selection
    if (!cursor->hasSelection()) {
        p->drawRect(cursor->cursorRect());
    }

    p->translate(-pos);
    p->setPen(Pen(engravingConfiguration()->formattingMarksColor(), 2.0 / currentViewScaling)); // 2 pixel pen size
    p->setBrush(BrushStyle::NoBrush);

    double m = spatium();
    RectF r = canvasBoundingRect().adjusted(-m, -m, m, m);

    p->drawRect(r);
    pen = Pen(engravingConfiguration()->defaultColor(), 0.0);
}

//---------------------------------------------------------
//   hasCustomFormatting
//---------------------------------------------------------

bool TextBase::hasCustomFormatting() const
{
    CharFormat fmt;
    fmt.setFontFamily(family());
    fmt.setFontSize(size());
    fmt.setStyle(fontStyle());
    fmt.setValign(VerticalAlignment::AlignNormal);

    for (const TextBlock& block : _layout) {
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
    String _txt;
    bool bold_      = false;
    bool italic_    = false;
    bool underline_ = false;
    bool strike_    = false;

    for (const TextBlock& block : _layout) {
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

    for (const TextBlock& block : _layout) {
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
    if (id == Pid::FONT_STYLE || id == Pid::FONT_FACE || id == Pid::FONT_SIZE || id == Pid::TEXT_SCRIPT_ALIGN) {
        // can't use standard change property as Undo might set to "undefined"
        score()->undo(new ChangeTextProperties(_cursor, id, v, ps));
    } else {
        EngravingItem::undoChangeProperty(id, v, ps);
    }
}
}
