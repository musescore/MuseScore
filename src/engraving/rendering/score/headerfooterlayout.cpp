/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "headerfooterlayout.h"

#include "dom/factory.h"
#include "dom/masterscore.h"
#include "dom/mscore.h"
#include "dom/page.h"
#include "dom/text.h"

#include "tlayout.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::score;

void HeaderFooterLayout::layoutHeaderFooter(const LayoutContext& ctx, Page* page)
{
    const bool isPageMode = ctx.conf().isMode(LayoutMode::PAGE) || ctx.conf().isMode(LayoutMode::FLOAT);
    if (!isPageMode) {
        return;
    }

    const page_idx_t n = page->pageNumber() + 1 + page->score()->pageNumberOffset();

    if (ctx.conf().styleB(Sid::showHeader) && (page->pageNumber() || ctx.conf().styleB(Sid::headerFirstPage))) {
        const bool odd = (n & 1) || !ctx.conf().styleB(Sid::headerOddEven);
        createUpdateHeaderText(ctx, page, 0, ctx.conf().styleSt(odd ? Sid::oddHeaderL : Sid::evenHeaderL));
        createUpdateHeaderText(ctx, page, 1, ctx.conf().styleSt(odd ? Sid::oddHeaderC : Sid::evenHeaderC));
        createUpdateHeaderText(ctx, page, 2, ctx.conf().styleSt(odd ? Sid::oddHeaderR : Sid::evenHeaderR));
    } else {
        for (int area = 0; area < MAX_HEADERS; ++area) {
            removeHeaderText(page, area);
        }
    }

    if (ctx.conf().styleB(Sid::showFooter) && (page->pageNumber() || ctx.conf().styleB(Sid::footerFirstPage))) {
        const bool odd = (n & 1) || !ctx.conf().styleB(Sid::footerOddEven);
        createUpdateFooterText(ctx, page, 0, ctx.conf().styleSt(odd ? Sid::oddFooterL : Sid::evenFooterL));
        createUpdateFooterText(ctx, page, 1, ctx.conf().styleSt(odd ? Sid::oddFooterC : Sid::evenFooterC));
        createUpdateFooterText(ctx, page, 2, ctx.conf().styleSt(odd ? Sid::oddFooterR : Sid::evenFooterR));
    } else {
        for (int area = 0; area < MAX_FOOTERS; ++area) {
            removeFooterText(page, area);
        }
    }
}

void HeaderFooterLayout::createUpdateHeaderText(const LayoutContext& ctx, Page* page, int area, const String& s)
{
    if (s.empty()) {
        removeHeaderText(page, area);
        return;
    }

    Text* text = page->headerText(area);
    if (!text) {
        text = Factory::createText(page, TextStyleType::HEADER);
        text->setParent(page);
        text->setFlag(ElementFlag::MOVABLE, false);
        text->setFlag(ElementFlag::GENERATED, true); // set to disable editing
        text->setLayoutToParentWidth(true);
        page->setHeaderText(area, text);

        switch (area) {
        case 0:
            text->setAlign({ AlignH::LEFT, AlignV::TOP });
            text->setPosition(AlignH::LEFT);
            break;
        case 1:
            text->setAlign({ AlignH::HCENTER, AlignV::TOP });
            text->setPosition(AlignH::HCENTER);
            break;
        case 2:
            text->setAlign({ AlignH::RIGHT, AlignV::TOP });
            text->setPosition(AlignH::RIGHT);
            break;
        default:
            break;
        }
    }

    if (!updateHeaderFooterText(ctx, page, text, s)) {
        removeHeaderText(page, area);
    }
}

void HeaderFooterLayout::createUpdateFooterText(const LayoutContext& ctx, Page* page, int area, const String& s)
{
    if (s.empty()) {
        removeFooterText(page, area);
        return;
    }

    Text* text = page->footerText(area);
    if (!text) {
        text = Factory::createText(page, TextStyleType::FOOTER);
        text->setParent(page);
        text->setFlag(ElementFlag::MOVABLE, false);
        text->setFlag(ElementFlag::GENERATED, true); // set to disable editing
        text->setLayoutToParentWidth(true);
        page->setFooterText(area, text);

        switch (area) {
        case 0:
            text->setAlign({ AlignH::LEFT, AlignV::BOTTOM });
            text->setPosition(AlignH::LEFT);
            break;
        case 1:
            text->setAlign({ AlignH::HCENTER, AlignV::BOTTOM });
            text->setPosition(AlignH::HCENTER);
            break;
        case 2:
            text->setAlign({ AlignH::RIGHT, AlignV::BOTTOM });
            text->setPosition(AlignH::RIGHT);
            break;
        default:
            break;
        }
    }

    if (!updateHeaderFooterText(ctx, page, text, s)) {
        removeFooterText(page, area);
    }
}

bool HeaderFooterLayout::updateHeaderFooterText(const LayoutContext& ctx, Page* page, Text* text, const String& s)
{
    // Hack: we can't use toXmlEscaped on the entire string because this would erase any manual XML
    // formatting, but we do want to be able to use a plain '&' in favour of XML character entities ...
    // ... also, opening less-than characters are escaped if they are followed by a space or a digit,
    // to avoid confusion with XML tags, but not if they are followed by a letter
    String escaped;
    for (size_t i = 0, n = s.size(); i < n; ++i) {
        const Char c = s.at(i);
        if (c == '&') {
            escaped += u"&amp;";
            continue;
        } else if (c == '<' && (i + 1 < n) && (s.at(i + 1).isSpace() || s.at(i + 1).isDigit())) {
            escaped += u"&lt;";
            continue;
        }
        escaped += c;
    }

    // first formatting pass - apply TextStyleType formatting and any manual XML formatting
    text->setXmlText(escaped);
    text->createBlocks();

    // second formatting pass - replace macros and apply their unique formatting (if any)
    int emptyBlocks = 0;
    std::vector<TextBlock> newBlocks;
    for (const TextBlock& oldBlock : text->ldata()->blocks) {
        Text* dummyText = Factory::createText(ctx.dom().dummyParent(), text->textStyleType());
        dummyText->mutldata()->blocks = { replaceTextMacros(ctx, page, oldBlock) };
        dummyText->genText();
        dummyText->createBlocks();
        if (dummyText->xmlText().isEmpty()) {
            emptyBlocks++; // count empty blocks to remove them later if there is only 1 empty block
        }
        newBlocks.insert(newBlocks.end(), dummyText->ldata()->blocks.cbegin(), dummyText->ldata()->blocks.cend());
        delete dummyText;
    }

    if (newBlocks.empty() || (newBlocks.size() == 1 && emptyBlocks == 1)) {
        // If there is no block, or the only block is empty, tell the caller that the text should be removed.
        // This way, empty lines are preserved if and only if there also is some text anywhere in the header/footer.
        return false;
    }

    text->mutldata()->blocks = newBlocks;
    TLayout::layoutText(text, text->mutldata());

    return true;
}

void HeaderFooterLayout::removeHeaderText(Page* page, int area)
{
    if (Text* t = page->headerText(area)) {
        page->setHeaderText(area, nullptr);
        delete t;
    }
}

void HeaderFooterLayout::removeFooterText(Page* page, int area)
{
    if (Text* t = page->footerText(area)) {
        page->setFooterText(area, nullptr);
        delete t;
    }
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

TextBlock HeaderFooterLayout::replaceTextMacros(const LayoutContext& ctx, const Page* page, const TextBlock& tb)
{
    std::list<TextFragment> newFragments;
    for (const TextFragment& tf: tb.fragments()) {
        const CharFormat defaultFormat = tf.format;
        const String& s = tf.text;

        // if this is the first fragment, or the current fragment has a different format, we need to start a new fragment
        if (newFragments.size() == 0 || newFragments.back().format != defaultFormat) {
            newFragments.emplace_back(TextFragment());
            newFragments.back().format = defaultFormat;
        }

        for (size_t i = 0, n = s.size(); i < n; ++i) {
            Char c = s.at(i);
            if (c == '$' && (i < (n - 1))) {
                Char nc = s.at(i + 1);
                switch (nc.toAscii()) {
                case 'p': // not on first page 1
                    if (!page->pageNumber()) {
                        break;
                    }
                    [[fallthrough]];
                case 'N': // on page 1 only if there are multiple pages
                    if ((page->score()->npages() + page->score()->pageNumberOffset()) <= 1) {
                        break;
                    }
                    [[fallthrough]];
                case 'P': // on all pages
                {
                    size_t no = page->pageNumber() + 1 + page->score()->pageNumberOffset();
                    if (no > 0) {
                        const String pageNumberString = String::number(no);
                        const CharFormat pageNumberFormat = formatForMacro(ctx, String('$' + nc));
                        appendFormattedString(newFragments, pageNumberString, defaultFormat, pageNumberFormat);
                    }
                }
                break;
                case 'n':
                {
                    size_t no = page->score()->npages() + page->score()->pageNumberOffset();
                    const String numberOfPagesString = String::number(no);
                    const CharFormat pageNumberFormat = formatForMacro(ctx, String('$' + nc));
                    appendFormattedString(newFragments, numberOfPagesString, defaultFormat, pageNumberFormat);
                }
                break;
                case 'i': // not on first page
                    if (!page->pageNumber()) {
                        break;
                    }
                    [[fallthrough]];
                case 'I':
                    newFragments.back().text += page->score()->metaTag(u"partName");
                    break;
                case 'f':
                    newFragments.back().text += page->score()->masterScore()->fileInfo()->fileName(false).toString();
                    break;
                case 'F':
                    newFragments.back().text += page->score()->masterScore()->fileInfo()->path().toString();
                    break;
                case 'd':
                    newFragments.back().text += muse::Date::currentDate().toString(muse::DateFormat::ISODate);
                    break;
                case 'D':
                {
                    String creationDate = page->score()->metaTag(u"creationDate");
                    if (creationDate.isEmpty()) {
                        newFragments.back().text += page->score()->masterScore()->fileInfo()->birthTime().date().toString(
                            muse::DateFormat::ISODate);
                    } else {
                        newFragments.back().text += muse::Date::fromStringISOFormat(creationDate).toString(
                            muse::DateFormat::ISODate);
                    }
                }
                break;
                case 'm':
                    if (page->score()->dirty() || !page->score()->masterScore()->saved()) {
                        newFragments.back().text += muse::Time::currentTime().toString(muse::DateFormat::ISODate);
                    } else {
                        newFragments.back().text += page->score()->masterScore()->fileInfo()->lastModified().time().toString(
                            muse::DateFormat::ISODate);
                    }
                    break;
                case 'M':
                    if (page->score()->dirty() || !page->score()->masterScore()->saved()) {
                        newFragments.back().text += muse::Date::currentDate().toString(muse::DateFormat::ISODate);
                    } else {
                        newFragments.back().text += page->score()->masterScore()->fileInfo()->lastModified().date().toString(
                            muse::DateFormat::ISODate);
                    }
                    break;
                case 'C': // only on first page
                    if (page->pageNumber()) {
                        break;
                    }
                    [[fallthrough]];
                case 'c':
                {
                    const String copyrightString = page->score()->metaTag(u"copyright");
                    const CharFormat copyrightFormat = formatForMacro(ctx, String('$' + nc));
                    appendFormattedString(newFragments, copyrightString, defaultFormat, copyrightFormat);
                }
                break;
                case 'v':
                    if (page->score()->dirty()) {
                        newFragments.back().text += page->score()->appVersion();
                    } else {
                        newFragments.back().text += page->score()->mscoreVersion();
                    }
                    break;
                case 'r':
                    if (page->score()->dirty()) {
                        static String revision; // FIXME
                        newFragments.back().text += revision;
                    } else {
                        int rev = page->score()->mscoreRevision();
                        if (rev > 99999) { // MuseScore 1.3 is decimal 5702, 2.0 and later uses a 7-digit hex SHA
                            newFragments.back().text += String::number(rev, 16);
                        } else {
                            newFragments.back().text += String::number(rev, 10);
                        }
                    }
                    break;
                case '$':
                    newFragments.back().text += '$';
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
                    if (k != n) {      // found ':' ?
                        if (tag == u"copyright") {
                            const String copyrightString = page->score()->metaTag(tag);
                            const CharFormat copyrightFormat = formatForMacro(ctx, String(u"$:" + tag + u":"));
                            appendFormattedString(newFragments, copyrightString, defaultFormat, copyrightFormat);
                        } else {
                            newFragments.back().text += page->score()->metaTag(tag);
                        }
                        i = k - 1;
                    }
                }
                break;
                default:
                    newFragments.back().text += '$';
                    newFragments.back().text += nc;
                    break;
                }
                ++i;
            } else {
                newFragments.back().text += c;
            }
        }
    }

    TextBlock newBlock;
    newBlock.fragments() = newFragments;
    return newBlock;
}

CharFormat HeaderFooterLayout::formatForMacro(const LayoutContext& ctx, const String& macro)
{
    CharFormat format;
    if (macro == "$c" || macro == "$C" || macro == "$:copyright:") {
        format.setStyle(ctx.conf().styleV(Sid::copyrightFontStyle).value<FontStyle>());
        format.setFontSize(ctx.conf().styleD(Sid::copyrightFontSize));
        format.setFontFamily(ctx.conf().styleSt(Sid::copyrightFontFace));
    } else if (macro == "$p" || macro == "$P" || macro == "$n" || macro == "$N") {
        format.setStyle(ctx.conf().styleV(Sid::pageNumberFontStyle).value<FontStyle>());
        format.setFontSize(ctx.conf().styleD(Sid::pageNumberFontSize));
        format.setFontFamily(ctx.conf().styleSt(Sid::pageNumberFontFace));
    }
    return format;
}

void HeaderFooterLayout::appendFormattedString(std::list<TextFragment>& fragments, const String& string, const CharFormat& defaultFormat,
                                               const CharFormat& newFormat)
{
    // If the default format equals the format for this macro, we don't need to create a new fragment...
    if (defaultFormat == newFormat) {
        fragments.back().text += string;
        return;
    }
    TextFragment newFragment(string);
    newFragment.format = newFormat;
    fragments.emplace_back(newFragment);
    fragments.emplace_back(TextFragment());    // Start next fragment
    fragments.back().format = defaultFormat;   // reset to default for next fragment
}

double HeaderFooterLayout::headerExtension(const LayoutContext& ctx, const Page* page)
{
    const bool isPageMode = ctx.conf().isMode(LayoutMode::PAGE) || ctx.conf().isMode(LayoutMode::FLOAT);
    if (!isPageMode) {
        return 0.0;
    }

    if (ctx.conf().styleB(Sid::showHeader) && (page->pageNumber() || ctx.conf().styleB(Sid::headerFirstPage))) {
        double maxHeight = 0.0;
        for (int area = 0; area < MAX_HEADERS; ++area) {
            if (Text* text = page->headerText(area)) {
                double h = text->height();
                if (h > maxHeight) {
                    maxHeight = h;
                }
            }
        }
        const double offset = ctx.conf().styleV(Sid::headerOffset).value<PointF>().y() * DPMM;
        return std::max(0.0, maxHeight - offset);
    }

    return 0.0;
}

double HeaderFooterLayout::footerExtension(const LayoutContext& ctx, const Page* page)
{
    const bool isPageMode = ctx.conf().isMode(LayoutMode::PAGE) || ctx.conf().isMode(LayoutMode::FLOAT);
    if (!isPageMode) {
        return 0.0;
    }

    if (ctx.conf().styleB(Sid::showFooter) && (page->pageNumber() || ctx.conf().styleB(Sid::footerFirstPage))) {
        double maxHeight = 0.0;
        for (int area = 0; area < MAX_FOOTERS; ++area) {
            if (Text* text = page->footerText(area)) {
                double h = text->height();
                if (h > maxHeight) {
                    maxHeight = h;
                }
            }
        }
        const double offset = ctx.conf().styleV(Sid::footerOffset).value<PointF>().y() * DPMM;
        return std::max(0.0, maxHeight - offset);
    }
    return 0.0;
}
