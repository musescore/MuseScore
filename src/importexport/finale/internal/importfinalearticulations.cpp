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

#include <algorithm>
#include <vector>
#include <exception>
#include <optional>

#include "musx/musx.h"
#include "smufl_mapping.h"

#include "types/string.h"

#include "engraving/dom/accidental.h"
#include "engraving/dom/arpeggio.h"
#include "engraving/dom/articulation.h"
#include "engraving/dom/breath.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/chordbracket.h"
#include "engraving/dom/chordline.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/fingering.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/navigate.h"
#include "engraving/dom/note.h"
#include "engraving/dom/ornament.h"
#include "engraving/dom/parenthesis.h"
#include "engraving/dom/part.h"
#include "engraving/dom/pedal.h"
#include "engraving/dom/playtechannotation.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/score.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stem.h"
#include "engraving/dom/system.h"
#include "engraving/dom/textbase.h"
#include "engraving/dom/tremolosinglechord.h"
#include "engraving/dom/utils.h"

#include "engraving/editing/transpose.h"

#include "engraving/types/symnames.h"

#include "log.h"

using namespace mu::engraving;
using namespace muse;
using namespace musx::dom;

namespace mu::iex::finale {
static const std::vector<OrnamentDefinition> ornamentList {
    { String(u"ornamentTrillFlatAbove"),          SymId::ornamentTrill, AccidentalType::FLAT,    AccidentalType::NONE },
    { String(u"ornamentTrillFlatAboveLegacy"),    SymId::ornamentTrill, AccidentalType::FLAT,    AccidentalType::NONE },
    { String(u"ornamentTrillNaturalAbove"),       SymId::ornamentTrill, AccidentalType::NATURAL, AccidentalType::NONE },
    { String(u"ornamentTrillNaturalAboveLegacy"), SymId::ornamentTrill, AccidentalType::NATURAL, AccidentalType::NONE },
    { String(u"ornamentTrillSharpAbove"),         SymId::ornamentTrill, AccidentalType::SHARP,   AccidentalType::NONE },
    { String(u"ornamentTrillSharpAboveLegacy"),   SymId::ornamentTrill, AccidentalType::SHARP,   AccidentalType::NONE },
    { String(u"ornamentTurnFlatAbove"),           SymId::ornamentTurn,  AccidentalType::FLAT,    AccidentalType::NONE },
    { String(u"ornamentTurnFlatAboveSharpBelow"), SymId::ornamentTurn,  AccidentalType::FLAT,    AccidentalType::SHARP },
    { String(u"ornamentTurnFlatBelow"),           SymId::ornamentTurn,  AccidentalType::NONE,    AccidentalType::FLAT },
    { String(u"ornamentTurnNaturalAbove"),        SymId::ornamentTurn,  AccidentalType::NATURAL, AccidentalType::NONE },
    { String(u"ornamentTurnNaturalBelow"),        SymId::ornamentTurn,  AccidentalType::NONE,    AccidentalType::NATURAL },
    { String(u"ornamentTurnSharpAbove"),          SymId::ornamentTurn,  AccidentalType::SHARP,   AccidentalType::NONE },
    { String(u"ornamentTurnSharpAboveFlatBelow"), SymId::ornamentTurn,  AccidentalType::SHARP,   AccidentalType::FLAT },
    { String(u"ornamentTurnSharpBelow"),          SymId::ornamentTurn,  AccidentalType::NONE,    AccidentalType::SHARP },
};

static const std::unordered_set<SymId> ornamentSymbols = {
    SymId::ornamentTurnInverted,
    SymId::ornamentMordent,
    SymId::ornamentTrill,
    SymId::ornamentTurn,
    SymId::ornamentShortTrill,
    SymId::ornamentTremblement,
    SymId::ornamentUpPrall,
    SymId::ornamentPrallUp,
    SymId::ornamentPrallDown,
    SymId::ornamentPrallMordent,
    SymId::ornamentUpMordent,
    SymId::ornamentDownMordent,
    SymId::ornamentPrecompMordentUpperPrefix,
    SymId::ornamentTurnSlash,
};

static const std::unordered_set<SymId> pedalEndTypes = {
    SymId::keyboardPedalUp,
    SymId::keyboardPedalUpNotch,
    SymId::keyboardPedalUpSpecial,
};

// Copied from Articulation::anchorGroup
static const std::unordered_set<SymId> recognisedArticulations = {
    SymId::articAccentAbove,
    SymId::articAccentBelow,
    SymId::articStaccatoAbove,
    SymId::articStaccatoBelow,
    SymId::articStaccatissimoAbove,
    SymId::articStaccatissimoBelow,
    SymId::articTenutoAbove,
    SymId::articTenutoBelow,
    SymId::articTenutoStaccatoAbove,
    SymId::articTenutoStaccatoBelow,
    SymId::articMarcatoAbove,
    SymId::articMarcatoBelow,

    SymId::articAccentStaccatoAbove,
    SymId::articAccentStaccatoBelow,
    SymId::articLaissezVibrerAbove,
    SymId::articLaissezVibrerBelow,
    SymId::articMarcatoStaccatoAbove,
    SymId::articMarcatoStaccatoBelow,
    SymId::articMarcatoTenutoAbove,
    SymId::articMarcatoTenutoBelow,
    SymId::articStaccatissimoStrokeAbove,
    SymId::articStaccatissimoStrokeBelow,
    SymId::articStaccatissimoWedgeAbove,
    SymId::articStaccatissimoWedgeBelow,
    SymId::articStressAbove,
    SymId::articStressBelow,
    SymId::articTenutoAccentAbove,
    SymId::articTenutoAccentBelow,
    SymId::articUnstressAbove,
    SymId::articUnstressBelow,

    SymId::articSoftAccentAbove,
    SymId::articSoftAccentBelow,
    SymId::articSoftAccentStaccatoAbove,
    SymId::articSoftAccentStaccatoBelow,
    SymId::articSoftAccentTenutoAbove,
    SymId::articSoftAccentTenutoBelow,
    SymId::articSoftAccentTenutoStaccatoAbove,
    SymId::articSoftAccentTenutoStaccatoBelow,

    SymId::wiggleSawtooth,
    SymId::wiggleSawtoothWide,
    SymId::wiggleVibratoLargeFaster,
    SymId::wiggleVibratoLargeSlowest,

    SymId::luteFingeringRHThumb,
    SymId::luteFingeringRHFirst,
    SymId::luteFingeringRHSecond,
    SymId::luteFingeringRHThird,

    SymId::tremoloDivisiDots2,
    SymId::tremoloDivisiDots3,
    SymId::tremoloDivisiDots4,
    SymId::tremoloDivisiDots6,
};

static ArpeggioType arpeggioTypeFromSpan(const musx::util::ArpeggioSpanCandidate& span)
{
    switch (span.direction) {
    case musx::util::ArpeggioDirection::Down:
        return ArpeggioType::DOWN;
    case musx::util::ArpeggioDirection::Up:
        return ArpeggioType::UP;
    case musx::util::ArpeggioDirection::Auto:
        break;
    }

    switch (span.arrow) {
    case musx::util::ArpeggioArrow::Down:
        return ArpeggioType::DOWN;
    case musx::util::ArpeggioArrow::Up:
        return ArpeggioType::UP;
    case musx::util::ArpeggioArrow::Auto:
    case musx::util::ArpeggioArrow::None:
        return ArpeggioType::NORMAL;
    }

    return ArpeggioType::NORMAL;
}

static std::optional<musx::util::ArpeggioSpanCandidate> singleGlyphArpeggioSpan(
    const EntryInfoPtr& sourceEntry, const String& symName)
{
    std::optional<musx::util::ArpeggioSpanCandidate> result;
    auto makeArpeggioCandidate = [&](musx::util::ArpeggioDirection direction, musx::util::ArpeggioArrow arrow) {
        auto& candidate = result.emplace();
        candidate.type = musx::util::ArpeggioSpanType::Normal;
        candidate.sourceEntry = sourceEntry;
        candidate.topEntry = sourceEntry;
        candidate.bottomEntry = sourceEntry;
        candidate.direction = direction;
        candidate.arrow = arrow;
    };

    if (symName == String(u"arpeggiato")) {
        makeArpeggioCandidate(musx::util::ArpeggioDirection::Auto, musx::util::ArpeggioArrow::None);
    } else if (symName == String(u"arpeggiatoUp")) {
        makeArpeggioCandidate(musx::util::ArpeggioDirection::Up, musx::util::ArpeggioArrow::Up);
    } else if (symName == String(u"arpeggiatoDown")) {
        makeArpeggioCandidate(musx::util::ArpeggioDirection::Down, musx::util::ArpeggioArrow::Down);
    }

    return result;
}

musx::util::ArpeggioSpanOptions FinaleParser::arpeggioSpanOptions(bool skipGraceEntries) const
{
    musx::util::ArpeggioSpanOptions options;
    options.skipGraceEntries = skipGraceEntries;
    options.staffOriginOffsetResolver = [this](const DocumentPtr& document, Cmper partId,
                                                const musx::util::StaffOriginOffsetRequest& request) {
        const staff_idx_t sourceStaffIdx = muse::value(m_inst2Staff, request.sourceStaffId, muse::nidx);
        const staff_idx_t targetStaffIdx = muse::value(m_inst2Staff, request.targetStaffId, muse::nidx);
        const Fraction tick = muse::value(m_meas2Tick, request.measureId, Fraction(-1, 1));
        Measure* measure = tick == Fraction(-1, 1) ? nullptr : m_score->tick2measure(tick);
        const System* system = measure ? measure->system() : nullptr;
        const auto musxSystem = document ? document->calcSystemFromMeasure(partId, request.measureId) : nullptr;

        if (sourceStaffIdx == muse::nidx || targetStaffIdx == muse::nidx || !system || !musxSystem) {
            return musx::util::StaffOriginOffsetResolverResult {
                musx::util::StaffOriginOffsetResolverDecision::Unavailable, 0.0
            };
        }
        if (!system->staff(sourceStaffIdx)->show() || !system->staff(targetStaffIdx)->show()) {
            return musx::util::StaffOriginOffsetResolverResult {
                musx::util::StaffOriginOffsetResolverDecision::Unavailable, 0.0
            };
        }

        const double sourceY = system->staff(sourceStaffIdx)->y();
        const double targetY = system->staff(targetStaffIdx)->y();
        const double systemSpatium = musxSystem->calcEffectiveScaling().toDouble() * FINALE_DEFAULT_SPATIUM;
        IF_ASSERT_FAILED(systemSpatium > 0.0) {
            return musx::util::StaffOriginOffsetResolverResult {
                musx::util::StaffOriginOffsetResolverDecision::Unavailable, 0.0
            };
        }
        return musx::util::StaffOriginOffsetResolverResult {
            musx::util::StaffOriginOffsetResolverDecision::Offset,
            (targetY - sourceY) * EVPU_PER_SPACE / systemSpatium
        };
    };
    return options;
}

bool FinaleParser::createArpeggioFromSpan(const musx::util::ArpeggioSpanCandidate& span,
                                          const MusxInstance<others::ArticulationDef>& articDef,
                                          bool visible)
{
    ChordRest* topCR = chordRestFromEntryInfoPtr(span.topEntry);
    ChordRest* bottomCR = chordRestFromEntryInfoPtr(span.bottomEntry);
    if (!topCR || !bottomCR || !topCR->isChord() || !bottomCR->isChord()) {
        return false;
    }
    if (topCR->part() != bottomCR->part()) {
        return false;
    }

    Chord* arpChord = toChord(topCR);
    if (span.type == musx::util::ArpeggioSpanType::Normal && arpChord->arpeggio()) {
        return false;
    }

    track_idx_t topChordTrack = arpChord->track();
    track_idx_t bottomChordTrack = std::min(bottomCR->track(), m_score->staff(track2staff(topChordTrack))->part()->endTrack() - 1);
    Segment* segment = arpChord->segment();
    if (!segment || bottomChordTrack < topChordTrack) {
        return false;
    }

    Arpeggio* arpeggio = nullptr;
    if (span.type == musx::util::ArpeggioSpanType::Bracket) {
        arpeggio = Factory::createArpeggio(arpChord);
        arpeggio->setArpeggioType(ArpeggioType::BRACKET);
    } else {
        arpeggio = Factory::createArpeggio(arpChord);
        arpeggio->setArpeggioType(arpeggioTypeFromSpan(span));
    }

    arpeggio->setTrack(topChordTrack);
    arpeggio->setVisible(visible);
    arpeggio->setSpan(int(bottomChordTrack + 1 - topChordTrack));
    for (track_idx_t track = topChordTrack; track <= bottomChordTrack; ++track) {
        if (segment->element(track) && segment->element(track)->isChord()) {
            toChord(segment->element(track))->setSpanArpeggio(arpeggio);
        }
    }

    if (span.type == musx::util::ArpeggioSpanType::Normal) {
        arpeggio->setPlayArpeggio(articDef ? articDef->playArtic : true);
        if (articDef) {
            Fraction totalArpDuration = eduToFraction(articDef->startTopNoteDelta - articDef->startBotNoteDelta);
            double beatsPerSecondRatio = m_score->tempo(segment->tick()).val / 2.0; // ratio against 120 bpm
            if (beatsPerSecondRatio > 0.0 && !arpChord->notes().empty()) {
                double timeIn120BPM = totalArpDuration.toDouble() / beatsPerSecondRatio;
                arpeggio->setStretch(timeIn120BPM * 8.0 / arpChord->notes().size());
            }
        }
    } else {
        arpeggio->setPlayArpeggio(false);
    }

    arpChord->add(arpeggio);
    return true;
}

ReadableArticulation::ReadableArticulation(FinaleParser& ctx, const MusxInstance<others::ArticulationDef>& articDef)
{
    auto calcArticSymbol = [&](char32_t theChar, const MusxInstance<FontInfo>& font,
                               bool isShape, Cmper shapeId) -> bool {
        if (isShape) {
            if (const auto shape = ctx.musxDocument()->getOthers()->get<others::ShapeDef>(ctx.currentMusxPartId(), shapeId)) {
                switch (shape->recognize()) {
                case KnownShapeDefType::TenutoMark:
                    articSym = SymId::articTenutoAbove;     // MuseScore figures out the actual above/below symbol
                    return true;
                /// @todo: add other cases if ever defined in musxdom
                default:
                    break;
                }
            }
            symName = String::fromAscii(SymNames::nameForSymId(articSym).ascii());
        } else {
            IF_ASSERT_FAILED(theChar) {
                ctx.logger()->logWarning(String(u"Incomplete shape!"));
                return false;
            }
            articSym = FinaleTextConv::symIdFromFinaleChar(theChar, font); // articDef fonts are guaranteed non-null by musxdom
            isMusicalSymbol = ctx.fontIsEngravingFont(font, true);
            articChar = isMusicalSymbol ? FinaleTextConv::mappedChar(theChar, font) : theChar;
            if (articSym == SymId::noSym) {
                symName = String::fromStdString(FinaleTextConv::charNameFinale(theChar, font));
            } else {
                symName = String::fromAscii(SymNames::nameForSymId(articSym).ascii());
            }
            // double articulationMag = FontTracker(font, ctx.score()->style().spatium()).symbolsSize / SYMBOLS_DEFAULT_SIZE;
            // ctx.collectGlobalProperty(Sid::articulationMag, articulationMag);
            return true;
        }
        return false;
    };

    if (calcArticSymbol(articDef->charMain, articDef->fontMain, articDef->mainIsShape, articDef->mainShape)) {
    } else if (calcArticSymbol(articDef->charAlt, articDef->fontAlt, articDef->altIsShape, articDef->altShape)) {
    } else {
        unrecognised = true;
        return;
    }

    isArticulation = muse::contains(recognisedArticulations, articSym);

    ctx.logger()->logInfo(String(u"Added articulation %1 with symbol %2 to library.").arg(String::number(articDef->getCmper()), symName));

    chordLineType = ChordLineType::NOTYPE; // overridden later

    if (articSym == SymId::noteheadParenthesisLeft || (!isMusicalSymbol && articChar.value() == U'(')) {
        isLeftNoteheadParen = true;
    } else if (articSym == SymId::noteheadParenthesisRight || (!isMusicalSymbol && articChar.value() == U')')) {
        isRightNoteheadParen = true;
    } else if (!isMusicalSymbol) {
        /// @todo figured bass
        String articText = String::fromUcs4(articChar.value());
        if (articText.contains(std::wregex(LR"([A-z]|[0-9])"))) {
            isFingering = true;
            isSticking = articText.contains(std::wregex(LR"(L|R)"));
        } else {
            unrecognised = true;
            return;
        }
    } else if (symName.startsWith(u"keyboardPedal")) {
        isPedalSym = true;
        isPedalEnd = muse::contains(pedalEndTypes, articSym);
    } else if (symName.startsWith(u"keyboardPlayWith")) {
        isChordBracket = true;
    } else if (fermataTypeFromSymId(articSym) != FermataType::Undefined) {
        isFermataSym = true;
    } else if (muse::contains(ornamentSymbols, articSym)) {
        isStandardOrnament = true;
    } else if (ctx.importCustomPositions() && symName == String(u"arpeggioVerticalSegment") && articChar.has_value()) {
        muse::draw::FontMetrics fm = FontTracker(articDef->fontMain, ctx.score()->style().spatium()).toFontMetrics();
        double arpeggioWidth = fm.boundingRect(articChar.value()).width();
        double arpeggioDistance = -evpuToSp(articDef->xOffsetMain) - (arpeggioWidth / ctx.score()->style().spatium()); // in sp
        // MuseScore accounts for distance to ledger lines, Finale does not. So we use an in-between value
        arpeggioDistance -= ctx.score()->style().styleS(Sid::ledgerLineLength).val() / 2.0;
        if (arpeggioDistance > 0) {
            ctx.collectGlobalProperty(Sid::arpeggioNoteDistance, Spatium(arpeggioDistance));
            ctx.collectGlobalProperty(Sid::arpeggioAccidentalDistance, Spatium(arpeggioDistance));
        }
    } else if (articSym == SymId::graceNoteAcciaccaturaStemUp || articSym == SymId::graceNoteAcciaccaturaStemDown
               || articSym == SymId::graceNoteAppoggiaturaStemUp || articSym == SymId::graceNoteAppoggiaturaStemDown) {
        isGraceNote = true;
    } else if (symName.startsWith(u"brassScoop") || symName.startsWith(u"brassLift") || symName.startsWith(u"brassPlop")
               || symName.startsWith(u"brassDoit") || symName.startsWith(u"brassFall")) {
        if (symName.contains(u"Scoop")) {
            chordLineType = ChordLineType::SCOOP;
        } else if (symName.contains(u"Plop")) {
            chordLineType = ChordLineType::PLOP;
        } else if (symName.contains(u"Doit")) {
            chordLineType = ChordLineType::DOIT;
        } else {
            chordLineType = ChordLineType::FALL;
        }
    } else {
        for (BreathType bt : Breath::BREATH_LIST) {
            if (articSym == bt.id) {
                breathType = bt;
                break;
            }
        }
        for (OrnamentDefinition od : ornamentList) {
            if (od.name == symName) {
                ornamentDefinition = od;
                break;
            }
        }
    }
    tremoloType = tremoloTypeFromSymId(articSym);
    playTechType = playTechTypeFromSymId(articSym);
}

bool FinaleParser::calculateUp(const MusxInstance<details::ArticulationAssign>& articAssign, others::ArticulationDef::AutoVerticalMode vm,
                               ChordRest* cr)
{
    if (articAssign->overridePlacement) {
        return articAssign->aboveEntry;
    }

    switch (vm) {
    /// @todo Rests can be affected by beams, but should otherwise be treated like a note on their given line.
    case others::ArticulationDef::AutoVerticalMode::AutoNoteStem:
        // On notehead side (no voices) or stem side (voices);
        if (cr->isChord() && muse::contains(m_fixedChords, toChord(cr))) {
            return cr->ldata()->up;
        } else {
            DirectionV dir = getDirectionVForLayer(cr);
            if (dir != DirectionV::AUTO) {
                return dir == DirectionV::UP;
            }
            if (cr->isRest()) {
                return muse::RealIsEqualOrLess(cr->y() * 2, cr->staff()->staffHeight(cr->segment()->tick()));
            }
        }
        [[fallthrough]];

    case others::ArticulationDef::AutoVerticalMode::AlwaysNoteheadSide:
        return !cr->ldata()->up;
    case others::ArticulationDef::AutoVerticalMode::StemSide:
    case others::ArticulationDef::AutoVerticalMode::AlwaysOnStem:
        return cr->ldata()->up;

    case others::ArticulationDef::AutoVerticalMode::AboveEntry:
        return true;
    case others::ArticulationDef::AutoVerticalMode::BelowEntry:
        return false;
    }
    return true;
}

static ArticulationAnchor calculateAnchor(const MusxInstance<details::ArticulationAssign>& articAssign,
                                          others::ArticulationDef::AutoVerticalMode vm, ChordRest* cr)
{
    if (articAssign->overridePlacement) {
        return articAssign->aboveEntry ? ArticulationAnchor::TOP : ArticulationAnchor::BOTTOM;
    }

    switch (vm) {
    /// @todo Rests can be affected by beams, but should otherwise be treated like a note on their given line.
    case others::ArticulationDef::AutoVerticalMode::AutoNoteStem:
        return ArticulationAnchor::AUTO;
    case others::ArticulationDef::AutoVerticalMode::AlwaysNoteheadSide:
        return !cr->ldata()->up ? ArticulationAnchor::TOP : ArticulationAnchor::BOTTOM;
    case others::ArticulationDef::AutoVerticalMode::StemSide:
    case others::ArticulationDef::AutoVerticalMode::AlwaysOnStem:
        return cr->ldata()->up ? ArticulationAnchor::TOP : ArticulationAnchor::BOTTOM;

    case others::ArticulationDef::AutoVerticalMode::AboveEntry:
        return ArticulationAnchor::TOP;
    case others::ArticulationDef::AutoVerticalMode::BelowEntry:
        return ArticulationAnchor::BOTTOM;
    }
    return ArticulationAnchor::AUTO;
}

// Position of articulation in CR coords
PointF FinaleParser::posForArticulation(const MusxInstance<details::ArticulationAssign>& articAssign,
                                        const MusxInstance<others::ArticulationDef>& articDef, ChordRest* cr,
                                        const ReadableArticulation* musxArtic)
{
    if (!cr->ldata()->isSetShape()) {
        return PointF();
    }

    PointF p;
    const bool above = calculateUp(articAssign, articDef->autoVertMode, cr);
    const bool usesAltSymbol = above ? articDef->aboveSymbolAlt : articDef->belowSymbolAlt;

    /// @todo compare with SMuFL info where appropriate - We need the original bbox as well though for weird offsets
    FontTracker scaledArticFont = FontTracker(usesAltSymbol ? articDef->fontAlt : articDef->fontMain, cr->spatium() * cr->mag());
    RectF articBbox = scaledArticFont.toFontMetrics().boundingRect(usesAltSymbol ? articDef->charAlt : articDef->charMain);

    /// @note Y is computed in local staff coords first, needed for staff centering
    double defaultOffset = articDef->autoVert ? ((above ? -1.0 : 1.0) * evpuToSp(articDef->defVertPos)) : 0.0;
    double lineDist = cr->spatium() * cr->staffType()->lineDistance().val();

    if (cr->isChord()) {
        Chord* c = toChord(cr);
        const engraving::Note* xNote = c->ldata()->up ? c->downNote() : c->upNote();
        p.rx() += xNote->pos().x();

        const bool tabStaff = c->staff()->isTabStaff(c->segment()->tick());

        // Vertical pos based on definition
        if (articDef->autoVert) {
            Shape chordShape = c->shape().translate(c->pos());
            chordShape.remove_if([](ShapeElement& shapeEl) {
                return !shapeEl.item() || !shapeEl.item()->isNote() || !shapeEl.item()->isStem();
            });
            if (above) {
                p.ry() += chordShape.top();
            } else {
                p.ry() += chordShape.bottom();
            }
            if (articDef->autoVertMode == others::ArticulationDef::AutoVerticalMode::AlwaysOnStem) {
                if (c->stem()) {
                    p.rx() += c->stem()->pos().x();
                }
                /// @note For on stem placement, the symbol is aligned based on font info.
                /// Notes with no stem (i.e. whole notes) seem to have inverse behaviour for alignment, to avoid collisions.
                if (above == bool(c->stem())) {
                    p.ry() += articBbox.top();
                } else {
                    p.ry() -= articBbox.bottom(); // += ?
                }
            }
        } else {
            // Manual positioning: Relative to vertical center of bottom chord note
            p.ry() += (tabStaff ? 2 * c->downNote()->string() : c->downNote()->line()) * lineDist * .5;
        }

        // Horizontal centering
        if (articDef->autoHorz) {
            p.rx() += cr->symWidth(SymId::noteheadBlack) / 2; // Relative to entry, not individual notes or symbol
            p.rx() -= articBbox.width() / 2;
        }
    } else {
        Rest* r = toRest(cr);

        // Vertical pos based on definition
        if (articDef->autoVert) {
            Shape restShape = r->shape().translate(r->pos());
            restShape.remove_if([](ShapeElement& shapeEl) {
                return !shapeEl.item() || !shapeEl.item()->isRest();
            });
            if (above) {
                p.ry() += restShape.top();
            } else {
                p.ry() += restShape.bottom();
            }
            if (articDef->autoVertMode == others::ArticulationDef::AutoVerticalMode::AlwaysOnStem) {
                /// @note Has no stem, follows inverse behaviour
                if (above) {
                    p.ry() -= articBbox.bottom();
                } else {
                    p.ry() += articBbox.top();
                }
            }
        } else {
            // Manual positioning: Relative to line rest is on
            /// @todo use actual line: For now, vertical center of symbol
            RectF restBbox = r->symBbox(r->ldata()->sym());
            p.ry() += restBbox.top() + restBbox.height() / 2;
        }

        // Horizontal centering
        if (articDef->autoHorz) {
            p.rx() += r->centerX();
            p.rx() -= articBbox.width() / 2;
        }
    }

    // Staff centering, avoid staff
    int line = int(std::lround(((p.ry() / lineDist) + defaultOffset) * 2.0));
    int staffLines = 2 * (cr->staff()->lines(cr->segment()->tick()) - 1);
    if (articDef->outsideStaff && (above ? line > -1 : line < staffLines + 1)) {
        line = above ? -1 : staffLines + 1;
        p.ry() = line * .5 * lineDist;
    } else if (articDef->avoidStaffLines && line > 0 && line < staffLines) {
        if (above) {
            line = std::min(2 * static_cast<int>(std::floor(line / 2)) - 1, staffLines + 1);
        } else {
            line = std::max(2 * static_cast<int>(std::ceil(line / 2)) + 1, -1);
        }
        p.ry() = line * .5 * lineDist;
    } else {
        // Non-manual placed articulations can never fall into ledger lines
        if (articDef->autoVert) {
            if (above) {
                p.ry() = std::min(p.y(), lineDist * (staffLines + 2));
            } else {
                p.ry() = std::max(p.y(), lineDist * -2);
            }
        }
        p.ry() += defaultOffset * lineDist;

        // observed fudge factor
        p.ry() += (above ? -1.0 : 1.0) * m_score->style().styleS(Sid::staffLineWidth).val() * cr->spatium() * .5;
    }

    if (usesAltSymbol) {
        p += evpuToPointF(articDef->xOffsetAlt, -articDef->yOffsetAlt) * cr->spatium();
    } else {
        p += evpuToPointF(articDef->xOffsetMain, -articDef->yOffsetMain) * cr->spatium();
    }

    // MuseScore doesn't use font info for positioning, so remove any offset generated by it.
    // The exception for this is grace notes, where Finale's positioning tells us the line the note is on.
    if (!musxArtic || !musxArtic->symName.startsWith(u"graceNote")) {
        p -= articBbox.topLeft();
    }

    // Add assignment offset
    p += evpuToPointF(articAssign->horzOffset, -articAssign->vertOffset) * cr->spatium();

    // Remove staff chords
    p.ry() -= cr->pos().y();

    /// @todo account for stacking, slur collisions
    return p;
}

static void setOrnamentIntervalFromAccidental(Ornament* o, engraving::Note* n, AccidentalType at, bool above)
{
    // Uses default
    if (at == AccidentalType::NONE) {
        return;
    }

    // Make sure accidentals are visible (prioritise false positives over false negatives)
    int newLine = n->line() + (above ? -1 : 1);
    Chord* c = n->chord();
    bool error;
    AccidentalVal defaultAcc = c->measure()->findAccidental(c->segment(), c->vStaffIdx(), newLine, error);
    AccidentalVal actualAcc = Accidental::subtype2value(at);
    if (actualAcc == defaultAcc) {
        o->setShowAccidental(OrnamentShowAccidental::ALWAYS);
    }

    // Determine interval
    static const std::unordered_map<int, IntervalType> intervalTable = {
        { 0, IntervalType::DIMINISHED },
        { 1, IntervalType::MINOR },
        { 2, IntervalType::MAJOR },
        { 3, IntervalType::AUGMENTED },
    };
    ClefType currClef = c->score()->staff(c->vStaffIdx())->clef(c->segment()->tick());
    int pitchDifference = std::abs(absStep2pitchByKey(absStep(newLine, currClef), Key::C) + int(actualAcc) - n->pitch());
    OrnamentInterval oi(IntervalStep::SECOND, muse::value(intervalTable, pitchDifference, IntervalType::AUTO));
    if (above && o->hasIntervalAbove()) {
        o->setIntervalAbove(oi);
    } else if (!above && o->hasIntervalBelow()) {
        o->setIntervalBelow(oi);
    }
}

void FinaleParser::importArticulations()
{
    std::vector<std::map<int, ReadableArticulation*> > pedalList;
    pedalList.assign(m_score->nstaves(), std::map<int, ReadableArticulation*> {});
    /// @todo finish and apply offset calculations? Would currently mess with slur placement

    auto findClosestNote = [this](const MusxInstance<details::ArticulationAssign>& articAssign,
                                  const MusxInstance<others::ArticulationDef>& articDef, Chord* c) {
        engraving::Note* n = c->upNote();

        // Find closest note, prioritise vertical position
        if (c->notes().size() > 1) {
            const PointF articPos = posForArticulation(articAssign, articDef, toChordRest(c));
            PointF bestMatch = PointF(DBL_MAX, DBL_MAX);
            for (engraving::Note* note : c->notes()) {
                double noteDistX = std::abs(note->pos().x() - articPos.x());
                double noteDistY = std::abs(note->pos().y() - articPos.y());
                if (muse::RealIsEqual(noteDistY, bestMatch.y()) ? noteDistX < bestMatch.x() : noteDistY < bestMatch.y()) {
                    bestMatch = PointF(noteDistX, noteDistY);
                    n = note;
                }
            }
        }
        return n;
    };

    for (auto [entryNumber, cr] : m_entryNumber2CR) {
        const auto articAssignList = m_doc->getDetails()->getArray<details::ArticulationAssign>(m_currentMusxPartId, entryNumber);
        const bool isDrumStaff = cr->staff()->isDrumStaff(cr->segment()->tick());
        for (const MusxInstance<details::ArticulationAssign>& articAssign : articAssignList) {
            const auto& articDef = m_doc->getOthers()->get<others::ArticulationDef>(m_currentMusxPartId, articAssign->articDef);
            ReadableArticulation* musxArtic = [&]() -> ReadableArticulation* {
                // Search our converted shape library, or if not found add to it
                ReadableArticulation* line = muse::value(m_articulations, articAssign->articDef, nullptr);
                if (!line) {
                    line = new ReadableArticulation(*this, articDef);
                    m_articulations.emplace(articAssign->articDef, line);
                }
                return line;
            }();

            if (cr->isChord() && (!musxArtic || musxArtic->unrecognised) && !articAssign->hide) {
                EntryInfoPtr sourceEntry = EntryInfoPtr::fromEntryNumber(m_doc, m_currentMusxPartId, entryNumber);
                if (const auto span = musx::util::calcNonArpeggioSpanForAssignment(sourceEntry, articAssign, arpeggioSpanOptions())) {
                    if (createArpeggioFromSpan(*span)) {
                        continue;
                    }
                }
            }

            // unknown value or shape
            if (!musxArtic || musxArtic->unrecognised) {
                continue;
            }

            // Pedal lines
            if (musxArtic->isPedalSym) {
                pedalList.at(cr->vStaffIdx()).emplace(cr->segment()->tick().ticks(), musxArtic);
                continue;
            }

            // Fermatas
            if (musxArtic->isFermataSym && !cr->isGrace()) {
                Fermata* fermata = Factory::createFermata(cr->segment());
                fermata->setTrack(cr->track());
                fermata->setPlacement(calculateUp(articAssign, articDef->autoVertMode, cr) ? PlacementV::ABOVE : PlacementV::BELOW);
                /// @todo Verify that fermatas have no playback effect in Finale.
                fermata->setSymIdAndTimeStretch(musxArtic->articSym);
                fermata->setPlay(false);
                // fermata->setSymId(mainSym);
                // fermata->setPlay(articDef->playArtic);
                fermata->setVisible(!articAssign->hide);
                cr->segment()->add(fermata);
                collectElementStyle(fermata);
                continue;
            }

            // Breaths and pauses
            if (musxArtic->breathType.has_value()) {
                Segment* breathSegment = cr->measure()->getSegment(SegmentType::Breath, cr->endTick());
                Breath* breath = Factory::createBreath(breathSegment);
                breath->setTrack(cr->track());
                breath->setPlacement(calculateUp(articAssign, articDef->autoVertMode, cr) ? PlacementV::BELOW : PlacementV::ABOVE);
                breath->setSymId(musxArtic->breathType.value().id);
                // breath->setPause(musxArtic->breathType.value().pause); until there is a toggleable play property, leave unset
                breath->setVisible(!articAssign->hide);
                breathSegment->add(breath);
                collectElementStyle(breath);
                continue;
            }

            // Sticking / Fingering for rests (added as staff text)
            if ((isDrumStaff && musxArtic->isSticking) || (cr->isRest() && musxArtic->isFingering)) {
                ElementType elementType = musxArtic->isSticking ? ElementType::STICKING : ElementType::STAFF_TEXT;
                TextBase* fingering = toTextBase(Factory::createItem(elementType, m_score->dummy()));
                fingering->setTrack(cr->track());
                fingering->setPlainText(String::fromUcs4(musxArtic->articChar.value()));
                FontTracker ft(articDef->fontMain);
                setAndStyleProperty(fingering, Pid::FONT_STYLE, int(ft.fontStyle));
                setAndStyleProperty(fingering, Pid::FONT_FACE, ft.fontName);
                setAndStyleProperty(fingering, Pid::FONT_SIZE, ft.fontSize);
                cr->segment()->add(fingering);
                continue;
            }

            // Grace notes
            if (musxArtic->isGraceNote) {
                Chord* parentChord = cr->isChord() ? toChord(cr) : nullptr;
                PointF pos = posForArticulation(articAssign, articDef, cr);
                bool after = pos.x() > 0.0;
                if (!parentChord) {
                    ChordRestNavigateOptions options;
                    options.skipGrace = true;
                    options.skipMeasureRepeatRests = false;
                    ChordRest* nextCR = after ? nextChordRest(cr, options) : prevChordRest(cr, options);
                    parentChord = nextCR->isChord() ? toChord(nextCR) : nullptr;
                    after = !after;
                }
                if (parentChord) {
                    NoteVal nval;
                    staff_idx_t idx = parentChord->vStaffIdx();
                    bool error = false;
                    pos.ry() += cr->pos().y(); // Get position in local staff coords
                    int noteLine = int(std::lround(pos.y() / (parentChord->spatium() * parentChord->staffType()->lineDistance().val())));
                    AccidentalVal accOffs = parentChord->measure()->findAccidental(parentChord->segment(), idx, noteLine, error);
                    if (error) {
                        accOffs = Accidental::subtype2value(AccidentalType::NONE);
                    }
                    int nStep = absStep(noteLine, m_score->staff(idx)->clef(parentChord->segment()->tick()));
                    nStep = std::clamp(nStep, MIN_STEP, MAX_STEP);
                    nval.pitch = clampPitch(absStep2pitchByKey(nStep, Key::C) + int(accOffs));
                    nval.tpc1 = step2tpc(nStep % STEP_DELTA_OCTAVE, accOffs);
                    nval.tpc2 = nval.tpc1;

                    Interval v = parentChord->staff()->transpose(parentChord->segment()->tick());
                    if (!v.isZero()) {
                        if (parentChord->concertPitch()) {
                            v.flip();
                            nval.tpc2 = Transpose::transposeTpc(nval.tpc1, v, true);
                        } else {
                            nval.pitch += v.chromatic;
                            nval.tpc1 = Transpose::transposeTpc(nval.tpc2, v, true);
                        }
                    }

                    Chord* graceChord = Factory::createChord(parentChord->segment());
                    graceChord->setDurationType(DurationType::V_EIGHTH);
                    graceChord->setStaffMove(parentChord->staffMove());
                    graceChord->setTrack(parentChord->track());
                    graceChord->setTicks(graceChord->durationType().fraction());
                    const bool slashed = musxArtic->symName.contains(u"Acciaccatura");
                    if (after) {
                        graceChord->setNoteType(engraving::NoteType::GRACE8_AFTER);
                        graceChord->setShowStemSlash(slashed);
                        graceChord->setGraceIndex(0);
                    } else {
                        graceChord->setNoteType(slashed ? engraving::NoteType::ACCIACCATURA : engraving::NoteType::APPOGGIATURA);
                        graceChord->setGraceIndex(static_cast<int>(parentChord->graceNotesBefore().size()));
                    }

                    engraving::Note* note = Factory::createNote(graceChord);
                    note->setParent(graceChord);
                    note->setTrack(graceChord->track());
                    note->setVisible(!articAssign->hide && !articDef->noPrint);
                    note->setPlay(false);
                    note->setNval(nval);

                    graceChord->add(note);
                    parentChord->add(graceChord);

                    continue;
                }
            }

            // Playing technique types (handbells)
            if (musxArtic->playTechType != PlayingTechniqueType::Undefined) {
                PlayTechAnnotation* pta = Factory::createPlayTechAnnotation(cr->segment(), musxArtic->playTechType,
                                                                            TextStyleType::ARTICULATION);
                pta->setTrack(cr->track());
                pta->setXmlText(String(u"<sym>%1</sym>").arg(musxArtic->symName));
                pta->setTechniqueType(musxArtic->playTechType);
                if (importCustomPositions()) {
                    pta->setAutoplace(false);
                    pta->setOffset(posForArticulation(articAssign, articDef, cr) + cr->pos());
                }
                pta->setVisible(!articAssign->hide && !articDef->noPrint);
                cr->segment()->add(pta);
                continue;
            }

            // Rests can't have any other articulations
            if (!cr->isChord()) {
                /* if (musxArtic->articSym != SymId::noSym) {
                    Rest* r = toRest(cr);
                    Symbol* sym = new Symbol(r);
                    sym->setTrack(r->track());
                    sym->setSym(musxArtic->articSym);
                    sym->setVisible(!articAssign->hide && !articDef->noPrint);
                    r->add(sym);
                } */
                continue;
            }
            Chord* c = toChord(cr);

            // Fingering
            if (musxArtic->isFingering) {
                Fingering* fingering = Factory::createFingering(c->upNote());
                fingering->setTrack(c->track());
                fingering->setPlainText(String::fromUcs4(musxArtic->articChar.value()));
                FontTracker ft(articDef->fontMain);
                setAndStyleProperty(fingering, Pid::FONT_STYLE, int(ft.fontStyle));
                setAndStyleProperty(fingering, Pid::FONT_FACE, ft.fontName);
                setAndStyleProperty(fingering, Pid::FONT_SIZE, ft.fontSize);
                c->upNote()->add(fingering);
                continue;
            }

            // Notehead parentheses
            if (musxArtic->isLeftNoteheadParen) {
                engraving::Note* n = findClosestNote(articAssign, articDef, c);
                if (!n->leftParen()) {
                    Parenthesis* p = Factory::createParenthesis(n);
                    p->setParent(n);
                    p->setTrack(n->track());
                    p->setVisible(!articAssign->hide);
                    p->setDirection(DirectionH::LEFT);
                    n->add(p);
                    collectElementStyle(p);
                    continue;
                }
            }
            if (musxArtic->isRightNoteheadParen) {
                engraving::Note* n = findClosestNote(articAssign, articDef, c);
                if (!n->rightParen()) {
                    Parenthesis* p = Factory::createParenthesis(n);
                    p->setParent(n);
                    p->setTrack(n->track());
                    p->setVisible(!articAssign->hide);
                    p->setDirection(DirectionH::RIGHT);
                    n->add(p);
                    collectElementStyle(p);
                    continue;
                }
            }

            // Single-chord tremolos
            if (musxArtic->tremoloType != TremoloType::INVALID_TREMOLO) {
                TremoloSingleChord* tremolo = Factory::createTremoloSingleChord(c);
                tremolo->setTrack(c->track());
                tremolo->setTremoloType(musxArtic->tremoloType);
                tremolo->setParent(c);
                tremolo->setDurationType(c->durationType());
                tremolo->setPlayTremolo(articDef->playArtic);
                tremolo->setVisible(!articAssign->hide);
                c->setTremoloSingleChord(tremolo);
                collectElementStyle(tremolo);
                continue;
            }

            // Chordlines
            if (musxArtic->chordLineType != ChordLineType::NOTYPE) {
                ChordLine* cl = Factory::createChordLine(c);
                cl->setChordLineType(musxArtic->chordLineType);
                cl->setStraight(!musxArtic->symName.contains(u"Scoop") && !musxArtic->symName.contains(u"Doit")
                                && !musxArtic->symName.contains(u"FallLip") && !musxArtic->symName.contains(u"Plop"));
                cl->setWavy(cl->isStraight() && !musxArtic->symName.contains(u"Smooth"));
                // cl->setLengthX(); cl->setLengthY(); cl->setPath();
                cl->setPlayChordLine(articDef->playArtic);
                cl->setNote(findClosestNote(articAssign, articDef, c));
                c->add(cl);
                continue;
            }

            // Arpeggios
            if (musxArtic->symName == String(u"arpeggioVerticalSegment") || musxArtic->symName == String(u"arpeggiato")
                || musxArtic->symName == String(u"arpeggiatoUp") || musxArtic->symName == String(u"arpeggiatoDown")) {
                if (c->isGrace() || articAssign->hide) {
                    continue;
                }
                if (c->arpeggio()) {
                    continue;
                }
                EntryInfoPtr sourceEntry = EntryInfoPtr::fromEntryNumber(m_doc, m_currentMusxPartId, entryNumber);
                const auto options = arpeggioSpanOptions();
                std::optional<musx::util::ArpeggioSpanCandidate> span;
                span = musx::util::calcArpeggioSpanForAssignment(sourceEntry, articAssign, options);
                if (!span) {
                    span = singleGlyphArpeggioSpan(sourceEntry, musxArtic->symName);
                }
                if (span) {
                    createArpeggioFromSpan(*span, articDef);
                }
                continue;
            }

            // Chord brackets
            if (musxArtic->isChordBracket) {
                ChordBracket* cb = Factory::createChordBracket(c);
                cb->setTrack(c->track());
                cb->setArpeggioType(ArpeggioType::BRACKET);
                if (musxArtic->symName.contains(u"RH")) {
                    setAndStyleProperty(cb, Pid::BRACKET_HOOK_POS, DirectionV::DOWN);
                } else {
                    setAndStyleProperty(cb, Pid::BRACKET_HOOK_POS, DirectionV::UP);
                }
                if (musxArtic->symName.contains(u"End")) {
                    setAndStyleProperty(cb, Pid::BRACKET_RIGHT_SIDE, true);
                }
                Spatium s = Spatium::fromAbsolute(cb->symWidth(musxArtic->articSym), cb->spatium());
                setAndStyleProperty(cb, Pid::BRACKET_HOOK_LEN, s);
                cb->setParent(c);
                c->add(cb);
                continue;
            }

            Articulation* a = nullptr;

            // Ornaments
            if (musxArtic->isStandardOrnament) {
                a = toArticulation(Factory::createOrnament(c));
                a->setSymId(musxArtic->articSym);
                /// @todo detect accidentals added as other articulations
            } else if (musxArtic->ornamentDefinition.has_value()) {
                Ornament* o = Factory::createOrnament(c);
                setOrnamentIntervalFromAccidental(o, c->upNote(), musxArtic->ornamentDefinition.value().accAbove, true);
                setOrnamentIntervalFromAccidental(o, c->upNote(), musxArtic->ornamentDefinition.value().accBelow, false);
                a = toArticulation(o);
                a->setSymId(musxArtic->ornamentDefinition.value().symId);
            } else if (musxArtic->isArticulation || musxArtic->isFermataSym) {
                a = Factory::createArticulation(c);
                a->setSymId(musxArtic->articSym);
            } else {
                // Non-recognised articulation, import as symbol instead to avoid false stacking
                if (musxArtic->articSym == SymId::noSym) {
                    continue;
                }
                engraving::Note* n = findClosestNote(articAssign, articDef, c);
                Symbol* sym = new Symbol(n);
                sym->setTrack(n->track());
                sym->setSym(musxArtic->articSym);
                sym->setVisible(!articAssign->hide && !articDef->noPrint);
                if (importCustomPositions()) {
                    sym->setAutoplace(false);
                    sym->setOffset(posForArticulation(articAssign, articDef, c) - n->pos());
                }
                n->add(sym);
                continue;
            }

            a->setTrack(c->track());
            a->setVisible(!articAssign->hide && !articDef->noPrint);
            setAndStyleProperty(a, Pid::ARTICULATION_ANCHOR, int(calculateAnchor(articAssign, articDef->autoVertMode, cr)), true);
            a->setPlayArticulation(articDef->playArtic);
            c->add(a);
            collectElementStyle(a);
        }
    }

    // Create pedal markings
    if (pedalList.empty()) {
        return;
    }

    for (staff_idx_t i = 0; i < m_score->nstaves(); ++i) {
        Pedal* currentPedal = nullptr;
        for (auto [ticks, musxArtic] : pedalList.at(i)) {
            String pedalText = u"<sym>" + musxArtic->symName + u"</sym>";
            if (currentPedal && musxArtic->isPedalEnd) {
                setAndStyleProperty(currentPedal, Pid::END_TEXT, pedalText, true);
                currentPedal->setTick2(Fraction::fromTicks(ticks));
                m_score->addElement(currentPedal);
                currentPedal = nullptr;
            } else {
                if (currentPedal) {
                    setAndStyleProperty(currentPedal, Pid::END_TEXT, String(), true);
                    currentPedal->setTick2(Fraction::fromTicks(ticks));
                    m_score->addElement(currentPedal);
                }
                currentPedal = Factory::createPedal(m_score->dummy());
                currentPedal->setTick(Fraction::fromTicks(ticks));
                currentPedal->setTrack(staff2track(i));
                currentPedal->setTrack2(staff2track(i));
                setAndStyleProperty(currentPedal, Pid::BEGIN_TEXT, pedalText, true);
                setAndStyleProperty(currentPedal, Pid::CONTINUE_TEXT, String(), true);
                setAndStyleProperty(currentPedal, Pid::LINE_VISIBLE, false);
            }
        }
        if (currentPedal) {
            setAndStyleProperty(currentPedal, Pid::END_TEXT, String(), true);
            currentPedal->setTick2(m_score->tick2measure(currentPedal->tick())->endTick());
            m_score->addElement(currentPedal);
        }
    }
}
}
