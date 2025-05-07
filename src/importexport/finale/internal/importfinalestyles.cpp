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
#include "dom/mscore.h"
#include "dom/textbase.h"
#include "dom/types.h"
#include "internal/importfinalelogger.h"
#include "internal/importfinalescoremap.h"
#include "internal/finaletypesconv.h"
#include "musx/musx.h"

using namespace mu::engraving;
using namespace musx::dom;

namespace mu::iex::finale {

// Finale uses music font size 24 to fill a space.
// MuseScore uses music font size 20 to fill a space.
constexpr static double MUSE_FINALE_SCALE_DIFFERENTIAL = 20.0 / 24.0;

static const std::set<std::string_view> museScoreSMuFLFonts{
    "Bravura",
    "Leland",
    "Emmentaler",
    "Gonville",
    "MuseJazz",
    "Petaluma",
    "Finale Maestro",
    "Finale Broadway"
};

struct FinalePreferences
{
    FinalePreferences(const EnigmaXmlImporter& context) :
        document(context.musxDocument()), logger(context.logger()) {}

    DocumentPtr document;
    std::shared_ptr<FinaleLogger> logger;
    std::shared_ptr<FontInfo> defaultMusicFont;
    Cmper forPartId{};
    // options
    std::shared_ptr<options::AccidentalOptions> accidentalOptions;
    std::shared_ptr<options::AlternateNotationOptions> alternateNotationOptions;
    std::shared_ptr<options::AugmentationDotOptions> augDotOptions;
    std::shared_ptr<options::BarlineOptions> barlineOptions;
    std::shared_ptr<options::BeamOptions> beamOptions;
    std::shared_ptr<options::ClefOptions> clefOptions;
    std::shared_ptr<options::FlagOptions> flagOptions;
    std::shared_ptr<options::GraceNoteOptions> graceOptions;
    std::shared_ptr<options::KeySignatureOptions> keyOptions;
    std::shared_ptr<options::LineCurveOptions> lineCurveOptions;
    std::shared_ptr<options::MiscOptions> miscOptions;
    std::shared_ptr<options::MultimeasureRestOptions> mmRestOptions;
    std::shared_ptr<options::MusicSpacingOptions> musicSpacing;
    std::shared_ptr<options::PageFormatOptions::PageFormat> pageFormat;
    std::shared_ptr<options::PianoBraceBracketOptions> braceOptions;
    std::shared_ptr<options::RepeatOptions> repeatOptions;
    std::shared_ptr<options::SmartShapeOptions> smartShapeOptions;
    std::shared_ptr<options::StaffOptions> staffOptions;
    std::shared_ptr<options::StemOptions> stemOptions;
    std::shared_ptr<options::TieOptions> tieOptions;
    std::shared_ptr<options::TimeSignatureOptions> timeOptions;
    std::shared_ptr<options::TupletOptions> tupletOptions;
    // others that function as options
    std::shared_ptr<others::LayerAttributes> layerOneAttributes;
    std::shared_ptr<others::MeasureNumberRegion::ScorePartData> measNumScorePart;
    std::shared_ptr<others::PartGlobals> partGlobals;
};

template <typename T>
static std::shared_ptr<T> getDocOptions(const EnigmaXmlImporter& context, const std::string& prefsName)
{
    auto result = context.musxDocument()->getOptions()->get<T>();
    if (!result) {
        throw std::invalid_argument("document contains no default " + prefsName);
    }
    return result;
}

static FinalePreferences getCurrentPrefs(const EnigmaXmlImporter& context, Cmper forPartId)
{
    FinalePreferences retval(context);
    retval.forPartId = forPartId;

    auto fontOptions = getDocOptions<options::FontOptions>(context, "font");
    retval.defaultMusicFont = fontOptions->getFontInfo(options::FontOptions::FontType::Music);
    //
    retval.accidentalOptions = getDocOptions<options::AccidentalOptions>(context, "accidental");
    retval.alternateNotationOptions = getDocOptions<options::AlternateNotationOptions>(context, "alternate notation");
    retval.augDotOptions = getDocOptions<options::AugmentationDotOptions>(context, "augmentation dot");
    retval.barlineOptions = getDocOptions<options::BarlineOptions>(context, "barline");
    retval.beamOptions = getDocOptions<options::BeamOptions>(context, "beam");
    retval.clefOptions = getDocOptions<options::ClefOptions>(context, "clef");
    retval.flagOptions = getDocOptions<options::FlagOptions>(context, "flag");
    retval.graceOptions = getDocOptions<options::GraceNoteOptions>(context, "grace note");
    retval.keyOptions = getDocOptions<options::KeySignatureOptions>(context, "key signature");
    retval.lineCurveOptions = getDocOptions<options::LineCurveOptions>(context, "lines & curves");
    retval.miscOptions = getDocOptions<options::MiscOptions>(context, "miscellaneous");
    retval.mmRestOptions = getDocOptions<options::MultimeasureRestOptions>(context, "multimeasure rest");
    retval.musicSpacing = getDocOptions<options::MusicSpacingOptions>(context, "music spacing");
    auto pageFormatOptions = getDocOptions<options::PageFormatOptions>(context, "page format");
    retval.pageFormat = pageFormatOptions->calcPageFormatForPart(forPartId);
    retval.braceOptions = getDocOptions<options::PianoBraceBracketOptions>(context, "piano braces & brackets");
    retval.repeatOptions = getDocOptions<options::RepeatOptions>(context, "repeat");
    retval.smartShapeOptions = getDocOptions<options::SmartShapeOptions>(context, "smart shape");
    retval.staffOptions = getDocOptions<options::StaffOptions>(context, "staff");
    retval.stemOptions = getDocOptions<options::StemOptions>(context, "stem");
    retval.tieOptions = getDocOptions<options::TieOptions>(context, "tie");
    retval.timeOptions = getDocOptions<options::TimeSignatureOptions>(context, "time signature");
    retval.tupletOptions = getDocOptions<options::TupletOptions>(context, "tuplet");
    //
    retval.layerOneAttributes = context.musxDocument()->getOthers()->get<others::LayerAttributes>(forPartId, 0);
    if (!retval.layerOneAttributes) {
        throw std::invalid_argument("document contains no options for Layer 1");
    }
    auto measNumRegions = context.musxDocument()->getOthers()->getArray<others::MeasureNumberRegion>(forPartId);
    if (measNumRegions.size() > 0) {
        retval.measNumScorePart = (forPartId && measNumRegions[0]->useScoreInfoForPart && measNumRegions[0]->partData)
        ? measNumRegions[0]->partData
        : measNumRegions[0]->scoreData;
        if (!retval.measNumScorePart) {
            throw std::invalid_argument("document contains no ScorePartData for measure number region " + std::to_string(measNumRegions[0]->getCmper()));
        }
    }
    retval.partGlobals = context.musxDocument()->getOthers()->get<others::PartGlobals>(forPartId, MUSX_GLOBALS_CMPER);
    if (!retval.layerOneAttributes) {
        throw std::invalid_argument("document contains no options for Layer 1");
    }

    return retval;
}

static uint16_t museFontEfx(const FontInfo* fontInfo)
{
    uint16_t retval = 0;

    if (fontInfo->bold) { retval |= 0x01; }
    if (fontInfo->italic) { retval |= 0x02; }
    if (fontInfo->underline) { retval |= 0x04; }
    if (fontInfo->strikeout) { retval |= 0x08; }

    return retval;
}

static double museMagVal(const FinalePreferences& prefs, const options::FontOptions::FontType type)
{
    auto fontPrefs = options::FontOptions::getFontInfo(prefs.document, type);
    if (fontPrefs->getName() == prefs.defaultMusicFont->getName()) {
        return double(fontPrefs->fontSize) / double(prefs.defaultMusicFont->fontSize);
    }
    return 1.0;
}

static Sid styleIdx(const std::string& name)
{
    auto nameStr = String::fromStdString(name);
    return MStyle::styleIdx(nameStr);
}

static void writeFontPref(MStyle& style, const std::string& namePrefix, const FontInfo* fontInfo)
{
    style.set(styleIdx(namePrefix + "FontFace"), String::fromStdString(fontInfo->getName()));
    style.set(styleIdx(namePrefix + "FontSize"),
                    double(fontInfo->fontSize) * (fontInfo->absolute ? 1.0 : MUSE_FINALE_SCALE_DIFFERENTIAL));
    style.set(styleIdx(namePrefix + "FontSpatiumDependent"), !fontInfo->absolute);
    style.set(styleIdx(namePrefix + "FontStyle"), museFontEfx(fontInfo));
}

static void writeDefaultFontPref(MStyle& style, const FinalePreferences& prefs, const std::string& namePrefix, options::FontOptions::FontType type)
{
    auto fontPrefs = options::FontOptions::getFontInfo(prefs.document, type);
    writeFontPref(style, namePrefix, fontPrefs.get());
}

void writeLinePrefs(MStyle& style,
                    const std::string& namePrefix,
                    double widthEfix,
                    double dashLength,
                    double dashGap,
                    const std::optional<LineType>& lineStyle = std::nullopt)
{
    const double lineWidthEvpu = widthEfix / EFIX_PER_EVPU;
    style.set(styleIdx(namePrefix + "LineWidth"), widthEfix / EFIX_PER_SPACE);
    if (lineStyle) {
        style.set(styleIdx(namePrefix + "LineStyle"), lineStyle.value());
    }
    style.set(styleIdx(namePrefix + "DashLineLen"), dashLength / lineWidthEvpu);
    style.set(styleIdx(namePrefix + "DashGapLen"), dashGap / lineWidthEvpu);
}

static void writeFramePrefs(MStyle& style, const std::string& namePrefix, const others::Enclosure* enclosure = nullptr)
{
    if (!enclosure || enclosure->shape == others::Enclosure::Shape::NoEnclosure || enclosure->lineWidth == 0) {
        style.set(styleIdx(namePrefix + "FrameType"), int(FrameType::NO_FRAME));
        if (!enclosure) return; // Do not override any other defaults if no enclosure
    } else if (enclosure->shape == others::Enclosure::Shape::Ellipse) {
        style.set(styleIdx(namePrefix + "FrameType"), int(FrameType::CIRCLE));
    } else {
        style.set(styleIdx(namePrefix + "FrameType"), int(FrameType::SQUARE));
    }
    style.set(styleIdx(namePrefix + "FramePadding"), enclosure->xMargin / EVPU_PER_SPACE);
    style.set(styleIdx(namePrefix + "FrameWidth"), enclosure->lineWidth / EFIX_PER_SPACE);
    style.set(styleIdx(namePrefix + "FrameRound"),
              enclosure->roundCorners ? enclosure->cornerRadius / EFIX_PER_EVPU : 0.0);
}

static void writeCategoryTextFontPref(MStyle& style, const FinalePreferences& prefs, const std::string& namePrefix, others::MarkingCategory::CategoryType categoryType)
{
    auto cat = prefs.document->getOthers()->get<others::MarkingCategory>(prefs.forPartId, Cmper(categoryType));
    if (!cat) {
        prefs.logger->logWarning(String::fromStdString("unable to load category def for " + namePrefix));
        return;
    }
    if (!cat->textFont) {
        prefs.logger->logWarning(String::fromStdString("marking category " + cat->getName() + " has no text font."));
        return;
    }
    writeFontPref(style, namePrefix, cat->textFont.get());
    for (auto& it : cat->textExpressions) {
        if (auto exp = it.second.lock()) {
            writeFramePrefs(style, namePrefix, exp->getEnclosure().get());
            break;
        } else {
            prefs.logger->logWarning(String::fromStdString("marking category " + cat->getName() + " has invalid text expression."));
        }
    }
}

static void writePagePrefs(MStyle& style, const FinalePreferences& prefs)
{
    const auto& pagePrefs = prefs.pageFormat;

    style.set(Sid::pageWidth, double(pagePrefs->pageWidth) / EVPU_PER_INCH);
    style.set(Sid::pageHeight, double(pagePrefs->pageHeight) / EVPU_PER_INCH);
    style.set(Sid::pagePrintableWidth,
               double(pagePrefs->pageWidth - pagePrefs->leftPageMarginLeft + pagePrefs->leftPageMarginRight) / EVPU_PER_INCH);
    style.set(Sid::pageEvenLeftMargin, pagePrefs->leftPageMarginLeft / EVPU_PER_INCH);
    style.set(Sid::pageOddLeftMargin,
               double(pagePrefs->facingPages ? pagePrefs->rightPageMarginLeft : pagePrefs->leftPageMarginLeft) / EVPU_PER_INCH);
    style.set(Sid::pageEvenTopMargin, double(-pagePrefs->leftPageMarginTop) / EVPU_PER_INCH);
    style.set(Sid::pageEvenBottomMargin, double(pagePrefs->leftPageMarginBottom) / EVPU_PER_INCH);
    style.set(Sid::pageOddTopMargin,
               double(pagePrefs->facingPages ? -pagePrefs->rightPageMarginTop : -pagePrefs->leftPageMarginTop) / EVPU_PER_INCH);
    style.set(Sid::pageOddBottomMargin,
               double(pagePrefs->facingPages ? pagePrefs->rightPageMarginBottom : pagePrefs->leftPageMarginBottom) / EVPU_PER_INCH);
    style.set(Sid::pageTwosided, pagePrefs->facingPages);
    style.set(Sid::enableIndentationOnFirstSystem, pagePrefs->differentFirstSysMargin);
    style.set(Sid::firstSystemIndentationValue, double(pagePrefs->firstSysMarginLeft) / EVPU_PER_SPACE);

    // Calculate Spatium
    const double pagePercent = double(pagePrefs->pagePercent) / 100.0;
    const double staffPercent = (double(pagePrefs->rawStaffHeight) / (EVPU_PER_SPACE * 4 * 16)) * (double(pagePrefs->sysPercent) / 100.0);
    style.set(Sid::spatium, ((EVPU_PER_SPACE * staffPercent * pagePercent) / EVPU_PER_MM) * DPMM);

    // Default music font
    const auto& defaultMusicFont = prefs.defaultMusicFont;
    const bool isSMuFL = [defaultMusicFont]() -> bool {
        if (defaultMusicFont->calcIsSMuFL())
            return true;
        auto it = museScoreSMuFLFonts.find(defaultMusicFont->getName());
        return it != museScoreSMuFLFonts.end();
    }();
    const String musicFontName = [&]() {
        if (isSMuFL) {
            return String::fromStdString(defaultMusicFont->getName());
        } else if (defaultMusicFont->getName() == "Maestro") {
            return String("Finale Maestro");
        } // other `else if` checks as required go here
        return String();
    }();
    if (!musicFontName.empty()) {
        style.set(Sid::musicalSymbolFont, musicFontName);
        style.set(Sid::musicalTextFont, musicFontName + " Text");
    }
}

static void writeLyricsPrefs(MStyle& style, const FinalePreferences& prefs)
{
    auto fontInfo = options::FontOptions::getFontInfo(prefs.document, options::FontOptions::FontType::LyricVerse);
    for (auto [verseNumber, evenOdd] : {
             std::make_pair(1, "Odd"),
             std::make_pair(2, "Even")
         }) {
        auto verseText = prefs.document->getTexts()->get<texts::LyricsVerse>(Cmper(verseNumber));
        if (verseText && !verseText->text.empty()) {
            auto font = verseText->parseFirstFontInfo();
            if (font) {
                fontInfo = font;
            }
        }
        writeFontPref(style, "lyrics" + std::string(evenOdd), fontInfo.get());
    }
}

void writeLineMeasurePrefs(MStyle& style, const FinalePreferences& prefs)
{
    using RepeatWingStyle = options::RepeatOptions::WingStyle;

    style.set(Sid::barWidth, prefs.barlineOptions->barlineWidth / EFIX_PER_SPACE);
    style.set(Sid::doubleBarWidth, prefs.barlineOptions->barlineWidth / EFIX_PER_SPACE);
    style.set(Sid::endBarWidth, prefs.barlineOptions->thickBarlineWidth / EFIX_PER_SPACE);

    // these calculations are based on observed behavior
    style.set(Sid::doubleBarDistance,
               (prefs.barlineOptions->doubleBarlineSpace - prefs.barlineOptions->barlineWidth) / EFIX_PER_SPACE);
    style.set(Sid::endBarDistance, prefs.barlineOptions->finalBarlineSpace / EFIX_PER_SPACE);
    style.set(Sid::repeatBarlineDotSeparation, prefs.repeatOptions->forwardDotHPos / EVPU_PER_SPACE);
    style.set(Sid::repeatBarTips, prefs.repeatOptions->wingStyle != RepeatWingStyle::None);

    style.set(Sid::startBarlineSingle, prefs.barlineOptions->drawLeftBarlineSingleStaff);
    style.set(Sid::startBarlineMultiple, prefs.barlineOptions->drawLeftBarlineMultipleStaves);

    style.set(Sid::bracketWidth, 0.5); // Hard-coded in Finale
    style.set(Sid::bracketDistance, -(prefs.braceOptions->defBracketPos) / EVPU_PER_SPACE);
    style.set(Sid::akkoladeBarDistance, -prefs.braceOptions->defBracketPos / EVPU_PER_SPACE);

    style.set(Sid::clefLeftMargin, prefs.clefOptions->clefFrontSepar / EVPU_PER_SPACE);
    style.set(Sid::keysigLeftMargin, prefs.keyOptions->keyFront / EVPU_PER_SPACE);

    const double timeSigSpaceBefore = prefs.forPartId
                                          ? prefs.timeOptions->timeFrontParts
                                          : prefs.timeOptions->timeFront;
    style.set(Sid::timesigLeftMargin, timeSigSpaceBefore / EVPU_PER_SPACE);

    style.set(Sid::clefKeyDistance,
               (prefs.clefOptions->clefBackSepar + prefs.clefOptions->clefKeySepar + prefs.keyOptions->keyFront) / EVPU_PER_SPACE);
    style.set(Sid::clefTimesigDistance,
               (prefs.clefOptions->clefBackSepar + prefs.clefOptions->clefTimeSepar + timeSigSpaceBefore) / EVPU_PER_SPACE);
    style.set(Sid::keyTimesigDistance,
               (prefs.keyOptions->keyBack + prefs.keyOptions->keyTimeSepar + timeSigSpaceBefore) / EVPU_PER_SPACE);
    style.set(Sid::keyBarlineDistance, prefs.repeatOptions->afterKeySpace / EVPU_PER_SPACE);

    // Skipped: systemHeaderDistance, systemHeaderTimeSigDistance: these do not translate well from Finale

    style.set(Sid::clefBarlineDistance, prefs.repeatOptions->afterClefSpace / EVPU_PER_SPACE);
    style.set(Sid::timesigBarlineDistance, prefs.repeatOptions->afterClefSpace / EVPU_PER_SPACE);

    style.set(Sid::measureRepeatNumberPos, -(prefs.alternateNotationOptions->twoMeasNumLift + 0.5) / EVPU_PER_SPACE);
    style.set(Sid::staffLineWidth, prefs.lineCurveOptions->staffLineWidth / EFIX_PER_SPACE);
    style.set(Sid::ledgerLineWidth, prefs.lineCurveOptions->legerLineWidth / EFIX_PER_SPACE);
    style.set(Sid::ledgerLineLength,
               (prefs.lineCurveOptions->legerFrontLength + prefs.lineCurveOptions->legerBackLength) / (2 * EVPU_PER_SPACE));
    style.set(Sid::keysigAccidentalDistance, (prefs.keyOptions->acciAdd + 4) / EVPU_PER_SPACE);  // Observed fudge factor
    style.set(Sid::keysigNaturalDistance, (prefs.keyOptions->acciAdd + 6) / EVPU_PER_SPACE);     // Observed fudge factor

    style.set(Sid::smallClefMag, prefs.clefOptions->clefChangePercent / 100.0);
    style.set(Sid::genClef, !prefs.clefOptions->showClefFirstSystemOnly);
    style.set(Sid::genKeysig, !prefs.keyOptions->showKeyFirstSystemOnly);
    style.set(Sid::genCourtesyTimesig, prefs.timeOptions->cautionaryTimeChanges);
    style.set(Sid::genCourtesyKeysig, prefs.keyOptions->cautionaryKeyChanges);
    style.set(Sid::genCourtesyClef, prefs.clefOptions->cautionaryClefChanges);

    style.set(Sid::keySigCourtesyBarlineMode,
              int(FinaleTConv::boolToCourtesyBarlineMode(prefs.barlineOptions->drawDoubleBarlineBeforeKeyChanges)));
    style.set(Sid::timeSigCourtesyBarlineMode, int(CourtesyBarlineMode::ALWAYS_SINGLE));  // Hard-coded as 0 in Finale
    style.set(Sid::hideEmptyStaves, !prefs.forPartId);
}

void writeStemPrefs(MStyle& style, const FinalePreferences& prefs)
{
    style.set(Sid::useStraightNoteFlags, prefs.flagOptions->straightFlags);
    style.set(Sid::stemWidth, prefs.stemOptions->stemWidth / EFIX_PER_SPACE);
    style.set(Sid::shortenStem, true);
    style.set(Sid::stemLength, prefs.stemOptions->stemLength / EVPU_PER_SPACE);
    style.set(Sid::shortestStem, prefs.stemOptions->shortStemLength / EVPU_PER_SPACE);
    style.set(Sid::stemSlashThickness, prefs.graceOptions->graceSlashWidth / EFIX_PER_SPACE);
}

void writeMusicSpacingPrefs(MStyle& style, const FinalePreferences& prefs)
{
    style.set(Sid::minMeasureWidth, prefs.musicSpacing->minWidth / EVPU_PER_SPACE);
    style.set(Sid::minNoteDistance, prefs.musicSpacing->minDistance / EVPU_PER_SPACE);
    style.set(Sid::measureSpacing, prefs.musicSpacing->scalingFactor);
    /// @todo find a conversion for note distance to tie length.
    style.set(Sid::minTieLength, prefs.musicSpacing->minDistTiedNotes / EVPU_PER_SPACE);
}

void writeNoteRelatedPrefs(MStyle& style, const FinalePreferences& prefs)
{
    style.set(Sid::accidentalDistance, prefs.accidentalOptions->acciAcciSpace / EVPU_PER_SPACE);
    style.set(Sid::accidentalNoteDistance, prefs.accidentalOptions->acciNoteSpace / EVPU_PER_SPACE);
    style.set(Sid::beamWidth, prefs.beamOptions->beamWidth / EFIX_PER_SPACE);
    style.set(Sid::useWideBeams, prefs.beamOptions->beamSepar > (0.75 * EVPU_PER_SPACE));

    // Finale randomly adds twice the stem width to the length of a beam stub. (Observed behavior)
    style.set(Sid::beamMinLen,
              (prefs.beamOptions->beamStubLength + (2.0 * prefs.stemOptions->stemWidth / EFIX_PER_EVPU)) / EVPU_PER_SPACE);

    style.set(Sid::beamNoSlope, prefs.beamOptions->beamingStyle == options::BeamOptions::FlattenStyle::AlwaysFlat);
    style.set(Sid::dotMag, museMagVal(prefs, options::FontOptions::FontType::AugDots));
    style.set(Sid::dotNoteDistance, prefs.augDotOptions->dotNoteOffset / EVPU_PER_SPACE);
    style.set(Sid::dotRestDistance, prefs.augDotOptions->dotNoteOffset / EVPU_PER_SPACE); // Same value as dotNoteDistance
    /// @todo Finale's value is calculated relative to the rightmost point of the previous dot, MuseScore the leftmost.(Observed behavior)
    /// We need to add on the symbol width of one dot for the correct value.
    style.set(Sid::dotDotDistance, prefs.augDotOptions->dotOffset / EVPU_PER_SPACE);
    style.set(Sid::articulationMag, museMagVal(prefs, options::FontOptions::FontType::Articulation));
    style.set(Sid::graceNoteMag, prefs.graceOptions->gracePerc / 100.0);
    style.set(Sid::concertPitch, !prefs.partGlobals->showTransposed);
    style.set(Sid::multiVoiceRestTwoSpaceOffset, std::labs(prefs.layerOneAttributes->restOffset) >= 4);
    style.set(Sid::mergeMatchingRests, prefs.miscOptions->consolidateRestsAcrossLayers);
}

void writeSmartShapePrefs(MStyle& style, const FinalePreferences& prefs)
{
    // Hairpin-related settings
    style.set(Sid::hairpinHeight, prefs.smartShapeOptions->shortHairpinOpeningWidth / EVPU_PER_SPACE);
    style.set(Sid::hairpinContHeight, 0.5); // Hardcoded to a half space
    writeCategoryTextFontPref(style, prefs, "hairpin", others::MarkingCategory::CategoryType::Dynamics);
    writeLinePrefs(style, "hairpin",
                   prefs.smartShapeOptions->crescLineWidth,
                   prefs.smartShapeOptions->smartDashOn,
                   prefs.smartShapeOptions->smartDashOff);

    // Slur-related settings
    style.set(Sid::slurEndWidth, prefs.smartShapeOptions->smartSlurTipWidth / EVPU_PER_SPACE);
    style.set(Sid::slurDottedWidth, prefs.smartShapeOptions->smartLineWidth / EFIX_PER_SPACE);

    // Tie-related settings
    style.set(Sid::tieEndWidth, prefs.tieOptions->tieTipWidth / EVPU_PER_SPACE);
    style.set(Sid::tieDottedWidth, prefs.smartShapeOptions->smartLineWidth / EFIX_PER_SPACE);
    style.set(Sid::tiePlacementSingleNote, prefs.tieOptions->useOuterPlacement ? TiePlacement::OUTSIDE : TiePlacement::INSIDE);
	// Note: Finale's 'outer placement' for notes within chords is much closer to inside placement. But outside placement is closer overall.
    style.set(Sid::tiePlacementChord, prefs.tieOptions->useOuterPlacement ? TiePlacement::OUTSIDE : TiePlacement::INSIDE);

    // Ottava settings
    style.set(Sid::ottavaHookAbove, prefs.smartShapeOptions->hookLength / EVPU_PER_SPACE);
    style.set(Sid::ottavaHookBelow, -prefs.smartShapeOptions->hookLength / EVPU_PER_SPACE);
    writeLinePrefs(style, "ottava",
                   prefs.smartShapeOptions->smartLineWidth,
                   prefs.smartShapeOptions->smartDashOn,
                   prefs.smartShapeOptions->smartDashOff,
                   LineType::DASHED);
    style.set(Sid::ottavaNumbersOnly, prefs.smartShapeOptions->showOctavaAsText);
}

void writeMeasureNumberPrefs(MStyle& style, const FinalePreferences& prefs)
{
    using MeasureNumberRegion = others::MeasureNumberRegion;
    style.set(Sid::showMeasureNumber, prefs.measNumScorePart != nullptr);

    if (prefs.measNumScorePart) {
        const auto& scorePart = prefs.measNumScorePart;
        style.set(Sid::showMeasureNumberOne, !scorePart->hideFirstMeasure);
        style.set(Sid::measureNumberInterval, scorePart->incidence);
        style.set(Sid::measureNumberSystem, scorePart->showOnStart && !scorePart->showOnEvery);

        auto justificationAlign = [](MeasureNumberRegion::AlignJustify justi) -> Align {
            switch (justi) {
            default:
            case MeasureNumberRegion::AlignJustify::Left: return Align(AlignH::LEFT, AlignV::BASELINE);
            case MeasureNumberRegion::AlignJustify::Center: return Align(AlignH::HCENTER, AlignV::BASELINE);
            case MeasureNumberRegion::AlignJustify::Right: return Align(AlignH::RIGHT, AlignV::BASELINE);
            }
        };

        auto horizontalAlignment = [](MeasureNumberRegion::AlignJustify align) -> PlacementH {
            switch (align) {
            default:
            case MeasureNumberRegion::AlignJustify::Left: return PlacementH::LEFT;
            case MeasureNumberRegion::AlignJustify::Center: return PlacementH::CENTER;
            case MeasureNumberRegion::AlignJustify::Right: return PlacementH::RIGHT;
            }
        };

        auto verticalAlignment = [](Evpu vertical) -> PlacementV {
            return (vertical >= 0) ? PlacementV::ABOVE : PlacementV::BELOW;
        };

        auto processSegment = [&](const std::shared_ptr<FontInfo>& fontInfo,
                                  const std::shared_ptr<others::Enclosure>& enclosure,
                                  bool useEnclosure,
                                  MeasureNumberRegion::AlignJustify justification,
                                  MeasureNumberRegion::AlignJustify alignment,
                                  Evpu vertical,
                                  const std::string& prefix)
        {
            writeFontPref(style, prefix, fontInfo.get());
            style.set(styleIdx(prefix + "VPlacement"), verticalAlignment(vertical));
            style.set(styleIdx(prefix + "HPlacement"), horizontalAlignment(alignment));
            style.set(styleIdx(prefix + "Align"), justificationAlign(justification));
            writeFramePrefs(style, prefix, useEnclosure ? enclosure.get() : nullptr);
        };

        // Determine source for primary segment
        auto fontInfo       = scorePart->showOnStart ? scorePart->startFont         : scorePart->multipleFont;
        auto enclosure      = scorePart->showOnStart ? scorePart->startEnclosure    : scorePart->multipleEnclosure;
        auto useEnclosure   = scorePart->showOnStart ? scorePart->useStartEncl      : scorePart->useMultipleEncl;
        auto justification  = scorePart->showOnEvery ? scorePart->multipleJustify   : scorePart->startJustify;
        auto alignment      = scorePart->showOnEvery ? scorePart->multipleAlign     : scorePart->startAlign;
        auto vertical       = scorePart->showOnStart ? scorePart->startYdisp        : scorePart->multipleYdisp;

        style.set(Sid::measureNumberOffsetType, int(OffsetType::SPATIUM)); // Hardcoded offset type
        processSegment(fontInfo, enclosure, useEnclosure, justification, alignment, vertical, "measureNumber");

        style.set(Sid::mmRestShowMeasureNumberRange, scorePart->showMmRange);
        if (scorePart->leftMmBracketChar == 0) {
            style.set(Sid::mmRestRangeBracketType, int(MMRestRangeBracketType::NONE));
        } else if (scorePart->leftMmBracketChar == '(') {
            style.set(Sid::mmRestRangeBracketType, int(MMRestRangeBracketType::PARENTHESES));
        } else {
            style.set(Sid::mmRestRangeBracketType, int(MMRestRangeBracketType::BRACKETS));
        }

        processSegment(scorePart->mmRestFont, scorePart->multipleEnclosure, scorePart->useMultipleEncl,
                       scorePart->mmRestJustify, scorePart->mmRestAlign, scorePart->mmRestYdisp, "mmRestRange");
    }

    style.set(Sid::createMultiMeasureRests, prefs.forPartId != 0);
    style.set(Sid::minEmptyMeasures, prefs.mmRestOptions->numStart);
    style.set(Sid::minMMRestWidth, prefs.mmRestOptions->measWidth / EVPU_PER_SPACE);
    style.set(Sid::mmRestNumberPos, (prefs.mmRestOptions->numAdjY / EVPU_PER_SPACE) + 1);
    style.set(Sid::oldStyleMultiMeasureRests,
              prefs.mmRestOptions->useSymbols && prefs.mmRestOptions->useSymsThreshold > 1);
    style.set(Sid::mmRestOldStyleMaxMeasures,
              std::max(prefs.mmRestOptions->useSymsThreshold - 1, 0));
    style.set(Sid::mmRestOldStyleSpacing, prefs.mmRestOptions->symSpacing / EVPU_PER_SPACE);
}

void writeRepeatEndingPrefs(MStyle& style, const FinalePreferences& prefs)
{
    style.set(Sid::voltaLineWidth, prefs.repeatOptions->bracketLineWidth / EFIX_PER_SPACE);
    style.set(Sid::voltaLineStyle, LineType::SOLID);
    writeDefaultFontPref(style, prefs, "volta", options::FontOptions::FontType::Ending);
    style.set(Sid::voltaAlign, Align(AlignH::LEFT, AlignV::BASELINE));
}

void writeTupletPrefs(MStyle& style, const FinalePreferences& prefs)
{
    using TupletOptions = options::TupletOptions;
    const auto& tupletOptions = prefs.tupletOptions;

    style.set(Sid::tupletOutOfStaff, tupletOptions->avoidStaff);
    style.set(Sid::tupletStemLeftDistance, tupletOptions->leftHookExt / EVPU_PER_SPACE);
    style.set(Sid::tupletStemRightDistance, tupletOptions->rightHookExt / EVPU_PER_SPACE);
    style.set(Sid::tupletNoteLeftDistance, tupletOptions->leftHookExt / EVPU_PER_SPACE);
    style.set(Sid::tupletNoteRightDistance, tupletOptions->rightHookExt / EVPU_PER_SPACE);
    style.set(Sid::tupletBracketWidth, tupletOptions->tupLineWidth / EFIX_PER_SPACE);

    switch (tupletOptions->posStyle) {
    case TupletOptions::PositioningStyle::Above:
        style.set(Sid::tupletDirection, DirectionV::UP);
        break;
    case TupletOptions::PositioningStyle::Below:
        style.set(Sid::tupletDirection, DirectionV::DOWN);
        break;
    default:
        style.set(Sid::tupletDirection, DirectionV::AUTO);
        break;
    }

    style.set(Sid::tupletNumberType, int(FinaleTConv::toMuseScoreTupletNumberType(tupletOptions->numStyle)));

    if (tupletOptions->brackStyle == TupletOptions::BracketStyle::Nothing) {
        style.set(Sid::tupletBracketType, int(TupletBracketType::SHOW_NO_BRACKET));
    } else if (tupletOptions->autoBracketStyle == TupletOptions::AutoBracketStyle::Always) {
        style.set(Sid::tupletBracketType, int(TupletBracketType::SHOW_BRACKET));
    } else {
        style.set(Sid::tupletBracketType, int(TupletBracketType::AUTO_BRACKET));
    }

    const auto& fontInfo = options::FontOptions::getFontInfo(prefs.document, options::FontOptions::FontType::Tuplet);
    if (!fontInfo) {
        throw std::invalid_argument("Unable to load font pref for tuplets");
    }

    if (fontInfo->calcIsSMuFL()) {
        style.set(Sid::tupletMusicalSymbolsScale, museMagVal(prefs, options::FontOptions::FontType::Tuplet));
        style.set(Sid::tupletUseSymbols, true);
    } else {
        writeFontPref(style, "tuplet", fontInfo.get());
        style.set(Sid::tupletMusicalSymbolsScale, 1.0);
        style.set(Sid::tupletUseSymbols, false);
    }

    style.set(Sid::tupletBracketHookHeight,
              (std::max)(-tupletOptions->leftHookLen, -tupletOptions->rightHookLen) / EVPU_PER_SPACE);
}

void writeMarkingPrefs(MStyle& style, const FinalePreferences& prefs)
{
    using FontType = options::FontOptions::FontType;
    using CategoryType = others::MarkingCategory::CategoryType;

    auto cat = prefs.document->getOthers()->get<others::MarkingCategory>(prefs.forPartId, Cmper(CategoryType::Dynamics));
    if (!cat) {
        throw std::invalid_argument("unable to find MarkingCategory for dynamics");
    }
    auto catFontInfo = cat->musicFont;
    bool override = catFontInfo && catFontInfo->calcIsSMuFL() && catFontInfo->fontId != 0;
    style.set(Sid::dynamicsOverrideFont, override);
    if (override) {
        style.set(Sid::dynamicsFont, String::fromStdString(catFontInfo->getName()));
        style.set(Sid::dynamicsSize, catFontInfo->fontSize / prefs.defaultMusicFont->fontSize);
    } else {
        style.set(Sid::dynamicsFont, String::fromStdString(prefs.defaultMusicFont->getName()));
        style.set(Sid::dynamicsSize,
                  catFontInfo->calcIsSMuFL() ? (catFontInfo->fontSize / prefs.defaultMusicFont->fontSize) : 1.0);
    }

    auto textBlockFont = options::FontOptions::getFontInfo(prefs.document, FontType::TextBlock);
    if (!textBlockFont) {
        throw std::invalid_argument("unable to find font prefs for Text Blocks");
    }
    writeFontPref(style, "default", textBlockFont.get());
    style.set(Sid::titleFontFace, String::fromStdString(textBlockFont->getName()));
    style.set(Sid::subTitleFontFace, String::fromStdString(textBlockFont->getName()));
    style.set(Sid::composerFontFace, String::fromStdString(textBlockFont->getName()));
    style.set(Sid::lyricistFontFace, String::fromStdString(textBlockFont->getName()));

    writeDefaultFontPref(style, prefs, "longInstrument", FontType::StaffNames);
    const auto fullPosition = prefs.staffOptions->namePos;
    if (!fullPosition) {
        throw std::invalid_argument("unable to find default full name positioning for staves");
    }
    style.set(Sid::longInstrumentAlign, FinaleTConv::justifyToAlignment(fullPosition->justify));

    writeDefaultFontPref(style, prefs, "shortInstrument", FontType::AbbrvStaffNames);
    const auto abbreviatedPosition = prefs.staffOptions->namePosAbbrv;
    if (!abbreviatedPosition) {
        throw std::invalid_argument("unable to find default abbreviated name positioning for staves");
    }
    style.set(Sid::shortInstrumentAlign, FinaleTConv::justifyToAlignment(abbreviatedPosition->justify));

    writeDefaultFontPref(style, prefs, "partInstrument", FontType::StaffNames);
    writeCategoryTextFontPref(style, prefs, "dynamics", CategoryType::Dynamics);
    writeCategoryTextFontPref(style, prefs, "expression", CategoryType::ExpressiveText);
    writeCategoryTextFontPref(style, prefs, "tempo", CategoryType::TempoMarks);
    writeCategoryTextFontPref(style, prefs, "tempoChange", CategoryType::ExpressiveText);
    writeLinePrefs(style, "tempoChange",
                   prefs.smartShapeOptions->smartLineWidth,
                   prefs.smartShapeOptions->smartDashOn,
                   prefs.smartShapeOptions->smartDashOff,
                   LineType::DASHED);
    writeCategoryTextFontPref(style, prefs, "metronome", CategoryType::TempoMarks);
    style.set(Sid::translatorFontFace, String::fromStdString(textBlockFont->getName()));
    writeCategoryTextFontPref(style, prefs, "systemText", CategoryType::ExpressiveText);
    writeCategoryTextFontPref(style, prefs, "staffText", CategoryType::TechniqueText);
    writeCategoryTextFontPref(style, prefs, "rehearsalMark", CategoryType::RehearsalMarks);
    writeDefaultFontPref(style, prefs, "repeatLeft", FontType::Repeat);
    writeDefaultFontPref(style, prefs, "repeatRight", FontType::Repeat);
    writeFontPref(style, "frame", textBlockFont.get());
    writeCategoryTextFontPref(style, prefs, "textLine", CategoryType::TechniqueText);
    writeCategoryTextFontPref(style, prefs, "systemTextLine", CategoryType::ExpressiveText);
    writeCategoryTextFontPref(style, prefs, "glissando", CategoryType::TechniqueText);
    writeCategoryTextFontPref(style, prefs, "bend", CategoryType::TechniqueText);
    writeFontPref(style, "header", textBlockFont.get());
    writeFontPref(style, "footer", textBlockFont.get());
    writeFontPref(style, "copyright", textBlockFont.get());
    writeFontPref(style, "pageNumber", textBlockFont.get());
    writeFontPref(style, "instrumentChange", textBlockFont.get());
    writeFontPref(style, "sticking", textBlockFont.get());
    for (int i = 1; i <= 12; ++i) {
        writeFontPref(style, "user" + std::to_string(i), textBlockFont.get());
    }
}

void EnigmaXmlImporter::importStyles(engraving::MStyle& style, musx::dom::Cmper partId)
{
    FinalePreferences prefs = getCurrentPrefs(*this, partId);
    writePagePrefs(style, prefs);
    writeLyricsPrefs(style, prefs);
    writeLineMeasurePrefs(style, prefs);
    writeStemPrefs(style, prefs);
    writeMusicSpacingPrefs(style, prefs);
    writeNoteRelatedPrefs(style, prefs);
    writeSmartShapePrefs(style, prefs);
    writeMeasureNumberPrefs(style, prefs);
    writeRepeatEndingPrefs(style, prefs);
    writeTupletPrefs(style, prefs);
    writeMarkingPrefs(style, prefs);
}

}
