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
#include "internal/importfinaleparser.h"
#include "internal/importfinalelogger.h"
#include "finaletypesconv.h"
#include "internal/text/finaletextconv.h"

#include <vector>
#include <exception>
#include <sstream>

#include "musx/musx.h"

#include "types/string.h"

#include "engraving/dom/excerpt.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/page.h"
#include "engraving/dom/score.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/sig.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/system.h"
#include "engraving/dom/utils.h"

#include "engraving/types/symnames.h"

#include "log.h"

using namespace mu::engraving;
using namespace muse;
using namespace musx::dom;

namespace mu::iex::finale {

FontTracker::FontTracker(const std::shared_ptr<const musx::dom::FontInfo>& fontInfo, double additionalSizeScaling)
{
    fontName = String::fromStdString(fontInfo->getName());
    fontSize = FinaleTConv::spatiumScaledFontSize(fontInfo);
    uchar styles = uint8_t(FontStyle::Normal);
    if (fontInfo->bold)
        styles |= uint8_t(FontStyle::Bold);
    if (fontInfo->italic)
        styles |= uint8_t(FontStyle::Italic);
    if (fontInfo->underline)
        styles |= uint8_t(FontStyle::Underline);
    if (fontInfo->strikeout)
        styles |= uint8_t(FontStyle::Strike);
    fontStyle = FontStyle(styles);
    spatiumIndependent = fontInfo->absolute;
    if (!fontInfo->absolute) {
        fontSize *= additionalSizeScaling;
    }
}

FontTracker::FontTracker(const MStyle& style, const String& sidNamePrefix)
{
    fontName = style.styleSt(MStyle::styleIdx(sidNamePrefix + u"FontFace"));
    fontSize = style.styleD(MStyle::styleIdx(sidNamePrefix + u"FontSize"));
    fontStyle = FontStyle(style.styleI(MStyle::styleIdx(sidNamePrefix + u"FontStyle")));
    spatiumIndependent = style.styleB(MStyle::styleIdx(sidNamePrefix + u"FontSpatiumDependent"));
}

// Passing in the firstFontInfo pointer suppresses any first font information from being generated in the output string.
// Instead, it is returned in the pointer.
String FinaleParser::stringFromEnigmaText(const musx::util::EnigmaParsingContext& parsingContext, const EnigmaParsingOptions& options, FontTracker* firstFontInfo) const
{
    String endString;
    const bool isHeaderOrFooter = options.hfType != HeaderFooterType::None;
    std::optional<FontTracker> prevFont = options.initialFont;
    std::unordered_set<FontStyle> emittedOpenTags; // tracks whose open tags we actually emitted here
    uint16_t flagsThatAreStillOpen = 0; // any flags we've opened that need to be closed.

    auto updateFontStyles = [&](const std::optional<FontTracker>& current, const std::optional<FontTracker>& previous) {
        uint8_t newStyles = current ? uint8_t(current->fontStyle) : 0;
        uint8_t oldStyles = previous ? uint8_t(previous->fontStyle) : 0;

        // Close styles that are no longer active — in fixed reverse order
        for (auto [bit, tag] : {
                                std::pair{FontStyle::Strike, u"s"},
                                std::pair{FontStyle::Underline, u"u"},
                                std::pair{FontStyle::Italic, u"i"},
                                std::pair{FontStyle::Bold, u"b"},
                                }) {
            if (oldStyles & uint8_t(bit)) {
                if (emittedOpenTags.find(bit) == emittedOpenTags.end()) {
                    // Patch in opening tag at start
                    endString.append(String(u"<") + tag + u">");
                    emittedOpenTags.insert(bit);
                }
                endString.append(String(u"</") + tag + u">");
                flagsThatAreStillOpen &= ~uint16_t(bit);
            }
        }

        // Open newly active styles — in fixed forward order
        for (auto [bit, tag] : {
                                std::pair{FontStyle::Bold, u"b"},
                                std::pair{FontStyle::Italic, u"i"},
                                std::pair{FontStyle::Underline, u"u"},
                                std::pair{FontStyle::Strike, u"s"},
                                }) {
            if (newStyles & uint8_t(bit)) {
                emittedOpenTags.insert(bit);
                endString.append(String(u"<") + tag + u">");
                flagsThatAreStillOpen |= uint16_t(bit);
            }
        }
    };

    // The processTextChunk function process each chunk of processed text with font information. It is only
    // called when the font information changes.
    auto processTextChunk = [&](const std::string& nextChunk, const musx::util::EnigmaStyles& styles) -> bool {
        const FontTracker font(styles.font, options.scaleFontSizeBy);
        if (firstFontInfo && !prevFont) {
            *firstFontInfo = font;
        } else {
            if (!prevFont || prevFont->fontName != font.fontName) {
                // When using musical fonts, don't actually set the font type since symbols are loaded separately.
                /// @todo decide when we want to not convert symbols/fonts, e.g. to allow multiple musical fonts in one score.
                /// @todo append this based on whether symbol ends up being replaced or not.
                //if (!font->calcIsDefaultMusic()) { /// @todo RGP changed from a name check, but each notation element has its own default font setting in Finale. We need to handle that.
                    endString.append(String(u"<font face=\"" + font.fontName + u"\"/>"));
                //}
            }
            if (!prevFont || prevFont->fontSize != font.fontSize) {
                endString.append(String(u"<font size=\""));
                endString.append(String::number(font.fontSize, 2) + String(u"\"/>"));
            }
            if (!prevFont || prevFont->fontStyle != font.fontStyle) {
                updateFontStyles(font, prevFont);
            }
        }
        prevFont = font;
        endString.append(String::fromStdString(nextChunk));
        return true;
    };

    // The processCommand function sends back to the parser a subsitution string for the Enigma command.
    // The command is parsed with the command in the first element and any parameters in subsequent elements.
    // Return "" to remove the command from the processed string. Return std::nullopt to have the parsing function insert
    // a default value.
    auto processCommand = [&](const std::vector<std::string>& parsedCommand) -> std::optional<std::string> {
        if (parsedCommand.empty()) {
            // log error
            return std::nullopt;
        }
        /// @todo Perhaps add parse functions to classes like PageTextAssign to handle this automatically. But it also may be important
        /// to handle it here for an intelligent import, if text can reference a page number offset in MuseScore.
        if (parsedCommand[0] == "page") {
            // ignore page offset argument. we set this once in the style settings.
            switch (options.hfType) {
                default:
                case HeaderFooterType::None: break;
                case HeaderFooterType::FirstPage: break;
                case HeaderFooterType::SecondPageToEnd:
                    if (parsedCommand.size() > 1) {
                        // always overwrite with the last one we find.
                        m_score->setPageNumberOffset(std::stoi(parsedCommand[1]));
                    }
                    return "$p";
            }
        } else if (parsedCommand[0] == "partname") {
            switch (options.hfType) {
                /// @todo maybe create a "partname" metatag instead? (Especially if excerpts can have different values.)
                default:
                case HeaderFooterType::None: break;
                case HeaderFooterType::FirstPage: return "$I";
                case HeaderFooterType::SecondPageToEnd: return "$i";
            }
        } else if (parsedCommand[0] ==  "totpages") {
            if (isHeaderOrFooter) {
                return "$n";
            }
            return std::to_string(m_score->npages());
        } else if (parsedCommand[0] == "filename") {
            if (isHeaderOrFooter) {
                return "$f";
            }
            /// @todo Does the file have a name at import time? Otherwise we could use the musx filename we opened.
            return m_score->masterScore()->name().toStdString();
        } else if (parsedCommand[0] ==  "perftime") {
            /// @todo: honor format code (see class comments for musx::util::EnigmaString)
            /// Note that Finale's UI does not support any format but m'ss", but plugins could have inserted other formats.
            int rawDurationSeconds = m_score->duration();
            int minutes = rawDurationSeconds / 60;
            int seconds = rawDurationSeconds % 60;
            std::ostringstream oss;
            oss << minutes << '\'' << std::setw(2) << std::setfill('0') << seconds << '"';
            return oss.str();
        } else if (parsedCommand[0] ==  "copyright") {
            /// @todo maybe not use $C/$c at all in favor of $:copyright:.?
            /// XM: It's common to only show a footer on the first page. This is only attainable with $C.
            switch (options.hfType) {
                default:
                case HeaderFooterType::None: return "$:copyright:"; /// @todo does this actually work? maybe not for non-headers?
                case HeaderFooterType::FirstPage: return "$C";
                case HeaderFooterType::SecondPageToEnd: return "$c";
            }
//        } else if (parsedCommand[0] == "flat") {
//            return "<sym>accidentalFlat</sym>";
        }
        // Find and insert metaTags when appropriate
        if (isHeaderOrFooter) {
            String metaTag = FinaleTConv::metaTagFromTextComponent(parsedCommand[0]);
            if (!metaTag.isEmpty()) {
                return "$:" + metaTag.toStdString() + ":";
            }
        }
        // Returning std::nullopt allows the musx library to fill in any we have not handled.
        return std::nullopt;
    };

    parsingContext.parseEnigmaText(processTextChunk, processCommand);
    updateFontStyles(FontTracker(u"Dummy", 12, FontStyle::Normal), FontTracker(u"Dummy", 12, FontStyle(flagsThatAreStillOpen)));

    return endString;
};

}
