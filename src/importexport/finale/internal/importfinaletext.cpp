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

#include "engraving/dom/barline.h"
#include "engraving/dom/box.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/dynamic.h"
#include "engraving/dom/excerpt.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/fret.h"
#include "engraving/dom/harmony.h"
#include "engraving/dom/harppedaldiagram.h"
#include "engraving/dom/jump.h"
#include "engraving/dom/layoutbreak.h"
#include "engraving/dom/lyrics.h"
#include "engraving/dom/marker.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/measurenumber.h"
#include "engraving/dom/note.h"
#include "engraving/dom/page.h"
#include "engraving/dom/pitchspelling.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/score.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/sig.h"
#include "engraving/dom/spacer.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stafftext.h"
#include "engraving/dom/stafftextbase.h"
#include "engraving/dom/system.h"
#include "engraving/dom/tempotext.h"
#include "engraving/dom/utils.h"

#include "engraving/rendering/score/stemlayout.h"

#include "engraving/types/symnames.h"
#include "engraving/types/types.h"
#include "engraving/types/typesconv.h"

#include "log.h"

using namespace mu::engraving;
using namespace muse;
using namespace musx::dom;

namespace mu::iex::finale {
FrameSettings::FrameSettings(const others::Enclosure* enclosure)
{
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
    /// @todo better approximation. Finale's corner radius values do not convert.
    frameRound   = enclosure->roundCorners ? 100 : 0;
}

FrameSettings::FrameSettings(const others::TextBlock* textBlock)
{
    if (!textBlock || textBlock->shapeId || textBlock->width) {
        // for now, we only support standard frames with no hard horizontal boundary
        return;
    }
    if (textBlock->stdLineThickness && textBlock->showShape) {
        frameType = FrameType::SQUARE;
        frameWidth = doubleFromEfix(textBlock->stdLineThickness);
        paddingWidth = doubleFromEfix(textBlock->inset) + 0.5; // fudge factor to ameliorate vertical discrepancy with Finale
        /// @todo better approximation. Finale's corner radius values do not convert.
        frameRound = textBlock->roundCorners ? 100 : 0;
    }
}

void FrameSettings::setFrameProperties(TextBase* item) const
{
    setAndStyleProperty(item, Pid::FRAME_TYPE, int(frameType));
    if (item->frameType() != FrameType::NO_FRAME) {
        setAndStyleProperty(item, Pid::FRAME_WIDTH, absoluteSpatium(frameWidth, item)); // is this the correct scaling?
        setAndStyleProperty(item, Pid::FRAME_PADDING, absoluteSpatium(paddingWidth, item)); // is this the correct scaling?
        setAndStyleProperty(item, Pid::FRAME_ROUND, frameRound);
    }
}

double FrameSettings::oneSidePaddingWidth() const
{
    if (frameType == FrameType::SQUARE) {
        return frameWidth + paddingWidth;
    }
    return 0.0;
}

FontTracker::FontTracker(const MusxInstance<FontInfo>& fontInfo, double referenceSpatium)
{
    fontName = String::fromStdString(fontInfo->getName());
    fontSize = double(fontInfo->fontSize);
    symbolsSize = double(fontInfo->fontSize);
    fontStyle = FinaleTextConv::museFontEfx(fontInfo);
    spatiumDependent = !fontInfo->absolute;
    if (spatiumDependent) {
        fontSize *= referenceSpatium / FINALE_DEFAULT_SPATIUM;
        symbolsSize *= referenceSpatium / FINALE_DEFAULT_SPATIUM;
    }
}

FontTracker::FontTracker(const MStyle& style, const String& sidNamePrefix)
{
    fontName = style.styleSt(MStyle::styleIdx(sidNamePrefix + u"FontFace"));
    fontSize = style.styleD(MStyle::styleIdx(sidNamePrefix + u"FontSize"));
    fontStyle = FontStyle(style.styleI(MStyle::styleIdx(sidNamePrefix + u"FontStyle")));
    spatiumDependent = style.styleB(MStyle::styleIdx(sidNamePrefix + u"FontSpatiumDependent"));
    if (spatiumDependent) {
        fontSize *= style.defaultSpatium() / FINALE_DEFAULT_SPATIUM;
    }
}

muse::draw::FontMetrics FontTracker::toFontMetrics(double mag)
{
    muse::draw::Font f(fontName, muse::draw::Font::Type::Unknown);
    f.setBold(fontStyle & FontStyle::Bold);
    f.setItalic(fontStyle & FontStyle::Italic);
    f.setUnderline(fontStyle & FontStyle::Underline);
    f.setStrike(fontStyle & FontStyle::Strike);
    f.setPointSizeF(fontSize * mag);
    return muse::draw::FontMetrics(f);
}

void FontTracker::setFontProperties(TextBase* item) const
{
    setAndStyleProperty(item, Pid::FONT_FACE, fontName, true);
    setAndStyleProperty(item, Pid::FONT_STYLE, int(fontStyle), true);
    setAndStyleProperty(item, Pid::SIZE_SPATIUM_DEPENDENT, spatiumDependent, true);
    setAndStyleProperty(item, Pid::FONT_SIZE, fontSize, true);
    if (symbolsSize > 0.0) {
        if (item->hasSymbolScale()) {
            setAndStyleProperty(item, Pid::MUSICAL_SYMBOLS_SCALE, symbolsSize / 20.0, true);
        } else if (item->hasSymbolSize()) {
            setAndStyleProperty(item, Pid::MUSIC_SYMBOL_SIZE, symbolsSize, true);
        }
    }
}

// Passing in the firstFontInfo pointer suppresses any first font information from being generated in the output string.
// Instead, it is returned in the pointer.
String FinaleParser::stringFromEnigmaText(const musx::util::EnigmaParsingContext& parsingContext,
                                          const EnigmaParsingOptions& options, FontTracker* firstFontInfo) const
{
    String endString;
    const bool isHeaderOrFooter = options.hfType != HeaderFooterType::None;
    std::optional<FontTracker> prevFont = options.initialFont;
    std::unordered_set<FontStyle> emittedOpenTags; // tracks whose open tags we actually emitted here
    FontStyle flagsThatAreStillOpen = FontStyle::Normal; // any flags we've opened that need to be closed.
    const bool canConvertSymbols = options.convertSymbols;
    /// @note Finale's text is scaled relative to its default spatium, noticeably larger than MuseScore's.
    /// To account for this, we need to scale non-absolute text accordingly.
    /// @todo verify correct spatium is used (staff level, score level, default)
    double scaling = options.referenceSpatium.value_or(m_score->style().defaultSpatium());

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
        String symIds = canConvertSymbols ? FinaleTextConv::symIdInsertsFromStdString(nextChunk, styles.font) : String();
        bool importAsSymbols = !symIds.empty();

        const FontTracker font(styles.font, scaling);
        if (firstFontInfo && !prevFont) {
            *firstFontInfo = font;
        } else if (!options.plainText) {
            if (importAsSymbols) {
                if (firstFontInfo) {
                    firstFontInfo->symbolsSize = std::max(firstFontInfo->symbolsSize, font.fontSize);
                }
            } else {
                if (!prevFont || prevFont->fontName != font.fontName) {
                    endString.append(String(u"<font face=\"" + font.fontName + u"\"/>"));
                }
                if (!prevFont || prevFont->fontSize != font.fontSize) {
                    endString.append(String(u"<font size=\""));
                    endString.append(String::number(font.fontSize, 2) + String(u"\"/>"));
                }
                if (!prevFont || prevFont->fontStyle != font.fontStyle) {
                    updateFontStyles(font, prevFont);
                }
            }
        }
        prevFont = font;

        if (importAsSymbols) {
            endString.append(symIds);
        } else {
            String convertedChunk = String::fromStdString(nextChunk);
            if (isHeaderOrFooter && convertedChunk == u"$") {
                endString.append(convertedChunk);
            }
            endString.append(convertedChunk); // do not use plainToXmlText: it turns all the xml tags into literals.
        }
        return true;
    };

    // The processCommand function sends back to the parser a subsitution string for the musx::util::Enigma command.
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
        } else if (parsedCommand[0] == "totpages") {
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
        } else if (parsedCommand[0] == "perftime") {
            /// @todo: honor format code (see class comments for musx::util::EnigmaString)
            /// Note that Finale's UI does not support any format but m'ss", but plugins could have inserted other formats.
            int rawDurationSeconds = m_score->duration();
            int minutes = rawDurationSeconds / 60;
            int seconds = rawDurationSeconds % 60;
            std::ostringstream oss;
            oss << minutes << '\'' << std::setw(2) << std::setfill('0') << seconds << '"';
            return oss.str();
        } else if (parsedCommand[0] == "copyright") {
            /// @todo maybe not use $C/$c at all in favor of $:copyright:.?
            /// XM: It's common to only show a footer on the first page. This is only attainable with $C.
            switch (options.hfType) {
            default:
            case HeaderFooterType::None: return m_score->metaTag(u"copyright").toStdString();
            case HeaderFooterType::FirstPage: return "$C";
            case HeaderFooterType::SecondPageToEnd: return "$c";
            }
        } else if (std::optional<options::AccidentalInsertSymbolType> acciInsertType
                       = musx::util::EnigmaString::commandIsAccidentalType(parsedCommand[0])) {
            if (const auto& acciData = muse::value(musxOptions().textOptions->symbolInserts, acciInsertType.value(), nullptr)) {
                String accSym = FinaleTextConv::symIdInsertFromFinaleChar(acciData->symChar, acciData->symFont);
                if (!accSym.empty()) {
                    return accSym.toStdString();
                }
            }
            // since we could not map the character to a symbol, let it fall thru and parse as glyphs
        } else if (String metaTag = metaTagFromTextComponent(parsedCommand[0]); !metaTag.isEmpty()) {
            // Find and insert metaTags when appropriate
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
}

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

ReadableExpression::ReadableExpression(const FinaleParser& context, const MusxInstance<others::TextExpressionDef>& textExpression)
{
    // Text
    others::MarkingCategory::CategoryType categoryType = others::MarkingCategory::CategoryType::Misc;
    MusxInstance<FontInfo> catMusicFont;
    if (MusxInstance<others::MarkingCategory> category
            = context.musxDocument()->getOthers()->get<others::MarkingCategory>(context.currentMusxPartId(), textExpression->categoryId)) {
        categoryType = category->categoryType;
        catMusicFont = category->musicFont;
    }
    EnigmaParsingOptions options;
    musx::util::EnigmaParsingContext parsingContext = textExpression->getRawTextCtx(context.currentMusxPartId());
    options.convertSymbols = !catMusicFont || catMusicFont->calcIsDefaultMusic()
                             || catMusicFont->getName() == context.musxOptions().calculatedEngravingFontName.toStdString();
    xmlText = context.stringFromEnigmaText(parsingContext, options, &startingFont);

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
        options.convertSymbols = true;
        // Dynamics (adapted from engraving/dom/dynamic.cpp)
        String plainExprText = context.stringFromEnigmaText(parsingContext, options);

        if (plainExprText.contains(u"dynamicMF")) {
            dynamicType = DynamicType::MF;
            return ElementType::DYNAMIC;
        }
        if (plainExprText.contains(u"dynamicMP")) {
            dynamicType = DynamicType::MP;
            return ElementType::DYNAMIC;
        }

        std::string utf8Tag = plainExprText.toStdString();

        auto begin = std::sregex_iterator(utf8Tag.begin(), utf8Tag.end(), dynamicRegex);
        for (auto it = begin; it != std::sregex_iterator(); ++it) {
            const std::smatch match = *it;
            const std::string matchStr = match.str();
            for (auto dyn : Dynamic::dynamicList()) {
                if (TConv::toXml(dyn.type).ascii() == matchStr || dyn.text == matchStr) {
                    utf8Tag.replace(match.position(0), match.length(0), dyn.text);
                    // xmlText = String::fromStdString(utf8Tag); // do we want this?
                    dynamicType = dyn.type;
                    return ElementType::DYNAMIC;
                }
            }
        }

        // Harp pedal diagrams (using symbols)
        if (std::regex_search(utf8Tag, hpdDetectRegex)) {
            std::optional<std::array<PedalPosition, HARP_STRING_NO> > maybePedalState = parseHarpPedalDiagram(utf8Tag);
            if (maybePedalState.has_value()) {
                pedalState = maybePedalState.value();
                return ElementType::HARP_DIAGRAM;
            }
            context.logger()->logWarning(String(u"Cannot create harp pedal diagram!"));
        }

        return elementTypeFromMarkingCategory(categoryType);
    }();

    context.logger()->logInfo(String(u"Converted expression of %1 type").arg(TConv::userName(elementType).translated()));
}

static String textFromRepeatDef(const MusxInstance<others::TextRepeatDef>& repeatDef, const FontTracker font)
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
    const MusxInstance<others::TextRepeatText> repeatText = repeatDef->getDocument()->getOthers()->get<others::TextRepeatText>(
        repeatDef->getSourcePartId(), repeatDef->getCmper());
    text.append(String::fromStdString(repeatText->text));
    return text;
}

ReadableRepeatText::ReadableRepeatText(const FinaleParser& context, const MusxInstance<others::TextRepeatDef>& repeatDef)
{
    xmlText = textFromRepeatDef(repeatDef, FontTracker(repeatDef->font, context.score()->style().defaultSpatium()));

    // Text frame/border (Finale: Enclosure)
    frameSettings
        = FrameSettings(repeatDef->hasEnclosure ? context.musxDocument()->getOthers()->get<others::TextRepeatEnclosure>(
                            context.currentMusxPartId(), repeatDef->getCmper()).get() : nullptr);

    // Text justification and alignment
    repeatAlignment = toAlignH(repeatDef->justification);

    // Element type and element-based options
    // Replace symbols with text
    String tempText = xmlText;
    tempText.replace(u"<sym>segno</sym>", u"Segno");
    tempText.replace(u"<sym>segnoSerpent1</sym>", u"Segno");
    tempText.replace(u"<sym>segnoSerpent2</sym>", u"Segno");
    tempText.replace(String::fromUcs4(0xF404u), u"Segno"); // Japanese-style Segno
    tempText.replace(String::fromUcs4(0xF405u), u"Coda"); // Japanese-style Coda
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
    const auto expressionAssignments = m_doc->getOthers()->getArray<others::MeasureExprAssign>(m_currentMusxPartId);
    std::vector<std::pair<Cmper, Inci> > parsedAssignments;
    parsedAssignments.reserve(expressionAssignments.size());
    logger()->logInfo(String(u"Import text expressions: Found %1 expressions.").arg(expressionAssignments.size()));
    const MusxInstanceList<others::StaffUsed> musxScrollView = m_doc->getScrollViewStaves(m_currentMusxPartId);
    const MusxInstanceList<others::Measure> musxMeasures = m_doc->getOthers()->getArray<others::Measure>(m_currentMusxPartId);
    for (const MusxInstance<others::Measure>& musxMeasure : musxMeasures) {
        Fraction mTick = muse::value(m_meas2Tick, musxMeasure->getCmper(), Fraction(-1, 1));
        Measure* measure = !mTick.negative() ? m_score->tick2measure(mTick) : nullptr;
        IF_ASSERT_FAILED(measure) {
            logger()->logWarning(String(u"Unable to retrieve measure by tick"), m_doc, 0, musxMeasure->getCmper());
            continue;
        }

        // Text expressions
        const auto exprsInMeasure = m_doc->getOthers()->getArray<others::MeasureExprAssign>(m_currentMusxPartId, musxMeasure->getCmper());
        for (const auto& expressionAssignment : exprsInMeasure) {
            if (!expressionAssignment->calcIsAssignedInRequestedPart()) {
                continue;
            }
            if (expressionAssignment->calcIsHiddenByAlternateNotation()) {
                /// @todo Expressions hidden by alt notation are primarily cue names, but we may need to get smarter for other edge cases
                /// @todo Revisit this when we know how we are importing cues
                continue;
            }
            if (!expressionAssignment->textExprId) {
                // Shapes are currently unsupported
                continue;
            }

            // Already added as system clone
            std::pair<Cmper, Inci> expressionId = std::make_pair(expressionAssignment->getCmper(),
                                                                 expressionAssignment->getInci().value_or(0));
            if (muse::contains(parsedAssignments, expressionId)) {
                continue;
            }

            // Search our converted expression library, or if not found add to it
            ReadableExpression* expression = muse::value(m_expressions, expressionAssignment->textExprId, nullptr); /// @todo does this code work for part scores?
            if (!expression) {
                expression = new ReadableExpression(*this, m_doc->getOthers()->get<others::TextExpressionDef>(
                                                        m_currentMusxPartId, expressionAssignment->textExprId));
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

            ElementType elementType = expression->elementType == ElementType::STAFF_TEXT
                                      && expressionAssignment->staffAssign < 0 ? ElementType::SYSTEM_TEXT : expression->elementType;

            const bool appliesToSingleVoice = expressionAssignment->layer > 0;
            track_idx_t curTrackIdx = staff2track(curStaffIdx);
            if (appliesToSingleVoice) {
                curTrackIdx += static_cast<voice_idx_t>(std::clamp(expressionAssignment->layer - 1, 0, int(VOICES) - 1));
            }
            Fraction rTick = eduToFraction(expressionAssignment->eduPosition);
            Segment* s = measure->getChordRestOrTimeTickSegment(measure->tick() + rTick);

            // Create item
            logger()->logInfo(String(u"Creating a %1 at tick %2 on track %3.").arg(TConv::userName(elementType).translated(),
                                                                                   s->tick().toString(), String::number(curTrackIdx)));
            TextBase* item = toTextBase(Factory::createItem(elementType, s));
            const MusxInstance<others::TextExpressionDef>& expressionDef = expressionAssignment->getTextExpression();
            item->setParent(s);
            item->setTrack(curTrackIdx);
            item->setVisible(!expressionAssignment->hidden); /// @todo staff visibility, and save adding excessive links
            item->setXmlText(expression->xmlText);
            expression->startingFont.setFontProperties(item);
            item->checkCustomFormatting(expression->xmlText);
            expression->frameSettings.setFrameProperties(item);
            if (importCustomPositions()) {
                setAndStyleProperty(item, Pid::POSITION, toAlignH(expressionDef->horzExprJustification));
            }
            s->add(item);

            // Set element-specific properties
            switch (elementType) {
            case ElementType::DYNAMIC: {
                Dynamic* dynamic = toDynamic(item);
                dynamic->setDynamicType(expression->dynamicType);
                // Don't set these as styles, so new dynamics have nicer behaviour
                if (importCustomPositions()) {
                    setAndStyleProperty(dynamic, Pid::CENTER_BETWEEN_STAVES, AutoOnOff::OFF);
                    setAndStyleProperty(dynamic, Pid::CENTER_ON_NOTEHEAD, false);
                }
                if (appliesToSingleVoice) {
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
                    tt->setFollowText(false);     /// @todo detect this
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
            auto positionExpression = [&](TextBase* expr, const MusxInstance<others::MeasureExprAssign> exprAssign) {
                if (!importCustomPositions()) {
                    return;
                }
                expr->setAutoplace(false);
                setAndStyleProperty(expr, Pid::PLACEMENT, PlacementV::ABOVE);
                setAndStyleProperty(expr, Pid::OFFSET, PointF());
                m_score->renderer()->layoutItem(expr);
                PointF p;
                switch (expressionDef->horzMeasExprAlign) {
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
                            if (expressionDef->horzMeasExprAlign == others::HorizontalMeasExprAlign::CenterPrimaryNotehead) {
                                p.rx() += n->noteheadCenterX();
                            }
                        } else {
                            Rest* rest = toRest(seg->element(expr->track()));
                            if (rest->isFullMeasureRest()) {
                                p.rx() = seg->pageX();
                            } else {
                                p.rx() = rest->pageX();
                                if (expressionDef->horzMeasExprAlign == others::HorizontalMeasExprAlign::CenterPrimaryNotehead) {
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
                            if (expressionDef->horzMeasExprAlign == others::HorizontalMeasExprAlign::Stem) {
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
                    p.rx() = staffShape.right() / 2 + s->pageX();
                    break;
                }
                case others::HorizontalMeasExprAlign::RightOfAllNoteheads: {
                    Shape staffShape = s->staffShape(expr->staffIdx());
                    staffShape.remove_if([](ShapeElement& el) { return el.height() == 0; });
                    p.rx() = staffShape.right() + s->pageX();
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
                p.rx() += absoluteDoubleFromEvpu(expressionDef->measXAdjust, expr);

                const MusxInstance<others::StaffComposite> musxStaff = exprAssign->createCurrentStaff();
                const Staff* staff = m_score->staff(expr->staffIdx());
                const double staffReferenceOffset = musxStaff->calcTopLinePosition() * 0.5 * staff->spatium(s->tick())
                                                    * staff->staffType(s->tick())->lineDistance().val();

                switch (expressionDef->vertMeasExprAlign) {
                case others::VerticalMeasExprAlign::AboveStaff: {
                    expr->setPlacement(PlacementV::ABOVE);
                    p.ry() = expr->pagePos().y() - scaledDoubleFromEvpu(expressionDef->yAdjustBaseline, expr);

                    SystemCmper sc = m_doc->calcSystemFromMeasure(m_currentMusxPartId, exprAssign->getCmper())->getCmper();
                    double baselinepos = scaledDoubleFromEvpu(musxStaff->calcBaselinePosition<details::BaselineExpressionsAbove>(sc), expr);     // Needs to be scaled correctly (offset topline/reference pos)?
                    p.ry() -= (baselinepos - staffReferenceOffset);
                    break;
                }
                case others::VerticalMeasExprAlign::Manual: {
                    expr->setPlacement(PlacementV::ABOVE);     // Finale default
                    p.ry() = expr->pagePos().y();
                    // Add staffreferenceoffset?
                    break;
                }
                case others::VerticalMeasExprAlign::RefLine: {
                    expr->setPlacement(PlacementV::ABOVE);
                    p.ry() = expr->pagePos().y() - staffReferenceOffset - scaledDoubleFromEvpu(expressionDef->yAdjustBaseline, expr);
                    break;
                }
                case others::VerticalMeasExprAlign::BelowStaff: {
                    expr->setPlacement(PlacementV::BELOW);
                    p.ry() = expr->pagePos().y() - scaledDoubleFromEvpu(expressionDef->yAdjustBaseline, expr);

                    SystemCmper sc = m_doc->calcSystemFromMeasure(m_currentMusxPartId, exprAssign->getCmper())->getCmper();
                    double baselinepos = scaledDoubleFromEvpu(musxStaff->calcBaselinePosition<details::BaselineExpressionsBelow>(sc), expr);     // Needs to be scaled correctly (offset topline/reference pos)?
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
                    } else {
                        // Sensible fallback
                        p.ry() = expr->pagePos().y();
                    }
                    p.ry() -= scaledDoubleFromEvpu(expressionDef->yAdjustEntry, expr);
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
                    } else {
                        // Sensible fallback
                        p.ry() = expr->pagePos().y();
                    }
                    p.ry() -= scaledDoubleFromEvpu(expressionDef->yAdjustEntry, expr);
                    break;
                }
                case others::VerticalMeasExprAlign::AboveEntry:
                case others::VerticalMeasExprAlign::AboveStaffOrEntry: {
                    expr->setPlacement(PlacementV::ABOVE);
                    Segment* seg = measure->findSegmentR(SegmentType::ChordRest, rTick);     // why is this needed

                    Shape staffShape = seg->staffShape(expr->staffIdx());
                    staffShape.removeTypes({ ElementType::FERMATA, ElementType::ARTICULATION, ElementType::ARPEGGIO });
                    staffShape.remove_if([expr](ShapeElement& shapeEl) {
                            return !shapeEl.item() || shapeEl.item()->voice() != expr->voice();
                        });
                    double entryY = expr->pagePos().y() + staffShape.top() - scaledDoubleFromEvpu(expressionDef->yAdjustEntry, expr);

                    SystemCmper sc = m_doc->calcSystemFromMeasure(m_currentMusxPartId, exprAssign->getCmper())->getCmper();
                    double baselinepos = scaledDoubleFromEvpu(musxStaff->calcBaselinePosition<details::BaselineExpressionsAbove>(sc), expr);     // Needs to be scaled correctly (offset topline/reference pos)?
                    baselinepos = expr->pagePos().y() - (baselinepos - staffReferenceOffset)
                                  - scaledDoubleFromEvpu(expressionDef->yAdjustBaseline, expr);
                    p.ry() = std::min(baselinepos, entryY);
                    break;
                }
                case others::VerticalMeasExprAlign::BelowEntry:
                case others::VerticalMeasExprAlign::BelowStaffOrEntry: {
                    expr->setPlacement(PlacementV::BELOW);

                    Shape staffShape = s->staffShape(expr->staffIdx());
                    staffShape.removeTypes({ ElementType::FERMATA, ElementType::ARTICULATION, ElementType::ARPEGGIO });
                    staffShape.remove_if([expr](ShapeElement& shapeEl) {
                            return !shapeEl.item() || shapeEl.item()->voice() != expr->voice();
                        });
                    double entryY = expr->pagePos().y() + staffShape.bottom() - scaledDoubleFromEvpu(expressionDef->yAdjustEntry, expr);

                    SystemCmper sc = m_doc->calcSystemFromMeasure(m_currentMusxPartId, exprAssign->getCmper())->getCmper();
                    double baselinepos = scaledDoubleFromEvpu(musxStaff->calcBaselinePosition<details::BaselineExpressionsBelow>(sc), expr);     // Needs to be scaled correctly (offset topline/reference pos)?
                    baselinepos = expr->pagePos().y() - (baselinepos - staffReferenceOffset)
                                  - scaledDoubleFromEvpu(expressionDef->yAdjustBaseline, expr);
                    p.ry() = std::max(baselinepos, entryY);
                    break;
                }
                default: {
                    expr->setPlacement(PlacementV::ABOVE);     // Finale default
                    p.ry() = expr->pagePos().y() - scaledDoubleFromEvpu(expressionDef->yAdjustEntry, expr);
                    break;
                }
                }
                p -= expr->pagePos();
                p += evpuToPointF(exprAssign->horzEvpuOff, -exprAssign->vertEvpuOff) * expr->spatium(); // assignment offset
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
                    setAndStyleProperty(expr, Pid::DIRECTION, expr->placeAbove() ? DirectionV::UP : DirectionV::DOWN, true);
                }
                setAndStyleProperty(expr, Pid::OFFSET, p);
            };
            positionExpression(item, expressionAssignment);
            collectElementStyle(item);

            auto resizeExpressionIfNeeded = [&](TextBase* expr, const MusxInstance<others::MeasureExprAssign> exprAssign) {
                if (!exprAssign->dontScaleWithEntry) {
                    Segment* crSeg = measure->findSegmentR(SegmentType::ChordRest, s->rtick());
                    if (crSeg && crSeg->element(expr->track())) {
                        ChordRest* scaleCR = toChordRest(crSeg->element(expr->track()));
                        if (exprAssign->graceNoteIndex && scaleCR->isChord()) {
                            if (Chord* gc = toChord(scaleCR)->graceNoteAt(static_cast<size_t>(exprAssign->graceNoteIndex - 1))) {
                                scaleCR = gc;
                            }
                        }
                        if (scaleCR->isSmall()) {
                            setAndStyleProperty(expr, Pid::FONT_SIZE,
                                                expr->getProperty(Pid::FONT_SIZE).toDouble() * m_score->style().styleD(Sid::smallNoteMag));
                        }
                    }
                }
            };

            if (item->systemFlag()) {
                m_systemObjectStaves.insert(item->staffIdx());
                parsedAssignments.push_back(expressionId);
                if (!expressionAssignment->staffList) {
                    continue;
                }
                /// @todo improved handling for bottom system objects
                for (const auto& linkedAssignment : exprsInMeasure) {
                    if (linkedAssignment->staffGroup != expressionAssignment->staffGroup // checking staffGroup by itself is probably sufficient.
                        || linkedAssignment->textExprId != expressionAssignment->textExprId
                        || !linkedAssignment->calcIsAssignedInRequestedPart()) {
                        continue;
                    }
                    staff_idx_t linkedStaffIdx = staffIdxFromAssignment(linkedAssignment->staffAssign);
                    std::pair<Cmper, Inci> linkedExpressionId = std::make_pair(linkedAssignment->getCmper(),
                                                                               linkedAssignment->getInci().value_or(0));
                    if (muse::contains(parsedAssignments, linkedExpressionId) || linkedStaffIdx == muse::nidx) {
                        continue;
                    }

                    TextBase* copy = toTextBase(item->clone());
                    copy->setVisible(!linkedAssignment->hidden);
                    copy->setStaffIdx(linkedStaffIdx);
                    if (importCustomPositions()) {
                        setAndStyleProperty(copy, Pid::POSITION, toAlignH(expressionDef->horzExprJustification));
                    }
                    copy->linkTo(item);
                    s->add(copy);
                    positionExpression(copy, linkedAssignment);
                    collectElementStyle(copy);
                    resizeExpressionIfNeeded(copy, linkedAssignment);
                    m_systemObjectStaves.insert(linkedStaffIdx);
                    parsedAssignments.push_back(linkedExpressionId);
                }
            }

            // After linking
            resizeExpressionIfNeeded(item, expressionAssignment);

            /// @todo use expressionAssignment->showStaffList to control sharing between score/parts. some elements can be hidden entirely, others will be made invisible
        }

        // Measure-anchored text (MeasureTextAssign)
        for (const MusxInstance<others::StaffUsed>& musxScrollViewItem : musxScrollView) {
            staff_idx_t curStaffIdx = muse::value(m_inst2Staff, musxScrollViewItem->staffId, muse::nidx);
            IF_ASSERT_FAILED(curStaffIdx != muse::nidx) {
                logger()->logWarning(String(u"MeasureTextAssign: Musx inst value not found for staff cmper %1"),
                                     m_doc, musxScrollViewItem->staffId);
                continue;
            }
            track_idx_t curTrackIdx = staff2track(curStaffIdx);

            for (const auto& measureTextAssign :
                 m_doc->getDetails()->getArray<details::MeasureTextAssign>(m_currentMusxPartId, musxScrollViewItem->staffId,
                                                                           musxMeasure->getCmper())) {
                Fraction rTick = eduToFraction(measureTextAssign->xDispEdu);
                Segment* s = measure->getChordRestOrTimeTickSegment(measure->tick() + rTick);

                StaffText* text = Factory::createStaffText(s);
                text->setTrack(curTrackIdx);
                EnigmaParsingOptions options;
                FontTracker firstFontInfo;
                text->setXmlText(stringFromEnigmaText(measureTextAssign->getRawTextCtx(m_currentMusxPartId), options, &firstFontInfo));
                text->checkCustomFormatting(text->xmlText());
                if (text->plainText().empty()) {
                    delete text;
                    continue;
                }
                text->setVisible(!measureTextAssign->hidden);
                setAndStyleProperty(text, Pid::SIZE_SPATIUM_DEPENDENT, false);
                text->setAutoplace(false);
                PointF p = evpuToPointF(rTick.isZero() ? measureTextAssign->xDispEvpu : 0, -measureTextAssign->yDisp);
                setAndStyleProperty(text, Pid::OFFSET, p * text->defaultSpatium(), true);
                /// @todo Account for Finale's weird handle placement. Anyhow, the following line gets us close. Measure Text is always aligned as follows.
                /// It can have different justification (i.e., a multiline centered text) but the handle is always left aligned.
                setAndStyleProperty(text, Pid::ALIGN, Align(AlignH::LEFT, AlignV::TOP));
                FrameSettings frameSettings(measureTextAssign->getTextBlock().get());
                frameSettings.setFrameProperties(text);
                s->add(text);
                collectElementStyle(text);
            }
        }

        // Repeat markings (markers and jumps)
        const auto textRepeatAssignments = m_doc->getOthers()->getArray<others::TextRepeatAssign>(m_currentMusxPartId,
                                                                                                  musxMeasure->getCmper());
        for (const auto& repeatAssignment : textRepeatAssignments) {
            // Search our converted repeat text library, or if not found add to it
            ReadableRepeatText* repeatText = muse::value(m_repeatTexts, repeatAssignment->textRepeatId, nullptr); /// @todo does this code work for part scores?
            const auto repeatDefinition = m_doc->getOthers()->get<others::TextRepeatDef>(m_currentMusxPartId,
                                                                                         repeatAssignment->textRepeatId);
            if (!repeatText) {
                repeatText = new ReadableRepeatText(*this, repeatDefinition);
                m_repeatTexts.emplace(repeatAssignment->textRepeatId, repeatText);
            }

            if (repeatText->xmlText.empty()) {
                continue;
            }

            // Find staff
            std::vector<std::pair<staff_idx_t, StaffCmper> > links;
            staff_idx_t curStaffIdx = staffIdxForRepeats(repeatAssignment->topStaffOnly, repeatAssignment->staffList,
                                                         repeatAssignment->getCmper(), links);

            if (curStaffIdx == muse::nidx) {
                logger()->logWarning(String(u"Add repeat text: Musx inst value not found."));
                continue;
            }

            track_idx_t curTrackIdx = staff2track(curStaffIdx);
            logger()->logInfo(String(u"Creating a %1 at tick %2 on track %3.").arg(TConv::userName(repeatText->elementType).translated(),
                                                                                   measure->tick().toString(),
                                                                                   String::number(curTrackIdx)));
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
                if (const auto targetRepeat = m_doc->getOthers()->get<others::TextRepeatDef>(m_currentMusxPartId,
                                                                                             repeatAssignment->targetValue)) {
                    FontTracker font = FontTracker(repeatDefinition->useThisFont ? repeatDefinition->font : targetRepeat->font,
                                                   score()->style().defaultSpatium());
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
            if (importCustomPositions()) {
                setAndStyleProperty(item, Pid::POSITION, repeatText->repeatAlignment);
                item->setAutoplace(false);
                setAndStyleProperty(item, Pid::PLACEMENT, PlacementV::ABOVE);
            }
            repeatText->frameSettings.setFrameProperties(item);
            PointF p = evpuToPointF(repeatAssignment->horzPos, -repeatAssignment->vertPos) * item->defaultSpatium(); /// @todo adjust for staff reference line?

            auto repositionRepeatMarking = [&](TextBase* repeatMarking, PointF point) {
                if (!importCustomPositions()) {
                    return;
                }
                // 'center' position centers over barline in MuseScore, over measure in Finale
                /// @todo this calculation doesn't hold up well (different measure widths) and should be reconsidered
                if (Segment* endBlSeg = measure->findSegmentR(SegmentType::EndBarLine, measure->ticks())) {
                    point.rx() -= endBlSeg->width();
                    if (repeatMarking->position() == AlignH::LEFT) {
                        point.rx() -= endBlSeg->x();
                    } else if (repeatMarking->position() == AlignH::HCENTER) {
                        point.rx() -= endBlSeg->x() * .5;
                    }
                }

                if (point.y() > repeatMarking->staff()->staffHeight(measure->tick()) / 2) {
                    setAndStyleProperty(repeatMarking, Pid::PLACEMENT, PlacementV::BELOW, true);
                    point.ry() -= repeatMarking->staff()->staffHeight(measure->tick());
                }
                setAndStyleProperty(repeatMarking, Pid::OFFSET, point);
            };
            repositionRepeatMarking(item, p);

            measure->add(item);
            collectElementStyle(item);
            m_systemObjectStaves.insert(curStaffIdx);

            for (auto [linkedStaffIdx, linkedMusxStaffId] : links) {
                /// @todo improved handling for bottom system objects
                TextBase* copy = toTextBase(item->clone());
                copy->setStaffIdx(linkedStaffIdx);
                const auto indiv = repeatAssignment->getIndividualPositioning(linkedMusxStaffId);
                if (repeatAssignment->individualPlacement && indiv) {
                    copy->setVisible(!indiv->hidden);
                    PointF p1 = evpuToPointF(indiv->x1add, -indiv->y1add) * copy->defaultSpatium(); /// @todo adjust for staff reference line?
                    repositionRepeatMarking(copy, p1);
                }
                copy->linkTo(item);
                measure->add(copy);
                collectElementStyle(copy);
                m_systemObjectStaves.insert(linkedStaffIdx);
            }
            /// @todo fine-tune playback
        }
    }

    // Lyrics
    const MusxInstanceList<others::StaffSystem> staffSystems = m_doc->getOthers()->getArray<others::StaffSystem>(m_currentMusxPartId);
    ChordRestNavigateOptions lyricNavigateOptions;
    lyricNavigateOptions.disableOverRepeats = true;
    lyricNavigateOptions.skipMeasureRepeatRests = false; /// ??
    for (const MusxInstance<others::StaffUsed>& musxScrollViewItem : musxScrollView) {
        for (const MusxInstance<others::StaffSystem>& staffSystem : staffSystems) {
            const MusxInstance<others::StaffComposite> musxStaff = others::StaffComposite::createCurrent(m_doc, m_currentMusxPartId,
                                                                                                         musxScrollViewItem->staffId,
                                                                                                         staffSystem->startMeas, 0);
            const std::vector<LyricsLineInfo> musxLyricsInfo = musxStaff->createLyricsLineInfo(staffSystem->getCmper());
            for (const LyricsLineInfo& musxLyricsList : musxLyricsInfo) {
                for (const MusxInstance<details::LyricAssign>& musxLyric : musxLyricsList.assignments) {
                    ChordRest* cr = muse::value(m_entryNumber2CR, musxLyric->getEntryNumber());

                    /// @todo handling for grace note lyrics?
                    if (!cr || cr->isGrace()) {
                        continue;
                    }
                    assert(musxLyric->syllable > 0);
                    size_t syllIndex = static_cast<size_t>(musxLyric->syllable - 1);
                    const std::shared_ptr<const LyricsSyllableInfo> syllInfo = musxLyric->getLyricText()->syllables.at(syllIndex);
                    if (!syllInfo) {
                        continue;
                    }
                    Lyrics* lyric = Factory::createLyrics(cr);
                    lyric->setTrack(cr->track());
                    lyric->setParent(cr);
                    lyric->setVerse(musxLyricsList.lyricNumber - 1);

                    // Text
                    String lyricText = String();
                    musxLyric->getLyricText()->iterateStylesForSyllable(syllIndex,
                                                                        [&](const std::string& chunk, const musx::util::EnigmaStyles& styles) -> bool {
                        const FontTracker font(
                            styles.font);
                        lyricText.append(String(u"<font face=\"" + font.fontName + u"\"/>"));
                        lyricText.append(String(u"<font size=\"") + String::number(font.fontSize, 2) + String(u"\"/>"));

                        for (const auto& [bit, tag] : fontStyleTags) {
                            if (font.fontStyle & bit) {
                                lyricText.append(String(u"<") + tag + u">");
                            }
                        }

                        lyricText.append(String::fromStdString(chunk));

                        for (auto it = fontStyleTags.rbegin(); it != fontStyleTags.rend(); ++it) {
                            if (font.fontStyle & it->first) {
                                lyricText.append(String(u"</") + it->second + u">");
                            }
                        }
                        return true;
                    });
                    lyric->setXmlText(lyricText);
                    if (lyric->plainText().empty()) {
                        delete lyric;
                        continue;
                    }
                    // Text alignment and position
                    if (const auto lyricInfo
                            = m_doc->getDetails()->get<details::LyricEntryInfo>(m_currentMusxPartId, musxLyric->getEntryNumber())) {
                        if (lyricInfo->align.has_value()) {
                            setAndStyleProperty(lyric, Pid::ALIGN, Align(toAlignH(lyricInfo->align.value()), lyric->align().vertical),
                                                true);
                        }
                        if (lyricInfo->justify.has_value()) {
                            setAndStyleProperty(lyric, Pid::POSITION, toAlignH(lyricInfo->justify.value()), true);
                        }
                    }

                    // Melisma and hyphenation
                    if (syllInfo->hasHyphenBefore && syllInfo->hasHyphenAfter) {
                        lyric->setSyllabic(LyricsSyllabic::MIDDLE);
                    } else if (syllInfo->hasHyphenAfter) {
                        lyric->setSyllabic(LyricsSyllabic::BEGIN);
                    } else if (syllInfo->hasHyphenBefore) {
                        lyric->setSyllabic(LyricsSyllabic::END);
                    }
                    if (EntryInfoPtr wordExtPtr = musxLyric->calcWordExtensionEndpoint()) {
                        if (ChordRest* extCR = chordRestFromEntryInfoPtr(wordExtPtr)) {
                            lyric->setTicks(extCR->tick() - cr->tick());
                        }
                    }

                    // Position
                    if (importCustomPositions()) {
                        const Staff* staff = cr->staff(); // or normal staff?
                        const double staffReferenceOffset = musxStaff->calcTopLinePosition() * 0.5 * staff->spatium(cr->tick())
                                                            * staff->staffType(cr->tick())->lineDistance().val();
                        const double baselinepos = scaledDoubleFromEvpu(musxLyricsList.baselinePosition, lyric); // Needs to be scaled correctly (offset topline/reference pos)?
                        double yPos = -(baselinepos - staffReferenceOffset) - scaledDoubleFromEvpu(musxLyric->vertOffset, lyric);
                        // MuseScore moves lyrics of cross-staff lyrics to the new staff, Finale does not.
                        double crossStaffOffset = 0.0;
                        SysStaff* ss = cr->measure()->system()->staff(cr->staffIdx());
                        if (ss->show()) {
                            SysStaff* ss2 = cr->measure()->system()->staff(cr->vStaffIdx());
                            if (ss2->show()) {
                                crossStaffOffset = ss2->y() - ss->y();
                            }
                            // We can't disable autoplace (needed for horizontal spacing),
                            // so to avoid further offset we set a negative minimum distance.
                            if (lyric->placeAbove()) {
                                double sp = -std::abs(ss->skyline().north().top()) - std::abs(crossStaffOffset);
                                setAndStyleProperty(lyric, Pid::MIN_DISTANCE, Spatium(sp / lyric->spatium()));
                            } else {
                                double sp = -std::abs(ss->skyline().south().bottom()) - std::abs(crossStaffOffset);
                                setAndStyleProperty(lyric, Pid::MIN_DISTANCE, Spatium(sp / lyric->spatium()));
                            }
                        }
                        if (lyric->placeBelow()) {
                            if (yPos < staff->staffHeight(cr->tick()) / 2) {
                                setAndStyleProperty(lyric, Pid::PLACEMENT, PlacementV::ABOVE, true);
                            } else {
                                yPos -= staff->staffHeight(cr->tick());
                            }
                        } else {
                            if (yPos > staff->staffHeight(cr->tick()) / 2) {
                                setAndStyleProperty(lyric, Pid::PLACEMENT, PlacementV::BELOW, true);
                                yPos -= staff->staffHeight(cr->tick());
                            }
                        }
                        setAndStyleProperty(lyric, Pid::OFFSET,
                                            PointF(scaledDoubleFromEvpu(musxLyric->horzOffset, lyric), yPos + crossStaffOffset));
                    }

                    if (musxLyric->displayVerseNum) {
                        /// @todo add text, use font metrics to calculate desired offset
                    }

                    cr->add(lyric);
                    collectElementStyle(lyric);
                }
            }
        }
    }
}

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

    const double fullWidth = bbox.width() + 2 * FrameSettings(pageTextAssign->getTextBlock().get()).oneSidePaddingWidth()
                             * page->defaultSpatium();
    if (pageTextAssign->indRpPos && !(page->no() & 1)) {
        switch (pageTextAssign->hPosRp) {
        case others::PageTextAssign::HorizontalAlignment::Left:
            break;
        case others::PageTextAssign::HorizontalAlignment::Center:
            p.rx() += (pageContentRect.width() - fullWidth) / 2;
            break;
        case others::PageTextAssign::HorizontalAlignment::Right:
            p.rx() += pageContentRect.width() - fullWidth;
            break;
        }
        p.rx() += absoluteDoubleFromEvpu(pageTextAssign->rightPgXDisp, page);
        p.ry() -= absoluteDoubleFromEvpu(pageTextAssign->rightPgYDisp, page);
    } else {
        switch (pageTextAssign->hPosLp) {
        case others::PageTextAssign::HorizontalAlignment::Left:
            break;
        case others::PageTextAssign::HorizontalAlignment::Center:
            p.rx() += (pageContentRect.width() - fullWidth) / 2;
            break;
        case others::PageTextAssign::HorizontalAlignment::Right:
            p.rx() += pageContentRect.width() - fullWidth;
            break;
        }
        p.rx() += absoluteDoubleFromEvpu(pageTextAssign->xDisp, page);
        p.ry() -= absoluteDoubleFromEvpu(pageTextAssign->yDisp, page);
    }
    return p;
}

/// @todo Instead of hard-coding page 1 and page 2, we need to find the first page in the Finale file with music on it
/// and use that as the first page. At least, that is my impression. How to handle blank pages in MuseScore is an open question.
/// - RGP

void FinaleParser::importPageTexts()
{
    const auto pageTextAssignList = m_doc->getOthers()->getArray<others::PageTextAssign>(m_currentMusxPartId);
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
        std::vector<MusxInstance<others::PageTextAssign> > firstPageTexts;
    };

    HeaderFooter header;
    HeaderFooter footer;
    std::vector<MusxInstance<others::PageTextAssign> > notHF;

    // gather texts by position
    for (const MusxInstance<others::PageTextAssign>& pageTextAssign : pageTextAssignList) {
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
            // there may be something we can do with hidden assignments created for Patterson's Copyist Helper plugin,
            // but generally it means the header is not applicable to this part.
            || pageTextAssign->hidden
            || startPage.value() >= 3 /// @todo must be changed to be first non-blank page + 2
            || endPage.value() < PageCmper(m_score->npages()) /// @todo allow copyright on just page 1?
            || !muse::RealIsNull(double(pageTextAssign->yDisp)) || !muse::RealIsNull(double(pageTextAssign->xDisp))
            || (pageTextAssign->indRpPos && (!muse::RealIsNull(double(pageTextAssign->rightPgXDisp))
                                             || !muse::RealIsNull(double(pageTextAssign->rightPgYDisp))))
            || pageTextAssign->hPosPageEdge || pageTextAssign->vPosPageEdge) {
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
                others::PageTextAssign::HorizontalAlignment align
                    = pageTextAssign->indRpPos ? pageTextAssign->hPosRp : pageTextAssign->hPosLp;
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

    std::vector<MusxInstance<others::PageTextAssign> > textsForFirstPage;
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

    auto stringFromPageText = [this](const MusxInstance<others::PageTextAssign>& pageText, const String& prefix) {
        std::optional<PageCmper> startPage = pageText->calcStartPageNumber(m_currentMusxPartId);
        std::optional<PageCmper> endPage = pageText->calcEndPageNumber(m_currentMusxPartId);
        HeaderFooterType hfType = (startPage == 2 && endPage.value() >= PageCmper(m_score->npages()))
                                  ? HeaderFooterType::FirstPage : HeaderFooterType::SecondPageToEnd;
        std::optional<PageCmper> forPageId = hfType != HeaderFooterType::SecondPageToEnd ? startPage : std::nullopt;
        musx::util::EnigmaParsingContext parsingContext = pageText->getRawTextCtx(m_currentMusxPartId, forPageId);
        EnigmaParsingOptions options(hfType);
        options.initialFont = FontTracker(score()->style(), prefix);
        /// @todo resize options.referenceSpatium by per-page scaling
        options.referenceSpatium = FINALE_DEFAULT_SPATIUM;
        return stringFromEnigmaText(parsingContext, options);
    };

    auto assignPageTextToHF = [&](Sid styleId, const MusxInstance<others::PageTextAssign>& pageText, const String& prefix) {
        m_score->style().set(styleId, pageText ? stringFromPageText(pageText, prefix) : String());
    };

    if (header.show) {
        m_score->style().set(Sid::showHeader,      true);
        m_score->style().set(Sid::headerFirstPage, header.showFirstPage);
        m_score->style().set(Sid::headerOddEven,   header.oddEven);
        assignPageTextToHF(Sid::evenHeaderL, header.evenLeftText, u"header");
        assignPageTextToHF(Sid::evenHeaderC, header.evenMiddleText, u"header");
        assignPageTextToHF(Sid::evenHeaderR, header.evenRightText, u"header");
        assignPageTextToHF(Sid::oddHeaderL,  header.oddLeftText, u"header");
        assignPageTextToHF(Sid::oddHeaderC,  header.oddMiddleText, u"header");
        assignPageTextToHF(Sid::oddHeaderR,  header.oddRightText, u"header");
    } else {
        m_score->style().set(Sid::showHeader, false);
    }

    if (footer.show) {
        m_score->style().set(Sid::showFooter,      true);
        m_score->style().set(Sid::footerFirstPage, footer.showFirstPage);
        m_score->style().set(Sid::footerOddEven,   footer.oddEven);
        assignPageTextToHF(Sid::evenFooterL, footer.evenLeftText, u"footer");
        assignPageTextToHF(Sid::evenFooterC, footer.evenMiddleText, u"footer");
        assignPageTextToHF(Sid::evenFooterR, footer.evenRightText, u"footer");
        assignPageTextToHF(Sid::oddFooterL,  footer.oddLeftText, u"footer");
        assignPageTextToHF(Sid::oddFooterC,  footer.oddMiddleText, u"footer");
        assignPageTextToHF(Sid::oddFooterR,  footer.oddRightText, u"footer");
    } else {
        m_score->style().set(Sid::showFooter, false);
    }

    auto getPages = [&](const MusxInstance<others::PageTextAssign>& pageTextAssign) -> std::vector<page_idx_t> {
        page_idx_t startP = page_idx_t(pageTextAssign->calcStartPageNumber(m_currentMusxPartId).value_or(1) - 1);
        if (startP + 1 > m_score->npages()) {
            return {};
        }
        // This text is part of a HF for pages 2+ and only needs to be added on page 1
        if (muse::contains(textsForFirstPage, pageTextAssign)) {
            return { startP };
        }
        page_idx_t endP = m_score->npages();
        std::optional<PageCmper> lastPage = pageTextAssign->calcEndPageNumber(m_currentMusxPartId);
        if (lastPage.has_value()) {
            endP = std::min(page_idx_t(lastPage.value()), endP);
        }

        std::vector<page_idx_t> pagesWithText;
        pagesWithText.reserve(endP - startP);
        for (page_idx_t i = startP; i < endP; ++i) {
            pagesWithText.emplace_back(i);
        }
        return pagesWithText;
    };

    auto addPageTextToMeasure = [&](const MusxInstance<others::PageTextAssign>& pageTextAssign, MeasureBase* mb, Page* page) {
        musx::util::EnigmaParsingContext parsingContext = pageTextAssign->getRawTextCtx(m_currentMusxPartId, PageCmper(page->no() + 1));
        EnigmaParsingOptions options;
        FontTracker firstFontInfo;
        double scale = FINALE_DEFAULT_SPATIUM;
        if (const auto musxPage = m_doc->getOthers()->get<others::Page>(m_currentMusxPartId, PageCmper(page->no() + 1))) {
            scale *= musxPage->calcPageScaling().toDouble();
        }
        options.referenceSpatium = scale;
        String pageText = stringFromEnigmaText(parsingContext, options, &firstFontInfo);

        TextBase* text;
        if (mb->isMeasure()) {
            // Add as staff text
            Measure* measure = toMeasure(mb);
            Segment* s = measure->getChordRestOrTimeTickSegment(measure->tick());
            text = toTextBase(Factory::createStaffText(s));
            text->setTrack(0);
            text->setXmlText(pageText);
            if (text->plainText().empty()) {
                delete text;
                return;
            }
            setAndStyleProperty(text, Pid::PLACEMENT, PlacementV::ABOVE);
            text->setAutoplace(false);
            s->add(text);
        } else if (mb->isBox()) {
            // Add as frame text
            text = toTextBase(Factory::createText(toBox(mb), TextStyleType::FRAME));
            text->setXmlText(pageText);
            if (text->plainText().empty()) {
                delete text;
                return;
            }
            text->setParent(mb);
            toBox(mb)->add(text);
        }

        if (text) {
            firstFontInfo.setFontProperties(text);
            setAndStyleProperty(text, Pid::SIZE_SPATIUM_DEPENDENT, false); // Page text does not scale to spatium
            text->checkCustomFormatting(pageText);
            text->setVisible(!pageTextAssign->hidden);
            AlignH hAlignment = toAlignH(pageTextAssign->indRpPos && !(page->no() & 1) ? pageTextAssign->hPosRp : pageTextAssign->hPosLp);
            setAndStyleProperty(text, Pid::ALIGN, Align(hAlignment, toAlignV(pageTextAssign->vPos)), true);
            setAndStyleProperty(text, Pid::POSITION, hAlignment, true);
            text->score()->renderer()->layoutItem(text);
            PointF p = pagePosOfPageTextAssign(page, pageTextAssign, text->ldata()->bbox());
            setAndStyleProperty(text, Pid::OFFSET, mb->isBox() ? p : p - mb->pagePos());
            FrameSettings frameSettings(pageTextAssign->getTextBlock().get());
            frameSettings.setFrameProperties(text);
            collectElementStyle(text);
        }
    };

    std::unordered_map<page_idx_t, MeasureBase*> topBoxes;
    std::unordered_map<page_idx_t, MeasureBase*> bottomBoxes;

    for (MusxInstance<others::PageTextAssign> pageTextAssign : notHF) {
        for (page_idx_t i : getPages(pageTextAssign)) {
            Page* page = m_score->pages().at(i);
            musx::util::EnigmaParsingContext parsingContext = pageTextAssign->getRawTextCtx(m_currentMusxPartId, PageCmper(i + 1));
            MeasureBase* mb = [&]() {
                // Create frames at given position if needed
                if (pageTextAssign->vPos == others::PageTextAssign::VerticalAlignment::Top) {
                    if (MeasureBase* presentBase = muse::value(topBoxes, page->no(), nullptr)) {
                        return presentBase;
                    }
                    System* system = page->systems().front();
                    if (system && system->vbox()) {
                        topBoxes.emplace(page->no(), system->first());
                        return system->first();
                    }
                    VBox* pageFrame = Factory::createVBox(m_score->dummy()->system());
                    double distToTopStaff = 0.0;
                    if (system) {
                        pageFrame->setTick(system->first()->tick());
                        pageFrame->setNext(system->first());
                        pageFrame->setPrev(system->first()->prev());
                        m_score->measures()->insert(pageFrame, pageFrame);
                        if (Spacer* spacer = system->upSpacer(system->firstVisibleStaff(), nullptr)) {
                            distToTopStaff = spacer->absoluteGap();
                            spacer->measure()->remove(spacer);
                        }
                    } else {
                        pageFrame->setTick(m_score->last() ? m_score->last()->endTick() : Fraction(0, 1));
                        m_score->measures()->append(pageFrame);
                    }
                    if (importCustomPositions()) {
                        /// @todo check scaling on this
                        double boxToNotationDist = m_score->style().styleMM(Sid::minVerticalDistance);
                        double boxToStaffDist = boxToNotationDist + (system ? system->minTop() : 0.0);
                        double headerExtension = page->headerExtension();
                        double headerFooterPadding = m_score->style().styleMM(Sid::staffHeaderFooterPadding);
                        double headerDistance = headerExtension ? headerExtension + headerFooterPadding : 0.0;
                        double maxBoxHeight = distToTopStaff - boxToStaffDist;
                        double preferredHeight = absoluteDouble(15, pageFrame);
                        if (maxBoxHeight > preferredHeight) {
                            boxToStaffDist += maxBoxHeight - preferredHeight;
                            boxToNotationDist += maxBoxHeight - preferredHeight;
                            maxBoxHeight = preferredHeight;
                        }
                        setAndStyleProperty(pageFrame, Pid::BOX_AUTOSIZE, false);
                        setAndStyleProperty(pageFrame, Pid::SIZE_SPATIUM_DEPENDENT, false);
                        pageFrame->setBoxHeight(Spatium(maxBoxHeight / m_score->style().defaultSpatium()));
                        setAndStyleProperty(pageFrame, Pid::PADDING_TO_NOTATION_BELOW,
                                            Spatium(boxToNotationDist / m_score->style().defaultSpatium()));
                        setAndStyleProperty(pageFrame, Pid::BOTTOM_GAP, Spatium(boxToStaffDist / m_score->style().defaultSpatium()));
                        pageFrame->ryoffset() -= headerDistance;
                    }
                    topBoxes.emplace(page->no(), toMeasureBase(pageFrame));
                    return toMeasureBase(pageFrame);
                } else if (pageTextAssign->vPos == others::PageTextAssign::VerticalAlignment::Bottom) {
                    if (MeasureBase* presentBase = muse::value(bottomBoxes, page->no(), nullptr)) {
                        return presentBase;
                    }
                    System* system = page->systems().back();
                    if (system && system->vbox()) {
                        bottomBoxes.emplace(page->no(), system->first());
                        return system->last();
                    }
                    VBox* pageFrame = Factory::createVBox(m_score->dummy()->system());
                    double distToBottomStaff = 0.0;
                    if (system) {
                        pageFrame->setTick(system->last()->endTick());
                        pageFrame->setPrev(system->last());
                        pageFrame->setNext(system->last()->next());
                        m_score->measures()->insert(pageFrame, pageFrame);
                        if (Spacer* spacer = system->downSpacer(system->lastVisibleStaff())) {
                            distToBottomStaff = spacer->absoluteGap();
                            spacer->measure()->remove(spacer);
                        }
                        // Move page break, if it exists
                        for (EngravingItem* e : system->last()->el()) {
                            if (e->isLayoutBreak() && toLayoutBreak(e)->layoutBreakType() == LayoutBreakType::PAGE) {
                                system->last()->remove(e);
                                pageFrame->add(e->clone());
                                break;
                            }
                        }
                    } else {
                        pageFrame->setTick(m_score->last() ? m_score->last()->endTick() : Fraction(0, 1));
                        m_score->measures()->append(pageFrame);
                    }
                    if (importCustomPositions()) {
                        /// @todo check scaling on this
                        double boxToNotationDist = m_score->style().styleMM(Sid::minVerticalDistance);
                        double boxToStaffDist = boxToNotationDist + (system ? system->minBottom() : 0.0);
                        double maxBoxHeight = distToBottomStaff - boxToStaffDist;
                        /// @todo account for footer distance?
                        setAndStyleProperty(pageFrame, Pid::BOX_AUTOSIZE, false);
                        setAndStyleProperty(pageFrame, Pid::SIZE_SPATIUM_DEPENDENT, false);
                        pageFrame->setBoxHeight(Spatium(maxBoxHeight / m_score->style().defaultSpatium()));
                        setAndStyleProperty(pageFrame, Pid::PADDING_TO_NOTATION_ABOVE,
                                            Spatium(boxToNotationDist / m_score->style().defaultSpatium()));
                        setAndStyleProperty(pageFrame, Pid::TOP_GAP, Spatium(boxToStaffDist / m_score->style().defaultSpatium()));
                    }
                    bottomBoxes.emplace(page->no(), toMeasureBase(pageFrame));
                    return toMeasureBase(pageFrame);
                }
                // Don't add frames for text vertically aligned to the center.
                /// @todo use sophisticated check for whether to import as frame or not. (i.e. distance to measure is too large, frame would get in the way of music)
                // Use font metrics to precompute bbox (inaccurate for multiline/multiformat)
                EnigmaParsingOptions options;
                options.plainText = true;
                FontTracker firstFontInfo;
                String pagePlainText = stringFromEnigmaText(parsingContext, options, &firstFontInfo);
                muse::draw::FontMetrics fm = firstFontInfo.toFontMetrics();
                RectF r = fm.boundingRect(pagePlainText);
                PointF pagePosOfPageText = pagePosOfPageTextAssign(page, pageTextAssign, r);
                double prevDist = DBL_MAX;
                MeasureBase* closest = nullptr;
                for (System* s : page->systems()) {
                    for (MeasureBase* m : s->measures()) {
                        double dist = m->ldata()->bbox().translated(m->pagePos()).distanceTo(pagePosOfPageText);
                        if (dist < prevDist) {
                            closest = m;
                            prevDist = dist;
                        }
                    }
                }
                return closest;
            }();
            addPageTextToMeasure(pageTextAssign, mb, page);
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
                setAndStyleProperty(e, Pid::OFFSET, e->offset() - (b->pagePos() + e->ldata()->bbox().topLeft()));
            }
        }
    }
}

void FinaleParser::importChordsFrets(const MusxInstance<others::StaffUsed>& musxScrollViewItem,
                                     const MusxInstance<others::Measure>& musxMeasure,
                                     Staff* staff, Measure* measure)
{
    const auto chordAssignments = m_doc->getDetails()->getArray<details::ChordAssign>(m_currentMusxPartId, musxScrollViewItem->staffId,
                                                                                      musxMeasure->getCmper());
    const MusxInstance<options::ChordOptions> config = musxOptions().chordOptions;
    using ChordStyle = options::ChordOptions::ChordStyle;
    HarmonyType ht = harmonyTypeFromChordStyle(config->chordStyle);

    // https://usermanuals.finalemusic.com/Finale2012Win/Content/Finale/ID_MENU_CHORD_STYLE_SOLFEGGIO.htm
    static const std::unordered_map<int, String> solfeggioTable = {
        { int(Tpc::TPC_C_B), u"Ti" },
        { int(Tpc::TPC_D_B), u"Ra" },
        { int(Tpc::TPC_E_B), u"Me" },
        { int(Tpc::TPC_F_B), u"Mi" },
        { int(Tpc::TPC_G_B), u"Se" },
        { int(Tpc::TPC_A_B), u"Le" },
        { int(Tpc::TPC_B_B), u"Te" },
        { int(Tpc::TPC_C), u"Do" },
        { int(Tpc::TPC_D), u"Re" },
        { int(Tpc::TPC_E), u"Mi" },
        { int(Tpc::TPC_F), u"Fa" },
        { int(Tpc::TPC_G), u"So" },
        { int(Tpc::TPC_A), u"La" },
        { int(Tpc::TPC_B), u"Ti" },
        { int(Tpc::TPC_C_S), u"Di" },
        { int(Tpc::TPC_D_S), u"Ri" },
        { int(Tpc::TPC_E_S), u"Fa" },
        { int(Tpc::TPC_F_S), u"Fi" },
        { int(Tpc::TPC_G_S), u"Si" },
        { int(Tpc::TPC_A_S), u"Li" },
        { int(Tpc::TPC_B_S), u"Do" },
    };

    static const std::unordered_map<String, String> romanNumerals = {
        { u"1", u"I" },
        { u"2", u"II" },
        { u"3", u"III" },
        { u"4", u"IV" },
        { u"5", u"V" },
        { u"6", u"VI" },
        { u"7", u"VII" },
    };

    for (const MusxInstance<details::ChordAssign>& chordAssignment : chordAssignments) {
        Segment* s = measure->getSegmentR(SegmentType::ChordRest, eduToFraction(chordAssignment->horzEdu));
        KeySigEvent se = staff->keySigEvent(s->tick());
        int minorOffset = se.mode() == KeyMode::MINOR ? 2 : 0; // add ionian?
        int rootTpc = clampEnharmonic(step2tpcByKey(se.degInKey(chordAssignment->rootScaleNum - minorOffset), se.key())
                                      + chordAssignment->rootAlter * TPC_DELTA_SEMITONE, config->chordStyle == ChordStyle::Solfeggio);
        int bassTpc = clampEnharmonic(step2tpcByKey(se.degInKey(chordAssignment->bassScaleNum - minorOffset), se.key())
                                      + chordAssignment->bassAlter * TPC_DELTA_SEMITONE, config->chordStyle == ChordStyle::Solfeggio);
        /// @todo capo options

        // Instead of writing the properties ourselves, convert to text and parse that.
        /// @todo this approach accounts for none of the custom formatting stored within the suffixes
        String harmonyText;
        auto getTextForTpc = [&](int tpc, int degree) {
            if (ht != HarmonyType::STANDARD) {
                int step = degree - minorOffset + STEP_DELTA_OCTAVE;
                static const String stepNames(u"1234567");
                assert(stepNames.size() == STEP_DELTA_OCTAVE);
                String stepStr = stepNames.at(step % STEP_DELTA_OCTAVE);

                String accStr;
                int alter = tpc2alterByKey(tpc, se.key()); // or bassAlter / rootAlter
                for (int i = 0; i < std::abs(alter); ++i) {
                    accStr.append(alter < 0 ? u"b" : u"#");
                }
                if (config->chordStyle == ChordStyle::NashvilleB) {
                    harmonyText.append(stepStr + accStr);
                } else if (ht == HarmonyType::ROMAN) {
                    harmonyText.append(accStr + muse::value(romanNumerals, stepStr, u"???"));
                } else {
                    harmonyText.append(accStr + stepStr);
                }
            } else if (config->chordStyle == ChordStyle::Solfeggio) {
                int solfeggioOffset = step2tpcByKey(se.degInKey(-minorOffset), se.key()) - int(Tpc::TPC_C);
                harmonyText.append(muse::value(solfeggioTable, tpc - solfeggioOffset, u"???"));
            } else {
                const NoteSpellingType nst = m_score->style().styleV(Sid::chordSymbolSpelling).value<NoteSpellingType>();
                String tpcName = tpc2name(tpc, nst, NoteCaseType::CAPITAL);
                harmonyText.append(tpcName);
                if (config->chordStyle == ChordStyle::European && tpcName == u"B") {
                    harmonyText.append(u"\u266e");
                }
            }
        };
        getTextForTpc(rootTpc, chordAssignment->rootScaleNum);
        static const std::unordered_map<others::ChordSuffixElement::Prefix, Char> prefixMap = {
            { others::ChordSuffixElement::Prefix::Minus, '-' },
            { others::ChordSuffixElement::Prefix::Plus,  '+' },
            { others::ChordSuffixElement::Prefix::Sharp, '#' },
            { others::ChordSuffixElement::Prefix::Flat,  'b' },
        };
        for (const MusxInstance<others::ChordSuffixElement>& suffixElement : chordAssignment->getChordSuffix()) {
            if (suffixElement->prefix != others::ChordSuffixElement::Prefix::None) {
                harmonyText.append(muse::value(prefixMap, suffixElement->prefix));
            }
            if (suffixElement->isNumber) {
                harmonyText += String::number(static_cast<int>(suffixElement->symbol));
            } else {
                std::optional<char32_t> suffixSym = FinaleTextConv::mappedChar(suffixElement->symbol, suffixElement->font);
                harmonyText += String::fromUcs4(suffixSym.value_or(suffixElement->symbol));
            }
        }
        if (chordAssignment->showAltBass) {
            harmonyText.append(u"/");
            getTextForTpc(bassTpc, chordAssignment->bassScaleNum);
            collectGlobalProperty(Sid::chordBassNoteStagger,
                                  chordAssignment->bassPosition != details::ChordAssign::BassPosition::AfterRoot);
        }

        // From Harmony::endEdit
        if (ht != HarmonyType::ROMAN) {
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
            // Not in Harmony::endEdit: SMuFL symbols
            harmonyText.replace(u"\ue871",  u"0");      // csymHalfDiminished
            harmonyText.replace(u"\ue870",  u"o");      // csymDiminished
            harmonyText.replace(u"\ue873",  u"t");      // csymMajorSeventh
            harmonyText.replace(u"\ue874",  u"-");      // csymMinor
            harmonyText.replace(u"\ue875",  u"(");      // csymParensLeftTall
            harmonyText.replace(u"\ue876",  u")");      // csymParensRightTall
            harmonyText.replace(u"\ue877",  u"[");      // csymBracketLeftTall
            harmonyText.replace(u"\ue878",  u"]");      // csymBracketRightTall
            harmonyText.replace(u"\ue879",  u"(");      // csymParensLeftVeryTall
            harmonyText.replace(u"\ue87a",  u")");      // csymParensRightVeryTall
            harmonyText.replace(u"\ue87b",  u"/");      // csymAlteredBassSlash
            harmonyText.replace(u"\ue87c",  u"/");      // csymDiagonalArrangementSlash
        } else {
            harmonyText.replace(u"\ue260",  u"\u266d");         // flat
            harmonyText.replace(u"\ue261",  u"\u266e");         // natural
            harmonyText.replace(u"\ue262",  u"\u266f");         // sharp
        }

        const MusxInstance<others::StaffComposite> musxStaff = others::StaffComposite::createCurrent(m_doc, m_currentMusxPartId,
                                                                                                     musxScrollViewItem->staffId,
                                                                                                     musxMeasure->getCmper(),
                                                                                                     chordAssignment->horzEdu);
        const double staffReferenceOffset = musxStaff->calcTopLinePosition() * 0.5 * staff->spatium(s->tick())
                                            * staff->staffType(s->tick())->lineDistance().val();
        const double baselinepos = absoluteDoubleFromEvpu(musxStaff->calcBaselinePosition<details::BaselineChords>(0), s); // Needs to be scaled correctly (offset topline/reference pos)?
        PointF offset = evpuToPointF(chordAssignment->horzOff, -chordAssignment->vertOff) * s->defaultSpatium();
        offset.ry() -= (baselinepos - staffReferenceOffset); /// @todo set this as style?

        FretDiagram* fret = nullptr;
        Harmony* h = nullptr;
        if (config->showFretboards && chordAssignment->showFretboard) {
            fret = Factory::createFretDiagram(s);
            fret->setTrack(staff2track(staff->idx()));
            fret->setHarmony(harmonyText);
            fret->updateDiagram(harmonyText);
            setAndStyleProperty(fret, Pid::MAG, doubleFromPercent(chordAssignment->fbPercent), true);
            h = fret->harmony();
            if (importCustomPositions()) {
                const double fbBaselinepos = absoluteDoubleFromEvpu(musxStaff->calcBaselinePosition<details::BaselineFretboards>(0), fret); // Needs to be scaled correctly (offset topline/reference pos)?
                PointF fbOffset = evpuToPointF(chordAssignment->fbHorzOff, -chordAssignment->fbVertOff) * fret->defaultSpatium();
                fbOffset.ry() -= (fbBaselinepos - staffReferenceOffset); /// @todo set this as style?
                offset.ry() -= fbOffset.y(); /// @todo also diagram height?
                setAndStyleProperty(fret, Pid::OFFSET, fbOffset, true);
            }
            if (!chordAssignment->useFretboardFont) {
                if (const MusxInstance<others::FretboardStyle>& fretboardStyle = chordAssignment->getFretboardStyle()) {
                    setAndStyleProperty(fret, Pid::ORIENTATION, fretboardStyle->rotate ? Orientation::HORIZONTAL : Orientation::VERTICAL);
                    setAndStyleProperty(fret, Pid::FRET_NUT, fretboardStyle->nutWidth > 0);
                    if (importCustomPositions()) {
                        setAndStyleProperty(fret, Pid::OFFSET,
                                            PointF(doubleFromEfix(fretboardStyle->horzHandleOff), doubleFromEfix(
                                                       fretboardStyle->vertHandleOff)) * fret->defaultSpatium()); // bind vertical to fretY
                    }
                    String suffix = String::fromStdString(fretboardStyle->fretNumText);
                    collectGlobalProperty(Sid::fretUseCustomSuffix, !suffix.empty());
                    if (!suffix.empty()) {
                        collectGlobalProperty(Sid::fretCustomSuffix, String::fromStdString(fretboardStyle->fretNumText));
                    }
                    collectGlobalProperty(Sid::fretDiagramFretNumberPosition, doubleFromEfix(
                                              fretboardStyle->horzTextOff) > -3.0 ? AlignH::RIGHT : AlignH::LEFT);
                    collectGlobalProperty(Sid::fretStringSpacing, doubleFromEfix(fretboardStyle->stringGap));
                    collectGlobalProperty(Sid::fretFretSpacing, doubleFromEfix(fretboardStyle->fretGap));
                    collectGlobalProperty(Sid::fretFrets, fretboardStyle->defNumFrets); // note: can be set individually too
                    if (fretboardStyle->nutWidth > 0) {
                        collectGlobalProperty(Sid::fretNutThickness, doubleFromEfix(fretboardStyle->nutWidth));
                    }
                    collectGlobalFont("fretDiagramFingering", fretboardStyle->fingNumFont);
                    collectGlobalFont("fretDiagramFretNumber", fretboardStyle->fretNumFont);
                    // todo: fretNumPos, fretShowFingerings
                }
                if (const MusxInstance<others::FretboardGroup>& fretboardGroup = chordAssignment->getFretboardGroup()) {
                    if (const MusxInstance<others::FretInstrument> instrument = fretboardGroup->getFretInstrument()) {
                        setAndStyleProperty(fret, Pid::FRET_STRINGS, instrument->numStrings);
                    }
                    if (fretboardGroup->getInci().has_value()) {
                        Cmper fbCmper = (Cmper(fretboardGroup->getInci().value()) * 16) + (tpc2pitch(rootTpc) + 3 + PITCH_DELTA_OCTAVE)
                                        % PITCH_DELTA_OCTAVE;
                        if (const MusxInstance<details::FretboardDiagram> fretDiagram
                                = m_doc->getDetails()->get<details::FretboardDiagram>(m_currentMusxPartId, fretboardGroup->getCmper(),
                                                                                      fbCmper)) {
                            // setAndStyleProperty(fret, Pid::FRET_SHOW_FINGERINGS, )
                            setAndStyleProperty(fret, Pid::FRET_FRETS, fretDiagram->numFrets);
                            fret->clear();
                            if (fretDiagram->showNum) {
                                setAndStyleProperty(fret, Pid::FRET_OFFSET, fretDiagram->fretboardNum);
                            }
                            std::vector<int> fingerings(fret->strings(), 0);
                            for (const std::shared_ptr<details::FretboardDiagram::Cell>& cell : fretDiagram->cells) {
                                int string = cell->string - 1;
                                if (cell->fret == 0) {
                                    fret->setMarker(string, toFretMarkerType(cell->shape));
                                } else if (cell->shape != details::FretboardDiagram::Shape::None) {
                                    fret->setDot(string, cell->fret, true, toFretDotType(cell->shape));
                                }
                                if (cell->fingerNum != 0) {
                                    fingerings.at(string) = cell->fingerNum;
                                }
                            }
                            setAndStyleProperty(fret, Pid::FRET_FINGERING, fingerings);
                            for (const std::shared_ptr<details::FretboardDiagram::Barre>& barre : fretDiagram->barres) {
                                fret->setBarre(barre->startString - 1, barre->endString - 1, barre->fret);
                            }
                        }
                    }
                }
            }
        } else {
            h = Factory::createHarmony(s);
            h->setTrack(staff2track(staff->idx()));
            h->setHarmony(harmonyText);
            h->afterRead(); // needed?
            if (importCustomPositions()) {
                h->setAutoplace(false);
            }
        }
        h->setHarmonyType(ht);
        h->setBassCase(chordAssignment->bassLowerCase ? NoteCaseType::LOWER : NoteCaseType::UPPER);
        h->setRootCase(chordAssignment->rootLowerCase ? NoteCaseType::LOWER : NoteCaseType::UPPER);
        setAndStyleProperty(h, Pid::PLAY, config->chordPlayback, true);
        setAndStyleProperty(h, Pid::FONT_SIZE, h->propertyDefault(Pid::FONT_SIZE).toDouble() * doubleFromPercent(
                                chordAssignment->chPercent), true);
        if (importCustomPositions()) {
            setAndStyleProperty(h, Pid::OFFSET, offset, true); /// @todo positioning relative to fretboard
        }
        if (fret) {
            s->add(fret);
            collectElementStyle(fret); // also h?
        } else {
            s->add(h);
            collectElementStyle(h);
        }
        /// @todo Pid::HARMONY_VOICE_LITERAL, Pid::HARMONY_VOICING, Pid::HARMONY_DURATION, Pid::HARMONY_DO_NOT_STACK_MODIFIERS
    }
}
}
