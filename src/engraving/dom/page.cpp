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

#include "page.h"

#ifndef ENGRAVING_NO_ACCESSIBILITY
#include "accessibility/accessibleitem.h"
#endif

#include "factory.h"
#include "masterscore.h"
#include "measurebase.h"
#include "mscore.h"
#include "score.h"
#include "system.h"
#include "text.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//! FIXME
//extern String revision;
static String revision;

//---------------------------------------------------------
//   Page
//---------------------------------------------------------

Page::Page(RootItem* parent)
    : EngravingItem(ElementType::PAGE, parent, ElementFlag::NOT_SELECTABLE), m_no(0)
{
    m_bspTreeValid = false;
}

//---------------------------------------------------------
//   items
//---------------------------------------------------------

std::vector<EngravingItem*> Page::items(const RectF& rect)
{
    if (!m_bspTreeValid) {
        doRebuildBspTree();
    }
    return bspTree.items(rect);
}

std::vector<EngravingItem*> Page::items(const PointF& point)
{
    if (!m_bspTreeValid) {
        doRebuildBspTree();
    }
    return bspTree.items(point);
}

//---------------------------------------------------------
//   appendSystem
//---------------------------------------------------------

void Page::appendSystem(System* s)
{
    s->moveToPage(this);
    m_systems.push_back(s);
}

//---------------------------------------------------------
//   layoutHeaderFooter
//---------------------------------------------------------

Text* Page::layoutHeaderFooter(int area, const String& ss) const
{
    bool isHeader = area < MAX_HEADERS;

    TextBlock tb = replaceTextMacros(isHeader, ss);
    if (tb.fragmentsWithoutEmpty().empty()) {
        return nullptr;
    }

    //! NOTE: Keep in sync with replaceTextMacros
    std::wregex copyrightSearch(LR"(\$[cC])");
    std::wregex pageNumberSearch(LR"(\$[pPnN])");
    bool containsCopyright = ss.contains(copyrightSearch);
    bool containsPageNumber = ss.contains(pageNumberSearch);

    // Slight hack - we'll use copyright/page number styling if the string contains copyright or page number
    // macros (hack because any non-copyright text in the same block will also adopt these style values)
    TextStyleType style = containsCopyright ? TextStyleType::COPYRIGHT
                          : (containsPageNumber ? TextStyleType::PAGE_NUMBER
                             : (isHeader ? TextStyleType::HEADER : TextStyleType::FOOTER));

    Text* text;
    if (isHeader) {
        text = score()->headerText(area);
        if (!text) {
            text = Factory::createText((Page*)this, style);
            text->setFlag(ElementFlag::MOVABLE, false);
            text->setFlag(ElementFlag::GENERATED, true);       // set to disable editing
            text->setLayoutToParentWidth(true);
            score()->setHeaderText(text, area);
        }
    } else {
        text = score()->footerText(area - MAX_HEADERS);     // because they are 3 4 5
        if (!text) {
            text = Factory::createText((Page*)this, style);
            text->setFlag(ElementFlag::MOVABLE, false);
            text->setFlag(ElementFlag::GENERATED, true);       // set to disable editing
            text->setLayoutToParentWidth(true);
            score()->setFooterText(text, area - MAX_HEADERS);
        }
    }
    text->setParent((Page*)this);

    Align align = { AlignH::LEFT, AlignV::TOP };
    switch (area) {
    case 0: align = { AlignH::LEFT, AlignV::TOP };
        break;
    case 1: align = { AlignH::HCENTER, AlignV::TOP };
        break;
    case 2: align = { AlignH::RIGHT, AlignV::TOP };
        break;
    case 3: align = { AlignH::LEFT, AlignV::BOTTOM };
        break;
    case 4: align = { AlignH::HCENTER, AlignV::BOTTOM };
        break;
    case 5: align = { AlignH::RIGHT, AlignV::BOTTOM };
        break;
    }
    text->setAlign(align);

    // Generates text from ldata, ensures newlines are formatted properly...
    text->mutldata()->blocks = { tb };
    text->genText();
    text->createBlocks();

    renderer()->layoutItem(text);

    return text;
}

#ifndef ENGRAVING_NO_ACCESSIBILITY
AccessibleItemPtr Page::createAccessible()
{
    return std::make_shared<AccessibleItem>(this, AccessibleItem::Group);
}

#endif

//---------------------------------------------------------
//   headerExtension
//   - how much the header extends into the page (i.e., not in the margins)
//---------------------------------------------------------

double Page::headerExtension() const
{
    bool shouldLayoutHeader = score()->isLayoutMode(LayoutMode::PAGE) || score()->isLayoutMode(LayoutMode::FLOAT);
    if (!shouldLayoutHeader) {
        return 0.0;
    }

    page_idx_t n = no() + 1 + score()->pageNumberOffset();

    String s1, s2, s3;

    if (style().styleB(Sid::showHeader) && (no() || style().styleB(Sid::headerFirstPage))) {
        bool odd = (n & 1) || !style().styleB(Sid::headerOddEven);
        if (odd) {
            s1 = style().styleSt(Sid::oddHeaderL);
            s2 = style().styleSt(Sid::oddHeaderC);
            s3 = style().styleSt(Sid::oddHeaderR);
        } else {
            s1 = style().styleSt(Sid::evenHeaderL);
            s2 = style().styleSt(Sid::evenHeaderC);
            s3 = style().styleSt(Sid::evenHeaderR);
        }

        Text* headerLeft = layoutHeaderFooter(0, s1);
        Text* headerCenter = layoutHeaderFooter(1, s2);
        Text* headerRight = layoutHeaderFooter(2, s3);

        double headerLeftHeight = headerLeft ? headerLeft->height() : 0.0;
        double headerCenterHeight = headerCenter ? headerCenter->height() : 0.0;
        double headerRightHeight = headerRight ? headerRight->height() : 0.0;

        double headerHeight = std::max(headerLeftHeight, std::max(headerCenterHeight, headerRightHeight));
        double headerOffset = style().styleV(Sid::headerOffset).value<PointF>().y() * DPMM;
        return std::max(0.0, headerHeight - headerOffset);
    }

    return 0.0;
}

//---------------------------------------------------------
//   footerExtension
//   - how much the footer extends into the page (i.e., not in the margins)
//---------------------------------------------------------

double Page::footerExtension() const
{
    bool shouldLayoutFooter = score()->isLayoutMode(LayoutMode::PAGE) || score()->isLayoutMode(LayoutMode::FLOAT);
    if (!shouldLayoutFooter) {
        return 0.0;
    }

    page_idx_t n = no() + 1 + score()->pageNumberOffset();

    String s1, s2, s3;

    if (style().styleB(Sid::showFooter) && (no() || style().styleB(Sid::footerFirstPage))) {
        bool odd = (n & 1) || !style().styleB(Sid::footerOddEven);
        if (odd) {
            s1 = style().styleSt(Sid::oddFooterL);
            s2 = style().styleSt(Sid::oddFooterC);
            s3 = style().styleSt(Sid::oddFooterR);
        } else {
            s1 = style().styleSt(Sid::evenFooterL);
            s2 = style().styleSt(Sid::evenFooterC);
            s3 = style().styleSt(Sid::evenFooterR);
        }

        Text* footerLeft = layoutHeaderFooter(3, s1);
        Text* footerCenter = layoutHeaderFooter(4, s2);
        Text* footerRight = layoutHeaderFooter(5, s3);

        double footerLeftHeight = footerLeft ? footerLeft->height() : 0.0;
        double footerCenterHeight = footerCenter ? footerCenter->height() : 0.0;
        double footerRightHeight = footerRight ? footerRight->height() : 0.0;

        double footerHeight = std::max(footerLeftHeight, std::max(footerCenterHeight, footerRightHeight));

        double footerOffset = style().styleV(Sid::footerOffset).value<PointF>().y() * DPMM;
        return std::max(0.0, footerHeight - footerOffset);
    }

    return 0.0;
}

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Page::scanElements(void* data, void (* func)(void*, EngravingItem*), bool all)
{
    for (System* s :m_systems) {
        for (MeasureBase* m : s->measures()) {
            m->scanElements(data, func, all);
        }
        s->scanElements(data, func, all);
    }
    func(data, this);
}

//---------------------------------------------------------
//   bspInsert
//---------------------------------------------------------

static void bspInsert(void* bspTree, EngravingItem* e)
{
    ((BspTree*)bspTree)->insert(e);
}

static void countElements(void* data, EngravingItem* /*e*/)
{
    ++(*(int*)data);
}

//---------------------------------------------------------
//   doRebuildBspTree
//---------------------------------------------------------

void Page::doRebuildBspTree()
{
    int n = 0;
    scanElements(&n, countElements, false);

    RectF r;
    if (score()->linearMode()) {
        double w = 0.0;
        double h = 0.0;
        if (!m_systems.empty()) {
            h = m_systems.front()->height();
            if (!m_systems.front()->measures().empty()) {
                MeasureBase* mb = m_systems.front()->measures().back();
                w = mb->x() + mb->width();
            }
        }
        r = RectF(0.0, 0.0, w, h);
    } else {
        r = pageBoundingRect();
    }

    bspTree.initialize(r, n);
    scanElements(&bspTree, &bspInsert, false);
    m_bspTreeValid = true;
}

//---------------------------------------------------------
//   replaceTextMacros
//   (keep in sync with toolTipHeaderFooter in EditStyle::EditStyle())
//    $p          - page number, except on first page
//    $P          - page number, on all pages
//    $N          - page number, if there is more than one
//    $n          - number of pages
//    $i          - part name, except on first page
//    $I          - part name, on all pages
//    $f          - file name
//    $F          - file path+name
//    $d          - current date
//    $D          - creation date
//    $m          - last modification time
//    $M          - last modification date
//    $C          - copyright, on first page only
//    $c          - copyright, on all pages
//    $v          - MuseScore version the score was last saved with
//    $r          - MuseScore revision the score was last saved with
//    $$          - the $ sign itself
//    $:tag:      - any metadata tag
//
//       tags always defined (see Score::init())):
//       copyright
//       creationDate
//       movementNumber
//       movementTitle
//       platform
//       source
//       workNumber
//       workTitle
//---------------------------------------------------------

TextBlock Page::replaceTextMacros(bool isHeader, const String& s) const
{
    // If the string in question consists solely of a "styled macro" (i.e. page number or copyright), we can set the default
    // format to the associated styling. We'll use this later to prevent the creation of unneccessary fragments...
    CharFormat defaultFormat = formatForMacro(s);
    if (defaultFormat == CharFormat()) {
        // The string isn't just a styled macro, use header/footer styling...
        defaultFormat.setStyle(style().styleV(isHeader ? Sid::headerFontStyle : Sid::footerFontStyle).value<FontStyle>());
        defaultFormat.setFontSize(style().styleD(isHeader ? Sid::headerFontSize : Sid::footerFontSize));
        defaultFormat.setFontFamily(style().styleSt(isHeader ? Sid::headerFontFace : Sid::footerFontFace));
    }

    std::list<TextFragment> fragments(1);
    for (size_t i = 0, n = s.size(); i < n; ++i) {
        fragments.back().format = defaultFormat;
        Char c = s.at(i);
        if (c == '$' && (i < (n - 1))) {
            Char nc = s.at(i + 1);
            switch (nc.toAscii()) {
            case 'p': // not on first page 1
                if (!m_no) {
                    break;
                }
            // FALLTHROUGH
            case 'N': // on page 1 only if there are multiple pages
                if ((score()->npages() + score()->pageNumberOffset()) <= 1) {
                    break;
                }
            // FALLTHROUGH
            case 'P': // on all pages
            {
                int no = static_cast<int>(m_no) + 1 + score()->pageNumberOffset();
                if (no > 0) {
                    const String pageNumberString = String::number(no);
                    const CharFormat pageNumberFormat = formatForMacro(String('$' + nc));
                    // If the default format equals the format for this macro, we don't need to create a new fragment...
                    if (defaultFormat == pageNumberFormat) {
                        fragments.back().text += pageNumberString;
                        break;
                    }
                    TextFragment pageNumberFragment(pageNumberString);
                    pageNumberFragment.format = pageNumberFormat;
                    fragments.emplace_back(pageNumberFragment);
                    fragments.emplace_back(TextFragment());     // Start next fragment
                }
            }
            break;
            case 'n':
                fragments.back().text += String::number(score()->npages() + score()->pageNumberOffset());
                break;
            case 'i': // not on first page
                if (!m_no) {
                    break;
                }
            // FALLTHROUGH
            case 'I':
                fragments.back().text += score()->metaTag(u"partName");
                break;
            case 'f':
                fragments.back().text += masterScore()->fileInfo()->fileName(false).toString();
                break;
            case 'F':
                fragments.back().text += masterScore()->fileInfo()->path().toString();
                break;
            case 'd':
                fragments.back().text += muse::Date::currentDate().toString(muse::DateFormat::ISODate);
                break;
            case 'D':
            {
                String creationDate = score()->metaTag(u"creationDate");
                if (creationDate.isEmpty()) {
                    fragments.back().text += masterScore()->fileInfo()->birthTime().date().toString(
                        muse::DateFormat::ISODate);
                } else {
                    fragments.back().text += muse::Date::fromStringISOFormat(creationDate).toString(
                        muse::DateFormat::ISODate);
                }
            }
            break;
            case 'm':
                if (score()->dirty() || !masterScore()->saved()) {
                    fragments.back().text += muse::Time::currentTime().toString(muse::DateFormat::ISODate);
                } else {
                    fragments.back().text += masterScore()->fileInfo()->lastModified().time().toString(
                        muse::DateFormat::ISODate);
                }
                break;
            case 'M':
                if (score()->dirty() || !masterScore()->saved()) {
                    fragments.back().text += muse::Date::currentDate().toString(muse::DateFormat::ISODate);
                } else {
                    fragments.back().text += masterScore()->fileInfo()->lastModified().date().toString(
                        muse::DateFormat::ISODate);
                }
                break;
            case 'C': // only on first page
                if (m_no) {
                    break;
                }
            // FALLTHROUGH
            case 'c':
            {
                const String copyrightString = score()->metaTag(u"copyright");
                const CharFormat copyrightFormat = formatForMacro(String('$' + nc));
                // If the default format equals the format for this macro, we don't need to create a new fragment...
                if (defaultFormat == copyrightFormat) {
                    fragments.back().text += copyrightString;
                    break;
                }
                TextFragment copyrightFragment(copyrightString);
                copyrightFragment.format = copyrightFormat;
                fragments.emplace_back(copyrightFragment);
                fragments.emplace_back(TextFragment());     // Start next fragment
            }
            break;
            case 'v':
                if (score()->dirty()) {
                    fragments.back().text += score()->appVersion();
                } else {
                    fragments.back().text += score()->mscoreVersion();
                }
                break;
            case 'r':
                if (score()->dirty()) {
                    fragments.back().text += revision;
                } else {
                    int rev = score()->mscoreRevision();
                    if (rev > 99999) { // MuseScore 1.3 is decimal 5702, 2.0 and later uses a 7-digit hex SHA
                        fragments.back().text += String::number(rev, 16);
                    } else {
                        fragments.back().text += String::number(rev, 10);
                    }
                }
                break;
            case '$':
                fragments.back().text += '$';
                break;
            case ':':
            {
                String tag;
                size_t k = i + 2;
                for (; k < n; ++k) {
                    if (s.at(k) == u':') {
                        break;
                    }
                    tag += s.at(k);
                }
                if (k != n) {       // found ':' ?
                    fragments.back().text += score()->metaTag(tag);
                    i = k - 1;
                }
            }
            break;
            default:
                fragments.back().text += '$';
                fragments.back().text += nc;
                break;
            }
            ++i;
        } else {
            fragments.back().text += c;
        }
    }

    TextBlock tb;
    tb.fragments() = fragments;
    return tb;
}

//---------------------------------------------------------
//   formatForMacro
//---------------------------------------------------------

const CharFormat Page::formatForMacro(const String& s) const
{
    CharFormat format;
    if (s == "$c" || s == "$C") {
        format.setStyle(style().styleV(Sid::copyrightFontStyle).value<FontStyle>());
        format.setFontSize(style().styleD(Sid::copyrightFontSize));
        format.setFontFamily(style().styleSt(Sid::copyrightFontFace));
    } else if (s == "$p" || s == "$P" || s == "$n" || s == "$N") {
        format.setStyle(style().styleV(Sid::pageNumberFontStyle).value<FontStyle>());
        format.setFontSize(style().styleD(Sid::pageNumberFontSize));
        format.setFontFamily(style().styleSt(Sid::pageNumberFontFace));
    }
    return format;
}

//---------------------------------------------------------
//   isOdd
//---------------------------------------------------------

bool Page::isOdd() const
{
    return (m_no + 1 + score()->pageNumberOffset()) & 1;
}

//---------------------------------------------------------
//   elements
//---------------------------------------------------------

std::vector<EngravingItem*> Page::elements() const
{
    std::vector<EngravingItem*> el;
    const_cast<Page*>(this)->scanElements(&el, collectElements, false);
    return el;
}

//---------------------------------------------------------
//   tm
//---------------------------------------------------------

double Page::tm() const
{
    return ((!style().styleB(Sid::pageTwosided) || isOdd())
            ? style().styleD(Sid::pageOddTopMargin) : style().styleD(Sid::pageEvenTopMargin)) * DPI;
}

//---------------------------------------------------------
//   bm
//---------------------------------------------------------

double Page::bm() const
{
    return ((!style().styleB(Sid::pageTwosided) || isOdd())
            ? style().styleD(Sid::pageOddBottomMargin) : style().styleD(Sid::pageEvenBottomMargin)) * DPI;
}

//---------------------------------------------------------
//   lm
//---------------------------------------------------------

double Page::lm() const
{
    return ((!style().styleB(Sid::pageTwosided) || isOdd())
            ? style().styleD(Sid::pageOddLeftMargin) : style().styleD(Sid::pageEvenLeftMargin)) * DPI;
}

//---------------------------------------------------------
//   rm
//---------------------------------------------------------

double Page::rm() const
{
    return (style().styleD(Sid::pageWidth) - style().styleD(Sid::pagePrintableWidth)) * DPI - lm();
}

//---------------------------------------------------------
//   tbbox
//    calculates and returns smallest rectangle containing all (visible) page elements
//---------------------------------------------------------

RectF Page::tbbox() const
{
    double x1 = width();
    double x2 = 0.0;
    double y1 = height();
    double y2 = 0.0;
    const std::vector<EngravingItem*> el = elements();
    for (EngravingItem* e : el) {
        if (e == this || !e->isPrintable()) {
            continue;
        }
        RectF ebbox = e->pageBoundingRect();
        if (ebbox.left() < x1) {
            x1 = ebbox.left();
        }
        if (ebbox.right() > x2) {
            x2 = ebbox.right();
        }
        if (ebbox.top() < y1) {
            y1 = ebbox.top();
        }
        if (ebbox.bottom() > y2) {
            y2 = ebbox.bottom();
        }
    }
    if (x1 < x2 && y1 < y2) {
        return RectF(x1, y1, x2 - x1, y2 - y1);
    } else {
        return pageBoundingRect();
    }
}

//---------------------------------------------------------
//   endTick
//---------------------------------------------------------

Fraction Page::endTick() const
{
    return m_systems.empty() ? Fraction(-1, 1) : m_systems.back()->measures().back()->endTick();
}
}
