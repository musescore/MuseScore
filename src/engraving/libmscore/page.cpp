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
    : EngravingItem(ElementType::PAGE, parent, ElementFlag::NOT_SELECTABLE), _no(0)
{
    bspTreeValid = false;
}

//---------------------------------------------------------
//   items
//---------------------------------------------------------

std::vector<EngravingItem*> Page::items(const RectF& rect)
{
    if (!bspTreeValid) {
        doRebuildBspTree();
    }
    return bspTree.items(rect);
}

std::vector<EngravingItem*> Page::items(const mu::PointF& point)
{
    if (!bspTreeValid) {
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
    _systems.push_back(s);
}

//---------------------------------------------------------
//   draw
//    bounding rectangle fr is relative to page PointF
//---------------------------------------------------------

void Page::draw(mu::draw::Painter* painter) const
{
    TRACE_ITEM_DRAW;
    if (!score()->isLayoutMode(LayoutMode::PAGE)) {
        return;
    }
    //
    // draw header/footer
    //

    page_idx_t n = no() + 1 + score()->pageNumberOffset();
    painter->setPen(curColor());

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

        drawHeaderFooter(painter, 0, s1);
        drawHeaderFooter(painter, 1, s2);
        drawHeaderFooter(painter, 2, s3);
    }

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

        drawHeaderFooter(painter, 3, s1);
        drawHeaderFooter(painter, 4, s2);
        drawHeaderFooter(painter, 5, s3);
    }
}

//---------------------------------------------------------
//   drawHeaderFooter
//---------------------------------------------------------

void Page::drawHeaderFooter(mu::draw::Painter* p, int area, const String& ss) const
{
    Text* text = layoutHeaderFooter(area, ss);
    if (!text) {
        return;
    }
    p->translate(text->pos());
    renderer()->drawItem(text, p);
    p->translate(-text->pos());
    text->resetExplicitParent();
}

//---------------------------------------------------------
//   layoutHeaderFooter
//---------------------------------------------------------

Text* Page::layoutHeaderFooter(int area, const String& ss) const
{
    String s = replaceTextMacros(ss);
    if (s.isEmpty()) {
        return nullptr;
    }

    Text* text;
    if (area < MAX_HEADERS) {
        text = score()->headerText(area);
        if (!text) {
            text = Factory::createText((Page*)this, TextStyleType::HEADER);
            text->setFlag(ElementFlag::MOVABLE, false);
            text->setFlag(ElementFlag::GENERATED, true);       // set to disable editing
            text->setLayoutToParentWidth(true);
            score()->setHeaderText(text, area);
        }
    } else {
        text = score()->footerText(area - MAX_HEADERS);     // because they are 3 4 5
        if (!text) {
            text = Factory::createText((Page*)this, TextStyleType::FOOTER);
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
    text->setXmlText(s);
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
    if (!score()->isLayoutMode(LayoutMode::PAGE)) {
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
    if (!score()->isLayoutMode(LayoutMode::PAGE)) {
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
    for (System* s :_systems) {
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
        if (!_systems.empty()) {
            h = _systems.front()->height();
            if (!_systems.front()->measures().empty()) {
                MeasureBase* mb = _systems.front()->measures().back();
                w = mb->x() + mb->width();
            }
        }
        r = RectF(0.0, 0.0, w, h);
    } else {
        r = abbox();
    }

    bspTree.initialize(r, n);
    scanElements(&bspTree, &bspInsert, false);
    bspTreeValid = true;
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

String Page::replaceTextMacros(const String& s) const
{
    String d;
    for (size_t i = 0, n = s.size(); i < n; ++i) {
        Char c = s.at(i);
        if (c == '$' && (i < (n - 1))) {
            Char nc = s.at(i + 1);
            switch (nc.toAscii()) {
            case 'p': // not on first page 1
                if (!_no) {
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
                int no = static_cast<int>(_no) + 1 + score()->pageNumberOffset();
                if (no > 0) {
                    d += String::number(no);
                }
            }
            break;
            case 'n':
                d += String::number(score()->npages() + score()->pageNumberOffset());
                break;
            case 'i': // not on first page
                if (!_no) {
                    break;
                }
            // FALLTHROUGH
            case 'I':
                d += score()->metaTag(u"partName").toXmlEscaped();
                break;
            case 'f':
                d += masterScore()->fileInfo()->fileName(false).toString().toXmlEscaped();
                break;
            case 'F':
                d += masterScore()->fileInfo()->path().toString().toXmlEscaped();
                break;
            case 'd':
                d += Date::currentDate().toString(DateFormat::ISODate);
                break;
            case 'D':
            {
                String creationDate = score()->metaTag(u"creationDate");
                if (creationDate.isEmpty()) {
                    d += masterScore()->fileInfo()->birthTime().date().toString(DateFormat::ISODate);
                } else {
                    d += Date::fromStringISOFormat(creationDate).toString(DateFormat::ISODate);
                }
            }
            break;
            case 'm':
                if (score()->dirty() || !masterScore()->saved()) {
                    d += Time::currentTime().toString(DateFormat::ISODate);
                } else {
                    d += masterScore()->fileInfo()->lastModified().time().toString(DateFormat::ISODate);
                }
                break;
            case 'M':
                if (score()->dirty() || !masterScore()->saved()) {
                    d += Date::currentDate().toString(DateFormat::ISODate);
                } else {
                    d += masterScore()->fileInfo()->lastModified().date().toString(DateFormat::ISODate);
                }
                break;
            case 'C': // only on first page
                if (_no) {
                    break;
                }
            // FALLTHROUGH
            case 'c':
                d += score()->metaTag(u"copyright").toXmlEscaped();
                break;
            case 'v':
                if (score()->dirty()) {
                    d += String::fromAscii(MUSESCORE_VERSION);
                } else {
                    d += score()->mscoreVersion();
                }
                break;
            case 'r':
                if (score()->dirty()) {
                    d += revision;
                } else {
                    int rev = score()->mscoreRevision();
                    if (rev > 99999) { // MuseScore 1.3 is decimal 5702, 2.0 and later uses a 7-digit hex SHA
                        d += String::number(rev, 16);
                    } else {
                        d += String::number(rev, 10);
                    }
                }
                break;
            case '$':
                d += '$';
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
                    d += score()->metaTag(tag).toXmlEscaped();
                    i = k - 1;
                }
            }
            break;
            default:
                d += '$';
                d += nc;
                break;
            }
            ++i;
        } else if (c == '&') {
            d += u"&amp;";
        } else {
            d += c;
        }
    }
    return d;
}

//---------------------------------------------------------
//   isOdd
//---------------------------------------------------------

bool Page::isOdd() const
{
    return (_no + 1 + score()->pageNumberOffset()) & 1;
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
        return abbox();
    }
}

//---------------------------------------------------------
//   endTick
//---------------------------------------------------------

Fraction Page::endTick() const
{
    return _systems.empty() ? Fraction(-1, 1) : _systems.back()->measures().back()->endTick();
}
}
