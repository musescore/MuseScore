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
#include "internal/finaletypesconv.h"
#include "internal/text/finaletextconv.h"

#include "musx/musx.h"

#include "types/string.h"

#include "global/stringutils.h"

#include "engraving/dom/measurenumber.h"
#include "engraving/dom/mmrestrange.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/score.h"
#include "engraving/dom/spanner.h"
#include "engraving/dom/textbase.h"

#include "engraving/style/defaultstyle.h"

#include "engraving/types/types.h"

using namespace mu::engraving;
using namespace musx::dom;
using namespace musx::dom::options;

namespace mu::iex::finale {
// Unused; we read the installed fonts instead (custom fonts since 4.6)
static const std::set<std::string_view> museScoreSMuFLFonts {
    "Bravura",
    "Leland",
    "Emmentaler",
    "Gonville",
    "MuseJazz",
    "Petaluma",
    "Finale Maestro",
    "Finale Broadway"
};

static const std::unordered_map<std::string, std::string> finaleToSMuFLFontMap {
    { "AshMusic",         "Finale Ash" },
    { "Broadway Copyist", "Finale Broadway" },
    { "Engraver",         "Finale Engraver" },
    { "EngraverFontSet",  "Finale Engraver" },
    { "Jazz",             "Finale Jazz" },
    { "Maestro",          "Finale Maestro" },
    { "Petrucci",         "Finale Legacy" },
    { "Pmusic",           "Finale Maestro" },
    { "Sonata",           "Finale Maestro" },
};

static const std::unordered_set<std::string> solidLinesWithHooks {
    "textLine",
    "systemTextLine",
    "letRing",
    "palmMute",
    "pedal"
};

static const std::unordered_set<std::string> solidLinesNoHooks {
    "noteLine",
    "glissando"
};

static const std::unordered_set<std::string> dashedLinesNoHooks {
    "ottava",
    "tempoChange"
};

template<typename T>
static MusxInstance<T> getDocOptions(const FinaleParser& context, const std::string& prefsName)
{
    auto result = context.musxDocument()->getOptions()->get<T>();
    if (!result) {
        throw std::invalid_argument("document contains no default " + prefsName);
    }
    return result;
}

void FinaleOptions::init(const FinaleParser& context)
{
    auto fontOptions = getDocOptions<FontOptions>(context, "font");
    defaultMusicFont = fontOptions->getFontInfo(FontOptions::FontType::Music);
    //
    accidentalOptions = getDocOptions<AccidentalOptions>(context, "accidental");
    alternateNotationOptions = getDocOptions<AlternateNotationOptions>(context, "alternate notation");
    augDotOptions = getDocOptions<AugmentationDotOptions>(context, "augmentation dot");
    barlineOptions = getDocOptions<BarlineOptions>(context, "barline");
    beamOptions = getDocOptions<BeamOptions>(context, "beam");
    chordOptions = getDocOptions<ChordOptions>(context, "chord");
    clefOptions = getDocOptions<ClefOptions>(context, "clef");
    flagOptions = getDocOptions<FlagOptions>(context, "flag");
    graceOptions = getDocOptions<GraceNoteOptions>(context, "grace note");
    keyOptions = getDocOptions<KeySignatureOptions>(context, "key signature");
    lineCurveOptions = getDocOptions<LineCurveOptions>(context, "lines & curves");
    miscOptions = getDocOptions<MiscOptions>(context, "miscellaneous");
    mmRestOptions = getDocOptions<MultimeasureRestOptions>(context, "multimeasure rest");
    musicSpacing = getDocOptions<MusicSpacingOptions>(context, "music spacing");
    musicSymbols = getDocOptions<MusicSymbolOptions>(context, "music symbols");
    auto pageFormatOptions = getDocOptions<PageFormatOptions>(context, "page format");
    pageFormat = pageFormatOptions->calcPageFormatForPart(context.currentMusxPartId());
    braceOptions = getDocOptions<PianoBraceBracketOptions>(context, "piano braces & brackets");
    repeatOptions = getDocOptions<RepeatOptions>(context, "repeat");
    smartShapeOptions = getDocOptions<SmartShapeOptions>(context, "smart shape");
    staffOptions = getDocOptions<StaffOptions>(context, "staff");
    stemOptions = getDocOptions<StemOptions>(context, "stem");
    textOptions = getDocOptions<TextOptions>(context, "text");
    tieOptions = getDocOptions<TieOptions>(context, "tie");
    timeOptions = getDocOptions<TimeSignatureOptions>(context, "time signature");
    tupletOptions = getDocOptions<TupletOptions>(context, "tuplet");
    //
    layerOneAttributes = context.musxDocument()->getOthers()->get<others::LayerAttributes>(context.currentMusxPartId(), 0);
    if (!layerOneAttributes) {
        throw std::invalid_argument("document contains no options for Layer 1");
    }
    auto measNumRegions = context.musxDocument()->getOthers()->getArray<others::MeasureNumberRegion>(context.currentMusxPartId());
    if (measNumRegions.size() > 0) {
        measNumScorePart = (context.partScore() && measNumRegions[0]->useScoreInfoForPart && measNumRegions[0]->partData)
                           ? measNumRegions[0]->partData
                           : measNumRegions[0]->scoreData;
        if (!measNumScorePart) {
            throw std::invalid_argument("document contains no ScorePartData for measure number region "
                                        + std::to_string(measNumRegions[0]->getCmper()));
        }
    }
    partGlobals = context.musxDocument()->getOthers()->get<others::PartGlobals>(context.currentMusxPartId(), MUSX_GLOBALS_CMPER);
    combinedDefaultStaffScaling = pageFormat->calcCombinedSystemScaling();

    // Musical symbols font
    std::string defaultMusicalSymbolsFont = context.musxOptions().defaultMusicFont->getName();
    defaultMusicalSymbolsFont = muse::value(finaleToSMuFLFontMap, defaultMusicalSymbolsFont, defaultMusicalSymbolsFont);
    if (context.fontIsEngravingFont(defaultMusicalSymbolsFont)) {
        calculatedEngravingFontName = String::fromStdString(defaultMusicalSymbolsFont);
    } else {
        calculatedEngravingFontName = engraving::DefaultStyle::defaultStyle().styleSt(Sid::musicalSymbolFont);
    }
}

bool FinaleParser::fontIsEngravingFont(const std::string& fontName, bool includeMapped) const
{
    std::string mapped = includeMapped ? muse::value(finaleToSMuFLFontMap, fontName, fontName) : fontName;
    return muse::contains(m_engravingFonts, muse::strings::toLower(mapped));
}

EvpuFloat FinaleParser::evpuAugmentationDotWidth() const
{
    EvpuFloat result = m_score->engravingFont()->width(SymId::augmentationDot, m_score->style().styleD(Sid::dotMag));
    return result * (EVPU_PER_SPACE / m_score->style().defaultSpatium());
}

static void setStyle(MStyle& style, const Sid sid, const PropertyValue& v)
{
    if (sid == Sid::NOSTYLE) {
        return;
    }
    if (v.type() == P_TYPE::REAL && style.valueType(sid) == P_TYPE::SPATIUM) {
        style.set(sid, Spatium(v.toDouble()));
    } else {
        style.set(sid, v);
    }
}

static double museMagVal(const FinaleParser& context, const FontOptions::FontType type)
{
    auto fontPrefs = FontOptions::getFontInfo(context.musxDocument(), type);
    if (fontPrefs && fontPrefs->getName() == context.musxOptions().defaultMusicFont->getName()) {
        return double(fontPrefs->fontSize) / double(context.musxOptions().defaultMusicFont->fontSize);
    }
    return 1.0;
}

static Sid styleIdx(const std::string& name)
{
    auto nameStr = String::fromStdString(name);
    return MStyle::styleIdx(nameStr);
}

static void writeEvpuSpace(MStyle& style, Sid sid, Evpu evpu)
{
    setStyle(style, sid, doubleFromEvpu(evpu));
}

static void writeEfixSpace(MStyle& style, Sid sid, Efix efix)
{
    setStyle(style, sid, doubleFromEfix(efix));
}

static void writeEvpuPointF(MStyle& style, Sid sid, Evpu xEvpu, Evpu yEvpu)
{
    setStyle(style, sid, evpuToPointF(xEvpu, yEvpu));
}

static void writeEvpuInch(MStyle& style, Sid sid, Evpu evpu)
{
    setStyle(style, sid, double(evpu) / EVPU_PER_INCH);
}

static void writeFontPref(MStyle& style, const std::string& namePrefix, const MusxInstance<FontInfo>& fontInfo)
{
    FontTracker converted(fontInfo, style.defaultSpatium());
    setStyle(style, styleIdx(namePrefix + "FontFace"), converted.fontName);
    setStyle(style, styleIdx(namePrefix + "FontSize"), converted.fontSize);
    setStyle(style, styleIdx(namePrefix + "FontSpatiumDependent"), converted.spatiumDependent);
    setStyle(style, styleIdx(namePrefix + "FontStyle"), int(converted.fontStyle));
}

static void writeDefaultFontPref(MStyle& style, const FinaleParser& context, const std::string& namePrefix, FontOptions::FontType type)
{
    if (const auto& fontPrefs = FontOptions::getFontInfo(context.musxDocument(), type)) {
        // If font is a symbols font, read only the symbol size and if it scales.
        if (context.fontIsEngravingFont(fontPrefs, true) && type != FontOptions::FontType::TextBlock) {
            writeDefaultFontPref(style, context, namePrefix, FontOptions::FontType::TextBlock);
            double symbolSize =  double(fontPrefs->fontSize) / double(context.musxOptions().defaultMusicFont->fontSize);
            setStyle(style, styleIdx(namePrefix + "MusicalSymbolsScale"), symbolSize);
            setStyle(style, styleIdx(namePrefix + "MusicalSymbolSize"), 20.0 * symbolSize);
            setStyle(style, styleIdx(namePrefix + "FontSpatiumDependent"), !fontPrefs->absolute);
        } else {
            writeFontPref(style, namePrefix, fontPrefs);
        }
    } else {
        context.logger()->logWarning(String(u"Unable to load default font info for %1 FontType").arg(int(type)));
    }
}

static void writeLinePrefs(MStyle& style, const std::string& namePrefix, double widthEfix, double dashLength,
                           double dashGap, const std::optional<LineType>& lineStyle = std::nullopt)
{
    writeEfixSpace(style, styleIdx(namePrefix + "LineWidth"), widthEfix);
    if (lineStyle.has_value()) {
        setStyle(style, styleIdx(namePrefix + "LineStyle"), lineStyle.value());
    }
    const double lineWidthEvpu = widthEfix / EFIX_PER_EVPU;
    setStyle(style, styleIdx(namePrefix + "DashLineLen"), dashLength / lineWidthEvpu);
    setStyle(style, styleIdx(namePrefix + "DashGapLen"), dashGap / lineWidthEvpu);
}

static void writeFramePrefs(MStyle& style, const std::string& namePrefix, const others::Enclosure* enclosure = nullptr)
{
    FrameSettings settings = FrameSettings(enclosure);
    setStyle(style, styleIdx(namePrefix + "FrameType"), int(settings.frameType));

    // Do not override any other defaults if no enclosure
    if (settings.frameType == FrameType::NO_FRAME) {
        return;
    }

    setStyle(style, styleIdx(namePrefix + "FrameWidth"), settings.frameWidth);
    setStyle(style, styleIdx(namePrefix + "FramePadding"), settings.paddingWidth);
    setStyle(style, styleIdx(namePrefix + "FrameRound"), settings.frameRound);
}

static void writeCategoryTextFontPref(MStyle& style, const FinaleParser& context, const std::string& namePrefix,
                                      others::MarkingCategory::CategoryType categoryType)
{
    auto cat = context.musxDocument()->getOthers()->get<others::MarkingCategory>(context.currentMusxPartId(), Cmper(categoryType));
    if (!cat) {
        context.logger()->logWarning(String(u"Unable to load category def for %1.").arg(String::fromStdString(namePrefix)));
        return;
    }
    if (cat->textFont) {
        writeFontPref(style, namePrefix, cat->textFont);
    } else {
        context.logger()->logWarning(String(u"Marking category %1 has no text font.").arg(String::fromStdString(cat->getName())));
        return;
    }
    if (cat->musicFont) {
        double symbolSize = double(cat->musicFont->fontSize) / double(context.musxOptions().defaultMusicFont->fontSize);
        setStyle(style, styleIdx(namePrefix + "MusicalSymbolsScale"), symbolSize);
        setStyle(style, styleIdx(namePrefix + "MusicalSymbolSize"), 20.0 * symbolSize);
    }
    for (auto& it : cat->textExpressions) {
        if (auto exp = it.second.lock()) {
            writeFramePrefs(style, namePrefix, exp->getEnclosure().get());
            break;
        } else {
            context.logger()->logWarning(String(u"Marking category %1 has invalid text expression.").arg(
                                             String::fromStdString(cat->getName())));
        }
    }
}

static void writePagePrefs(MStyle& style, const FinaleParser& context)
{
    const auto& prefs = context.musxOptions();
    const auto& pagePrefs = prefs.pageFormat;

    writeEvpuInch(style, Sid::pageWidth, pagePrefs->pageWidth);
    writeEvpuInch(style, Sid::pageHeight, pagePrefs->pageHeight);
    writeEvpuInch(style, Sid::pagePrintableWidth,
                  pagePrefs->pageWidth - pagePrefs->leftPageMarginLeft + pagePrefs->leftPageMarginRight);
    writeEvpuInch(style, Sid::pageEvenLeftMargin, pagePrefs->leftPageMarginLeft);
    writeEvpuInch(style, Sid::pageEvenTopMargin, -pagePrefs->leftPageMarginTop);
    writeEvpuInch(style, Sid::pageEvenBottomMargin, pagePrefs->leftPageMarginBottom);
    if (pagePrefs->facingPages) {
        writeEvpuInch(style, Sid::pageOddLeftMargin, pagePrefs->rightPageMarginLeft);
        writeEvpuInch(style, Sid::pageOddTopMargin, -pagePrefs->rightPageMarginTop);
        writeEvpuInch(style, Sid::pageOddBottomMargin, pagePrefs->rightPageMarginBottom);
    } else {
        for (auto [odd, even] : {
                std::make_pair(Sid::pageOddLeftMargin, Sid::pageEvenLeftMargin),
                std::make_pair(Sid::pageOddTopMargin, Sid::pageEvenTopMargin),
                std::make_pair(Sid::pageEvenBottomMargin, Sid::pageEvenBottomMargin)
            }) {
            setStyle(style, odd, style.styleD(even));
        }
    }
    setStyle(style, Sid::pageTwosided, pagePrefs->facingPages);
    setStyle(style, Sid::enableIndentationOnFirstSystem, pagePrefs->differentFirstSysMargin);
    setStyle(style, Sid::lastSystemFillLimit, 0.0); // Always fill systems
    writeEvpuSpace(style, Sid::firstSystemIndentationValue, pagePrefs->firstSysMarginLeft);

    // Calculate Spatium
    setStyle(style, Sid::spatium, prefs.combinedDefaultStaffScaling.toDouble() * FINALE_DEFAULT_SPATIUM);

    // Calculate small staff size and small note size from first system, if any is there
    if (const auto& firstSystem = context.musxDocument()->getOthers()->get<others::StaffSystem>(context.currentMusxPartId(), 1)) {
        auto [minSize, maxSize] = firstSystem->calcMinMaxStaffSizes();
        if (minSize < 1) {
            setStyle(style, Sid::smallStaffMag, minSize.toDouble());
            // Finale has no global style for this, but we override it later if we find cue-size chords.
            setStyle(style, Sid::smallNoteMag, minSize.toDouble());
        }
    }

    // Default music font
    setStyle(style, Sid::musicalSymbolFont, prefs.calculatedEngravingFontName);
    setStyle(style, Sid::musicalTextFont, prefs.calculatedEngravingFontName + " Text"); // Perhaps this should be more sophisticated
}

static void writeLyricsPrefs(MStyle& style, const FinaleParser& context)
{
    auto fontInfo = FontOptions::getFontInfo(context.musxDocument(), FontOptions::FontType::LyricVerse);
    for (auto [verseNumber, evenOdd] : {
            std::make_pair(1, "Odd"),
            std::make_pair(2, "Even")
        }) {
        auto verseText = context.musxDocument()->getTexts()->get<texts::LyricsVerse>(Cmper(verseNumber));
        if (verseText && !verseText->text.empty()) {
            if (auto font = verseText->getRawTextCtx(verseText, context.currentMusxPartId()).parseFirstFontInfo()) {
                fontInfo = font;
            }
        }
        writeFontPref(style, "lyrics" + std::string(evenOdd), fontInfo);
    }
}

static void writeLineMeasurePrefs(MStyle& style, const FinaleParser& context)
{
    const auto& prefs = context.musxOptions();

    writeEfixSpace(style, Sid::barWidth, prefs.barlineOptions->barlineWidth);
    writeEfixSpace(style, Sid::doubleBarWidth, prefs.barlineOptions->barlineWidth);
    writeEfixSpace(style, Sid::endBarWidth, prefs.barlineOptions->thickBarlineWidth);

    // these calculations are based on observed behavior
    writeEfixSpace(style, Sid::doubleBarDistance,
                   prefs.barlineOptions->doubleBarlineSpace - prefs.barlineOptions->barlineWidth);
    writeEfixSpace(style, Sid::endBarDistance, prefs.barlineOptions->finalBarlineSpace);

    // Average forward/backward dot distance and subtract half the dot width
    const double mag = style.spatium() / style.defaultSpatium();
    const double dotDistance = doubleFromEvpu(prefs.repeatOptions->forwardDotHPos + prefs.repeatOptions->backwardDotHPos)
                               - context.score()->engravingFont()->width(SymId::repeatDot, mag) / style.spatium();
    setStyle(style, Sid::repeatBarlineDotSeparation, dotDistance * .5);

    setStyle(style, Sid::repeatBarTips, prefs.repeatOptions->wingStyle != RepeatOptions::WingStyle::None);

    setStyle(style, Sid::startBarlineSingle, prefs.barlineOptions->drawLeftBarlineSingleStaff);
    setStyle(style, Sid::startBarlineMultiple, prefs.barlineOptions->drawLeftBarlineMultipleStaves);

    setStyle(style, Sid::bracketWidth, 0.5); // Hard-coded in Finale
    writeEvpuSpace(style, Sid::bracketDistance, -(prefs.braceOptions->defBracketPos) - 0.25 * EVPU_PER_SPACE); // Finale subtracts half the bracket width on layout (observed).
    writeEvpuSpace(style, Sid::akkoladeBarDistance, -prefs.braceOptions->defBracketPos);

    writeEvpuSpace(style, Sid::clefLeftMargin, prefs.clefOptions->clefFrontSepar);
    writeEvpuSpace(style, Sid::keysigLeftMargin, prefs.keyOptions->keyFront);

    const double timeSigSpaceBefore = context.partScore()
                                      ? prefs.timeOptions->timeFrontParts
                                      : prefs.timeOptions->timeFront;
    writeEvpuSpace(style, Sid::timesigLeftMargin, timeSigSpaceBefore);

    writeEvpuSpace(style, Sid::clefKeyDistance,
                   (prefs.clefOptions->clefBackSepar + prefs.clefOptions->clefKeySepar + prefs.keyOptions->keyFront));
    writeEvpuSpace(style, Sid::clefTimesigDistance,
                   (prefs.clefOptions->clefBackSepar + prefs.clefOptions->clefTimeSepar + timeSigSpaceBefore));
    writeEvpuSpace(style, Sid::keyTimesigDistance,
                   (prefs.keyOptions->keyBack + prefs.keyOptions->keyTimeSepar + timeSigSpaceBefore));
    writeEvpuSpace(style, Sid::keyBarlineDistance, prefs.repeatOptions->afterKeySpace - 1.5 * EVPU_PER_SPACE); // observed fudge factor

    // Skipped: systemHeaderDistance, systemHeaderTimeSigDistance: these do not translate well from Finale
    // const double timeSigSpaceAfter = context.partScore()
    //                                ? prefs.timeOptions->timeBackParts
    //                                : prefs.timeOptions->timeBack;
    // writeEvpuSpace(style, Sid::headerToLineStartDistance, (prefs.keyOptions->keyBack + timeSigSpaceAfter) / 2);

    writeEvpuSpace(style, Sid::clefBarlineDistance, -prefs.clefOptions->clefChangeOffset);
    writeEvpuSpace(style, Sid::timesigBarlineDistance, prefs.repeatOptions->afterTimeSpace - 1.5 * EVPU_PER_SPACE);

    writeEvpuSpace(style, Sid::measureRepeatNumberPos, -(prefs.alternateNotationOptions->twoMeasNumLift + 0.5));
    writeEfixSpace(style, Sid::staffLineWidth, prefs.lineCurveOptions->staffLineWidth);
    writeEfixSpace(style, Sid::ledgerLineWidth, prefs.lineCurveOptions->legerLineWidth);
    writeEvpuSpace(style, Sid::ledgerLineLength,
                   (prefs.lineCurveOptions->legerFrontLength + prefs.lineCurveOptions->legerBackLength) / 2);
    writeEvpuSpace(style, Sid::keysigAccidentalDistance, (prefs.keyOptions->acciAdd + 4));  // Observed fudge factor
    writeEvpuSpace(style, Sid::keysigNaturalDistance, (prefs.keyOptions->acciAdd + 6));     // Observed fudge factor

    setStyle(style, Sid::smallClefMag, doubleFromPercent(prefs.clefOptions->clefChangePercent));
    setStyle(style, Sid::genClef, !prefs.clefOptions->showClefFirstSystemOnly);
    setStyle(style, Sid::genKeysig, !prefs.keyOptions->showKeyFirstSystemOnly);
    setStyle(style, Sid::genCourtesyTimesig, prefs.timeOptions->cautionaryTimeChanges);
    setStyle(style, Sid::genCourtesyKeysig, prefs.keyOptions->cautionaryKeyChanges);
    setStyle(style, Sid::genCourtesyClef, prefs.clefOptions->cautionaryClefChanges);

    setStyle(style, Sid::keySigCourtesyBarlineMode,
             int(boolToCourtesyBarlineMode(prefs.barlineOptions->drawDoubleBarlineBeforeKeyChanges)));
    setStyle(style, Sid::timeSigCourtesyBarlineMode, int(CourtesyBarlineMode::ALWAYS_SINGLE));  // Hard-coded as 0 in Finale
    setStyle(style, Sid::barlineBeforeSigChange, true);
    setStyle(style, Sid::doubleBarlineBeforeKeySig, prefs.barlineOptions->drawDoubleBarlineBeforeKeyChanges);
    setStyle(style, Sid::doubleBarlineBeforeTimeSig, false);

    setStyle(style, Sid::keySigNaturals, prefs.keyOptions->doKeyCancel ? int(KeySigNatural::BEFORE) : int(KeySigNatural::NONE));
    setStyle(style, Sid::keySigShowNaturalsChangingSharpsFlats, prefs.keyOptions->doKeyCancelBetweenSharpsFlats);

    setStyle(style, Sid::hideEmptyStaves, context.musxDocument()->calcHasVaryingSystemStaves(context.currentMusxPartId()));

    setStyle(style, Sid::placeClefsBeforeRepeats, true);
    setStyle(style, Sid::showCourtesiesRepeats, false);
    setStyle(style, Sid::showCourtesiesOtherJumps, false);
    setStyle(style, Sid::showCourtesiesAfterCancellingRepeats, false);
    setStyle(style, Sid::showCourtesiesAfterCancellingOtherJumps, false);
    setStyle(style, Sid::repeatPlayCountShow, false);
}

static void writeStemPrefs(MStyle& style, const FinaleParser& context)
{
    const auto& prefs = context.musxOptions();

    bool useStraightFlags = prefs.flagOptions->straightFlags;
    if (!useStraightFlags) {
        // some documents using certain music fonts (e.g., Pmusic) have straight flags as their regular flag characters.
        // we check the flagUp and if that is straight, we consider that the others must be straight as well.
        if (MusxInstance<FontInfo> flagFont = FontOptions::getFontInfo(context.musxDocument(), FontOptions::FontType::Flags)) {
            SymId flagChar = FinaleTextConv::symIdFromFinaleChar(prefs.musicSymbols->flagUp, flagFont, SymId::noSym);
            if (flagChar == SymId::flag8thUpStraight) {
                useStraightFlags = true;
            }
        }
    }
    setStyle(style, Sid::useStraightNoteFlags, useStraightFlags);
    writeEfixSpace(style, Sid::stemWidth, prefs.stemOptions->stemWidth);
    setStyle(style, Sid::shortenStem, true);
    writeEvpuSpace(style, Sid::stemLength, prefs.stemOptions->stemLength);
    writeEvpuSpace(style, Sid::shortestStem, prefs.stemOptions->shortStemLength);
    writeEfixSpace(style, Sid::stemSlashThickness, prefs.graceOptions->graceSlashWidth);
}

void writeMusicSpacingPrefs(MStyle& style, const FinaleParser& context)
{
    const auto& prefs = context.musxOptions();

    writeEvpuSpace(style, Sid::minMeasureWidth, prefs.musicSpacing->minWidth);
    writeEvpuSpace(style, Sid::minNoteDistance, prefs.musicSpacing->minDistance);
    writeEvpuSpace(style, Sid::barNoteDistance, prefs.musicSpacing->musFront);
    writeEvpuSpace(style, Sid::barAccidentalDistance, prefs.musicSpacing->musFront);
    writeEvpuSpace(style, Sid::noteBarDistance, prefs.musicSpacing->minDistance + prefs.musicSpacing->musBack);
    setStyle(style, Sid::measureSpacing, prefs.musicSpacing->scalingFactor);

    // In Finale this distance is added to the regular note spacing,
    // whereas MuseScore's value determines effective tie length.
    // Thus we set the value based on the (usually) smallest tie length: ones using inside placement.
    auto horizontalTieEndPointValue = [&](TieOptions::ConnectStyleType type) {
        return muse::value(prefs.tieOptions->tieConnectStyles, type, nullptr)->offsetX;
    };
    writeEvpuSpace(style, Sid::minTieLength,
                   prefs.musicSpacing->minDistTiedNotes + prefs.musicSpacing->minDistance
                   + (horizontalTieEndPointValue(TieOptions::ConnectStyleType::OverEndPosInner)
                      - horizontalTieEndPointValue(TieOptions::ConnectStyleType::OverStartPosInner)
                      + horizontalTieEndPointValue(TieOptions::ConnectStyleType::UnderEndPosInner)
                      - horizontalTieEndPointValue(TieOptions::ConnectStyleType::UnderStartPosInner)) / 2);

    // This value isn't always in used in Finale, but we can't use manual positioning.
    writeEvpuSpace(style, Sid::graceToMainNoteDist, prefs.musicSpacing->minDistGrace);
    writeEvpuSpace(style, Sid::graceToGraceNoteDist, prefs.musicSpacing->minDistGrace);
}

static void writeNoteRelatedPrefs(MStyle& style, FinaleParser& context)
{
    const auto& prefs = context.musxOptions();

    writeEvpuSpace(style, Sid::accidentalDistance, prefs.accidentalOptions->acciAcciSpace);
    writeEvpuSpace(style, Sid::accidentalNoteDistance, prefs.accidentalOptions->acciNoteSpace);
    writeEfixSpace(style, Sid::beamWidth, prefs.beamOptions->beamWidth);
    setStyle(style, Sid::useWideBeams, prefs.beamOptions->beamSepar > (0.75 * EVPU_PER_SPACE));

    // Finale randomly adds twice the stem width to the length of a beam stub. (Observed behavior)
    writeEvpuSpace(style, Sid::beamMinLen,
                   (prefs.beamOptions->beamStubLength + (2.0 * prefs.stemOptions->stemWidth / EFIX_PER_EVPU)));

    setStyle(style, Sid::beamNoSlope, prefs.beamOptions->beamingStyle == BeamOptions::FlattenStyle::AlwaysFlat);
    setStyle(style, Sid::dotMag, museMagVal(context, FontOptions::FontType::AugDots));
    writeEvpuSpace(style, Sid::dotNoteDistance, prefs.augDotOptions->dotNoteOffset);
    writeEvpuSpace(style, Sid::dotRestDistance, prefs.augDotOptions->dotNoteOffset); // Same value as dotNoteDistance
    // Finale's value is calculated relative to the rightmost point of the previous dot, MuseScore the leftmost (observed behavior).
    // We need to add on the symbol width of one dot for the correct value.
    writeEvpuSpace(style, Sid::dotDotDistance, prefs.augDotOptions->dotOffset + context.evpuAugmentationDotWidth());
    setStyle(style, Sid::articulationMag, museMagVal(context, FontOptions::FontType::Articulation));
    setStyle(style, Sid::graceNoteMag, doubleFromPercent(prefs.graceOptions->gracePerc));
    setStyle(style, Sid::concertPitch, !prefs.partGlobals->showTransposed);
    setStyle(style, Sid::multiVoiceRestTwoSpaceOffset, std::labs(prefs.layerOneAttributes->restOffset) >= 4);
    setStyle(style, Sid::mergeMatchingRests, prefs.miscOptions->consolidateRestsAcrossLayers);
    setStyle(style, Sid::tremoloStyle, int(TremoloStyle::TRADITIONAL));
}

static void writeSmartShapePrefs(MStyle& style, const FinaleParser& context)
{
    const MusxInstance<options::SmartShapeOptions>& smartShapePrefs = context.musxOptions().smartShapeOptions;
    const MusxInstance<options::TieOptions>& tiePrefs = context.musxOptions().tieOptions;

    // Hairpins
    writeEvpuSpace(style, Sid::hairpinHeight, (smartShapePrefs->shortHairpinOpeningWidth + smartShapePrefs->crescHeight) * 0.5);
    setStyle(style, Sid::hairpinContHeight, 0.5); // Hardcoded to a half space
    writeLinePrefs(style, "hairpin", smartShapePrefs->crescLineWidth, smartShapePrefs->smartDashOn, smartShapePrefs->smartDashOff);
    writeCategoryTextFontPref(style, context, "hairpin", others::MarkingCategory::CategoryType::Dynamics);
    // Cresc. / Decresc. lines
    const double hairpinLineLineWidthEvpu = smartShapePrefs->smartLineWidth / EFIX_PER_EVPU;
    setStyle(style, Sid::hairpinLineDashLineLen, smartShapePrefs->smartDashOn / hairpinLineLineWidthEvpu);
    setStyle(style, Sid::hairpinLineDashGapLen, smartShapePrefs->smartDashOff / hairpinLineLineWidthEvpu);

    // Slurs
    constexpr double contourScaling = 0.5; // observed scaling factor
    constexpr double minMuseScoreEndWidth = 0.01; // MuseScore slur- and tie thickness go crazy if the endpoint thickness is zero.
    const double slurEndpointWidth = std::max(minMuseScoreEndWidth, doubleFromEvpu(smartShapePrefs->smartSlurTipWidth));
    setStyle(style, Sid::slurEndWidth, slurEndpointWidth);
    // Ignore horizontal thickness values as they hardly affect mid width.
    const double slurMidPointWidth = doubleFromEvpu(smartShapePrefs->slurThicknessCp1Y + smartShapePrefs->slurThicknessCp2Y) * 0.5;
    setStyle(style, Sid::slurMidWidth, slurMidPointWidth * contourScaling);
    writeEfixSpace(style, Sid::slurDottedWidth, smartShapePrefs->smartLineWidth);

    // Ties
    const double tieEndpointWidth = std::max(minMuseScoreEndWidth, doubleFromEvpu(tiePrefs->tieTipWidth));
    setStyle(style, Sid::tieEndWidth, tieEndpointWidth);
    setStyle(style, Sid::tieMidWidth, doubleFromEvpu(tiePrefs->thicknessRight + tiePrefs->thicknessLeft) * 0.5 * contourScaling);
    writeEfixSpace(style, Sid::tieDottedWidth, smartShapePrefs->smartLineWidth);
    setStyle(style, Sid::tiePlacementSingleNote, tiePrefs->useOuterPlacement ? TiePlacement::OUTSIDE : TiePlacement::INSIDE);
    /// @note Finale's 'outer placement' for notes within chords is much closer to inside placement. But outside placement is closer overall.
    setStyle(style, Sid::tiePlacementChord, tiePrefs->useOuterPlacement ? TiePlacement::OUTSIDE : TiePlacement::INSIDE);

    // Ottavas
    writeEvpuSpace(style, Sid::ottavaHookAbove, smartShapePrefs->hookLength);
    writeEvpuSpace(style, Sid::ottavaHookBelow, smartShapePrefs->hookLength);
    setStyle(style, Sid::ottavaNumbersOnly, smartShapePrefs->showOctavaAsText);

    // Guitar bends
    writeEfixSpace(style, Sid::guitarBendLineWidth,    smartShapePrefs->smartLineWidth);
    writeEfixSpace(style, Sid::bendLineWidth,          smartShapePrefs->smartLineWidth); // shape-dependent
    writeEfixSpace(style, Sid::guitarBendLineWidthTab, smartShapePrefs->smartLineWidth); // shape-dependent
    setStyle(style, Sid::guitarBendUseFull, smartShapePrefs->guitarBendUseFull);
    setStyle(style, Sid::showFretOnFullBendRelease, !smartShapePrefs->guitarBendHideBendTo);

    // General line settings
    for (const std::string& prefix : solidLinesWithHooks) {
        writeLinePrefs(style, prefix, smartShapePrefs->smartLineWidth, smartShapePrefs->smartDashOn, smartShapePrefs->smartDashOff);
        writeEvpuSpace(style, styleIdx(prefix + "HookHeight"), smartShapePrefs->hookLength);
    }
    for (const std::string& prefix : solidLinesNoHooks) {
        writeLinePrefs(style, prefix, smartShapePrefs->smartLineWidth, smartShapePrefs->smartDashOn, smartShapePrefs->smartDashOff);
    }
    for (const std::string& prefix : dashedLinesNoHooks) {
        writeLinePrefs(style, prefix, smartShapePrefs->smartLineWidth,
                       smartShapePrefs->smartDashOn, smartShapePrefs->smartDashOff, LineType::DASHED);
    }
}

// Separate function called in two places with two different height values.
// The challenge is that Finale always offsets measure numbers from the baseline, but MuseScore offsets measure numbers below
// from the top of the character string.
static void setMeasureNumberPosBelow(MStyle& style, const std::string& prefix, Evpu horizontal, Evpu vertical, double heightSp)
{
    constexpr static Evpu normalStaffHeight = 4 * EVPU_PER_SPACE;
    setStyle(style, styleIdx(prefix + "PosBelow"),
             PointF(horizontal / EVPU_PER_SPACE, std::max(-(vertical + normalStaffHeight) / EVPU_PER_SPACE - heightSp, 0.0)));
}

static void writeMeasureNumberPrefs(MStyle& style, const FinaleParser& context)
{
    const auto& prefs = context.musxOptions();

    setStyle(style, Sid::showMeasureNumber, prefs.measNumScorePart != nullptr);

    if (prefs.measNumScorePart) {
        const auto& scorePart = prefs.measNumScorePart;
        setStyle(style, Sid::showMeasureNumberOne, !scorePart->hideFirstMeasure);
        setStyle(style, Sid::measureNumberInterval, scorePart->incidence);
        const bool useShowOnStart = scorePart->showOnStart && !scorePart->showOnEvery;
        setStyle(style, Sid::measureNumberSystem, useShowOnStart);
        const MusxInstanceList<others::StaffUsed> scrollView = context.musxDocument()->getScrollViewStaves(context.currentMusxPartId());
        bool topOn = false;
        bool bottomOn = false;
        bool anyInteriorOn = false;
        bool allStavesOn = !scrollView.empty();   // empty => false
        for (std::size_t i = 0; i < scrollView.size(); ++i) {
            if (const MusxInstance<others::Staff> staff = scrollView[i]->getStaffInstance()) {
                const bool isOn = !staff->hideMeasNums;
                allStavesOn = allStavesOn && isOn;
                if (i == 0) {
                    topOn = isOn;
                } else if (i < scrollView.size() - 1) {
                    if (isOn) {
                        anyInteriorOn = true;
                    }
                } else {
                    bottomOn = isOn;
                }
            }
        }
        const bool useAbove = scorePart->excludeOthers || (!anyInteriorOn && !bottomOn);
        const bool useBelow = scorePart->excludeOthers || (!anyInteriorOn && !topOn);
        if (useAbove && scorePart->showOnTop) {
            setStyle(style, Sid::measureNumberPlacementMode, MeasureNumberPlacement::ABOVE_SYSTEM);
        } else if (useBelow && scorePart->showOnBottom) {
            setStyle(style, Sid::measureNumberPlacementMode, MeasureNumberPlacement::BELOW_SYSTEM);
        } else if (allStavesOn) {
            setStyle(style, Sid::measureNumberPlacementMode, MeasureNumberPlacement::ON_ALL_STAVES);
        } else {
            if (scorePart->showOnBottom) {
                context.logger()->logWarning(u"Show on Bottom not supported when other staves also show measure numbers.");
            }
            setStyle(style, Sid::measureNumberPlacementMode, MeasureNumberPlacement::ON_SYSTEM_OBJECT_STAVES);
        }

        auto processSegment = [&](const MusxInstance<FontInfo>& fontInfo, const others::Enclosure* enclosure,
                                  AlignJustify justification, AlignJustify alignment,
                                  Evpu horizontal, Evpu vertical, const std::string& prefix)
        {
            writeFontPref(style, prefix, fontInfo);
            setStyle(style, styleIdx(prefix + "VPlacement"), (vertical >= 0) ? PlacementV::ABOVE : PlacementV::BELOW);
            setStyle(style, styleIdx(prefix + "HPlacement"), toAlignH(justification));
            setStyle(style, styleIdx(prefix + "Align"), Align(toAlignH(alignment), AlignV::BASELINE));
            setStyle(style, styleIdx(prefix + "Position"), toAlignH(justification));
            /// @note This algorithm takes a rough stab at getting close to the correct height for measure numbers.
            /// Then after layout we come back and calculate it again with the actual measure number height. However,
            /// this is a stand-in in case that routine fails to find a measure number of the right type.
            RectF bbox = FontTracker(fontInfo, style.spatium()).toFontMetrics().tightBoundingRect(u"0123456789");
            double heightSp = bbox.height() / style.defaultSpatium();
            setStyle(style, styleIdx(prefix + "PosAbove"), PointF(horizontal / EVPU_PER_SPACE, std::min(-vertical / EVPU_PER_SPACE, 0.0)));
            setMeasureNumberPosBelow(style, prefix, horizontal, vertical, heightSp);
            writeFramePrefs(style, prefix, enclosure);
        };

        // Determine source for primary segment
        auto fontInfo      = useShowOnStart ? scorePart->startFont : scorePart->multipleFont;
        auto enclosure     = useShowOnStart ? scorePart->startEnclosure : scorePart->multipleEnclosure;
        auto useEnclosure  = useShowOnStart ? scorePart->useStartEncl : scorePart->useMultipleEncl;
        auto justification = useShowOnStart ? scorePart->startJustify : scorePart->multipleJustify;
        auto alignment     = useShowOnStart ? scorePart->startAlign : scorePart->multipleAlign;
        auto horizontal    = useShowOnStart ? scorePart->startXdisp : scorePart->multipleXdisp;
        auto vertical      = useShowOnStart ? scorePart->startYdisp : scorePart->multipleYdisp;

        setStyle(style, Sid::measureNumberAlignToBarline, alignment == AlignJustify::Left);
        setStyle(style, Sid::measureNumberOffsetType, int(OffsetType::SPATIUM)); // Hardcoded offset type
        processSegment(fontInfo, useEnclosure ? enclosure.get() : nullptr, justification, alignment, horizontal, vertical, "measureNumber");
        /// @todo write other stored styles to measureNumberAlternate (VPlacement/HPlacement not supported)
        processSegment(fontInfo, useEnclosure ? enclosure.get() : nullptr,
                       justification, alignment, horizontal, vertical, "measureNumberAlternate");

        setStyle(style, Sid::mmRestShowMeasureNumberRange, scorePart->showMmRange);
        if (scorePart->leftMmBracketChar == 0) {
            setStyle(style, Sid::mmRestRangeBracketType, int(MMRestRangeBracketType::NONE));
        } else if (scorePart->leftMmBracketChar == U'(') {
            setStyle(style, Sid::mmRestRangeBracketType, int(MMRestRangeBracketType::PARENTHESES));
        } else {
            setStyle(style, Sid::mmRestRangeBracketType, int(MMRestRangeBracketType::BRACKETS));
        }

        processSegment(scorePart->mmRestFont, nullptr, scorePart->mmRestJustify, scorePart->mmRestAlign,
                       scorePart->mmRestXdisp, scorePart->mmRestYdisp, "mmRestRange");
    }

    const auto mmRests = context.musxDocument()->getOthers()->getArray<others::MultimeasureRest>(context.currentMusxPartId());
    /// @todo create the mm rests correctly
    setStyle(style, Sid::createMultiMeasureRests, context.partScore() || !mmRests.empty());
    setStyle(style, Sid::minEmptyMeasures, prefs.mmRestOptions->numStart);
    writeEvpuSpace(style, Sid::minMMRestWidth, prefs.mmRestOptions->measWidth);
    setStyle(style, Sid::mmRestNumberPos, doubleFromEvpu(prefs.mmRestOptions->numAdjY) + 1);
    setStyle(style, Sid::oldStyleMultiMeasureRests,
             prefs.mmRestOptions->useSymbols && prefs.mmRestOptions->useSymsThreshold > 1);
    setStyle(style, Sid::mmRestOldStyleMaxMeasures,
             std::max(prefs.mmRestOptions->useSymsThreshold - 1, 0));
    writeEvpuSpace(style, Sid::mmRestOldStyleSpacing, prefs.mmRestOptions->symSpacing);
}

static void writeRepeatEndingPrefs(MStyle& style, const FinaleParser& context)
{
    const auto& repeatOptions = context.musxOptions().repeatOptions;

    writeEfixSpace(style, Sid::voltaLineWidth, repeatOptions->bracketLineWidth);
    writeEvpuPointF(style, Sid::voltaPosAbove, 0, -repeatOptions->bracketHeight);
    writeEvpuSpace(style, Sid::voltaHook, repeatOptions->bracketHookLen);
    setStyle(style, Sid::voltaLineStyle, LineType::SOLID);
    writeDefaultFontPref(style, context, "volta", FontOptions::FontType::Ending);
    // setStyle(style, Sid::voltaAlign, Align(AlignH::LEFT, AlignV::BASELINE));
    writeEvpuPointF(style, Sid::voltaOffset, repeatOptions->bracketTextHPos,
                    repeatOptions->bracketHookLen - repeatOptions->bracketTextHPos);
    // setStyle(style, Sid::voltaAlignStartBeforeKeySig, false);
    // This option actually moves the front of the volta after the repeat forwards.
    // Finale only has the option to move the end of the volta before the repeat backwards, so we leave this unset.
    // setStyle(style, Sid::voltaAlignEndLeftOfBarline, false);
}

static void writeTupletPrefs(MStyle& style, const FinaleParser& context)
{
    const auto& tupletOptions = context.musxOptions().tupletOptions;

    setStyle(style, Sid::tupletOutOfStaff, tupletOptions->avoidStaff);
    setStyle(style, Sid::tupletNumberRythmicCenter, tupletOptions->metricCenter);
    setStyle(style, Sid::tupletExtendToEndOfDuration, tupletOptions->fullDura);
    writeEvpuSpace(style, Sid::tupletStemLeftDistance, tupletOptions->leftHookExt);
    writeEvpuSpace(style, Sid::tupletStemRightDistance, tupletOptions->rightHookExt);
    writeEvpuSpace(style, Sid::tupletNoteLeftDistance, tupletOptions->leftHookExt);
    writeEvpuSpace(style, Sid::tupletNoteRightDistance, tupletOptions->rightHookExt);
    writeEfixSpace(style, Sid::tupletBracketWidth, tupletOptions->tupLineWidth);

    // manualSlopeAdj does not translate well, so else leave value as default
    if (tupletOptions->alwaysFlat) {
        setStyle(style, Sid::tupletMaxSlope, 0.0);
    }

    switch (tupletOptions->posStyle) {
    case TupletOptions::PositioningStyle::Above:
        setStyle(style, Sid::tupletDirection, DirectionV::UP);
        break;
    case TupletOptions::PositioningStyle::Below:
        setStyle(style, Sid::tupletDirection, DirectionV::DOWN);
        break;
    default:
        setStyle(style, Sid::tupletDirection, DirectionV::AUTO);
        break;
    }

    setStyle(style, Sid::tupletNumberType, int(toMuseScoreTupletNumberType(tupletOptions->numStyle)));

    if (tupletOptions->brackStyle == TupletOptions::BracketStyle::Nothing) {
        setStyle(style, Sid::tupletBracketType, int(TupletBracketType::SHOW_NO_BRACKET));
    } else if (tupletOptions->autoBracketStyle == TupletOptions::AutoBracketStyle::Always) {
        setStyle(style, Sid::tupletBracketType, int(TupletBracketType::SHOW_BRACKET));
    } else {
        setStyle(style, Sid::tupletBracketType, int(TupletBracketType::AUTO_BRACKET));
    }

    writeEvpuSpace(style, Sid::tupletBracketHookHeight, -(std::max)(tupletOptions->leftHookLen, tupletOptions->rightHookLen)); /// or use average

    if (const auto fontInfo = FontOptions::getFontInfo(context.musxDocument(), FontOptions::FontType::Tuplet)) {
        if (context.fontIsEngravingFont(fontInfo)) {
            setStyle(style, Sid::tupletMusicalSymbolsScale, museMagVal(context, FontOptions::FontType::Tuplet));
            setStyle(style, Sid::tupletUseSymbols, true);
        } else {
            writeFontPref(style, "tuplet", fontInfo);
            setStyle(style, Sid::tupletMusicalSymbolsScale, 1.0);
            setStyle(style, Sid::tupletUseSymbols, false);
        }
    } else {
        context.logger()->logWarning(String(u"Unable to load font pref for tuplets"));
    }
}

static void writeMarkingPrefs(MStyle& style, const FinaleParser& context)
{
    using FontType = FontOptions::FontType;
    using CategoryType = others::MarkingCategory::CategoryType;
    const auto& prefs = context.musxOptions();

    const auto cat
        = context.musxDocument()->getOthers()->get<others::MarkingCategory>(context.currentMusxPartId(), Cmper(CategoryType::Dynamics));
    if (cat && cat->musicFont) {
        if (context.fontIsEngravingFont(cat->musicFont) && !cat->musicFont->calcIsDefaultMusic()) {
            setStyle(style, Sid::dynamicsOverrideFont, true);
            setStyle(style, Sid::dynamicsFont, String::fromStdString(cat->musicFont->getName()));
        } else {
            setStyle(style, Sid::dynamicsOverrideFont, false);
            setStyle(style, Sid::dynamicsFont, prefs.calculatedEngravingFontName);
        }
        setStyle(style, Sid::dynamicsSize, double(cat->musicFont->fontSize) / double(prefs.defaultMusicFont->fontSize));
    } else {
        context.logger()->logWarning(String(u"unable to find MarkingCategory for dynamics"));
    }

    writeCategoryTextFontPref(style, context, "dynamics", CategoryType::Dynamics);
    writeCategoryTextFontPref(style, context, "expression", CategoryType::ExpressiveText);
    writeCategoryTextFontPref(style, context, "tempo", CategoryType::TempoMarks);
    writeCategoryTextFontPref(style, context, "tempoChange", CategoryType::TempoAlterations);
    writeCategoryTextFontPref(style, context, "metronome", CategoryType::TempoMarks);
    writeCategoryTextFontPref(style, context, "systemText", CategoryType::ExpressiveText);
    writeCategoryTextFontPref(style, context, "staffText", CategoryType::TechniqueText);
    writeCategoryTextFontPref(style, context, "rehearsalMark", CategoryType::RehearsalMarks);

    writeDefaultFontPref(style, context, "longInstrument", FontType::StaffNames);
    writeDefaultFontPref(style, context, "shortInstrument", FontType::AbbrvStaffNames);
    writeDefaultFontPref(style, context, "partInstrument", FontType::StaffNames);
    writeDefaultFontPref(style, context, "tabFretNumber",  FontType::Tablature);
    writeDefaultFontPref(style, context, "repeatLeft", FontType::Repeat);
    writeDefaultFontPref(style, context, "repeatRight", FontType::Repeat);
    writeDefaultFontPref(style, context, "repeatPlayCount", FontType::Repeat);
    writeDefaultFontPref(style, context, "chordSymbolA", FontType::Chord);
    writeDefaultFontPref(style, context, "chordSymbolB", FontType::Chord);
    writeDefaultFontPref(style, context, "nashvilleNumber", FontType::Chord);
    writeDefaultFontPref(style, context, "romanNumeral", FontType::Chord);
    writeDefaultFontPref(style, context, "ottava", FontType::SmartShape8va);

    if (const auto fullPosition = prefs.staffOptions->namePos) {
        setStyle(style, Sid::longInstrumentAlign, Align(toAlignH(fullPosition->justify), AlignV::VCENTER));
        setStyle(style, Sid::longInstrumentPosition, toAlignH(fullPosition->hAlign));
    } else {
        context.logger()->logWarning(String(u"unable to find default full name positioning for staves"));
    }

    if (const auto abbreviatedPosition = prefs.staffOptions->namePosAbbrv) {
        setStyle(style, Sid::shortInstrumentAlign, Align(toAlignH(abbreviatedPosition->justify), AlignV::VCENTER));
        setStyle(style, Sid::shortInstrumentPosition, toAlignH(abbreviatedPosition->hAlign));
    } else {
        context.logger()->logWarning(String(u"unable to find default abbreviated name positioning for staves"));
    }

    setStyle(style, Sid::fretMag, doubleFromPercent(prefs.chordOptions->fretPercent));
    setStyle(style, Sid::chordSymPosition,
             prefs.chordOptions->chordAlignment == ChordOptions::ChordAlignment::Left ? AlignH::LEFT : AlignH::HCENTER);
    setStyle(style, Sid::barreAppearanceSlur, true); // Not detectable (uses shapes), but default in most templates
    // setStyle(style, Sid::verticallyAlignChordSymbols, false); // Otherwise offsets are not accounted for

    static const std::unordered_map<ChordOptions::ChordStyle, NoteSpellingType> spellingTypeTable = {
        // { ChordOptions::ChordStyle::Standard, NoteSpellingType::STANDARD },
        // { ChordOptions::ChordStyle::European, NoteSpellingType::STANDARD },
        { ChordOptions::ChordStyle::German,       NoteSpellingType::GERMAN_PURE },
        // { ChordOptions::ChordStyle::Roman,      NoteSpellingType::STANDARD },
        // { ChordOptions::ChordStyle::NashvilleA, NoteSpellingType::STANDARD },
        // { ChordOptions::ChordStyle::NashvilleB, NoteSpellingType::STANDARD },
        // { ChordOptions::ChordStyle::Solfeggio,  NoteSpellingType::STANDARD },
        { ChordOptions::ChordStyle::Scandinavian, NoteSpellingType::GERMAN },
    };
    setStyle(style, Sid::chordSymbolSpelling, muse::value(spellingTypeTable, prefs.chordOptions->chordStyle, NoteSpellingType::STANDARD));

    if (const auto textBlockFont = FontOptions::getFontInfo(context.musxDocument(), FontType::TextBlock)) {
        writeFontPref(style, "default", textBlockFont);
        setStyle(style, Sid::titleFontFace, String::fromStdString(textBlockFont->getName()));
        setStyle(style, Sid::subTitleFontFace, String::fromStdString(textBlockFont->getName()));
        setStyle(style, Sid::composerFontFace, String::fromStdString(textBlockFont->getName()));
        setStyle(style, Sid::lyricistFontFace, String::fromStdString(textBlockFont->getName()));
        setStyle(style, Sid::translatorFontFace, String::fromStdString(textBlockFont->getName()));
        writeFontPref(style, "frame", textBlockFont);
        for (const std::string& prefix : solidLinesWithHooks) {
            writeFontPref(style, prefix, textBlockFont);
        }
        for (const std::string& prefix : solidLinesNoHooks) {
            writeFontPref(style, prefix, textBlockFont);
        }
        writeFontPref(style, "bend", textBlockFont);
        writeFontPref(style, "header", textBlockFont);
        writeFontPref(style, "footer", textBlockFont);
        writeFontPref(style, "copyright", textBlockFont);
        writeFontPref(style, "pageNumber", textBlockFont);
        writeFontPref(style, "instrumentChange", textBlockFont);
        writeFontPref(style, "sticking", textBlockFont);
        writeFontPref(style, "fingering", textBlockFont);
        for (int i = 1; i <= 12; ++i) {
            writeFontPref(style, "user" + std::to_string(i), textBlockFont);
        }
    } else {
        context.logger()->logWarning(String(u"unable to find font prefs for Text Blocks"));
    }
}

void FinaleParser::importStyles()
{
    MStyle& style = m_score->style();
    writePagePrefs(style, *this);
    writeLyricsPrefs(style, *this);
    writeLineMeasurePrefs(style, *this);
    writeStemPrefs(style, *this);
    writeMusicSpacingPrefs(style, *this);
    writeNoteRelatedPrefs(style, *this);
    writeSmartShapePrefs(style, *this);
    writeMeasureNumberPrefs(style, *this);
    writeRepeatEndingPrefs(style, *this);
    writeTupletPrefs(style, *this);
    writeMarkingPrefs(style, *this);
}

void FinaleParser::repositionMeasureNumbersBelow()
{
    const auto& scorePart = musxOptions().measNumScorePart;
    if (!scorePart) {
        return;
    }

    const bool useShowOnStart = scorePart->showOnStart && !scorePart->showOnEvery;
    auto fontInfo      = useShowOnStart ? scorePart->startFont : scorePart->multipleFont;
    auto horizontal    = useShowOnStart ? scorePart->startXdisp : scorePart->multipleXdisp;
    auto vertical      = useShowOnStart ? scorePart->startYdisp : scorePart->multipleYdisp;
    if (vertical >= 0 && scorePart->mmRestYdisp >= 0) {
        return;
    }

    std::optional<double> measNumHeightSp;
    std::optional<double> mmRestHeightSp;
    for (Measure* meas = m_score->firstMeasure(); meas; meas = meas->nextMeasure()) {
        for (size_t idx = 0; idx < m_score->staves().size(); idx++) {
            if (!measNumHeightSp) {
                if (MeasureNumber* measNum = meas->measureNumber(idx)) {
                    measNumHeightSp = measNum->height() / m_score->staff(idx)->spatium(meas->tick());
                }
            }
            if (!mmRestHeightSp) {
                if (MMRestRange* mmRestRange = meas->mmRangeText(idx)) {
                    mmRestHeightSp = mmRestRange->height() / m_score->staff(idx)->spatium(meas->tick());
                }
            }
            if (measNumHeightSp && mmRestHeightSp) {
                break;
            }
        }
        if (measNumHeightSp && mmRestHeightSp) {
            break;
        }
    }

    if (!measNumHeightSp && !mmRestHeightSp) {
        return;
    } else if (!mmRestHeightSp) {
        if (fontInfo->isSame(*scorePart->mmRestFont)) {
            mmRestHeightSp = measNumHeightSp;
        }
    }

    if (measNumHeightSp) {
        setMeasureNumberPosBelow(m_score->style(), "measureNumber", horizontal, vertical, measNumHeightSp.value());
        setMeasureNumberPosBelow(m_score->style(), "measureNumberAlternate", horizontal, vertical, measNumHeightSp.value());
    }
    if (mmRestHeightSp) {
        setMeasureNumberPosBelow(m_score->style(), "mmRestRange", scorePart->mmRestXdisp, scorePart->mmRestYdisp, mmRestHeightSp.value());
    }
}

static PropertyValue compareStyledProperty(const PropertyValue& oldP, const PropertyValue& newP)
{
    // Properties may not always match (double/spatium)
    if (!(oldP.isValid() && newP.isValid() && (oldP.type() == newP.type()))) {
        return oldP;
    }
    /// @todo add more sensible conflict management and exceptions on a case-by-case basis (perhaps using Pid).
    /// Styles are default values and in most cases don't override existing behaviour,
    /// what's important is sensible results.
    /// Note that non-element properties don't have a Pid, so logic elsewhere may be needed.
    switch (newP.type()) {
    case P_TYPE::POINT:
        /// @todo base offset off placement?
        return (std::abs(oldP.value<PointF>().y()) > std::abs(newP.value<PointF>().y())) ? newP : oldP;
    default:
        break;
    }
    return (oldP == newP) ? oldP : PropertyValue();
}

static PropertyValue getFormattedValue(const EngravingObject* e, const Pid id)
{
    const PropertyValue& v = e->getProperty(id);
    // perhaps always e->style().spatium() ?
    const double sp = e->isEngravingItem() ? toEngravingItem(e)->spatium() : e->style().spatium();
    if (e->isSpannerSegment()) {
        // We only want the y-offset for spanners, as x-offset affects ending pos as well.
        if (id == Pid::OFFSET) {
            PointF p = v.value<PointF>() / sp;
            return PointF(0.0, p.ry());
        }
    }
    // if (v.type() == P_TYPE::SPATIUM) {
    //     return v.value<Spatium>() / sp;
    // }
    // or all point types?
    if (id == Pid::OFFSET) {
        return v.value<PointF>() / sp;
    }
    return v;
}

static PropertyValue styledValueByElement(const EngravingObject* e, const Pid id)
{
    if (e->isSpanner()) {
        const Spanner* s = toSpanner(e);
        assert(!s->spannerSegments().empty());
        const EngravingObject* fs = s->frontSegment();

        // Shortcut
        if (fs->propertyDelegate(id) == s) {
            return getFormattedValue(s, id);
        }
        PropertyValue v = getFormattedValue(fs, id);
        for (SpannerSegment* ss : s->spannerSegments()) {
            if (!v.isValid()) {
                break;
            }
            v = compareStyledProperty(v, getFormattedValue(ss, id));
        }
        return v;
    }
    return getFormattedValue(e, id);
}

void FinaleParser::collectElementStyle(const EngravingObject* e)
{
    for (int i = 0; i < static_cast<int>(Pid::END); ++i) {
        const Pid propertyId = static_cast<Pid>(i);
        Sid styleId = e->getPropertyStyle(propertyId);
        if (styleId == Sid::NOSTYLE) {
            continue;
        }
        if (!importCustomPositions() && propertyId == Pid::OFFSET) {
            continue;
        }
        logger()->logDebugTrace(String(u"Collecting property %1").arg(propertyUserName(propertyId)));
        collectGlobalProperty(styleId, styledValueByElement(e, propertyId));
    }
}

void FinaleParser::collectGlobalProperty(const Sid styleId, const PropertyValue& newV)
{
    if (!newV.isValid()) {
        return;
    }
    if (muse::contains(m_elementStyles, styleId)) {
        // Replace currently found value with new match, assuming there has been no bad match
        PropertyValue v = muse::value(m_elementStyles, styleId);
        if (v.isValid()) {
            muse::remove(m_elementStyles, styleId);
            m_elementStyles.emplace(styleId, compareStyledProperty(v, newV));
        }
    } else {
        m_elementStyles.emplace(styleId, newV);
    }
}

void FinaleParser::collectGlobalFont(const std::string& namePrefix, const MusxInstance<FontInfo>& fontInfo)
{
    FontTracker converted(fontInfo, score()->style().defaultSpatium());
    collectGlobalProperty(styleIdx(namePrefix + "FontFace"), converted.fontName);
    collectGlobalProperty(styleIdx(namePrefix + "FontSize"), converted.fontSize);
    collectGlobalProperty(styleIdx(namePrefix + "FontSpatiumDependent"), converted.spatiumDependent);
    collectGlobalProperty(styleIdx(namePrefix + "FontStyle"), int(converted.fontStyle));
}
}
