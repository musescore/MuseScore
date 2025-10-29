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

#include "engraving/dom/box.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/dynamic.h"
#include "engraving/dom/excerpt.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/fret.h"
#include "engraving/dom/harmony.h"
#include "engraving/dom/harppedaldiagram.h"
#include "engraving/dom/jump.h"
#include "engraving/dom/marker.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/measurenumber.h"
#include "engraving/dom/note.h"
#include "engraving/dom/page.h"
#include "engraving/dom/score.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/sig.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stafftext.h"
#include "engraving/dom/stafftextbase.h"
#include "engraving/dom/system.h"
#include "engraving/dom/tempotext.h"
#include "engraving/dom/utils.h"

#include "engraving/rendering/score/stemlayout.h"

#include "engraving/types/symnames.h"

#include "log.h"

using namespace mu::engraving;
using namespace muse;
using namespace musx::dom;

namespace mu::iex::finale {

FrameSettings::FrameSettings(const others::Enclosure* enclosure) {
    if (!enclosure || enclosure->shape == others::Enclosure::Shape::NoEnclosure || enclosure->lineWidth == 0) {
        return;
    }
    if (enclosure->shape == others::Enclosure::Shape::Ellipse) {
        frameType = FrameType::CIRCLE;
    } else {
        frameType = FrameType::SQUARE;
    }

    frameWidth   = doubleFromEfix(enclosure->lineWidth);
    paddingWidth = doubleFromEvpu(enclosure->xMargin);
    frameRound   = enclosure->roundCorners ? int(lround(doubleFromEfix(enclosure->cornerRadius))) : 0;
}

FontTracker::FontTracker(const MusxInstance<musx::dom::FontInfo>& fontInfo, double additionalSizeScaling)
{
    fontName = String::fromStdString(fontInfo->getName());
    fontSize = spatiumScaledFontSize(fontInfo);
    fontStyle = FinaleTextConv::museFontEfx(fontInfo);
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
    spatiumIndependent = !style.styleB(MStyle::styleIdx(sidNamePrefix + u"FontSpatiumDependent"));
}

FontTracker FontTracker::fromEngravingFont(const engraving::MStyle& style, engraving::Sid styleId, double scaling)
{
    FontTracker result;
    result.fontName = style.styleSt(styleId);
    result.fontSize = 20.0 * scaling;
    return result;
}

// Passing in the firstFontInfo pointer suppresses any first font information from being generated in the output string.
// Instead, it is returned in the pointer.
String FinaleParser::stringFromEnigmaText(const musx::util::EnigmaParsingContext& parsingContext, const EnigmaParsingOptions& options, FontTracker* firstFontInfo) const
{
    String endString;
    const bool isHeaderOrFooter = options.hfType != HeaderFooterType::None;
    std::optional<FontTracker> prevFont = options.initialFont;
    std::optional<FontTracker> symFont = options.musicSymbolFont;
    std::unordered_set<FontStyle> emittedOpenTags; // tracks whose open tags we actually emitted here
    FontStyle flagsThatAreStillOpen = FontStyle::Normal; // any flags we've opened that need to be closed.

    auto updateFontStyles = [&](const std::optional<FontTracker>& current, const std::optional<FontTracker>& previous) {
        // Close styles that are no longer active — in fixed reverse order
        if (previous) {
            for (auto it = fontStyleTags.rbegin(); it != fontStyleTags.rend(); ++it) {
                if (previous->fontStyle & it->first) {
                    if (!muse::contains(emittedOpenTags, it->first)) {
                        // Patch in opening tag at start
                        endString.append(String(u"<") + it->second + u">");
                        emittedOpenTags.insert(it->first);
                    }
                    endString.append(String(u"</") + it->second + u">");
                    flagsThatAreStillOpen = flagsThatAreStillOpen - it->first;
                }
            }
        }

        // Open newly active styles — in fixed forward order
        if (current) {
            for (const auto& [bit, tag] : fontStyleTags) {
                if (current->fontStyle & bit) {
                    emittedOpenTags.insert(bit);
                    endString.append(String(u"<") + tag + u">");
                    flagsThatAreStillOpen = flagsThatAreStillOpen + bit;
                }
            }
        }
    };

    // The processTextChunk function process each chunk of processed text with font information. It is only
    // called when the font information changes.
    auto processTextChunk = [&](const std::string& nextChunk, const musx::util::EnigmaStyles& styles) -> bool {
        std::optional<String> symIds = options.convertSymbols ? FinaleTextConv::symIdInsertsFromStdString(nextChunk, styles.font) : std::nullopt;

        const FontTracker font(styles.font, options.scaleFontSizeBy);
        const bool isSymFont = options.convertSymbols && symFont && font == symFont.value();
        if (firstFontInfo && !prevFont) {
            *firstFontInfo = font;
        } else if (!options.plainText && (!symIds || !isSymFont)) {
            if (!prevFont || prevFont->fontName != font.fontName) {
                if (!symIds) { // if this chunk is not all mapped sym tags
                    endString.append(String(u"<font face=\"" + font.fontName + u"\"/>"));
                }
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
        endString.append(symIds ? symIds.value() : String::fromStdString(nextChunk));
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
                case HeaderFooterType::None: return m_score->metaTag(u"copyright").toStdString();
                case HeaderFooterType::FirstPage: return "$C";
                case HeaderFooterType::SecondPageToEnd: return "$c";
            }
        } else if (std::optional<options::AccidentalInsertSymbolType> acciInsertType = musx::util::EnigmaString::commandIsAccidentalType(parsedCommand[0])) {
            const auto& acciDataIt = musxOptions().textOptions->symbolInserts.find(acciInsertType.value());
            if (acciDataIt != musxOptions().textOptions->symbolInserts.end()) {
                if (std::optional<String> symTag = FinaleTextConv::symIdInsertFromFinaleChar(acciDataIt->second->symChar, acciDataIt->second->symFont)) {
                    return symTag.value().toStdString();
                }
            }
            // since we could not map the character to a symbol, let it fall thru and parse as glyphs
        }
        // Find and insert metaTags when appropriate
        String metaTag = metaTagFromTextComponent(parsedCommand[0]);
        if (!metaTag.isEmpty()) {
            if (isHeaderOrFooter) {
                return "$:" + metaTag.toStdString() + ":";
            } else {
                return m_score->metaTag(metaTag).toStdString();
            }
        }
        // Returning std::nullopt allows the musx library to fill in any we have not handled.
        return std::nullopt;
    };

    parsingContext.parseEnigmaText(processTextChunk, processCommand);
    // Close any open font style tags
    updateFontStyles(std::nullopt, FontTracker(u"Dummy", 12, flagsThatAreStillOpen));

    return endString;
};

static const std::regex dynamicRegex(R"((?:<sym>dynamic[^>]*?</sym>)+|(?:\b)[fmnprsz]+(?:\b(?=[^>]|$)))");
static const std::regex hpdDetectRegex(R"(^ *?(?:<sym>harpPedal[^>]*?</sym> *?)+$)");
static const std::regex hpdFragmentRegex(R"(<sym>harpPedal[^>]*?</sym>)");

static std::optional<std::array<PedalPosition, HARP_STRING_NO> > parseHarpPedalDiagram(const std::string& utf8Tag)
{
    auto begin = std::sregex_iterator(utf8Tag.begin(), utf8Tag.end(), hpdFragmentRegex);
    size_t matchIdx = 0;
    std::array<PedalPosition, HARP_STRING_NO> pedalState;
    std::fill(pedalState.begin(), pedalState.end(), PedalPosition::NATURAL);

    for (auto it = begin; it != std::sregex_iterator() && int(matchIdx) < HARP_STRING_NO; ++it, ++matchIdx) {
        const std::string harpSym = (*it).str();
        if (harpSym == "<sym>harpPedalRaised</sym>") {
            pedalState[matchIdx] = PedalPosition::FLAT;
        } else if (harpSym == "<sym>harpPedalCentered</sym>") {
            pedalState[matchIdx] = PedalPosition::NATURAL;
        } else if (harpSym == "<sym>harpPedalLowered</sym>") {
            pedalState[matchIdx] = PedalPosition::SHARP;
        } else if (harpSym == "<sym>harpPedalDivider</sym>" && matchIdx == 3 /* SEPARATOR_IDX */) {
            // Don't count separator in pedal state index
            --matchIdx;
        } else {
            // Bad symbol or separator at invalid location, don't import
            return std::nullopt;
        }
    }
    return pedalState;
}

ReadableExpression::ReadableExpression(const FinaleParser& context, const MusxInstance<musx::dom::others::TextExpressionDef>& textExpression)
{
    // Text
    /// @todo Rather than rely only on marking category, it probably makes more sense to interpret the playback features to detect what kind of marking
    /// this is. Or perhaps a combination of both. This would provide better support to legacy files whose expressions are all Misc.
    others::MarkingCategory::CategoryType categoryType = others::MarkingCategory::CategoryType::Misc;
    MusxInstance<FontInfo> catMusicFont;
    if (MusxInstance<others::MarkingCategory> category = context.musxDocument()->getOthers()->get<others::MarkingCategory>(context.currentMusxPartId(), textExpression->categoryId)) {
        categoryType = category->categoryType;
        catMusicFont = category->musicFont;
    }
    EnigmaParsingOptions options;
    musx::util::EnigmaParsingContext parsingContext = textExpression->getRawTextCtx(context.currentMusxPartId());
    FontTracker firstFontInfo;
    xmlText = context.stringFromEnigmaText(parsingContext, options, &firstFontInfo);

    // Text frame/border (Finale: Enclosure)
    frameSettings = FrameSettings(textExpression->getEnclosure().get());

    // Element type and element-based options
    elementType = [&]() {
        switch (textExpression->playbackType) {
        case others::PlaybackType::None: break;
        case others::PlaybackType::Tempo: return ElementType::TEMPO_TEXT;
        case others::PlaybackType::MidiController: break; // staff text is best?
        case others::PlaybackType::KeyVelocity: break; // dynamics, but let's process the string and dynamic type later
        case others::PlaybackType::Transpose: break; // instrument change is best?
        case others::PlaybackType::Channel: break; // instrument change is best?
        case others::PlaybackType::MidiPatchChange: break; // instrument change is best?
        case others::PlaybackType::PercussionMidiMap: break; // instrument change is best?
        case others::PlaybackType::MidiPitchWheel: break; // instrument change is best?
        case others::PlaybackType::ChannelPressure: break; // instrument change is best?
        case others::PlaybackType::RestrikeKeys: break; // add/remove invisible pedal lines?
        case others::PlaybackType::Dump: break; // staff text is best?
        case others::PlaybackType::PlayTempoToolChanges: return ElementType::TEMPO_TEXT;
        case others::PlaybackType::IgnoreTempoToolChanges: return ElementType::TEMPO_TEXT;
        case others::PlaybackType::Swing: return ElementType::STAFF_TEXT; // possibly system text is better for playback, but could mess up layout
        case others::PlaybackType::SmartPlaybackOn: break; // staff text is best?
        case others::PlaybackType::SmartPlaybackOff: break; // staff text is best?
        }
        /// @todo introduce more sophisticated (regex-based) checks

        options.plainText = true;
        std::string utf8Tag = context.stringFromEnigmaText(parsingContext, options).toStdString();

        // Dynamics (adapted from engraving/dom/dynamic.cpp)
        /// @todo This regex fails for `dynamicMF` and `dynamicMP`, among several others.
        auto begin = std::sregex_iterator(utf8Tag.begin(), utf8Tag.end(), dynamicRegex);
        for (auto it = begin; it != std::sregex_iterator(); ++it) {
            const std::smatch match = *it;
            const std::string matchStr = match.str();
            for (auto dyn : Dynamic::dynamicList()) {
                if (TConv::toXml(dyn.type).ascii() == matchStr || dyn.text == matchStr) {
                    utf8Tag.replace(match.position(0), match.length(0), dyn.text);
                    xmlText = String::fromStdString(utf8Tag); // do we want this?
                    dynamicType = dyn.type;
                    return ElementType::DYNAMIC;
                }
            }
        }

        // Harp pedal (not text) diagrams
        /// @todo allow font style/face/size tags
        if (std::regex_search(utf8Tag, hpdDetectRegex)) {
            std::optional<std::array<PedalPosition, HARP_STRING_NO> > maybePedalState = parseHarpPedalDiagram(utf8Tag);
            if (maybePedalState.has_value()) {
                pedalState = maybePedalState.value();
                return ElementType::HARP_DIAGRAM;
            }
            context.logger()->logWarning(String(u"Cannot create harp pedal diagram!"));
        }

        static const std::unordered_map<others::MarkingCategory::CategoryType, ElementType> categoryTypeTable = {
            { others::MarkingCategory::CategoryType::Dynamics, ElementType::DYNAMIC },
            { others::MarkingCategory::CategoryType::ExpressiveText, ElementType::EXPRESSION },
            { others::MarkingCategory::CategoryType::TempoMarks, ElementType::TEMPO_TEXT },
            { others::MarkingCategory::CategoryType::TempoAlterations, ElementType::TEMPO_TEXT },
            { others::MarkingCategory::CategoryType::TechniqueText, ElementType::STAFF_TEXT },
            { others::MarkingCategory::CategoryType::RehearsalMarks, ElementType::REHEARSAL_MARK },
        };
        return muse::value(categoryTypeTable, categoryType, ElementType::STAFF_TEXT);
    }();

    FontTracker defaultFontForElement(context.score()->style(), fontStylePrefixFromElementType(elementType));
    if (firstFontInfo != defaultFontForElement) {
        options.plainText = false;
        options.initialFont = defaultFontForElement;
        // Whichever font we choose here will be stripped out in favor of the default for the kind of marking it is.
        options.musicSymbolFont = [&]() -> std::optional<FontTracker> {
            if (!catMusicFont) {
                return FontTracker(context.musxOptions().defaultMusicFont); // we get here for the Misc category, and this seems like the best choice for legacy files. See todo comments, though.
            } else if (context.fontIsEngravingFont(catMusicFont->getName())) {
                return FontTracker(catMusicFont); // if it's an engraving font use it
            } else if (catMusicFont->calcIsDefaultMusic() && !context.musxOptions().calculatedEngravingFontName.empty()) {
                return FontTracker(catMusicFont); // if it's not an engraving font, but we are using an alternative as engraving font,
                // specify the non-engraving font here. The parsing routine strips it out and MuseScore
                // uses the engraving font instead.
            }
            return std::nullopt;
        }();
        xmlText = context.stringFromEnigmaText(parsingContext, options);
    }

    context.logger()->logInfo(String(u"Converted expression of %1 type").arg(TConv::userName(elementType).translated()));
}

static String textFromRepeatDef(const MusxInstance<musx::dom::others::TextRepeatDef>& repeatDef, const FontTracker font)
{
    String text;
    for (const auto& [bit, tag] : fontStyleTags) {
        if (font.fontStyle & bit) {
            text.append(String(u"<") + tag + u">");
        }
    }
    text.append(String(u"<font face=\"" + font.fontName + u"\"/>"));
    text.append(String(u"<font size=\""));
    text.append(String::number(font.fontSize, 2) + String(u"\"/>"));

    // Text
    const MusxInstance<others::TextRepeatText> repeatText = repeatDef->getDocument()->getOthers()->get<others::TextRepeatText>(repeatDef->getSourcePartId(), repeatDef->getCmper());
    text.append(String::fromStdString(repeatText->text));
    return text;
}

ReadableRepeatText::ReadableRepeatText(const FinaleParser& context, const MusxInstance<musx::dom::others::TextRepeatDef>& repeatDef)
{
    xmlText = textFromRepeatDef(repeatDef, FontTracker(repeatDef->font));

    // Text frame/border (Finale: Enclosure)
    frameSettings = FrameSettings(repeatDef->hasEnclosure ? context.musxDocument()->getOthers()->get<others::TextRepeatEnclosure>(context.currentMusxPartId(), repeatDef->getCmper()).get() : nullptr);

    // Text justification and alignment
    repeatAlignment = toAlignH(repeatDef->justification);

    // Element type and element-based options
    // Replace symbols with text
    String tempText = xmlText;
    tempText.replace(u"<sym>segno</sym>", u"Segno");
    tempText.replace(u"<sym>segnoSerpent1</sym>", u"Segno");
    tempText.replace(u"<sym>segnoSerpent2</sym>", u"Segno");
    tempText.replace(u"<sym>coda</sym>", u"Coda");
    tempText.replace(u"<sym>codaSquare</sym>", u"Coda");
    tempText.replace(u"<sym>dalSegno</sym>", u"D.S.");
    tempText.replace(u"<sym>daCapo</sym>", u"D.C.");

    auto match = [tempText](const String& s) {
        return tempText.contains(s, CaseSensitivity::CaseInsensitive);
    };

    if (match(u"al Fine")) {
        if (match(u"D.D.S.") || match(u"Doppio Segno")) {
            jumpType = JumpType::DSS_AL_FINE;
        } else if (match(u"D.S.") || match(u"Segno")) {
            jumpType = JumpType::DS_AL_FINE;
        } else {
            jumpType = JumpType::DC_AL_FINE;
        }
    } else if (match(u"al Coda")) {
        if (match(u"D.D.S.") || match(u"Doppio Segno")) {
            jumpType = JumpType::DSS_AL_CODA;
        } else if (match(u"D.S.") || match(u"Segno")) {
            jumpType = JumpType::DS_AL_CODA;
        } else {
            jumpType = JumpType::DC_AL_CODA;
        }
    } else if (match(u"al Doppia Coda")) {
        if (match(u"D.D.S.") || match(u"Doppio Segno")) {
            jumpType = JumpType::DSS_AL_DBLCODA;
        } else if (match(u"D.S.") || match(u"Segno")) {
            jumpType = JumpType::DS_AL_DBLCODA;
        } else {
            jumpType = JumpType::DC_AL_DBLCODA;
        }
    } else if (match(u"D.D.S.") || match(u"Dal Doppio Segno")) {
        jumpType = JumpType::DSS;
    } else if (match(u"D.S.") || match(u"Dal Segno")) {
        jumpType = JumpType::DS;
    } else if (match(u"D.C.") || match(u"Da Capo")) {
        jumpType = JumpType::DC;
    } else if (match(u"Da Coda")) {
        markerType = MarkerType::DA_CODA;
    } else if (match(u"Da Doppia Coda")) {
        markerType = MarkerType::DA_DBLCODA;
    } else if (match(u"To Coda")) {
        bool codaSymbols = xmlText.contains(u"<sym>coda</sym>") || xmlText.contains(u"<sym>codaSquare</sym>");
        markerType = codaSymbols ? MarkerType::TOCODASYM : MarkerType::TOCODA;
    } else if (match(u"Fine")) {
        markerType = MarkerType::FINE;
    } else if (match(u"Coda")) {
        markerType = xmlText.contains(u"<sym>codaSquare</sym>") ? MarkerType::VARCODA : MarkerType::CODA;
    } else if (match(u"Segno")) {
        markerType = xmlText.contains(u"<sym>segnoSerpent1</sym>") ? MarkerType::VARSEGNO : MarkerType::SEGNO;
    } else {
        elementType = (repeatAlignment == AlignH::LEFT) ? ElementType::MARKER : ElementType::JUMP;
    }
    if (elementType == ElementType::INVALID) {
        elementType = (jumpType == JumpType::USER) ? ElementType::MARKER : ElementType::JUMP;
    }
}

void FinaleParser::importTextExpressions()
{
    // Iterate through assigned expressions
    const MusxInstanceList<others::MeasureExprAssign> expressionAssignments = m_doc->getOthers()->getArray<others::MeasureExprAssign>(m_currentMusxPartId);
    std::vector<std::pair<Cmper, Inci>> parsedAssignments;
    parsedAssignments.reserve(expressionAssignments.size());
    logger()->logInfo(String(u"Import text expressions: Found %1 expressions.").arg(expressionAssignments.size()));
    for (const auto& expressionAssignment : expressionAssignments) {
        if (!expressionAssignment->textExprId) {
            // Shapes are currently unsupported
            continue;
        }

        // Already added as system clone
        std::pair<Cmper, Inci> expressionId = std::make_pair(expressionAssignment->getCmper(), expressionAssignment->getInci().value_or(0));
        if (muse::contains(parsedAssignments, expressionId)) {
            continue;
        }

        // Search our converted expression library, or if not found add to it
        ReadableExpression* expression = muse::value(m_expressions, expressionAssignment->textExprId, nullptr); /// @todo does this code work for part scores?
        if (!expression) {
            expression = new ReadableExpression(*this, m_doc->getOthers()->get<others::TextExpressionDef>(m_currentMusxPartId, expressionAssignment->textExprId));
            m_expressions.emplace(expressionAssignment->textExprId, expression);
        }

        if (expression->xmlText.empty()) {
            continue;
        }

        // Find staff
        staff_idx_t curStaffIdx = staffIdxFromAssignment(expressionAssignment->staffAssign);
        if (curStaffIdx == muse::nidx) {
            logger()->logWarning(String(u"Add text: Musx inst value not found."), m_doc, expressionAssignment->staffAssign);
            continue;
        }

        ElementType elementType = expression->elementType == ElementType::STAFF_TEXT && expressionAssignment->staffAssign < 0 ? ElementType::SYSTEM_TEXT : expression->elementType;

        // Find location in measure
        Fraction mTick = muse::value(m_meas2Tick, expressionAssignment->getCmper(), Fraction(-1, 1));
        Measure* measure = !mTick.negative() ? m_score->tick2measure(mTick) : nullptr;
        if (!measure) {
            continue;
        }
        track_idx_t curTrackIdx = staff2track(curStaffIdx) + static_cast<voice_idx_t>(std::clamp(expressionAssignment->layer - 1, 0, int(VOICES) - 1));
        Fraction rTick = eduToFraction(expressionAssignment->eduPosition);
        Segment* s = measure->getChordRestOrTimeTickSegment(measure->tick() + rTick);

        // Create item
        logger()->logInfo(String(u"Creating a %1 at tick %2 on track %3.").arg(TConv::userName(elementType).translated(), s->tick().toString(), String::number(curTrackIdx)));
        TextBase* item = toTextBase(Factory::createItem(elementType, s));
        const MusxInstance<others::TextExpressionDef>& expressionDef = expressionAssignment->getTextExpression();
        item->setParent(s);
        item->setTrack(curTrackIdx);
        item->setVisible(!expressionAssignment->hidden);
        item->setXmlText(expression->xmlText);
        item->checkCustomFormatting(expression->xmlText);
        item->setFrameType(expression->frameSettings.frameType);
        if (item->frameType() != FrameType::NO_FRAME) {
            item->setFrameWidth(absoluteSpatium(expression->frameSettings.frameWidth, item)); // is this the correct scaling?
            item->setPaddingWidth(absoluteSpatium(expression->frameSettings.paddingWidth, item)); // is this the correct scaling?
            item->setFrameRound(expression->frameSettings.frameRound);
        }

        item->setAlign(Align(toAlignH(expressionDef->horzExprJustification), AlignV::BASELINE));
        s->add(item);

        // Set element-specific properties
        switch (elementType) {
            case ElementType::DYNAMIC: {
                Dynamic* dynamic = toDynamic(item);
                dynamic->setDynamicType(expression->dynamicType);
                // Don't set these as styles, so new dynamics have nicer behaviour
                setAndStyleProperty(dynamic, Pid::CENTER_BETWEEN_STAVES, AutoOnOff::OFF);
                setAndStyleProperty(dynamic, Pid::CENTER_ON_NOTEHEAD, false);
                if (expressionAssignment->layer != 0) {
                    dynamic->setVoiceAssignment(VoiceAssignment::CURRENT_VOICE_ONLY);
                }
                if (expressionDef->playbackType == others::PlaybackType::KeyVelocity) {
                    dynamic->setVelocity(expressionDef->value);
                } else {
                    dynamic->setPlayDynamic(false);
                }
                break;
            }
            case ElementType::REHEARSAL_MARK: {
                if (expressionDef->hideMeasureNum && measure->measureNumber(curStaffIdx)) {
                    measure->measureNumber(curStaffIdx)->setVisible(false);
                }
                break;
            }
            case ElementType::TEMPO_TEXT: {
                TempoText* tt = toTempoText(item);
                if (expressionDef->playbackType == others::PlaybackType::Tempo) {
                    tt->setFollowText(false); /// @todo detect this
                    tt->setTempo(expressionDef->value * eduToFraction(Edu(expressionDef->auxData1)).toDouble() / 15.0);
                } else {
                    tt->setPlayTempoText(false);
                }
                break;
            }
            case ElementType::STAFF_TEXT:
            case ElementType::SYSTEM_TEXT: {
                StaffTextBase* stb = toStaffTextBase(item);
                if (expressionDef->playbackType == others::PlaybackType::Swing) {
                    int swingValue = expressionDef->value;
                    Fraction swingUnit = measure->timesig().denominator() >= 8 ? Fraction(1, 16) : Fraction(1, 8);
                    stb->setSwing(swingValue != 0);
                    stb->setSwingParameters(swingUnit.ticks(), 50 + (swingValue / 6));
                }
                break;
            }
            case ElementType::HARP_DIAGRAM: {
                toHarpPedalDiagram(item)->setPedalState(expression->pedalState);
                break;
            }
            default: break;
        }

        // Calculate position in score
        auto positionExpression = [&](TextBase* expr, const MusxInstance<others::MeasureExprAssign> exprAssign,
                                      const MusxInstance<others::TextExpressionDef>& exprDef) {
            expr->setAutoplace(false);
            setAndStyleProperty(expr, Pid::PLACEMENT, PlacementV::ABOVE);
            m_score->renderer()->layoutItem(expr);
            PointF p;
            switch (exprDef->horzMeasExprAlign) {
                case others::HorizontalMeasExprAlign::LeftBarline: {
                    if (measure == measure->system()->first()) {
                        if (const BarLine* bl = measure->startBarLine()) {
                            p.rx() = bl->pageX();
                        }
                    } else if (measure->prevMeasureMM()) {
                        if (const BarLine* bl = measure->prevMeasureMM()->endBarLine()) {
                            p.rx() = bl->pageX();
                        }
                    }
                    break;
                }
                case others::HorizontalMeasExprAlign::CenterPrimaryNotehead:
                case others::HorizontalMeasExprAlign::LeftOfPrimaryNotehead: {
                    Segment* seg = measure->findSegmentR(SegmentType::ChordRest, rTick);
                    if (seg && seg->element(expr->track())) {
                        if (seg->element(expr->track())->isChord()) {
                            Chord* c = toChord(seg->element(expr->track()));
                            if (exprAssign->graceNoteIndex) {
                                if (Chord* gc = c->graceNoteAt(static_cast<size_t>(exprAssign->graceNoteIndex - 1))) {
                                    c = gc;
                                }
                            }
                            engraving::Note* n = c->up() ? c->downNote() : c->upNote();
                            p.rx() = n->pageX();
                            if (exprDef->horzMeasExprAlign == others::HorizontalMeasExprAlign::CenterPrimaryNotehead) {
                                p.rx() += n->noteheadCenterX();
                            }
                        } else {
                            Rest* rest = toRest(seg->element(expr->track()));
                            if (rest->isFullMeasureRest()) {
                                p.rx() = seg->pageX();
                            } else {
                                p.rx() = rest->pageX();
                                if (exprDef->horzMeasExprAlign == others::HorizontalMeasExprAlign::CenterPrimaryNotehead) {
                                    p.rx() += rest->centerX();
                                }
                            }
                        }
                    } else {
                        p.rx() = s->pageX();
                    }
                    break;
                }
                case others::HorizontalMeasExprAlign::Manual:
                case others::HorizontalMeasExprAlign::StartOfMusic:
                case others::HorizontalMeasExprAlign::Stem:
                case others::HorizontalMeasExprAlign::LeftOfAllNoteheads: {
                    Segment* seg = measure->findSegmentR(SegmentType::ChordRest, rTick);
                    if (seg && seg->element(expr->track())) {
                        if (seg->element(expr->track())->isChord()) {
                            Chord* c = toChord(seg->element(expr->track()));
                            if (exprAssign->graceNoteIndex) {
                                if (Chord* gc = c->graceNoteAt(static_cast<size_t>(exprAssign->graceNoteIndex - 1))) {
                                    c = gc;
                                }
                            }
                            p.rx() = c->pageX();
                            if (exprDef->horzMeasExprAlign == others::HorizontalMeasExprAlign::Stem) {
                                p.rx() += rendering::score::StemLayout::stemPosX(c);
                            }
                        } else {
                            Rest* rest = toRest(seg->element(expr->track()));
                            p.rx() = rest->isFullMeasureRest() ? seg->pageX() : rest->pageX();
                        }
                    } else {
                        p.rx() = s->pageX();
                    }
                    break;
                }
                case others::HorizontalMeasExprAlign::CenterAllNoteheads: {
                    Shape staffShape = s->staffShape(expr->staffIdx());
                    staffShape.remove_if([](ShapeElement& el) { return el.height() == 0; });
                    p.rx() = staffShape.right() / 2 + s->x() + s->measure()->x();
                    break;
                }
                case others::HorizontalMeasExprAlign::RightOfAllNoteheads: {
                    Shape staffShape = s->staffShape(expr->staffIdx());
                    staffShape.remove_if([](ShapeElement& el) { return el.height() == 0; });
                    p.rx() = staffShape.right() + s->x() + s->measure()->x();
                    break;
                }
                case others::HorizontalMeasExprAlign::StartTimeSig: {
                    // Observed: Elements placed .45sp too far left when there is a custom offset
                    Segment* seg = measure->findSegmentR(SegmentType::TimeSig, rTick);
                    p.rx() = seg ? seg->pageX() : s->pageX();
                    break;
                }
                case others::HorizontalMeasExprAlign::AfterClefKeyTime: {
                    Segment* seg = s->prev(SegmentType::TimeSig | SegmentType::KeySig | SegmentType::HeaderClef
                                           | SegmentType::StartRepeatBarLine | SegmentType::BeginBarLine);
                    p.rx() = seg ? seg->pageX() : s->pageX();
                    break;
                }
                case others::HorizontalMeasExprAlign::CenterOverMusic: {
                    p.rx() = measure->findSegmentR(SegmentType::ChordRest, Fraction(0, 1))->x() / 2;
                    [[fallthrough]];
                }
                case others::HorizontalMeasExprAlign::CenterOverBarlines: {
                    const BarLine* bl = measure->endBarLine();
                    p.rx() += measure->pageX() + (bl ? bl->segment()->x() + bl->ldata()->bbox().width() : measure->width()) / 2;
                    break;
                }
                case others::HorizontalMeasExprAlign::RightBarline: {
                    if (const BarLine* bl = measure->endBarLine()) {
                        p.rx() = bl->pageX() + bl->ldata()->bbox().width();
                    } else {
                        p.rx() = measure->pageX() + measure->width();
                    }
                    break;
                }
            }
            p.rx() += doubleFromEvpu(exprDef->measXAdjust) * SPATIUM20;
            // We will need this when justify differs from text alignment (currently we set alignment to justify)
            /* switch (item->hAlign()) {
                case AlignH::LEFT:
                    break;
                case AlignH::HCENTER:
                    p.rx() -= expr->ldata()->bbox().center().y() / 2;
                    break;
                case AlignH::RIGHT:
                    p.rx() -= expr->ldata()->bbox().center().y();
                    break;
            } */

            StaffCmper effectiveMusxStaffId = expressionAssignment->staffAssign >= 0 ? expressionAssignment->staffAssign : muse::value(m_staff2Inst, expr->staffIdx(), 1);
            const MusxInstance<others::StaffComposite> musxStaff = others::StaffComposite::createCurrent(m_doc, m_currentMusxPartId, effectiveMusxStaffId, exprAssign->getCmper(), 0);
            const Staff* staff = m_score->staff(expr->staffIdx());
            const double staffReferenceOffset = musxStaff->calcTopLinePosition() * 0.5 * staff->spatium(s->tick()) * staff->staffType(s->tick())->lineDistance().val();

            switch (exprDef->vertMeasExprAlign) {
                case others::VerticalMeasExprAlign::AboveStaff: {
                    expr->setPlacement(PlacementV::ABOVE);
                    p.ry() = expr->pagePos().y() - doubleFromEvpu(exprDef->yAdjustBaseline) * SPATIUM20;

                    SystemCmper sc = m_doc->calculateSystemFromMeasure(m_currentMusxPartId, exprAssign->getCmper())->getCmper();
                    double baselinepos = doubleFromEvpu(musxStaff->calcBaselinePosition<details::BaselineExpressionsAbove>(sc)) * SPATIUM20; // Needs to be scaled correctly (offset topline/reference pos)?
                    p.ry() -= (baselinepos - staffReferenceOffset);
                    break;
                }
                case others::VerticalMeasExprAlign::Manual: {
                    expr->setPlacement(PlacementV::ABOVE); // Finale default
                    p.ry() = expr->pagePos().y();
                    // Add staffreferenceoffset?
                    break;
                }
                case others::VerticalMeasExprAlign::RefLine: {
                    expr->setPlacement(PlacementV::ABOVE);
                    p.ry() = expr->pagePos().y() - staffReferenceOffset - doubleFromEvpu(exprDef->yAdjustBaseline) * SPATIUM20;
                    break;
                }
                case others::VerticalMeasExprAlign::BelowStaff: {
                    expr->setPlacement(PlacementV::BELOW);
                    p.ry() = expr->pagePos().y() - doubleFromEvpu(exprDef->yAdjustBaseline) * SPATIUM20;

                    SystemCmper sc = m_doc->calculateSystemFromMeasure(m_currentMusxPartId, exprAssign->getCmper())->getCmper();
                    double baselinepos = doubleFromEvpu(musxStaff->calcBaselinePosition<details::BaselineExpressionsBelow>(sc)) * SPATIUM20; // Needs to be scaled correctly (offset topline/reference pos)?
                    p.ry() -= (baselinepos - staffReferenceOffset);
                    break;
                }
                case others::VerticalMeasExprAlign::TopNote: {
                    expr->setPlacement(PlacementV::ABOVE);
                    Segment* seg = measure->findSegmentR(SegmentType::ChordRest, rTick);
                    if (seg && seg->element(expr->track())) {
                        if (seg->element(expr->track())->isChord()) {
                            Chord* c = toChord(seg->element(expr->track()));
                            if (exprAssign->graceNoteIndex) {
                                if (Chord* gc = c->graceNoteAt(static_cast<size_t>(exprAssign->graceNoteIndex - 1))) {
                                    c = gc;
                                }
                            }
                            const engraving::Note* n = c->upNote();
                            p.ry() = n->pagePos().y() - n->headHeight() / 2;
                        } else {
                            Rest* rest = toRest(seg->element(expr->track()));
                            p.ry() = rest->pagePos().y() - rest->ldata()->bbox().center().y();
                        }
                    }
                    p.ry() -= doubleFromEvpu(exprDef->yAdjustEntry) * SPATIUM20;
                    break;
                }
                case others::VerticalMeasExprAlign::BottomNote: {
                    expr->setPlacement(PlacementV::BELOW);
                    Segment* seg = measure->findSegmentR(SegmentType::ChordRest, rTick);
                    if (seg && seg->element(expr->track())) {
                        if (seg->element(expr->track())->isChord()) {
                            Chord* c = toChord(seg->element(expr->track()));
                            if (exprAssign->graceNoteIndex) {
                                if (Chord* gc = c->graceNoteAt(static_cast<size_t>(exprAssign->graceNoteIndex - 1))) {
                                    c = gc;
                                }
                            }
                            const engraving::Note* n = c->downNote();
                            p.ry() = n->pagePos().y() - n->headHeight() / 2;
                        } else {
                            Rest* rest = toRest(seg->element(expr->track()));
                            p.ry() = rest->pagePos().y() - rest->ldata()->bbox().center().y();
                        }
                    }
                    p.ry() -= doubleFromEvpu(exprDef->yAdjustEntry) * SPATIUM20;
                    break;
                }
                case others::VerticalMeasExprAlign::AboveEntry:
                case others::VerticalMeasExprAlign::AboveStaffOrEntry: {
                    expr->setPlacement(PlacementV::ABOVE);
                    Segment* seg = measure->findSegmentR(SegmentType::ChordRest, rTick); // why is this needed

                    // should this really be all tracks?
                    Shape staffShape = seg->staffShape(expr->staffIdx());
                    staffShape.translate(PointF(seg->pageX(), seg->system()->pagePos().y() + seg->system()->staff(expr->staffIdx())->y()));
                    // staffShape.remove_if([](ShapeElement& el) { return el.height() == 0; });
                    double entryY = staffShape.top() - doubleFromEvpu(exprDef->yAdjustEntry) * SPATIUM20;

                    SystemCmper sc = m_doc->calculateSystemFromMeasure(m_currentMusxPartId, exprAssign->getCmper())->getCmper();
                    double baselinepos = doubleFromEvpu(musxStaff->calcBaselinePosition<details::BaselineExpressionsAbove>(sc)) * SPATIUM20; // Needs to be scaled correctly (offset topline/reference pos)?
                    baselinepos = expr->pagePos().y() - (baselinepos - staffReferenceOffset) - doubleFromEvpu(exprDef->yAdjustBaseline) * SPATIUM20;
                    p.ry() = std::min(baselinepos, entryY);
                    break;
                }
                case others::VerticalMeasExprAlign::BelowEntry:
                case others::VerticalMeasExprAlign::BelowStaffOrEntry: {
                    // doesn't seem to be working reliably yet in all cases
                    expr->setPlacement(PlacementV::BELOW);

                    // should this really be all tracks?
                    Shape staffShape = s->staffShape(expr->staffIdx());
                    // staffShape.remove_if([](ShapeElement& el) { return el.height() == 0; });
                    staffShape.translate(PointF(s->pageX(), s->system()->pagePos().y() + s->system()->staff(expr->staffIdx())->y()));
                    double entryY = staffShape.bottom() - doubleFromEvpu(exprDef->yAdjustEntry) * SPATIUM20;

                    SystemCmper sc = m_doc->calculateSystemFromMeasure(m_currentMusxPartId, exprAssign->getCmper())->getCmper();
                    double baselinepos = doubleFromEvpu(musxStaff->calcBaselinePosition<details::BaselineExpressionsBelow>(sc)) * SPATIUM20; // Needs to be scaled correctly (offset topline/reference pos)?
                    baselinepos = expr->pagePos().y() - (baselinepos - staffReferenceOffset) - doubleFromEvpu(exprDef->yAdjustBaseline) * SPATIUM20;
                    p.ry() = std::max(baselinepos, entryY);
                    break;
                }
                default: {
                    expr->setPlacement(PlacementV::ABOVE); // Finale default
                    p.ry() = expr->pagePos().y() - doubleFromEvpu(exprDef->yAdjustEntry) * SPATIUM20;
                    break;
                }
            }
            p -= expr->pagePos();
            if (expr->placeBelow()) {
                if (p.y() < staff->staffHeight(s->tick()) / 2) {
                    setAndStyleProperty(expr, Pid::PLACEMENT, PlacementV::ABOVE, true);
                } else {
                    p.ry() -= staff->staffHeight(s->tick());
                }
            } else {
                if (p.y() > staff->staffHeight(s->tick()) / 2) {
                    setAndStyleProperty(expr, Pid::PLACEMENT, PlacementV::BELOW, true);
                    p.ry() -= staff->staffHeight(s->tick());
                }
            }
            if (expr->hasVoiceAssignmentProperties()) {
                setAndStyleProperty(expr, Pid::DIRECTION, expr->placeAbove() ? DirectionV::UP : DirectionV::DOWN);
            }
            p += evpuToPointF(exprAssign->horzEvpuOff, -exprAssign->vertEvpuOff) * SPATIUM20; // assignment offset
            setAndStyleProperty(expr, Pid::OFFSET, p);
        };
        positionExpression(item, expressionAssignment, expressionDef);

        if (item->systemFlag()) {
            m_systemObjectStaves.insert(item->staffIdx());
            parsedAssignments.push_back(expressionId);
            if (!expressionAssignment->staffList) {
                continue;
            }
            /// @todo improved handling for bottom system objects
            const MusxInstanceList<others::MeasureExprAssign> possibleLinks = m_doc->getOthers()->getArray<others::MeasureExprAssign>(m_currentMusxPartId, expressionAssignment->getCmper());
            for (const auto& linkedAssignment : possibleLinks) {
                if (linkedAssignment->eduPosition != expressionAssignment->eduPosition
                    || linkedAssignment->textExprId != expressionAssignment->textExprId) {
                    continue;
                }
                staff_idx_t linkedStaffIdx = staffIdxFromAssignment(linkedAssignment->staffAssign);
                std::pair<Cmper, Inci> linkedExpressionId = std::make_pair(expressionAssignment->getCmper(), expressionAssignment->getInci().value_or(0));
                if (muse::contains(parsedAssignments, linkedExpressionId) || linkedStaffIdx == muse::nidx) {
                    continue;
                }

                TextBase* copy = toTextBase(item->clone());
                copy->setVisible(!linkedAssignment->hidden);
                copy->setStaffIdx(linkedStaffIdx);
                const MusxInstance<others::TextExpressionDef>& linkedDef = linkedAssignment->getTextExpression();
                copy->setAlign(Align(toAlignH(linkedDef->horzExprJustification), AlignV::BASELINE));
                copy->linkTo(item);
                s->add(copy);
                positionExpression(copy, linkedAssignment, linkedDef);
                m_systemObjectStaves.insert(linkedStaffIdx);
                parsedAssignments.push_back(linkedExpressionId);
            }
        }
        /// @todo use expressionAssignment->showStaffList to control sharing between score/parts. some elements can be hidden entirely, others will be made invisible
    }

    // Measure-anchored text (MeasureTextAssign)
    MusxInstanceList<others::StaffUsed> musxScrollView = m_doc->getOthers()->getArray<others::StaffUsed>(m_currentMusxPartId, BASE_SYSTEM_ID);
    MusxInstanceList<others::Measure> musxMeasures = m_doc->getOthers()->getArray<others::Measure>(m_currentMusxPartId);
    for (const MusxInstance<others::StaffUsed>& musxScrollViewItem : musxScrollView) {
        // per staff style calculations
        const MusxInstance<others::Staff>& rawStaff = m_doc->getOthers()->get<others::Staff>(m_currentMusxPartId, musxScrollViewItem->staffId);

        staff_idx_t curStaffIdx = muse::value(m_inst2Staff, musxScrollViewItem->staffId, muse::nidx);
        IF_ASSERT_FAILED(curStaffIdx != muse::nidx) {
            logger()->logWarning(String(u"MeasureTextAssign: Musx inst value not found for staff cmper %1").arg(String::fromStdString(std::to_string(rawStaff->getCmper()))));
            continue;
        }
        track_idx_t curTrackIdx = staff2track(curStaffIdx);

        for (const MusxInstance<others::Measure>& musxMeasure : musxMeasures) {
            Fraction currTick = muse::value(m_meas2Tick, musxMeasure->getCmper(), Fraction(-1, 1));
            Measure* measure = !currTick.negative() ? m_score->tick2measure(currTick) : nullptr;
            IF_ASSERT_FAILED(measure) {
                logger()->logWarning(String(u"Unable to retrieve measure by tick"), m_doc, 0, musxMeasure->getCmper());
                return;
            }

            for (const auto& measureTextAssign : m_doc->getDetails()->getArray<details::MeasureTextAssign>(m_currentMusxPartId, rawStaff->getCmper(), musxMeasure->getCmper())) {
                Fraction rTick = eduToFraction(measureTextAssign->xDispEdu);
                Segment* s = measure->getChordRestOrTimeTickSegment(measure->tick() + rTick);

                StaffText* text = Factory::createStaffText(s);
                text->setTrack(curTrackIdx);
                text->setXmlText(stringFromEnigmaText(measureTextAssign->getRawTextCtx(m_currentMusxPartId)));
                text->checkCustomFormatting(text->xmlText());
                if (text->plainText().empty()) {
                    delete text;
                    continue;
                }
                text->setVisible(!measureTextAssign->hidden);
                text->setSizeIsSpatiumDependent(false);
                text->setAutoplace(false);
                setAndStyleProperty(text, Pid::OFFSET, (evpuToPointF(rTick.isZero() ? measureTextAssign->xDispEvpu : 0, -measureTextAssign->yDisp) * SPATIUM20), true);
                s->add(text);
            }
        }
    }

    // Repeat markings (markers and jumps)
    const MusxInstanceList<others::TextRepeatAssign> textRepeatAssignments = m_doc->getOthers()->getArray<others::TextRepeatAssign>(m_currentMusxPartId);
    logger()->logInfo(String(u"Import repeat texts: Found %1 texts.").arg(textRepeatAssignments.size()));
    for (const auto& repeatAssignment : textRepeatAssignments) {
        // Search our converted repeat text library, or if not found add to it
        ReadableRepeatText* repeatText = muse::value(m_repeatTexts, repeatAssignment->textRepeatId, nullptr); /// @todo does this code work for part scores?
        const MusxInstance<others::TextRepeatDef> repeatDefinition = m_doc->getOthers()->get<others::TextRepeatDef>(m_currentMusxPartId, repeatAssignment->textRepeatId);
        if (!repeatText) {
            repeatText = new ReadableRepeatText(*this, repeatDefinition);
            m_repeatTexts.emplace(repeatAssignment->textRepeatId, repeatText);
        }

        if (repeatText->xmlText.empty()) {
            continue;
        }

        // Find staff
        std::vector<std::pair<staff_idx_t, StaffCmper>> links;
        staff_idx_t curStaffIdx = staffIdxForRepeats(repeatAssignment->topStaffOnly, repeatAssignment->staffList, links);

        if (curStaffIdx == muse::nidx) {
            logger()->logWarning(String(u"Add repeat text: Musx inst value not found."));
            continue;
        }

        // Find location in measure
        Fraction mTick = muse::value(m_meas2Tick, repeatAssignment->getCmper(), Fraction(-1, 1));
        Measure* measure = !mTick.negative() ? m_score->tick2measure(mTick) : nullptr;
        if (!measure) {
            continue;
        }

        track_idx_t curTrackIdx = staff2track(curStaffIdx);
        logger()->logInfo(String(u"Creating a %1 at tick %2 on track %3.").arg(TConv::userName(repeatText->elementType).translated(), measure->tick().toString(), String::number(curTrackIdx)));
        TextBase* item = toTextBase(Factory::createItem(repeatText->elementType, measure));
        item->setParent(measure);
        item->setVisible(!repeatAssignment->hidden);
        item->setTrack(curTrackIdx);
        if (item->isJump()) {
            toJump(item)->setJumpType(repeatText->jumpType);
        } else if (item->isMarker()) {
            toMarker(item)->setMarkerType(repeatText->markerType);
        }
        String replaceText = String();
        switch (repeatDefinition->poundReplace) {
        case others::TextRepeatDef::PoundReplaceOption::RepeatID: {
            /// @todo support correct font styling
            if (const auto& targetRepeat = m_doc->getOthers()->get<others::TextRepeatDef>(m_currentMusxPartId, repeatAssignment->targetValue)) {
                FontTracker font = FontTracker(repeatDefinition->useThisFont ? repeatDefinition->font : targetRepeat->font);
                replaceText = textFromRepeatDef(targetRepeat, font);
            }
            break;
        }
        case others::TextRepeatDef::PoundReplaceOption::MeasureNumber:
            replaceText = String::number(repeatAssignment->targetValue);
            break;
        default:
            replaceText = String::number(repeatAssignment->passNumber);
            break;
        }
        item->setXmlText(repeatText->xmlText.replace(u"#", replaceText));
        item->checkCustomFormatting(item->xmlText());
        item->setAlign(Align(repeatText->repeatAlignment, AlignV::BASELINE));
        if (item->isMarker()) {
            item->setPosition(repeatText->repeatAlignment); /// @todo 'center' position centers over barline in musescore, over measure in finale
        }
        item->setFrameType(repeatText->frameSettings.frameType);
        if (item->frameType() != FrameType::NO_FRAME) {
            item->setFrameWidth(absoluteSpatium(repeatText->frameSettings.frameWidth, item)); // is this the correct scaling?
            item->setPaddingWidth(absoluteSpatium(repeatText->frameSettings.paddingWidth, item)); // is this the correct scaling?
            item->setFrameRound(repeatText->frameSettings.frameRound);
        }
        item->setAutoplace(false);
        setAndStyleProperty(item, Pid::PLACEMENT, PlacementV::ABOVE);
        PointF p = evpuToPointF(repeatAssignment->horzPos, -repeatAssignment->vertPos) * SPATIUM20; /// @todo adjust for staff reference line?
        double blAdjust = item->align() == AlignH::RIGHT && measure->endBarLine()
                            ? measure->endBarLine()->ldata()->bbox().width() : 0.0;
        p.rx() -= blAdjust;
        setAndStyleProperty(item, Pid::OFFSET, p);
        measure->add(item);
        m_systemObjectStaves.insert(curStaffIdx);

        for (auto [linkedStaffIdx, linkedMusxStaffId] : links) {
            /// @todo improved handling for bottom system objects
            TextBase* copy = toTextBase(item->clone());
            copy->setStaffIdx(linkedStaffIdx);
            const MusxInstance<others::RepeatIndividualPositioning>& indiv = repeatAssignment->getIndividualPositioning(linkedMusxStaffId);
            if (repeatAssignment->individualPlacement && indiv) {
                copy->setVisible(!indiv->hidden);
                PointF p1 = evpuToPointF(indiv->x1add, -indiv->y1add) * SPATIUM20; /// @todo adjust for staff reference line?
                p1.rx() -= blAdjust;
                setAndStyleProperty(item, Pid::OFFSET, p1);
            } else {
                copy->setVisible(!repeatAssignment->hidden);
                setAndStyleProperty(copy, Pid::OFFSET, p);
            }
            copy->linkTo(item);
            measure->add(copy);
            m_systemObjectStaves.insert(linkedStaffIdx);
        }
        /// @todo fine-tune playback
    }
}

bool FinaleParser::isOnlyPage(const MusxInstance<others::PageTextAssign>& pageTextAssign, PageCmper page)
{
    const std::optional<PageCmper> startPageNum = pageTextAssign->calcStartPageNumber(m_currentMusxPartId);
    const std::optional<PageCmper> endPageNum = pageTextAssign->calcEndPageNumber(m_currentMusxPartId); // calcEndPageNumber handles case when endPage is zero
    return (startPageNum == page && endPageNum == page);
};

static PointF pagePosOfPageTextAssign(Page* page, const MusxInstance<others::PageTextAssign>& pageTextAssign, RectF bbox)
{
    /// @todo once position and alignment are decoupled, don't use font bbox
    RectF pageContentRect = page->ldata()->bbox().translated(PointF());
    if (!pageTextAssign->hPosPageEdge) {
        pageContentRect.adjust(page->lm(), 0.0, -page->rm(), 0.0);
    }
    if (!pageTextAssign->vPosPageEdge) {
        pageContentRect.adjust(0.0, page->tm(), 0.0, -page->bm());
    }
    PointF p(pageContentRect.x(), pageContentRect.y());

    switch (pageTextAssign->vPos) {
    case others::PageTextAssign::VerticalAlignment::Top:
        break;
    case others::PageTextAssign::VerticalAlignment::Center:
        p.ry() += (pageContentRect.height() - bbox.height()) / 2;
        break;
    case others::PageTextAssign::VerticalAlignment::Bottom:
        p.ry() += pageContentRect.height() - bbox.height();
        break;
    }

    if (pageTextAssign->indRpPos && !(page->no() & 1)) {
        switch(pageTextAssign->hPosRp) {
        case others::PageTextAssign::HorizontalAlignment::Left:
            break;
        case others::PageTextAssign::HorizontalAlignment::Center:
            p.rx() += (pageContentRect.width() - bbox.width()) / 2;
            break;
        case others::PageTextAssign::HorizontalAlignment::Right:
            p.rx() += pageContentRect.width() - bbox.width();
            break;
        }
        p.rx() += doubleFromEvpu(pageTextAssign->rightPgXDisp) * SPATIUM20;
        p.ry() -= doubleFromEvpu(pageTextAssign->rightPgYDisp) * SPATIUM20;
    } else {
        switch(pageTextAssign->hPosLp) {
        case others::PageTextAssign::HorizontalAlignment::Left:
            break;
        case others::PageTextAssign::HorizontalAlignment::Center:
            p.rx() += (pageContentRect.width() - bbox.width()) / 2;
            break;
        case others::PageTextAssign::HorizontalAlignment::Right:
            p.rx() += pageContentRect.width() - bbox.width();
            break;
        }
        p.rx() += doubleFromEvpu(pageTextAssign->xDisp) * SPATIUM20;
        p.ry() -= doubleFromEvpu(pageTextAssign->yDisp) * SPATIUM20;
    }
    return p;
}

/// @todo Instead of hard-coding page 1 and page 2, we need to find the first page in the Finale file with music on it
/// and use that as the first page. At least, that is my impression. How to handle blank pages in MuseScore is an open question.
/// - RGP

void FinaleParser::importPageTexts()
{
    MusxInstanceList<others::PageTextAssign> pageTextAssignList = m_doc->getOthers()->getArray<others::PageTextAssign>(m_currentMusxPartId);
    logger()->logInfo(String(u"Importing %1 page text assignments").arg(pageTextAssignList.size()));

    struct HeaderFooter {
        bool show = false;
        bool showFirstPage = true; // always show first page
        bool oddEven = true; // always different odd/even pages
        MusxInstance<others::PageTextAssign> oddLeftText;
        MusxInstance<others::PageTextAssign> oddMiddleText;
        MusxInstance<others::PageTextAssign> oddRightText;
        MusxInstance<others::PageTextAssign> evenLeftText;
        MusxInstance<others::PageTextAssign> evenMiddleText;
        MusxInstance<others::PageTextAssign> evenRightText;
        std::vector<MusxInstance<others::PageTextAssign>> firstPageTexts;
    };

    HeaderFooter header;
    HeaderFooter footer;
    std::vector<MusxInstance<others::PageTextAssign>> notHF;

    // gather texts by position
    for (const MusxInstance<others::PageTextAssign>& pageTextAssign : pageTextAssignList) {
        if (pageTextAssign->hidden) {
            // there may be something we can do with hidden assignments created for Patterson's Copyist Helper plugin,
            // but generally it means the header is not applicable to this part.
            continue;
        }
        const std::optional<PageCmper> startPage = pageTextAssign->calcStartPageNumber(m_currentMusxPartId);
        const std::optional<PageCmper> endPage = pageTextAssign->calcEndPageNumber(m_currentMusxPartId);
        if (!startPage || !endPage) {
            // this page text does not appear on any page in this musx score/linked part.
            // it happens
            //  1) when the assignment is to a leading blank page that does not exist in this score/part
            //  2) when the start page assignment is beyond the number of pages in this score/part
            continue;
        }

        // if text is not at top or bottom, invisible, or not recurring don't import as hf
        // For 2-page scores, we can import text only assigned to page 2 as a regular even hf.
        if (pageTextAssign->vPos == others::PageTextAssign::VerticalAlignment::Center
            || pageTextAssign->hidden
            || startPage.value() >= 3 /// @todo must be changed to be first non-blank page + 2
            || endPage.value() < PageCmper(m_score->npages()) /// @todo allow copyright on just page 1?
            || !muse::RealIsEqual(pageTextAssign->yDisp, 0.0)
            || !muse::RealIsEqual(pageTextAssign->xDisp, 0.0)
            || (pageTextAssign->indRpPos && (!muse::RealIsEqual(pageTextAssign->rightPgXDisp, 0.0)
                                             || !muse::RealIsEqual(pageTextAssign->rightPgYDisp, 0.0)))
            || pageTextAssign->hPosPageEdge
            || pageTextAssign->vPosPageEdge) {
            notHF.emplace_back(pageTextAssign);
            continue;
        }

        HeaderFooter& hf = pageTextAssign->vPos == others::PageTextAssign::VerticalAlignment::Top ? header : footer;
        /// @todo Finale bases right/left on the actual page numbers, not the visual page numbers. But MuseScore's
        /// left/right headers display based on visual page numbers. So the whole calculation must be reversed if
        /// m_score->pageNumberOffset() is odd.

        // odd pages
        MusxInstance<others::PageTextAssign>* oddLocation = [&]() -> MusxInstance<others::PageTextAssign>* {
            if (pageTextAssign->oddEven != others::PageTextAssign::PageAssignType::Even) {
                others::PageTextAssign::HorizontalAlignment align = pageTextAssign->indRpPos ? pageTextAssign-> hPosRp : pageTextAssign->hPosLp;
                switch (align) {
                case others::PageTextAssign::HorizontalAlignment::Left:
                    return &hf.oddLeftText;
                case others::PageTextAssign::HorizontalAlignment::Center:
                    return &hf.oddMiddleText;
                case others::PageTextAssign::HorizontalAlignment::Right:
                    return &hf.oddRightText;
                }
            }
            return nullptr;
        }();

        // even pages
        MusxInstance<others::PageTextAssign>* evenLocation = [&]() -> MusxInstance<others::PageTextAssign>* {
            if (pageTextAssign->oddEven != others::PageTextAssign::PageAssignType::Even) {
                others::PageTextAssign::HorizontalAlignment align = pageTextAssign->hPosLp;
                switch (align) {
                case others::PageTextAssign::HorizontalAlignment::Left:
                    return &hf.evenLeftText;
                case others::PageTextAssign::HorizontalAlignment::Center:
                    return &hf.evenMiddleText;
                case others::PageTextAssign::HorizontalAlignment::Right:
                    return &hf.evenRightText;
                }
            }
            return nullptr;
        }();

        if ((!oddLocation && !evenLocation) // text can't be assigned
            || (oddLocation && (*oddLocation)) // odd text location already full
            || (evenLocation && (*evenLocation))) {
            notHF.emplace_back(pageTextAssign);
        }

        if (oddLocation || evenLocation) {
            hf.show = true;
            if (oddLocation) {
                *oddLocation = pageTextAssign;
            }
            if (evenLocation) {
                *evenLocation = pageTextAssign;
            }

            // Handle visibility on page 1
            if (startPage.value() > 1) {
                hf.showFirstPage = false;
            } else {
                hf.firstPageTexts.emplace_back(pageTextAssign);
            }
        }
    }

    std::vector<MusxInstance<others::PageTextAssign>> textsForFirstPage;
    for (HeaderFooter* hf : { &header, &footer }) {
        hf->oddEven = (hf->evenLeftText != hf->oddLeftText)
                      || (hf->evenMiddleText != hf->oddMiddleText)
                      || (hf->evenRightText != hf->oddRightText);
        if (hf->showFirstPage) {
            continue;
        }
        muse::join(textsForFirstPage, hf->firstPageTexts);
        muse::join(notHF, hf->firstPageTexts);
    }

    auto stringFromPageText = [this](const MusxInstance<others::PageTextAssign>& pageText, bool isForHeaderFooter = true) {
        std::optional<PageCmper> startPage = pageText->calcStartPageNumber(m_currentMusxPartId);
        std::optional<PageCmper> endPage = pageText->calcEndPageNumber(m_currentMusxPartId);
        HeaderFooterType hfType = isForHeaderFooter ? HeaderFooterType::FirstPage : HeaderFooterType::None;
        if (isForHeaderFooter && startPage == 2 && endPage.value() == PageCmper(m_score->npages())) {
            hfType = HeaderFooterType::SecondPageToEnd;
        }
        std::optional<PageCmper> forPageId = hfType != HeaderFooterType::SecondPageToEnd ? startPage : std::nullopt;
        musx::util::EnigmaParsingContext parsingContext = pageText->getRawTextCtx(m_currentMusxPartId, forPageId);
        EnigmaParsingOptions options(hfType);
        /// @todo set options.scaleFontSizeBy to per-page scaling if MuseScore can't do per-page scaling directly.
        return stringFromEnigmaText(parsingContext, hfType);
    };

    if (header.show) {
        m_score->style().set(Sid::showHeader,      true);
        m_score->style().set(Sid::headerFirstPage, header.showFirstPage);
        m_score->style().set(Sid::headerOddEven,   header.oddEven);
        m_score->style().set(Sid::evenHeaderL,     header.evenLeftText ? stringFromPageText(header.evenLeftText) : String());
        m_score->style().set(Sid::evenHeaderC,     header.evenMiddleText ? stringFromPageText(header.evenMiddleText) : String());
        m_score->style().set(Sid::evenHeaderR,     header.evenRightText ? stringFromPageText(header.evenRightText) : String());
        m_score->style().set(Sid::oddHeaderL,      header.oddLeftText ? stringFromPageText(header.oddLeftText) : String());
        m_score->style().set(Sid::oddHeaderC,      header.oddMiddleText ? stringFromPageText(header.oddMiddleText) : String());
        m_score->style().set(Sid::oddHeaderR,      header.oddRightText ? stringFromPageText(header.oddRightText) : String());
    } else {
        m_score->style().set(Sid::showHeader,      false);
    }

    if (footer.show) {
        m_score->style().set(Sid::showFooter,      true);
        m_score->style().set(Sid::footerFirstPage, footer.showFirstPage);
        m_score->style().set(Sid::footerOddEven,   footer.oddEven);
        m_score->style().set(Sid::evenFooterL,     footer.evenLeftText ? stringFromPageText(footer.evenLeftText) : String());
        m_score->style().set(Sid::evenFooterC,     footer.evenMiddleText ? stringFromPageText(footer.evenMiddleText) : String());
        m_score->style().set(Sid::evenFooterR,     footer.evenRightText ? stringFromPageText(footer.evenRightText) : String());
        m_score->style().set(Sid::oddFooterL,      footer.oddLeftText ? stringFromPageText(footer.oddLeftText) : String());
        m_score->style().set(Sid::oddFooterC,      footer.oddMiddleText ? stringFromPageText(footer.oddMiddleText) : String());
        m_score->style().set(Sid::oddFooterR,      footer.oddRightText ? stringFromPageText(footer.oddRightText) : String());
    } else {
        m_score->style().set(Sid::showFooter,      false);
    }

    auto getPages = [&](const MusxInstance<others::PageTextAssign>& pageTextAssign) -> std::vector<page_idx_t> {
        std::vector<page_idx_t> pagesWithText;
        page_idx_t startP = page_idx_t(pageTextAssign->calcStartPageNumber(m_currentMusxPartId).value_or(1) - 1);
        // This text is part of a HF for pages 2+ and only needs to be added on page 1
        if (muse::contains(textsForFirstPage, pageTextAssign)) {
            return { startP };
        }
        page_idx_t endP = page_idx_t(pageTextAssign->calcEndPageNumber(m_currentMusxPartId).value_or(PageCmper(m_score->npages())) - 1);
        for (page_idx_t i = startP; i <= endP; ++i) {
            pagesWithText.emplace_back(i);
        }
        return pagesWithText;
    };

    auto addPageTextToMeasure = [&](const MusxInstance<others::PageTextAssign>& pageTextAssign, MeasureBase* mb, Page* page, const String& pageText) {
        /// @todo set text alignment / position
        if (mb->isMeasure()) {
            // Add as staff text
            Measure* measure = toMeasure(mb);
            Segment* s = measure->getChordRestOrTimeTickSegment(measure->tick());
            StaffText* text = Factory::createStaffText(s);
            text->setTrack(0);
            text->setXmlText(pageText);
            if (text->plainText().empty()) {
                delete text;
                return;
            }
            text->checkCustomFormatting(pageText);
            text->setVisible(!pageTextAssign->hidden);
            text->setSizeIsSpatiumDependent(false);
            text->setAutoplace(false);
            setAndStyleProperty(text, Pid::PLACEMENT, PlacementV::ABOVE);
            PointF p = pagePosOfPageTextAssign(page, pageTextAssign, RectF()); //
            AlignH hAlignment = toAlignH(pageTextAssign->indRpPos && !(page->no() & 1) ? pageTextAssign->hPosRp : pageTextAssign->hPosLp);
            setAndStyleProperty(text, Pid::ALIGN, Align(hAlignment, toAlignV(pageTextAssign->vPos)), true);
            setAndStyleProperty(text, Pid::OFFSET, (p - mb->pagePos()), true); // is this accurate enough?
            s->add(text);
        } else if (mb->isBox()) {
            Text* text = Factory::createText(toBox(mb));
            text->setXmlText(pageText);
            if (text->plainText().empty()) {
                delete text;
                return;
            }
            text->setParent(mb);
            text->checkCustomFormatting(pageText);
            text->setVisible(!pageTextAssign->hidden);
            text->setSizeIsSpatiumDependent(false);
            text->score()->renderer()->layoutItem(text);
            PointF p = pagePosOfPageTextAssign(page, pageTextAssign, text->ldata()->bbox());
            AlignH hAlignment = toAlignH(pageTextAssign->indRpPos && !(page->no() & 1) ? pageTextAssign->hPosRp : pageTextAssign->hPosLp);
            setAndStyleProperty(text, Pid::ALIGN, Align(hAlignment, toAlignV(pageTextAssign->vPos)), true);
            setAndStyleProperty(text, Pid::OFFSET, p);
            toBox(mb)->add(text);
        }
    };

    std::unordered_map<page_idx_t, MeasureBase*> topBoxes;
    std::unordered_map<page_idx_t, MeasureBase*> bottomBoxes;

    for (MusxInstance<others::PageTextAssign> pageTextAssign : notHF) {
        musx::util::EnigmaParsingContext parsingContext = pageTextAssign->getRawTextCtx(m_currentMusxPartId);
        String pageText = stringFromEnigmaText(parsingContext);
        for (page_idx_t i : getPages(pageTextAssign)) {
            Page* page = m_score->pages().at(i);
            MeasureBase* mb = [&]() {
                // Don't add frames for text vertically aligned to the center.
                if (pageTextAssign->vPos == others::PageTextAssign::VerticalAlignment::Center) {
                    // Get text
                    /// @todo move this out of the for loop??
                    EnigmaParsingOptions options;
                    options.plainText = true;
                    musx::util::EnigmaParsingContext parsingContext = pageTextAssign->getRawTextCtx(m_currentMusxPartId);
                    FontTracker firstFontInfo;
                    String pagePlainText = stringFromEnigmaText(parsingContext, options, &firstFontInfo);

                    // Use font metrics to precompute bbox (inaccurate for multiline/multiformat)
                    muse::draw::Font f(firstFontInfo.fontName, muse::draw::Font::Type::Unknown);
                    f.setPointSizeF(firstFontInfo.fontSize);
                    f.setBold(firstFontInfo.fontStyle & FontStyle::Bold);
                    f.setItalic(firstFontInfo.fontStyle & FontStyle::Italic);
                    f.setUnderline(firstFontInfo.fontStyle & FontStyle::Underline);
                    f.setStrike(firstFontInfo.fontStyle & FontStyle::Strike);
                    muse::draw::FontMetrics fm(f);
                    RectF r = fm.boundingRect(pagePlainText);
                    PointF pagePosOfPageText = pagePosOfPageTextAssign(page, pageTextAssign, r);
                    double prevDist = DBL_MAX;
                    for (System* s : page->systems()) {
                        for (MeasureBase* m : s->measures()) {
                            double dist = m->ldata()->bbox().translated(m->pagePos()).distanceTo(pagePosOfPageText);
                            if (dist < prevDist) {
                                mb = m;
                                prevDist = dist;
                            }
                        }
                    }
                    return mb;
                }
                // Create frames at given position if needed
                if (pageTextAssign->vPos == others::PageTextAssign::VerticalAlignment::Top) {
                    if (MeasureBase* presentBase = muse::value(topBoxes, page->no(), nullptr)) {
                        return presentBase;
                    }
                    System* system = page->systems().front();
                    if (system->vbox()) {
                        topBoxes.emplace(page->no(), system->first());
                        return system->first();
                    }
                    VBox* pageFrame = Factory::createVBox(m_score->dummy()->system());
                    pageFrame->setTick(system->first()->tick());
                    pageFrame->setNext(system->first());
                    pageFrame->setPrev(system->first()->prev());
                    m_score->measures()->insert(pageFrame, pageFrame);
                    double distToTopStaff = 0.0;
                    if (Spacer* spacer = system->upSpacer(0, nullptr)) {
                        distToTopStaff = spacer->absoluteGap();
                        spacer->measure()->remove(spacer);
                    }
                    /// @todo check scaling on this
                    double boxToNotationDist = m_score->style().styleMM(Sid::minVerticalDistance);
                    double boxToStaffDist = boxToNotationDist + system->minTop();
                    double headerExtension = page->headerExtension();
                    double headerFooterPadding = m_score->style().styleMM(Sid::staffHeaderFooterPadding);
                    double headerDistance = headerExtension ? headerExtension + headerFooterPadding : 0.0;
                    double maxBoxHeight = distToTopStaff - boxToStaffDist;
                    double preferredHeight = 15 * SPATIUM20;
                    if (maxBoxHeight > preferredHeight) {
                        boxToStaffDist += maxBoxHeight - preferredHeight;
                        boxToNotationDist += maxBoxHeight - preferredHeight;
                        maxBoxHeight = preferredHeight;
                    }
                    pageFrame->setSizeIsSpatiumDependent(false);
                    pageFrame->setAutoSizeEnabled(false);
                    pageFrame->setBoxHeight(Spatium(maxBoxHeight / m_score->style().defaultSpatium()));
                    setAndStyleProperty(pageFrame, Pid::PADDING_TO_NOTATION_BELOW, Spatium(boxToNotationDist / m_score->style().defaultSpatium()));
                    setAndStyleProperty(pageFrame, Pid::BOTTOM_GAP, Spatium(boxToStaffDist / m_score->style().defaultSpatium()));
                    pageFrame->ryoffset() -= headerDistance;
                    topBoxes.emplace(page->no(), toMeasureBase(pageFrame));
                    return toMeasureBase(pageFrame);
                } else {
                    if (MeasureBase* presentBase = muse::value(bottomBoxes, page->no(), nullptr)) {
                        return presentBase;
                    }
                    System* system = page->systems().back();
                    if (system->vbox()) {
                        bottomBoxes.emplace(page->no(), system->first());
                        return system->last();
                    }
                    VBox* pageFrame = Factory::createVBox(m_score->dummy()->system());
                    pageFrame->setTick(system->last()->endTick());
                    pageFrame->setPrev(system->last());
                    pageFrame->setNext(system->last()->next());
                    m_score->measures()->insert(pageFrame, pageFrame);
                    double distToBottomStaff = 0.0;
                    if (Spacer* spacer = system->downSpacer(m_score->nstaves() - 1)) {
                        distToBottomStaff = spacer->absoluteGap();
                        spacer->measure()->remove(spacer);
                    }
                    /// @todo check scaling on this
                    double boxToNotationDist = m_score->style().styleMM(Sid::minVerticalDistance);
                    double boxToStaffDist = boxToNotationDist + system->minBottom();
                    double maxBoxHeight = distToBottomStaff - boxToStaffDist;
                    /// @todo account for footer distance?
                    pageFrame->setAutoSizeEnabled(false);
                    pageFrame->setSizeIsSpatiumDependent(false);
                    pageFrame->setBoxHeight(Spatium(maxBoxHeight / m_score->style().defaultSpatium()));
                    setAndStyleProperty(pageFrame, Pid::PADDING_TO_NOTATION_ABOVE, Spatium(boxToNotationDist / m_score->style().defaultSpatium()));
                    setAndStyleProperty(pageFrame, Pid::TOP_GAP, Spatium(boxToStaffDist / m_score->style().defaultSpatium()));

                    // Move page break, if it exists
                    for (EngravingItem* e : system->last()->el()) {
                        if (e->isLayoutBreak() && toLayoutBreak(e)->layoutBreakType() == LayoutBreakType::PAGE) {
                            system->last()->remove(e);
                            pageFrame->add(e->clone());
                            break;
                        }
                    }
                    bottomBoxes.emplace(page->no(), toMeasureBase(pageFrame));
                    return toMeasureBase(pageFrame);
                }
                /// @todo use sophisticated check for whether to import as frame or not. (i.e. distance to measure is too large, frame would get in the way of music)
            }();
            addPageTextToMeasure(pageTextAssign, mb, page, pageText);
        }
    }
    // if top or bottom, we should hopefully be able to check for distance to surrounding music and work from that
    // if not enough space, attempt to position based on closest measure
    //note: text is placed slightly lower than indicated position (line space? Or ascent instead of bbox)
}

void FinaleParser::rebasePageTextOffsets()
{
    for (System* s : m_score->systems()) {
        if (!s->vbox()) {
            continue;
        }
        Box* b = toBox(s->first());
        for (EngravingItem* e : b->el()) {
            if (e->isTextBase()) {
                // Use current pagePos to counteract layout oddities for small/negative height frames when text is not top aligned
                // Using the bbox position accounts for text alignment
                setAndStyleProperty(e, Pid::OFFSET, e->offset() - (b->pagePos() + e->ldata()->bbox().topLeft()), true);
            }
        }
    }
}

void FinaleParser::importChordsFrets(const MusxInstance<others::StaffUsed>& musxScrollViewItem, const MusxInstance<others::Measure>& musxMeasure,
                                     Staff* staff, Measure* measure)
{
    MusxInstanceList<details::ChordAssign> chordAssignments = m_doc->getDetails()->getArray<details::ChordAssign>(m_currentMusxPartId, musxScrollViewItem->staffId, musxMeasure->getCmper());
    const MusxInstance<options::ChordOptions> config = musxOptions().chordOptions;
    const MusxInstance<FontInfo>& harmonyFont = options::FontOptions::getFontInfo(m_doc, options::FontOptions::FontType::Chord);
    FontStyle f = FinaleTextConv::museFontEfx(harmonyFont);
    String fontFamily = String::fromStdString(harmonyFont->getName());

    for (const MusxInstance<details::ChordAssign>& chordAssignment : chordAssignments) {
        Segment* s = measure->getSegmentR(SegmentType::ChordRest, eduToFraction(chordAssignment->horzEdu));
        KeySigEvent se = staff->keySigEvent(s->tick());
        int minorOffset = se.mode() == KeyMode::MINOR ? 2 : 0; // add ionian?
        int rootTpc = clampEnharmonic(step2tpcByKey(se.degInKey(chordAssignment->rootScaleNum - minorOffset), se.key())
                                      + chordAssignment->rootAlter * TPC_DELTA_SEMITONE);
        int bassTpc = clampEnharmonic(step2tpcByKey(se.degInKey(chordAssignment->bassScaleNum - minorOffset), se.key())
                                      + chordAssignment->bassAlter * TPC_DELTA_SEMITONE);
        /// @todo capo options

        // HarmonyInfo* info = new HarmonyInfo(m_score);
        // info->setBassTpc(bassTpc);
        // info->setRootTpc(rootTpc);
        // h->addChord(info);

        // Instead of writing the properties ourselves, convert to text and parse that.
        /// @todo this approach accounts for none of the custom formatting stored within the suffixes
        String harmonyText = tpc2name(rootTpc, NoteSpellingType::STANDARD, NoteCaseType::CAPITAL);
        static const std::unordered_map<others::ChordSuffixElement::Prefix, Char> prefixMap = {
            { musx::dom::others::ChordSuffixElement::Prefix::Minus, '-' },
            { musx::dom::others::ChordSuffixElement::Prefix::Plus,  '+' },
            { musx::dom::others::ChordSuffixElement::Prefix::Sharp, '#' }, // good enough (even preferred) for MuseScore's parsing
            { musx::dom::others::ChordSuffixElement::Prefix::Flat,  'b' },
        };
        for (const MusxInstance<others::ChordSuffixElement>& suffixElement : chordAssignment->getChordSuffix()) {
            if (suffixElement->prefix != musx::dom::others::ChordSuffixElement::Prefix::None) {
                harmonyText.append(muse::value(prefixMap, suffixElement->prefix));
            }
            if (suffixElement->isNumber) {
                harmonyText += String::number(static_cast<int>(suffixElement->symbol));
            } else {
                /// @todo MuseScore doesn't allow multiple fonts within chord symbols, but some conversion might still be needed
                harmonyText += String::fromUcs4(suffixElement->symbol);
            }
        }
        if (chordAssignment->showAltBass) {
            harmonyText += u"/" + tpc2name(bassTpc, NoteSpellingType::STANDARD, NoteCaseType::CAPITAL);
            m_score->style().set(Sid::chordBassNoteStagger, chordAssignment->bassPosition != details::ChordAssign::BassPosition::AfterRoot);
        }

        // From Harmony::endEdit
        if (true /*h->harmonyType() != HarmonyType::ROMAN*/) {
            harmonyText.replace(u"\u1d12b", u"bb");     // double-flat
            harmonyText.replace(u"\u266d",  u"b");      // flat
            harmonyText.replace(u"\ue260",  u"b");      // flat
            // do not replace natural sign
            // (right now adding the symbol explicitly is the only way to force a natural sign to appear at all)
            //harmonyText.replace("\u266e",  "n");  // natural, if one day we support that too
            //harmonyText.replace("\ue261",  "n");  // natural, if one day we support that too
            harmonyText.replace(u"\u266f",  u"#");      // sharp
            harmonyText.replace(u"\ue262",  u"#");      // sharp
            harmonyText.replace(u"\u1d12a", u"x");      // double-sharp
            harmonyText.replace(u"\u0394",  u"^");      // &Delta;
            harmonyText.replace(u"\u00d0",  u"o");      // &deg;
            harmonyText.replace(u"\u00f8",  u"0");      // &oslash;
            harmonyText.replace(u"\u00d8",  u"0");      // &Oslash;
        } else {
            harmonyText.replace(u"\ue260",  u"\u266d");         // flat
            harmonyText.replace(u"\ue261",  u"\u266e");         // natural
            harmonyText.replace(u"\ue262",  u"\u266f");         // sharp
        }

        FretDiagram* fret;
        Harmony* h;
        const bool fretVisible = config->showFretboards && chordAssignment->showFretboard;
        if (fretVisible) {
            fret = Factory::createFretDiagram(s);
            fret->setTrack(staff2track(staff->idx()));
            fret->setHarmony(harmonyText);
            fret->updateDiagram(harmonyText);
            setAndStyleProperty(fret, Pid::MAG, doubleFromPercent(chordAssignment->fbPercent), true);
            h = fret->harmony();
        } else {
            h = Factory::createHarmony(s);
            h->setTrack(staff2track(staff->idx()));
            h->setHarmony(harmonyText);
            h->afterRead(); // needed?
        }
        h->setBassCase(chordAssignment->bassLowerCase ? NoteCaseType::LOWER : NoteCaseType::UPPER);
        h->setRootCase(chordAssignment->rootLowerCase ? NoteCaseType::LOWER : NoteCaseType::UPPER);
        setAndStyleProperty(h, Pid::PLAY, config->chordPlayback, true);
        setAndStyleProperty(h, Pid::FONT_STYLE, int(f), true);
        setAndStyleProperty(h, Pid::FONT_SIZE, harmonyFont->fontSize * doubleFromPercent(chordAssignment->chPercent), true);
        setAndStyleProperty(h, Pid::FONT_FACE, fontFamily, true);

        const MusxInstance<others::StaffComposite> musxStaff = others::StaffComposite::createCurrent(m_doc, m_currentMusxPartId, musxScrollViewItem->staffId, musxMeasure->getCmper(), chordAssignment->horzEdu);
        const double staffReferenceOffset = musxStaff->calcTopLinePosition() * 0.5 * staff->spatium(s->tick()) * staff->staffType(s->tick())->lineDistance().val();
        const double baselinepos = doubleFromEvpu(musxStaff->calcBaselinePosition<details::BaselineChords>(0)) * SPATIUM20; // Needs to be scaled correctly (offset topline/reference pos)?
        PointF offset = evpuToPointF(chordAssignment->horzOff, -chordAssignment->vertOff) * SPATIUM20;
        offset.ry() -= (baselinepos - staffReferenceOffset); /// @todo set this as style?
        if (fretVisible) {
            const double fbBaselinepos = doubleFromEvpu(musxStaff->calcBaselinePosition<details::BaselineFretboards>(0)) * SPATIUM20; // Needs to be scaled correctly (offset topline/reference pos)?
            PointF fbOffset = evpuToPointF(chordAssignment->fbHorzOff, -chordAssignment->fbVertOff) * SPATIUM20;
            fbOffset.ry() -= (fbBaselinepos - staffReferenceOffset); /// @todo set this as style?
            offset.ry() -= fbOffset.y(); /// @todo also diagram height?
            setAndStyleProperty(fret, Pid::OFFSET, fbOffset, true);
        }
        setAndStyleProperty(h, Pid::OFFSET, offset, true);
        if (fretVisible) {
            s->add(fret);
        } else {
            s->add(h);
        }
        /// @todo Pid::HARMONY_VOICE_LITERAL, Pid::HARMONY_VOICING, Pid::HARMONY_DURATION, Pid::HARMONY_DO_NOT_STACK_MODIFIERS
    }
}

}
