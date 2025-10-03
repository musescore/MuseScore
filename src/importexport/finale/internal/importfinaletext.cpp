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

#include "engraving/dom/anchors.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/dynamic.h"
#include "engraving/dom/excerpt.h"
#include "engraving/dom/factory.h"
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

FontTracker::FontTracker(const MusxInstance<musx::dom::FontInfo>& fontInfo, double additionalSizeScaling)
{
    fontName = String::fromStdString(fontInfo->getName());
    fontSize = spatiumScaledFontSize(fontInfo);
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
        std::optional<String> symIds = FinaleTextConv::symIdInsertsFromStdString(nextChunk, styles.font);
        const FontTracker font(styles.font, options.scaleFontSizeBy);
        const bool isSymFont = symFont && font == symFont.value();
        if (firstFontInfo && !prevFont) {
            *firstFontInfo = font;
        } else if (!symIds || !isSymFont) {
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
                case HeaderFooterType::None: return "$:copyright:"; /// @todo does this actually work? maybe not for non-headers?
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
        if (isHeaderOrFooter) {
            String metaTag = metaTagFromTextComponent(parsedCommand[0]);
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

void FinaleParser::importTextExpressions()
{
    using Chord = mu::engraving::Chord; // seemingly needed for Windows builds (20251001)
    struct ReadableExpression {
        String xmlText = String();
        ElementType elementType = ElementType::STAFF_TEXT;
        DynamicType dynamicType = DynamicType::OTHER;
    };

    // Layout score (needed for offset calculations)
    m_score->setLayoutAll();
    m_score->doLayout();

    // 1st: map all expressions to strings
    std::unordered_map<Cmper, ReadableExpression> mappedExpressions;
    for (const auto& textExpression : m_doc->getOthers()->getArray<others::TextExpressionDef>(m_currentMusxPartId)) {
        ReadableExpression readableExpression;
        /// @todo Rather than rely only on marking category, it probably makes more sense to interpret the playback features to detect what kind of marking
        /// this is. Or perhaps a combination of both. This would provide better support to legacy files whose expressions are all Misc.
        others::MarkingCategory::CategoryType categoryType = others::MarkingCategory::CategoryType::Misc;
        MusxInstance<FontInfo> catMusicFont;
        if (MusxInstance<others::MarkingCategory> category = m_doc->getOthers()->get<others::MarkingCategory>(m_currentMusxPartId, textExpression->categoryId)) {
            categoryType = category->categoryType;
            catMusicFont = category->musicFont;
        }
        EnigmaParsingOptions options;
        musx::util::EnigmaParsingContext parsingContext = textExpression->getRawTextCtx(m_currentMusxPartId);
        FontTracker firstFontInfo;
        readableExpression.xmlText = stringFromEnigmaText(parsingContext, options, &firstFontInfo);
        // Option 2 (to match style setting if possible, with font info tags at the beginning if they differ)
        options.initialFont = FontTracker(m_score->style(), fontStyleSuffixFromCategoryType(categoryType));
        /// @todo The musicSymbol font here must be the inferred font *in Finale* that we expect, based on the detection of what kind
        /// of marking it is mentioned above. Right now this code relies on the category, but probably that is too naive a design.
        /// Whichever font we choose here will be stripped out in favor of the default for the kind of marking it is.
        options.musicSymbolFont = [&]() -> std::optional<FontTracker> {
            if (!catMusicFont) {
                return FontTracker(musxOptions().defaultMusicFont); // we get here for the Misc category, and this seems like the best choice for legacy files. See todo comments, though.
            } else if (fontIsEngravingFont(catMusicFont->getName())) {
                return FontTracker(catMusicFont); // if it's an engraving font use it
            } else if (catMusicFont->calcIsDefaultMusic() && !musxOptions().calculatedEngravingFontName.empty()) {
                return FontTracker(catMusicFont); // if it's not an engraving font, but we are using an alternative as engraving font,
                                                  // specify the non-engraving font here. The parsing routine strips it out and MuseScore
                                                  // uses the engraving font instead.
            }
            return std::nullopt;
        }();
        if (catMusicFont && (fontIsEngravingFont(catMusicFont->getName()) || catMusicFont->calcIsDefaultMusic())) {
            options.musicSymbolFont = FontTracker(catMusicFont);
        }
        String exprString2 = stringFromEnigmaText(parsingContext, options); // currently unused

        // Element type
        readableExpression.elementType = [&]() {
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

            // Dynamics (adapted from engraving/dom/dynamic.cpp)
            std::string utf8Tag = readableExpression.xmlText.toStdString();
            const std::regex dynamicRegex(R"((?:<sym>.*?</sym>)+|(?:\b)[fmnprsz]+(?:\b(?=[^>]|$)))");
            auto begin = std::sregex_iterator(utf8Tag.begin(), utf8Tag.end(), dynamicRegex);
            for (auto it = begin; it != std::sregex_iterator(); ++it) {
                const std::smatch match = *it;
                const std::string matchStr = match.str();
                for (auto dyn : Dynamic::dynamicList()) {
                    if (TConv::toXml(dyn.type).ascii() == matchStr || dyn.text == matchStr) {
                        utf8Tag.replace(match.position(0), match.length(0), dyn.text);
                        readableExpression.xmlText = String::fromStdString(utf8Tag); // do we want this?
                        readableExpression.dynamicType = dyn.type;
                        return ElementType::DYNAMIC;
                    }
                }
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
        mappedExpressions.emplace(textExpression->getCmper(), readableExpression);
    }

    // 2nd: iterate each expression assignment and assign the mapped String instances as needed
    for (const auto& expressionAssignment : m_doc->getOthers()->getArray<others::MeasureExprAssign>(m_currentMusxPartId)) {
        if (!expressionAssignment->textExprId) {
            // Shapes are currently unsupported
            continue;
        }
        ReadableExpression def;
        ReadableExpression expression = muse::value(mappedExpressions, expressionAssignment->textExprId, def);
        if (expression.xmlText.empty()) {
            continue;
        }

        // Find staff
        /// @todo use system object staves and linked clones to avoid duplicate elements
        staff_idx_t curStaffIdx = [&]() -> staff_idx_t {
            switch(expressionAssignment->staffAssign) {
            case -1: return 0;
            case -2: return m_score->nstaves() - 1;
            default: return muse::value(m_inst2Staff, StaffCmper(expressionAssignment->staffAssign), muse::nidx);
            }
        }();
        if (curStaffIdx == muse::nidx) {
            /// @todo system object staves
            logger()->logWarning(String(u"Add text: Musx inst value not found."), m_doc, expressionAssignment->staffAssign);
            continue;
        }
        ElementType elementType = expression.elementType == ElementType::STAFF_TEXT && expressionAssignment->staffAssign < 0 ? ElementType::SYSTEM_TEXT : expression.elementType;

        // Find location in measure
        Fraction mTick = muse::value(m_meas2Tick, expressionAssignment->getCmper(), Fraction(-1, 1));
        Measure* measure = !mTick.negative() ? m_score->tick2measure(mTick) : nullptr;
        if (!measure) {
            continue;
        }
        track_idx_t curTrackIdx = staff2track(curStaffIdx) + static_cast<voice_idx_t>(std::clamp(expressionAssignment->layer - 1, 0, int(VOICES) - 1));
        Fraction rTick = eduToFraction(expressionAssignment->eduPosition);
        Segment* s = measure->findSegmentR(Segment::CHORD_REST_OR_TIME_TICK_TYPE, rTick);
        if (!s) {
            TimeTickAnchor* anchor = EditTimeTickAnchors::createTimeTickAnchor(measure, rTick, curStaffIdx);
            EditTimeTickAnchors::updateLayout(measure);
            s = anchor->segment();
        }

        // Create item
        TextBase* item = toTextBase(Factory::createItem(elementType, s));
        const MusxInstance<others::TextExpressionDef> expressionDef = expressionAssignment->getTextExpression();
        item->setTrack(curTrackIdx);
        item->setVisible(!expressionAssignment->hidden);
        item->setXmlText(expression.xmlText);
        item->setPropertyFlags(Pid::OFFSET, PropertyFlags::UNSTYLED);
        AlignH hAlign = toAlignH(expressionDef->horzExprJustification);
        item->setAlign(Align(hAlign, AlignV::BASELINE));
        s->add(item);

        // Calculate position in score
        PointF p;
        item->setOffset(PointF());
        item->setAutoplace(false);
        m_score->renderer()->layoutItem(item);
        // if (measure->system()) {
            // m_score->doLayoutRange(measure->system()->first()->tick(), measure->system()->last()->endTick());
        // }
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
                if (seg && seg->element(item->track())) {
                    if (seg->element(item->track())->isChord()) {
                        Chord* c = toChord(seg->element(item->track()));
                        if (expressionAssignment->graceNoteIndex) {
                            if (Chord* gc = c->graceNoteAt(static_cast<size_t>(expressionAssignment->graceNoteIndex - 1))) {
                                c = gc;
                            }
                        }
                        engraving::Note* n = c->up() ? c->downNote() : c->upNote();
                        p.rx() = n->pageX();
                        if (expressionDef->horzMeasExprAlign == others::HorizontalMeasExprAlign::CenterPrimaryNotehead) {
                            p.rx() += n->noteheadCenterX();
                        }
                    } else {
                        Rest* rest = toRest(seg->element(item->track()));
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
                if (seg && seg->element(item->track())) {
                    if (seg->element(item->track())->isChord()) {
                        Chord* c = toChord(seg->element(item->track()));
                        if (expressionAssignment->graceNoteIndex) {
                            if (Chord* gc = c->graceNoteAt(static_cast<size_t>(expressionAssignment->graceNoteIndex - 1))) {
                                c = gc;
                            }
                        }
                        p.rx() = c->pageX();
                        if (expressionDef->horzMeasExprAlign == others::HorizontalMeasExprAlign::Stem) {
                            p.rx() += rendering::score::StemLayout::stemPosX(c);
                        }
                    } else {
                        Rest* rest = toRest(seg->element(item->track()));
                        p.rx() = rest->isFullMeasureRest() ? seg->pageX() : rest->pageX();
                    }
                } else {
                    p.rx() = s->pageX();
                }
                break;
            }
            case others::HorizontalMeasExprAlign::CenterAllNoteheads: {
                Shape staffShape = s->staffShape(curStaffIdx);
                staffShape.remove_if([](ShapeElement& el) { return el.height() == 0; });
                p.rx() = staffShape.right() / 2 + s->x() + s->measure()->x();
                break;
            }
            case others::HorizontalMeasExprAlign::RightOfAllNoteheads: {
                Shape staffShape = s->staffShape(curStaffIdx);
                staffShape.remove_if([](ShapeElement& el) { return el.height() == 0; });
                p.rx() = staffShape.right() + s->x() + s->measure()->x();
                break;
            }
            case others::HorizontalMeasExprAlign::StartTimeSig: {
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
        p.rx() += doubleFromEvpu(expressionDef->measXAdjust) * SPATIUM20;
        // We will need this when justify differs from text alignment (currently we set alignment to justify)
        /* switch (hAlign) {
            case AlignH::LEFT:
                break;
            case AlignH::HCENTER:
                p.rx() -= item->ldata()->bbox().center().y() / 2;
                break;
            case AlignH::RIGHT:
                p.rx() -= item->ldata()->bbox().center().y();
                break;
        } */
        switch (expressionDef->vertMeasExprAlign) {
            case others::VerticalMeasExprAlign::AboveStaff: {
                item->setPlacement(PlacementV::ABOVE);
                p.ry() = item->pagePos().y() + doubleFromEvpu(expressionDef->yAdjustBaseline) * SPATIUM20;
                break;
            }
            case others::VerticalMeasExprAlign::Manual: {
                item->setPlacement(PlacementV::ABOVE); // Finale default
                p.ry() = item->pagePos().y() + doubleFromEvpu(expressionDef->yAdjustBaseline) * SPATIUM20;
                break;
            }
            case others::VerticalMeasExprAlign::RefLine: {
                item->setPlacement(PlacementV::ABOVE);
                p.ry() = item->pagePos().y() + doubleFromEvpu(expressionDef->yAdjustBaseline) * SPATIUM20;
                break;
            }
            case others::VerticalMeasExprAlign::BelowStaff: {
                item->setPlacement(PlacementV::BELOW);
                p.ry() = item->pagePos().y() + doubleFromEvpu(expressionDef->yAdjustBaseline) * SPATIUM20;
                break;
            }
            case others::VerticalMeasExprAlign::TopNote: {
                item->setPlacement(PlacementV::ABOVE);
                Segment* seg = measure->findSegmentR(SegmentType::ChordRest, rTick);
                if (seg && seg->element(item->track())) {
                    if (seg->element(item->track())->isChord()) {
                        Chord* c = toChord(seg->element(item->track()));
                        if (expressionAssignment->graceNoteIndex) {
                            if (Chord* gc = c->graceNoteAt(static_cast<size_t>(expressionAssignment->graceNoteIndex - 1))) {
                                c = gc;
                            }
                        }
                        const engraving::Note* n = c->upNote();
                        p.ry() = n->pagePos().y() - n->headHeight() / 2;
                    } else {
                        Rest* rest = toRest(seg->element(item->track()));
                        p.ry() = rest->pagePos().y() - rest->ldata()->bbox().center().y();
                    }
                }
                p.ry() -= doubleFromEvpu(expressionDef->yAdjustEntry) * SPATIUM20;
                break;
            }
            case others::VerticalMeasExprAlign::BottomNote: {
                item->setPlacement(PlacementV::BELOW);
                Segment* seg = measure->findSegmentR(SegmentType::ChordRest, rTick);
                if (seg && seg->element(item->track())) {
                    if (seg->element(item->track())->isChord()) {
                        Chord* c = toChord(seg->element(item->track()));
                        if (expressionAssignment->graceNoteIndex) {
                            if (Chord* gc = c->graceNoteAt(static_cast<size_t>(expressionAssignment->graceNoteIndex - 1))) {
                                c = gc;
                            }
                        }
                        const engraving::Note* n = c->downNote();
                        p.ry() = n->pagePos().y() - n->headHeight() / 2;
                    } else {
                        Rest* rest = toRest(seg->element(item->track()));
                        p.ry() = rest->pagePos().y() - rest->ldata()->bbox().center().y();
                    }
                }
                p.ry() -= doubleFromEvpu(expressionDef->yAdjustEntry) * SPATIUM20;
                break;
            }
            case others::VerticalMeasExprAlign::AboveEntry:
            case others::VerticalMeasExprAlign::AboveStaffOrEntry: {
                // maybe always set entry to entry value?
                item->setPlacement(PlacementV::ABOVE);
                Segment* seg = measure->findSegmentR(SegmentType::ChordRest, rTick);
                if (seg && seg->element(item->track())) { // should be all tracks?
                    p.ry() = item->pagePos().y() - doubleFromEvpu(expressionDef->yAdjustEntry) * SPATIUM20;
                } else {
                    p.ry() = item->pagePos().y() + doubleFromEvpu(expressionDef->yAdjustBaseline) * SPATIUM20;
                }
                break;
            }
            case others::VerticalMeasExprAlign::BelowEntry:
            case others::VerticalMeasExprAlign::BelowStaffOrEntry: {
                /// use std::max (entrypos, baselinepos, 1sp below staffline).
                item->setPlacement(PlacementV::BELOW);
                Segment* seg = measure->findSegmentR(SegmentType::ChordRest, rTick);
                if (seg && seg->element(item->track())) { // should be all tracks?
                    p.ry() = item->pagePos().y() - doubleFromEvpu(expressionDef->yAdjustEntry) * SPATIUM20;
                } else {
                    p.ry() = item->pagePos().y() + doubleFromEvpu(expressionDef->yAdjustBaseline) * SPATIUM20;
                }
                break;
            }
            default: {
                p.ry() = item->pagePos().y() - doubleFromEvpu(expressionDef->yAdjustEntry) * SPATIUM20;
                break;
            }
        }
        p -= item->pagePos(); // is this correct? or do we need the pagex of first cr segment, or measure position
        p += evpuToPointF(expressionAssignment->horzEvpuOff, -expressionAssignment->vertEvpuOff) * SPATIUM20; // assignment offset
        item->setOffset(p);

        switch (elementType) {
            case ElementType::DYNAMIC: {
                Dynamic* dynamic = toDynamic(item);
                dynamic->setDynamicType(expression.dynamicType);
                if (expressionAssignment->layer != 0) {
                    dynamic->setVoiceAssignment(VoiceAssignment::CURRENT_VOICE_ONLY);
                }
                switch (expressionDef->playbackType) {
                case others::PlaybackType::None: break;
                case others::PlaybackType::KeyVelocity: {
                    dynamic->setVelocity(expressionDef->value);
                    break;
                }
                default: break;
                }
                break;
            }
            case ElementType::REHEARSAL_MARK: {
                if (expressionDef->hideMeasureNum && measure->measureNumber(curStaffIdx)) {
                    /// @todo needs to be created via layout first???
                    measure->measureNumber(curStaffIdx)->setVisible(false);
                }
                break;
            }
            case ElementType::TEMPO_TEXT: {
                TempoText* tt = toTempoText(item);
                switch (expressionDef->playbackType) {
                case others::PlaybackType::None: break;
                case others::PlaybackType::Tempo: {
                    tt->setFollowText(false); /// @todo detect this
                    tt->setTempo(expressionDef->value / 60.0); // Assume quarter for now
                    break;
                }
                default: break;
                }
                break;
            }
            case ElementType::STAFF_TEXT:
            case ElementType::SYSTEM_TEXT: {
                StaffTextBase* stb = toStaffTextBase(item);
                switch (expressionDef->playbackType) {
                case others::PlaybackType::None: break;
                case others::PlaybackType::Swing: {
                    int swingValue = expressionDef->value;
                    int swingUnit = Fraction(1, measure->timesig().denominator() > 8 ? 16 : 8).ticks();
                    stb->setSwing(swingValue != 0);
                    stb->setSwingParameters(swingUnit, 50 + (swingValue / 6));
                    break;
                }
                default: break;
                }
                break;
            }
            default: break;
        }
        /// @todo assign this to the appropriate location(s), taking into account if this is a single-staff or stafflist assignment.
        /// @note Finale provides a per-staff assignment *in addition* to top-staff (-1) or bot-staff (-2) assignments. Whether it is visible is
        /// determined by the stafflist (if any) or by whether it is assigned to score or part or both. I don't know enough about MuseScore
        /// features to suggest an exact import strategy. (I may need to add staff lists to musx. I don't remember adding them yet. But since
        /// Finale adds an assignment on every staff dictated by the staff list, we may not need to reference it.)
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
                EnigmaParsingOptions options;
                musx::util::EnigmaParsingContext parsingContext = measureTextAssign->getRawTextCtx(m_currentMusxPartId);
                FontTracker firstFontInfo;
                String measureText = stringFromEnigmaText(parsingContext, options, &firstFontInfo);

                Fraction rTick = eduToFraction(measureTextAssign->xDispEdu);
                Segment* s = measure->findSegmentR(Segment::CHORD_REST_OR_TIME_TICK_TYPE, rTick);
                if (!s) {
                    TimeTickAnchor* anchor = EditTimeTickAnchors::createTimeTickAnchor(measure, rTick, curStaffIdx);
                    EditTimeTickAnchors::updateLayout(measure);
                    s = anchor->segment();
                }

                StaffText* text = Factory::createStaffText(s);
                text->setTrack(curTrackIdx);
                text->setXmlText(measureText);
                if (text->plainText().empty()) {
                    delete text;
                    continue;
                }
                text->setVisible(!measureTextAssign->hidden);
                text->setAutoplace(false);
                text->setOffset(evpuToPointF(rTick.isZero() ? measureTextAssign->xDispEvpu : 0, -measureTextAssign->yDisp) * SPATIUM20);
                text->setPropertyFlags(Pid::OFFSET, PropertyFlags::UNSTYLED);
                s->add(text);
            }
        }
    }
}

bool FinaleParser::isOnlyPage(const MusxInstance<others::PageTextAssign>& pageTextAssign, PageCmper page)
{
    const std::optional<PageCmper> startPageNum = pageTextAssign->calcStartPageNumber(m_currentMusxPartId);
    const std::optional<PageCmper> endPageNum = pageTextAssign->calcEndPageNumber(m_currentMusxPartId); // calcEndPageNumber handles case when endPage is zero
    return (startPageNum == page && endPageNum == page);
};

/// @todo Instead of hard-coding page 1 and page 2, we need to find the first page in the Finale file with music on it
/// and use that as the first page. At least, that is my impression. How to handle blank pages in MuseScore is an open question.
/// - RGP

void FinaleParser::importPageTexts()
{
    MusxInstanceList<others::PageTextAssign> pageTextAssignList = m_doc->getOthers()->getArray<others::PageTextAssign>(m_currentMusxPartId);

    // we need to work with real-time positions and pages, so we layout the score.
    m_score->setLayoutAll();
    m_score->doLayout();


    // code idea:
    // first, read score metadata
    // then, handle page text as header/footer vframes where applicable and if not, assign it to a measure
    // each handled textblock is parsed as possible with fontdata and appended to a list of used fontdata
    // whenever an element is read, check with Cmper in a map to see if textblock has already been parsed, if so, used it from there, if not, parse and add
    // measure-anchored-texts: determine text style based on (??? contents? font settings?), add to score, same procedure with text block
    // if centered or rightmost, create as marking to get correct anchoring???
    // expressions::
    // tempo changes
    // need to create character conversion map for non-smufl fonts???

    struct HeaderFooter {
        bool show = false;
        bool showFirstPage = true; // always show first page
        bool oddEven = true; // always different odd/even pages
        std::vector<MusxInstance<others::PageTextAssign>> oddLeftTexts;
        std::vector<MusxInstance<others::PageTextAssign>> oddMiddleTexts;
        std::vector<MusxInstance<others::PageTextAssign>> oddRightTexts;
        std::vector<MusxInstance<others::PageTextAssign>> evenLeftTexts;
        std::vector<MusxInstance<others::PageTextAssign>> evenMiddleTexts;
        std::vector<MusxInstance<others::PageTextAssign>> evenRightTexts;
    };

    HeaderFooter header;
    HeaderFooter footer;
    std::vector<MusxInstance<others::PageTextAssign>> notHF;
    std::vector<MusxInstance<others::PageTextAssign>> remainder;

    // gather texts by position
    for (MusxInstance<others::PageTextAssign> pageTextAssign : pageTextAssignList) {
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

        // if text is not at top or bottom, invisible,
        // not recurring, or not on page 1, don't import as hf
        // For 2-page scores, we can import text only assigned to page 2 as a regular even hf.
        // For 3-page scores, we can import text only assigned to page 2 as a regular odd hf if we disable hf on page one.
        // RGP: I don't think we should do the 3rd option. Disabling hf on page one is a non-starter. We should never do that,
        //      because it causes far more damage than benefit. I changed it not to.
        /// @todo add sensible limits for xDisp and such.
        if (pageTextAssign->vPos == others::PageTextAssign::VerticalAlignment::Center
                                 || pageTextAssign->hidden
                                 || startPage.value() >= 3 /// @todo must be changed to be first non-blank page + 2
                                 || endPage.value() < PageCmper(m_score->npages())) {
            notHF.emplace_back(pageTextAssign);
            continue;
        }
        remainder.emplace_back(pageTextAssign);
    }

    for (MusxInstance<others::PageTextAssign> pageTextAssign : remainder) {
        HeaderFooter& hf = pageTextAssign->vPos == others::PageTextAssign::VerticalAlignment::Top ? header : footer;
        hf.show = true;
    }
    for (MusxInstance<others::PageTextAssign> pageTextAssign : remainder) {
        HeaderFooter& hf = pageTextAssign->vPos == others::PageTextAssign::VerticalAlignment::Top ? header : footer;
        /// @todo this has got to take into account the page text's hPosLp or hPosRp based on indRpPos.
        /// @todo Finale bases right/left on the actual page numbers, not the visual page numbers. But MuseScore's
        /// left/right headers display based on visual page numbers. So the whole calculation must be reversed if
        /// m_score->pageNumberOffset() is odd.
        switch (pageTextAssign->hPosLp) {
        case others::PageTextAssign::HorizontalAlignment::Left:
            if (pageTextAssign->oddEven != others::PageTextAssign::PageAssignType::Even) {
                hf.oddLeftTexts.emplace_back(pageTextAssign);
            }
            if (pageTextAssign->oddEven != others::PageTextAssign::PageAssignType::Odd) {
                hf.evenLeftTexts.emplace_back(pageTextAssign);
            }
            break;
        case others::PageTextAssign::HorizontalAlignment::Center:
            if (pageTextAssign->oddEven != others::PageTextAssign::PageAssignType::Even) {
                hf.oddMiddleTexts.emplace_back(pageTextAssign);
            }
            if (pageTextAssign->oddEven != others::PageTextAssign::PageAssignType::Odd) {
                hf.evenMiddleTexts.emplace_back(pageTextAssign);
            }
            break;
        case others::PageTextAssign::HorizontalAlignment::Right:
            if (pageTextAssign->oddEven != others::PageTextAssign::PageAssignType::Even) {
                hf.oddRightTexts.emplace_back(pageTextAssign);
            }
            if (pageTextAssign->oddEven != others::PageTextAssign::PageAssignType::Odd) {
                hf.evenRightTexts.emplace_back(pageTextAssign);
            }
            break;
        }
    }

    auto stringFromPageText = [&](const MusxInstance<others::PageTextAssign>& pageText, bool isForHeaderFooter = true) {
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
        m_score->style().set(Sid::evenHeaderL,     header.evenLeftTexts.empty() ? String() : stringFromPageText(header.evenLeftTexts.front())); // for now
        m_score->style().set(Sid::evenHeaderC,     header.evenMiddleTexts.empty() ? String() : stringFromPageText(header.evenMiddleTexts.front()));
        m_score->style().set(Sid::evenHeaderR,     header.evenRightTexts.empty() ? String() : stringFromPageText(header.evenRightTexts.front()));
        m_score->style().set(Sid::oddHeaderL,      header.oddLeftTexts.empty() ? String() : stringFromPageText(header.oddLeftTexts.front()));
        m_score->style().set(Sid::oddHeaderC,      header.oddMiddleTexts.empty() ? String() : stringFromPageText(header.oddMiddleTexts.front()));
        m_score->style().set(Sid::oddHeaderR,      header.oddRightTexts.empty() ? String() : stringFromPageText(header.oddRightTexts.front()));
    }

    if (footer.show) {
        m_score->style().set(Sid::showFooter,      true);
        m_score->style().set(Sid::footerFirstPage, footer.showFirstPage);
        m_score->style().set(Sid::footerOddEven,   footer.oddEven);
        m_score->style().set(Sid::evenFooterL,     footer.evenLeftTexts.empty() ? String() : stringFromPageText(footer.evenLeftTexts.front())); // for now
        m_score->style().set(Sid::evenFooterC,     footer.evenMiddleTexts.empty() ? String() : stringFromPageText(footer.evenMiddleTexts.front()));
        m_score->style().set(Sid::evenFooterR,     footer.evenRightTexts.empty() ? String() : stringFromPageText(footer.evenRightTexts.front()));
        m_score->style().set(Sid::oddFooterL,      footer.oddLeftTexts.empty() ? String() : stringFromPageText(footer.oddLeftTexts.front()));
        m_score->style().set(Sid::oddFooterC,      footer.oddMiddleTexts.empty() ? String() : stringFromPageText(footer.oddMiddleTexts.front()));
        m_score->style().set(Sid::oddFooterR,      footer.oddRightTexts.empty() ? String() : stringFromPageText(footer.oddRightTexts.front()));
    }

    std::vector<Cmper> pagesWithHeaderFrames;
    std::vector<Cmper> pagesWithFooterFrames;

    auto getPages = [this](const MusxInstance<others::PageTextAssign>& pageTextAssign) -> std::vector<page_idx_t> {
        std::vector<page_idx_t> pagesWithText;
        page_idx_t startP = page_idx_t(pageTextAssign->calcStartPageNumber(m_currentMusxPartId).value_or(1) - 1);
        page_idx_t endP = page_idx_t(pageTextAssign->calcStartPageNumber(m_currentMusxPartId).value_or(PageCmper(m_score->npages())) - 1);
        for (page_idx_t i = startP; i <= endP; ++i) {
            pagesWithText.emplace_back(i);
        }
        return pagesWithText;
    };

    auto pagePosOfPageTextAssign = [](Page* page, const MusxInstance<others::PageTextAssign>& pageTextAssign) -> PointF {
        /// @todo e.g. center-aligned text in vframes is also in the center of the page, account for that here
        RectF pageBox = page->ldata()->bbox(); // height and width definitely work, this hopefully too
        PointF p;

        switch (pageTextAssign->vPos) {
        case others::PageTextAssign::VerticalAlignment::Center:
            p.ry() = pageBox.y() / 2;
            break;
        case others::PageTextAssign::VerticalAlignment::Top:
            p.ry() = pageBox.y();
            break;
        case others::PageTextAssign::VerticalAlignment::Bottom:;
            break;
        }

        if (pageTextAssign->indRpPos && !(page->no() & 1)) {
            switch(pageTextAssign->hPosRp) {
            case others::PageTextAssign::HorizontalAlignment::Center:
                p.rx() = pageBox.x() / 2;
                break;
            case others::PageTextAssign::HorizontalAlignment::Right:;
                p.rx() = pageBox.x();
                break;
            case others::PageTextAssign::HorizontalAlignment::Left:
                break;
            }
            p.rx() += doubleFromEvpu(pageTextAssign->rightPgXDisp);
            p.ry() += doubleFromEvpu(pageTextAssign->rightPgYDisp);
        } else {
            switch(pageTextAssign->hPosLp) {
            case others::PageTextAssign::HorizontalAlignment::Center:
                p.rx() = pageBox.x() / 2;
                break;
            case others::PageTextAssign::HorizontalAlignment::Right:;
                p.rx() = pageBox.x();
                break;
            case others::PageTextAssign::HorizontalAlignment::Left:
                break;
            }
            p.rx() += doubleFromEvpu(pageTextAssign->xDisp);
            p.ry() += doubleFromEvpu(pageTextAssign->yDisp);
        }
        return p;
    };

    /* auto getMeasureForPageTextAssign = [](Page* page, PointF p, bool allowNonMeasures = true) -> MeasureBase* {
        MeasureBase* closestMB = nullptr;
        double prevDist = DBL_MAX;
        for (System* s : page->systems()) {
            for (MeasureBase* m : s->measures()) {
                if (allowNonMeasures || m->isMeasure()) {
                    if (m->ldata()->bbox().distanceTo(p) < prevDist) {
                        closestMB = m;
                        prevDist = m->ldata()->bbox().distanceTo(p);
                    }
                }
            }
        }
        return closestMB;
    };

    auto addPageTextToMeasure = [](const MusxInstance<others::PageTextAssign>& pageTextAssign, PointF p, MeasureBase* mb, Page* page) {
        PointF relativePos = p - mb->pagePos();
        // if (item->placeBelow()) {
        // ldata->setPosY(item->staff() ? item->staff()->staffHeight(item->tick()) : 0.0);
    };

    for (MusxInstance<others::PageTextAssign> pageTextAssign : remainder) {
        //@todo: use sophisticated check for whether to import as frame or not. (i.e. distance to measure is too large, frame would get in the way of music)
        if (pageTextAssign->vPos == others::PageTextAssign::VerticalAlignment::Center) {
            std::vector<page_idx_t> pagesWithText = getPages(pageTextAssign);
            for (page_idx_t i : pagesWithText) {
                Page* page = m_score->pages().at(i);
                PointF pagePosOfPageText = pagePosOfPageTextAssign(page, pageTextAssign);
                MeasureBase* mb = getMeasureForPageTextAssign(page, pagePosOfPageText);
                IF_ASSERT_FAILED (mb) {
                    // RGP: Finale pages can be blank, so this will definitely happen on the Finale side. (Check others::Page::isBlank to determine if it is blank)
                    // XM: We handle blank pages by adding a frame (which inherits MeasureBase*) and a page break. Asserting we find something should.work.
                    // log error
                    // this should never happen! all pages need at least one measurebase
                }
                addPageTextToMeasure(pageTextAssign, pagePosOfPageText, mb, page);
            }
        }
    } */

    // Don't add frames for text vertically aligned to the center.
    // if top or bottom, we should hopefully be able to check for distance to surrounding music and work from that
    // if not enough space, attempt to position based on closest measure
    //note: text is placed slightly lower than indicated position (line space?)
    // todo: read text properties but also tempo, swing, etc
    //  NOTE from RGP: tempo, swing, etc. are text expressions and will be handled separately from page text.
}

}
