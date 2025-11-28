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

#pragma once

#include <unordered_map>
#include <memory>

#include "engraving/dom/hairpin.h"
#include "engraving/iengravingfontsprovider.h"
#include "engraving/dom/harppedaldiagram.h"
#include "engraving/dom/tuplet.h"
#include "engraving/dom/staff.h"

#include "engraving/types/types.h"

#include "musx/musx.h"

#include "importfinalelogger.h"

namespace mu::engraving {
class InstrumentTemplate;
class Part;
class Score;
}

namespace mu::iex::finale {

struct FinaleOptions
{
    void init(const FinaleParser& context);
    // common
    musx::dom::MusxInstance<musx::dom::FontInfo> defaultMusicFont;
    musx::util::Fraction combinedDefaultStaffScaling;  // cache this so we don't need to calculate it every time
    engraving::String calculatedEngravingFontName;
    // options
    musx::dom::MusxInstance<musx::dom::options::AccidentalOptions> accidentalOptions;
    musx::dom::MusxInstance<musx::dom::options::AlternateNotationOptions> alternateNotationOptions;
    musx::dom::MusxInstance<musx::dom::options::AugmentationDotOptions> augDotOptions;
    musx::dom::MusxInstance<musx::dom::options::BarlineOptions> barlineOptions;
    musx::dom::MusxInstance<musx::dom::options::BeamOptions> beamOptions;
    musx::dom::MusxInstance<musx::dom::options::ChordOptions> chordOptions;
    musx::dom::MusxInstance<musx::dom::options::ClefOptions> clefOptions;
    musx::dom::MusxInstance<musx::dom::options::FlagOptions> flagOptions;
    musx::dom::MusxInstance<musx::dom::options::GraceNoteOptions> graceOptions;
    musx::dom::MusxInstance<musx::dom::options::KeySignatureOptions> keyOptions;
    musx::dom::MusxInstance<musx::dom::options::LineCurveOptions> lineCurveOptions;
    musx::dom::MusxInstance<musx::dom::options::MiscOptions> miscOptions;
    musx::dom::MusxInstance<musx::dom::options::MultimeasureRestOptions> mmRestOptions;
    musx::dom::MusxInstance<musx::dom::options::MusicSpacingOptions> musicSpacing;
    musx::dom::MusxInstance<musx::dom::options::PageFormatOptions::PageFormat> pageFormat;
    musx::dom::MusxInstance<musx::dom::options::PianoBraceBracketOptions> braceOptions;
    musx::dom::MusxInstance<musx::dom::options::RepeatOptions> repeatOptions;
    musx::dom::MusxInstance<musx::dom::options::SmartShapeOptions> smartShapeOptions;
    musx::dom::MusxInstance<musx::dom::options::StaffOptions> staffOptions;
    musx::dom::MusxInstance<musx::dom::options::StemOptions> stemOptions;
    musx::dom::MusxInstance<musx::dom::options::TextOptions> textOptions;
    musx::dom::MusxInstance<musx::dom::options::TieOptions> tieOptions;
    musx::dom::MusxInstance<musx::dom::options::TimeSignatureOptions> timeOptions;
    musx::dom::MusxInstance<musx::dom::options::TupletOptions> tupletOptions;
    // others that function as options
    musx::dom::MusxInstance<musx::dom::others::LayerAttributes> layerOneAttributes;
    musx::dom::MusxInstance<musx::dom::others::MeasureNumberRegion::ScorePartData> measNumScorePart;
    musx::dom::MusxInstance<musx::dom::others::PartGlobals> partGlobals;
};

struct ReadableTuplet {
    engraving::Fraction startTick;
    engraving::Fraction endTick;
    musx::dom::MusxInstance<musx::dom::details::TupletDef> musxTuplet = nullptr; // actual tuplet object. used for writing properties
    engraving::Tuplet* scoreTuplet = nullptr; // to be created tuplet object.
    int layer = 0; // for nested tuplets. 0 = outermost
};

struct FrameSettings {
    FrameSettings() = default;
    FrameSettings(const musx::dom::others::Enclosure* enclosure);
    engraving::FrameType frameType = engraving::FrameType::NO_FRAME;
    double frameWidth = 0.1;
    double paddingWidth = 0.2;
    int frameRound = 0;
};

enum class HeaderFooterType {
    None,
    FirstPage,
    SecondPageToEnd
};

struct FontTracker
{
    FontTracker() = default;
    FontTracker(const engraving::String& name, double size, engraving::FontStyle styles = engraving::FontStyle::Normal, bool spatiumInd = false)
        : fontName(name), fontSize(size), fontStyle(styles), spatiumIndependent(spatiumInd) {}
    FontTracker(const musx::dom::MusxInstance<musx::dom::FontInfo>& fontInfo, double additionalSizeScaling = 1.0);
    FontTracker(const engraving::MStyle& style, const engraving::String& sidNamePrefix);

    static FontTracker fromEngravingFont(const engraving::MStyle& style, engraving::Sid styleId = engraving::Sid::musicalSymbolFont, double scaling = 1.0);

    muse::draw::FontMetrics toFontMetrics(double mag);

    engraving::String fontName;
    double fontSize = 0.0;
    engraving::FontStyle fontStyle = engraving::FontStyle::Normal;
    bool spatiumIndependent = false;

    bool operator==(const FontTracker& src) const {
        return fontName == src.fontName && muse::RealIsEqual(fontSize, src.fontSize)
               && fontStyle == src.fontStyle && spatiumIndependent == src.spatiumIndependent;
    }
    bool operator!=(const FontTracker& src) const { return !(*this == src); }
};

struct EnigmaParsingOptions
{
    EnigmaParsingOptions() = default;
    EnigmaParsingOptions(HeaderFooterType hf) : hfType(hf)  {};

    HeaderFooterType hfType = HeaderFooterType::None;
    double scaleFontSizeBy = 1.0;
    std::optional<FontTracker> initialFont;         ///< This is the default text font for the text we are parsing
    std::optional<FontTracker> musicSymbolFont;     ///< This is the default font for musical `<sym>` tags
    bool plainText = false;
    bool convertSymbols = true;
};

struct MusxEmbeddedGraphic {
    engraving::String fileName;        // <cmper-value>.ext
    muse::ByteArray fileData;
};
using MusxEmbeddedGraphicsMap = std::unordered_map<musx::dom::Cmper, MusxEmbeddedGraphic>;

struct ReadableExpression
{
    ReadableExpression() = default;
    ReadableExpression(const FinaleParser&, const musx::dom::MusxInstance<musx::dom::others::TextExpressionDef>&);

    engraving::String xmlText = engraving::String();
    FrameSettings frameSettings;
    engraving::ElementType elementType = engraving::ElementType::STAFF_TEXT;

    // Element-specific
    engraving::DynamicType dynamicType = engraving::DynamicType::OTHER;
    std::array<engraving::PedalPosition, engraving::HARP_STRING_NO> pedalState;
};
using ReadableExpressionMap = std::map<musx::dom::Cmper, ReadableExpression*>;

struct ReadableRepeatText
{
    ReadableRepeatText() = default;
    ReadableRepeatText(const FinaleParser&, const musx::dom::MusxInstance<musx::dom::others::TextRepeatDef>&);

    engraving::String xmlText = engraving::String();
    FrameSettings frameSettings;
    engraving::ElementType elementType = engraving::ElementType::INVALID;
    engraving::AlignH repeatAlignment;

    // Element-specific
    engraving::MarkerType markerType = engraving::MarkerType::USER;
    engraving::JumpType jumpType = engraving::JumpType::USER;
};
using ReadableRepeatTextMap = std::map<musx::dom::Cmper, ReadableRepeatText*>;

struct ReadableCustomLine
{
    ReadableCustomLine() = default;
    ReadableCustomLine(const FinaleParser&, const musx::dom::MusxInstance<musx::dom::others::SmartShapeCustomLine>&);

    // General
    engraving::ElementType elementType;
    bool playSpanner = true;

    // Line and hook settings
    bool lineVisible;
    engraving::HookType beginHookType;
    engraving::HookType endHookType;
    engraving::Spatium beginHookHeight;
    engraving::Spatium endHookHeight;
    engraving::Spatium gapBetweenTextAndLine;
    bool textSizeSpatiumDependent;
    bool diagonal;
    engraving::LineType lineStyle;
    engraving::Spatium lineWidth;
    double dashLineLen;
    double dashGapLen;

    // Type-specific settings
    engraving::TrillType trillType;
    engraving::VibratoType vibratoType;
    // OttavaType requires placement, so must be computed after layout
    engraving::GlissandoType glissandoType = engraving::GlissandoType::STRAIGHT;
    engraving::HairpinType hairpinType = engraving::HairpinType::CRESC_LINE;

    // Begin text
    engraving::TextPlace beginTextPlace;
    engraving::String beginText;
    engraving::Align beginTextAlign;
    engraving::String beginFontFamily;
    double beginFontSize;
    engraving::FontStyle beginFontStyle;
    engraving::PointF beginTextOffset;

    // Continue text
    engraving::TextPlace continueTextPlace;
    engraving::String continueText;
    engraving::Align continueTextAlign;
    engraving::String continueFontFamily;
    double continueFontSize;
    engraving::FontStyle continueFontStyle;
    engraving::PointF continueTextOffset;

    // End text
    engraving::TextPlace endTextPlace;
    engraving::String endText;
    engraving::Align endTextAlign;
    engraving::String endFontFamily;
    double endFontSize;
    engraving::FontStyle endFontStyle;
    engraving::PointF endTextOffset;

    // Center text (long), unused
    engraving::String centerLongText;
    engraving::AlignH centerLongTextAlign; // Doesn't have vertical AlignV property
    engraving::String centerLongFontFamily;
    double centerLongFontSize;
    engraving::FontStyle centerLongFontStyle;
    engraving::PointF centerLongTextOffset;

    // Center text (short), unused
    engraving::String centerShortText;
    engraving::AlignH centerShortTextAlign; // Doesn't have vertical AlignV property
    engraving::String centerShortFontFamily;
    double centerShortFontSize;
    engraving::FontStyle centerShortFontStyle;
    engraving::PointF centerShortTextOffset;
};
using ReadableCustomLineMap = std::map<musx::dom::Cmper, ReadableCustomLine*>;

using Chord = mu::engraving::Chord; // seemingly needed for Windows builds (20251003)

class FinaleParser : public muse::Injectable
{
public:
    muse::Inject<mu::engraving::IEngravingFontsProvider> engravingFonts = { this };

    FinaleParser(engraving::Score* score, const std::shared_ptr<musx::dom::Document>& doc, MusxEmbeddedGraphicsMap&& graphics, FinaleLoggerPtr& logger);

    void parse();

    // Document
    const engraving::Score* score() const { return m_score; }
    std::shared_ptr<musx::dom::Document> musxDocument() const { return m_doc; }
    const FinaleOptions& musxOptions() const { return m_finaleOptions; }
    musx::dom::Cmper currentMusxPartId() const { return m_currentMusxPartId; }
    bool partScore() const { return m_currentMusxPartId != musx::dom::SCORE_PARTID; }

    // Text
    engraving::String stringFromEnigmaText(const musx::util::EnigmaParsingContext& parsingContext, const EnigmaParsingOptions& options = {}, FontTracker* firstFontInfo = nullptr) const;
    bool fontIsEngravingFont(const std::string& fontName) const;
    bool fontIsEngravingFont(const musx::dom::MusxInstance<musx::dom::FontInfo>& fontInfo) const { return fontIsEngravingFont(fontInfo->getName()); }
    bool fontIsEngravingFont(const engraving::String& fontName) const { return fontIsEngravingFont(fontName.toStdString()); }

    // Utility
    musx::dom::EvpuFloat evpuAugmentationDotWidth() const;
    engraving::staff_idx_t staffIdxFromAssignment(musx::dom::StaffCmper assign);
    engraving::staff_idx_t staffIdxForRepeats(bool onlyTop, musx::dom::Cmper staffList, musx::dom::Cmper measureId,
                                              std::vector<std::pair<engraving::staff_idx_t, musx::dom::StaffCmper>>& links);
    musx::dom::MusxInstance<musx::dom::others::LayerAttributes> layerAttributes(const engraving::Fraction& tick, engraving::track_idx_t track);

    FinaleLoggerPtr logger() const { return m_logger; }

private:
    // scoremap
    void importParts();
    void importBrackets();
    void importMeasures();
    void importPageLayout();
    void importStaffItems();
    void importBarlines();

    engraving::Staff* createStaff(engraving::Part* part, const musx::dom::MusxInstance<musx::dom::others::Staff> musxStaff, const engraving::InstrumentTemplate* it = nullptr);
    engraving::Clef* createClef(const musx::dom::MusxInstance<musx::dom::others::Staff>& musxStaff,
                                engraving::staff_idx_t staffIdx, musx::dom::ClefIndex musxClef,
                                engraving::Measure* measure, musx::dom::Edu musxEduPos,
                                bool afterBarline, bool visible);
    void importClefs(const musx::dom::MusxInstance<musx::dom::others::StaffUsed>& musxScrollViewItem,
                     const musx::dom::MusxInstance<musx::dom::others::Measure>& musxMeasure,
                     engraving::Measure* measure, engraving::staff_idx_t curStaffIdx,
                     musx::dom::ClefIndex& musxCurrClef,
                     const musx::dom::MusxInstance<musx::dom::others::Measure>& nextMusxMeasure);
    bool collectStaffType(engraving::StaffType* staffType, const musx::dom::MusxInstance<musx::dom::others::StaffComposite>& currStaff);

    // entries
    void mapLayers();
    void importEntries();
    void importEntryAdjustments();
    void importArticulations();

    std::unordered_map<int, engraving::track_idx_t> mapFinaleVoices(const std::map<musx::dom::LayerIndex, bool>& finaleVoiceMap,
                                                         musx::dom::StaffCmper curStaff, musx::dom::MeasCmper curMeas) const;
    void createTupletsFromMap(engraving::Measure* measure, engraving::track_idx_t curTrackIdx, std::vector<ReadableTuplet>& tupletMap);
    bool processEntryInfo(musx::dom::EntryInfoPtr entryInfo, engraving::track_idx_t curTrackIdx, engraving::Measure* measure, bool graceNotes,
                          std::vector<engraving::Note*>& notesWithUnmanagedTies,
                          std::vector<ReadableTuplet>& tupletMap);
    bool processBeams(musx::dom::EntryInfoPtr entryInfoPtr, engraving::track_idx_t curTrackIdx);
    bool calculateUp(const musx::dom::MusxInstance<musx::dom::details::ArticulationAssign>& articAssign,
                     musx::dom::others::ArticulationDef::AutoVerticalMode vm, engraving::ChordRest* cr);
    engraving::DirectionV getDirectionVForLayer(const engraving::ChordRest* e);
    engraving::DirectionV calculateTieDirection(engraving::Tie* tie, musx::dom::EntryNumber entryNumber);
    engraving::Note* noteFromEntryInfoAndNumber(const musx::dom::EntryInfoPtr& entryInfoPtr, musx::dom::NoteNumber nn);
    engraving::Note* noteFromNoteInfoPtr(const musx::dom::NoteInfoPtr& noteInfoPtr);
    engraving::ChordRest* chordRestFromEntryInfoPtr(const musx::dom::EntryInfoPtr& entryInfoPtr);

    // styles
    void importStyles();
    void collectElementStyle(const mu::engraving::EngravingObject* e);
    void collectGlobalProperty(const mu::engraving::Sid styleId, const mu::engraving::PropertyValue& newV);
    void collectGlobalFont(const std::string& namePrefix, const musx::dom::MusxInstance<musx::dom::FontInfo>& fontInfo);
    std::unordered_map<engraving::Sid, engraving::PropertyValue> m_elementStyles;

    // smart shapes
    void importSmartShapes();
    engraving::DirectionV calculateSlurDirection(engraving::Slur* slur);

    // texts
    void importTextExpressions();
    void importPageTexts();
    void rebasePageTextOffsets();
    void importChordsFrets(const musx::dom::MusxInstance<musx::dom::others::StaffUsed>& musxScrollViewItem,
                           const musx::dom::MusxInstance<musx::dom::others::Measure>& musxMeasure,
                           engraving::Staff* staff, engraving::Measure* measure);

    bool isOnlyPage(const musx::dom::MusxInstance<musx::dom::others::PageTextAssign>& pageTextAssign, musx::dom::PageCmper page);

    engraving::Score* m_score;
    const std::shared_ptr<musx::dom::Document> m_doc;
    FinaleOptions m_finaleOptions;
    FinaleLoggerPtr m_logger;
    const musx::dom::Cmper m_currentMusxPartId = musx::dom::SCORE_PARTID; // eventually this may be changed per excerpt/linked part
    bool m_smallNoteMagFound = false;
    std::unordered_map<std::string, const engraving::IEngravingFontPtr> m_engravingFonts;

    MusxEmbeddedGraphicsMap m_embeddedGraphics;
    std::unordered_map<engraving::staff_idx_t, musx::dom::StaffCmper> m_staff2Inst;
    std::unordered_map<musx::dom::StaffCmper, engraving::staff_idx_t> m_inst2Staff;
    std::unordered_map<musx::dom::MeasCmper, engraving::Fraction> m_meas2Tick;
    std::map<engraving::Fraction, musx::dom::MeasCmper> m_tick2Meas; // use std::map to avoid need for Fraction hash function
    std::unordered_map<musx::dom::LayerIndex, engraving::voice_idx_t> m_layer2Voice;
    std::map<std::pair<musx::dom::EntryNumber, musx::dom::NoteNumber>, engraving::Note*> m_entryNoteNumber2Note; // use std::map to avoid need for std::pair hash function
    std::unordered_map<musx::dom::EntryNumber, engraving::ChordRest*> m_entryNumber2CR;
    std::vector<std::map<int, musx::dom::LayerIndex>> m_track2Layer;
    std::set<engraving::Chord*> m_fixedChords;
    ReadableCustomLineMap m_customLines;
    ReadableExpressionMap m_expressions;
    ReadableRepeatTextMap m_repeatTexts;
    std::set<engraving::staff_idx_t> m_systemObjectStaves;
};

extern void setAndStyleProperty(mu::engraving::EngravingObject* e, mu::engraving::Pid id,
                                mu::engraving::PropertyValue v, bool inheritStyle = false);

}
