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

#include "style/style.h"

#include "engraving/iengravingfontsprovider.h"
#include "engraving/dom/tuplet.h"

#include "musx/musx.h"

#include "importfinalelogger.h"

namespace mu::engraving {
class InstrumentTemplate;
class Part;
class Score;
class Staff;
}

namespace mu::iex::finale {

struct FinaleOptions
{
    void init(const FinaleParser& context);
    // common
    std::shared_ptr<const musx::dom::FontInfo> defaultMusicFont;
    musx::util::Fraction combinedDefaultStaffScaling;  // cache this so we don't need to calculate it every time
    // options
    std::shared_ptr<const musx::dom::options::AccidentalOptions> accidentalOptions;
    std::shared_ptr<const musx::dom::options::AlternateNotationOptions> alternateNotationOptions;
    std::shared_ptr<const musx::dom::options::AugmentationDotOptions> augDotOptions;
    std::shared_ptr<const musx::dom::options::BarlineOptions> barlineOptions;
    std::shared_ptr<const musx::dom::options::BeamOptions> beamOptions;
    std::shared_ptr<const musx::dom::options::ClefOptions> clefOptions;
    std::shared_ptr<const musx::dom::options::FlagOptions> flagOptions;
    std::shared_ptr<const musx::dom::options::GraceNoteOptions> graceOptions;
    std::shared_ptr<const musx::dom::options::KeySignatureOptions> keyOptions;
    std::shared_ptr<const musx::dom::options::LineCurveOptions> lineCurveOptions;
    std::shared_ptr<const musx::dom::options::MiscOptions> miscOptions;
    std::shared_ptr<const musx::dom::options::MultimeasureRestOptions> mmRestOptions;
    std::shared_ptr<const musx::dom::options::MusicSpacingOptions> musicSpacing;
    std::shared_ptr<const musx::dom::options::PageFormatOptions::PageFormat> pageFormat;
    std::shared_ptr<const musx::dom::options::PianoBraceBracketOptions> braceOptions;
    std::shared_ptr<const musx::dom::options::RepeatOptions> repeatOptions;
    std::shared_ptr<const musx::dom::options::SmartShapeOptions> smartShapeOptions;
    std::shared_ptr<const musx::dom::options::StaffOptions> staffOptions;
    std::shared_ptr<const musx::dom::options::StemOptions> stemOptions;
    std::shared_ptr<const musx::dom::options::TextOptions> textOptions;
    std::shared_ptr<const musx::dom::options::TieOptions> tieOptions;
    std::shared_ptr<const musx::dom::options::TimeSignatureOptions> timeOptions;
    std::shared_ptr<const musx::dom::options::TupletOptions> tupletOptions;
    // others that function as options
    std::shared_ptr<const musx::dom::others::LayerAttributes> layerOneAttributes;
    std::shared_ptr<const musx::dom::others::MeasureNumberRegion::ScorePartData> measNumScorePart;
    std::shared_ptr<const musx::dom::others::PartGlobals> partGlobals;
};

struct ReadableTuplet {
    engraving::Fraction startTick;
    engraving::Fraction endTick;
    std::shared_ptr<const musx::dom::details::TupletDef> musxTuplet = nullptr; // actual tuplet object. used for writing properties
    engraving::Tuplet* scoreTuplet = nullptr; // to be created tuplet object.
    int layer = 0; // for nested tuplets. 0 = outermost
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
    FontTracker(const std::shared_ptr<const musx::dom::FontInfo>& fontInfo, double additionalSizeScaling = 1.0);
    FontTracker(const engraving::MStyle& style, const engraving::String& sidNamePrefix);

    engraving::String fontName;
    double fontSize = 0.0;
    engraving::FontStyle fontStyle = engraving::FontStyle::Normal;
    bool spatiumIndependent = false;
};

struct EnigmaParsingOptions
{
    EnigmaParsingOptions() = default;
    EnigmaParsingOptions(HeaderFooterType hf) : hfType(hf)  {};

    HeaderFooterType hfType = HeaderFooterType::None;
    double scaleFontSizeBy = 1.0;
    std::optional<FontTracker> initialFont;
};

struct ReadableCustomLine
{
    ReadableCustomLine() = default;
    ReadableCustomLine(const FinaleParser&, const std::shared_ptr<musx::dom::others::SmartShapeCustomLine>&);

    engraving::ElementType elementType;
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

    engraving::TextPlace beginTextPlace;
    engraving::String beginText;
    engraving::Align beginTextAlign;
    engraving::String beginFontFamily;
    double beginFontSize;
    engraving::FontStyle beginFontStyle;
    engraving::PointF beginTextOffset;

    engraving::TextPlace continueTextPlace;
    engraving::String continueText;
    engraving::Align continueTextAlign;
    engraving::String continueFontFamily;
    double continueFontSize;
    engraving::FontStyle continueFontStyle;
    engraving::PointF continueTextOffset;

    engraving::TextPlace endTextPlace;
    engraving::String endText;
    engraving::Align endTextAlign;
    engraving::String endFontFamily;
    double endFontSize;
    engraving::FontStyle endFontStyle;
    engraving::PointF endTextOffset;

    engraving::String centerLongText;
    engraving::AlignH centerLongTextAlign; // Doesn't have vertical AlignV property
    engraving::String centerLongFontFamily;
    double centerLongFontSize;
    engraving::FontStyle centerLongFontStyle;
    engraving::PointF centerLongTextOffset;

    engraving::String centerShortText;
    engraving::AlignH centerShortTextAlign; // Doesn't have vertical AlignV property
    engraving::String centerShortFontFamily;
    double centerShortFontSize;
    engraving::FontStyle centerShortFontStyle;
    engraving::PointF centerShortTextOffset;
};
using ReadableCustomLineMap = std::map<musx::dom::Cmper, ReadableCustomLine*>;

class FinaleParser : public muse::Injectable
{
public:
    muse::Inject<mu::engraving::IEngravingFontsProvider> engravingFonts = { this };

    FinaleParser(engraving::Score* score, const std::shared_ptr<musx::dom::Document>& doc, FinaleLoggerPtr& logger);

    void parse();

    // Document
    const engraving::Score* score() const { return m_score; }
    std::shared_ptr<musx::dom::Document> musxDocument() const { return m_doc; }
    const FinaleOptions& musxOptions() const { return m_finaleOptions; }
    musx::dom::Cmper currentMusxPartId() const { return m_currentMusxPartId; }

    // Text
    engraving::String stringFromEnigmaText(const musx::util::EnigmaParsingContext& parsingContext, const EnigmaParsingOptions& options = {}, FontTracker* firstFontInfo = nullptr) const;
    bool fontIsEngravingFont(const std::string& fontName) const;
    bool fontIsEngravingFont(const std::shared_ptr<const musx::dom::FontInfo>& fontInfo) const { return fontIsEngravingFont(fontInfo->getName()); }
    bool fontIsEngravingFont(const engraving::String& fontName) const { return fontIsEngravingFont(fontName.toStdString()); }

    // Utility
    musx::dom::EvpuFloat evpuAugmentationDotWidth() const;

    FinaleLoggerPtr logger() const { return m_logger; }

private:
    // scoremap
    void importParts();
    void importBrackets();
    void importMeasures();
    void importPageLayout();
    void importStaffItems();
    engraving::Clef* createClef(engraving::Score* score,
                                const std::shared_ptr<musx::dom::others::Staff>& musxStaff,
                                engraving::staff_idx_t staffIdx,
                                musx::dom::ClefIndex musxClef,
                                engraving::Measure* measure, musx::dom::Edu musxEduPos,
                                bool afterBarline, bool visible);
    void importClefs(const std::shared_ptr<musx::dom::others::InstrumentUsed>& musxScrollViewItem,
                     const std::shared_ptr<musx::dom::others::Measure>& musxMeasure,
                     engraving::Measure* measure, engraving::staff_idx_t curStaffIdx,
                     musx::dom::ClefIndex& musxCurrClef,
                     const std::shared_ptr<musx::dom::others::Measure>& nextMusxMeasure);
    bool applyStaffSyles(engraving::StaffType* staffType, const std::shared_ptr<const musx::dom::others::StaffComposite>& currStaff);

    // entries
    void mapLayers();
    void importEntries();

    std::unordered_map<int, engraving::track_idx_t> mapFinaleVoices(const std::map<musx::dom::LayerIndex, bool>& finaleVoiceMap,
                                                         musx::dom::InstCmper curStaff, musx::dom::MeasCmper curMeas) const;
    bool processEntryInfo(musx::dom::EntryInfoPtr entryInfo, engraving::track_idx_t curTrackIdx, engraving::Measure* measure, bool graceNotes,
                          std::vector<engraving::Note*>& notesWithUnmanagedTies,
                          std::vector<ReadableTuplet>& tupletMap, std::unordered_map<engraving::Rest*, musx::dom::NoteInfoPtr>& fixedRests);
    bool processBeams(musx::dom::EntryInfoPtr entryInfoPtr, engraving::track_idx_t curTrackIdx);
    bool positionFixedRests(const std::unordered_map<engraving::Rest*, musx::dom::NoteInfoPtr>& fixedRests);
    engraving::Note* noteFromEntryInfoAndNumber(const musx::dom::EntryInfoPtr& entryInfoPtr, musx::dom::NoteNumber nn);
    engraving::Note* noteFromNoteInfoPtr(const musx::dom::NoteInfoPtr& noteInfoPtr);
    engraving::ChordRest* chordRestFromEntryInfoPtr(const musx::dom::EntryInfoPtr& entryInfoPtr);

    // styles
    void importStyles();

    // smart shapes
    void importSmartShapes();
    engraving::Score* m_score;
    const std::shared_ptr<musx::dom::Document> m_doc;
    FinaleOptions m_finaleOptions;
    FinaleLoggerPtr m_logger;
    const musx::dom::Cmper m_currentMusxPartId = musx::dom::SCORE_PARTID; // eventually this may be changed per excerpt/linked part
    bool m_smallNoteMagFound = false;
    std::unordered_map<std::string, const engraving::IEngravingFontPtr> m_engravingFonts;

    std::unordered_map<engraving::staff_idx_t, musx::dom::InstCmper> m_staff2Inst;
    std::unordered_map<musx::dom::InstCmper, engraving::staff_idx_t> m_inst2Staff;
    std::unordered_map<musx::dom::MeasCmper, engraving::Fraction> m_meas2Tick;
    std::map<engraving::Fraction, musx::dom::MeasCmper> m_tick2Meas; // use std::map to avoid need for Fraction hash function
    std::unordered_map<musx::dom::LayerIndex, engraving::voice_idx_t> m_layer2Voice;
    std::unordered_set<musx::dom::LayerIndex> m_layerForceStems;
    std::map<std::pair<musx::dom::EntryNumber, musx::dom::NoteNumber>, engraving::Note*> m_entryNoteNumber2Note; // use std::map to avoid need for std::pair hash function
    std::unordered_map<musx::dom::EntryNumber, engraving::ChordRest*> m_entryNumber2CR;
    ReadableCustomLineMap m_customLines;
};

}
