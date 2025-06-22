/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
#include "textlayout.h"
#include "tlayout.h"
#include "dom/box.h"
#include "dom/page.h"
#include "dom/staff.h"
#include "dom/textlinebase.h"
#include "types/typesconv.h"

using namespace mu::engraving::rendering::score;
using namespace mu::engraving;
using namespace muse::draw;

void TextLayout::layoutBaseTextBase(const TextBase* item, TextBase::LayoutData* ldata)
{
    IF_ASSERT_FAILED(item->explicitParent()) {
        return;
    }

    ldata->setPos(PointF());

    if (item->placeBelow()) {
        ldata->setPosY(item->staff() ? item->staff()->staffHeight(item->tick()) : 0.0);
    }

    layoutBaseTextBase1(item, ldata);
}

void TextLayout::layoutBaseTextBase(TextBase* item, LayoutContext&)
{
    layoutBaseTextBase(item, item->mutldata());
}

void TextLayout::layoutBaseTextBase1(const TextBase* item, TextBase::LayoutData* ldata)
{
    if (item->explicitParent() && item->layoutToParentWidth()) {
        LD_CONDITION(item->parentItem()->ldata()->isSetBbox());
    }

    if (ldata->layoutInvalid) {
        item->createBlocks(ldata);
    }

    double y = 0.0;

    double maxBlockWidth = -DBL_MAX;
    // adjust the bounding box for the text item
    for (size_t i = 0; i < ldata->blocks.size(); ++i) {
        TextBlock& t = ldata->blocks[i];
        layoutTextBlock(&t, item);
        y += t.lineSpacing();
        t.setY(y);
        maxBlockWidth = std::max(maxBlockWidth, t.boundingRect().width());
    }

    Shape shape;
    textHorizontalLayout(item, shape, maxBlockWidth, ldata);

    RectF bb = shape.bbox();

    double yoff = 0;
    double h    = 0;
    if (item->explicitParent()) {
        if (item->layoutToParentWidth()) {
            if (item->explicitParent()->isTBox()) {
                // hack: vertical alignment is always TOP
                const_cast<TextBase*>(item)->setAlign({ item->align().horizontal, AlignV::TOP });
            } else if (item->explicitParent()->isBox()) {
                // consider inner margins of frame
                Box* b = toBox(item->explicitParent());
                yoff = b->topMargin() * DPMM;

                if (b->height() < bb.bottom()) {
                    h = b->height() / 2 + bb.height();
                } else {
                    h  = b->height() - yoff - b->bottomMargin() * DPMM;
                }
            } else if (item->explicitParent()->isPage()) {
                Page* p = toPage(item->explicitParent());
                h = p->height() - p->tm() - p->bm();
                yoff = p->tm();
            } else if (item->explicitParent()->isMeasure()) {
            } else {
                h  = item->parentItem()->height();
            }
        }
    } else {
        ldata->setPos(PointF());
    }

    if (item->align() == AlignV::BOTTOM) {
        yoff += h - bb.bottom();
    } else if (item->align() == AlignV::VCENTER) {
        yoff +=  (h - (bb.top() + bb.bottom())) * .5;
    } else if (item->align() == AlignV::BASELINE) {
        yoff += h * .5 - ldata->blocks.front().lineSpacing();
    } else {
        yoff += -bb.top();
    }

    for (TextBlock& t : ldata->blocks) {
        t.setY(t.y() + yoff);
    }

    shape.translateY(yoff);
    ldata->setShape(shape);

    if (item->hasFrame()) {
        item->layoutFrame(ldata);
    }

    if (!item->isDynamic() && !(item->explicitParent() && item->parent()->isBox())) {
        computeTextHighResShape(item, ldata);
    }
}

void TextLayout::layoutBaseTextBase1(TextBase* item, const LayoutContext&)
{
    layoutBaseTextBase1(item, item->mutldata());
}

void TextLayout::computeTextHighResShape(const TextBase* item, TextBase::LayoutData* ldata)
{
    Shape& shape = ldata->highResShape.mut_value();
    shape.clear();
    shape.elements().reserve(item->xmlText().size());

    for (const TextBlock& block : ldata->blocks) {
        double y = block.y();
        for (const TextFragment& fragment : block.fragments()) {
            FontMetrics fontMetrics = FontMetrics(fragment.font(item));
            double x = fragment.pos.x();
            size_t textSize = fragment.text.size();
            for (size_t i = 0; i < textSize; ++i) {
                Char character = fragment.text.at(i);
                RectF characterBoundingRect = fontMetrics.tightBoundingRect(fragment.text.at(i));
                characterBoundingRect.translate(x, y);
                shape.add(characterBoundingRect);
                if (i + 1 < textSize) {
                    x += fontMetrics.horizontalAdvance(character);
                }
            }
        }
    }

    ldata->highResShape = shape;
}

void TextLayout::textHorizontalLayout(const TextBase* item, Shape& shape, double maxBlockWidth, TextBase::LayoutData* ldata)
{
    double leftMargin = 0.0;
    double layoutWidth = 0;
    EngravingItem* parent = item->parentItem();
    if (parent && item->layoutToParentWidth()) {
        layoutWidth = parent->width();
        switch (parent->type()) {
        case ElementType::HBOX:
        case ElementType::VBOX:
        case ElementType::TBOX: {
            Box* b = toBox(parent);
            layoutWidth -= ((b->leftMargin() + b->rightMargin()) * DPMM);
            leftMargin = b->leftMargin() * DPMM;
        }
        break;
        case ElementType::PAGE: {
            Page* p = toPage(parent);
            layoutWidth -= (p->lm() + p->rm());
            leftMargin = p->lm();
        }
        break;
            break;
        default:
            ASSERT_X("Lay out to parent width only valid for items with page or frame as parent");
        }
    }

    // Position and alignment
    for (size_t i = 0; i < ldata->blocks.size(); ++i) {
        TextBlock& textBlock = ldata->blocks[i];
        double xAdj = leftMargin - textBlock.boundingRect().left();

        // Set position relative to reference point
        AlignH position = item->position();
        if (position == AlignH::HCENTER) {
            xAdj += (layoutWidth - maxBlockWidth) * .5;
        } else if (position == AlignH::RIGHT) {
            xAdj += layoutWidth - maxBlockWidth;
        }

        double diff = maxBlockWidth - textBlock.boundingRect().width();
        if (muse::RealIsNull(diff)) {
            // This is the longest line, don't align
            for (TextFragment& f : textBlock.fragments()) {
                f.pos.rx() += xAdj;
            }
            textBlock.shape().translate(PointF(xAdj, 0.0));
            shape.add(textBlock.shape().translated(PointF(0.0, textBlock.y())));
            continue;
        }
        // Align relative to the longest line
        AlignH alignH = item->align().horizontal;
        if (alignH == AlignH::HCENTER) {
            xAdj += diff * 0.5;
        } else if (alignH == AlignH::RIGHT) {
            xAdj += diff;
        } else if (alignH == AlignH::JUSTIFY) {
            // Don't justify final line
            if (i != ldata->blocks.size() - 1) {
                justifyLine(item, &textBlock, maxBlockWidth);
            }
        }

        for (TextFragment& fragment : textBlock.fragments()) {
            fragment.pos.rx() += xAdj;
        }
        textBlock.shape().translate(PointF(xAdj, 0.0));
        shape.add(textBlock.shape().translated(PointF(0.0, textBlock.y())));
    }
}

void TextLayout::justifyLine(const TextBase* item, TextBlock* textBlock, double maxBlockWidth)
{
    struct SubFragment {
        String text;
        CharFormat format;
        double width;
    };
    std::vector<SubFragment> subfrags;
    for (const TextFragment& f : textBlock->fragments()) {
        String current;
        FontMetrics fm(f.font(item));
        for (size_t i = 0; i < f.text.size(); ++i) {
            Char c = f.text.at(i);
            current += c;
            if (c.isSpace()) {
                double w = fm.width(current);
                subfrags.push_back({ current, f.format, w });
                current.clear();
            }
        }
        if (!current.isEmpty()) {
            double w = fm.width(current);
            subfrags.push_back({ current, f.format, w });
        }
    }

    int numSpaces = 0;
    double textWidth = 0;
    for (const SubFragment& sf : subfrags) {
        numSpaces++;
        textWidth += sf.width; // accumulate text width
    }
    numSpaces--;
    double spaceToFill = maxBlockWidth - textWidth;
    if (numSpaces > 0 && spaceToFill > 0) {
        textBlock->fragments().clear();
        double extraSpacing = spaceToFill / numSpaces;
        double x = 0.0;
        for (const SubFragment& sf : subfrags) {
            TextFragment frag;
            frag.text = sf.text;
            frag.format = sf.format;
            frag.pos.rx() = x;
            textBlock->fragments().push_back(frag);
            x += sf.width + extraSpacing;
        }
    }
}

void TextLayout::layoutTextBlock(TextBlock* item, const TextBase* t)
{
    std::list<TextFragment>& fragments = item->fragments();
    Shape& shape = item->shape();
    shape.clear();
    double x      = 0.0;
    double lineSpacing = 0.0;

    if (fragments.empty()) {
        FontMetrics fm = t->fontMetrics();
        shape.add(RectF(0.0, -fm.ascent(), 1.0, fm.descent()), t);
        lineSpacing = fm.lineSpacing();
    } else if (fragments.size() == 1 && fragments.front().text.isEmpty()) {
        auto fi = fragments.begin();
        TextFragment& f = *fi;
        f.pos.setX(x);
        FontMetrics fm(f.font(t));
        if (f.format.valign() != VerticalAlignment::AlignNormal) {
            double voffset = fm.xHeight() / SUBSCRIPT_SIZE;   // use original height
            if (f.format.valign() == VerticalAlignment::AlignSubScript) {
                voffset *= SUBSCRIPT_OFFSET;
            } else {
                voffset *= SUPERSCRIPT_OFFSET;
            }

            f.pos.setY(voffset);
        } else {
            f.pos.setY(0.0);
        }

        RectF temp(0.0, -fm.ascent(), 1.0, fm.descent());
        shape.add(temp, t);
        lineSpacing = std::max(lineSpacing, fm.lineSpacing());
    } else {
        const auto fiLast = --fragments.end();
        for (auto fi = fragments.begin(); fi != fragments.end(); ++fi) {
            TextFragment& f = *fi;
            f.pos.setX(x);
            Font fragmentFont = f.font(t);
            FontMetrics fm(fragmentFont);
            if (f.format.valign() != VerticalAlignment::AlignNormal) {
                double voffset = fm.xHeight() / SUBSCRIPT_SIZE;           // use original height
                if (f.format.valign() == VerticalAlignment::AlignSubScript) {
                    voffset *= SUBSCRIPT_OFFSET;
                } else {
                    voffset *= SUPERSCRIPT_OFFSET;
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

            double yOffset = musicSymbolBaseLineAdjust(item, t, f, fi);
            f.pos.ry() -= yOffset;

            RectF textBRect = fm.tightBoundingRect(f.text).translated(f.pos);
            bool useDynamicSymShape = fragmentFont.type() == Font::Type::MusicSymbol && t->isDynamic();
            if (useDynamicSymShape) {
                const Dynamic* dyn = toDynamic(t);
                SymId symId = TConv::symId(dyn->dynamicType());
                if (symId != SymId::noSym) {
                    shape.add(dyn->symShapeWithCutouts(symId).translated(f.pos));
                } else {
                    shape.add(textBRect, t);
                }
            } else {
                shape.add(textBRect, t);
            }
            if (fragmentFont.type() == Font::Type::MusicSymbol || fragmentFont.type() == Font::Type::MusicSymbolText) {
                // SEMI-HACK: Music fonts can have huge linespacing because of tall symbols, so instead of using the
                // font linespacing value we just use the height of the individual fragment with some added margin

                lineSpacing = std::max(lineSpacing, 1.25 * (shape.bbox().height() - shape.bbox().bottom()) + yOffset);
            } else {
                lineSpacing = std::max(lineSpacing, fm.lineSpacing());
            }
        }
    }

    // Apply style/custom line spacing
    lineSpacing *= t->textLineSpacing();
    item->setLineSpacing(lineSpacing);
}

double TextLayout::musicSymbolBaseLineAdjust(const TextBlock* block, const TextBase* t, const TextFragment& f,
                                             const std::list<TextFragment>::iterator fi)
{
    Font fragmentFont = f.font(t);
    FontMetrics fm(fragmentFont);
    const bool adjustSymbol = fragmentFont.type() == Font::Type::MusicSymbolText && t->isMarker();
    if (!adjustSymbol) {
        return 0.0;
    }

    // Align the x-height of the coda symbol to half the x-height of the surrounding text
    Font refFont;
    if (block->fragments().size() == 1) {
        refFont = t->font();
    } else {
        TextFragment& refFragment = fi != block->fragments().begin() ? *(std::prev(fi)) : *(std::next(fi));
        refFont = refFragment.font(t);
    }
    FontMetrics refFm(refFont);

    const double middle = (fm.tightBoundingRect(f.text).height() / 2) - fm.tightBoundingRect(f.text).bottom();
    const double refXHeight = refFm.capHeight() / 2;
    return refXHeight - middle;
}
