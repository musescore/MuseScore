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

#include <vector>
#include <exception>

#include "musx/musx.h"
#include "smufl_mapping.h"

#include "types/string.h"

#include "engraving/dom/arpeggio.h"
#include "engraving/dom/articulation.h"
#include "engraving/dom/breath.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/ornament.h"
#include "engraving/dom/parenthesis.h"
#include "engraving/dom/pedal.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/score.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/staff.h"
// #include "engraving/dom/stem.h"
// #include "engraving/dom/tie.h"
#include "engraving/dom/tremolosinglechord.h"

#include "log.h"

using namespace mu::engraving;
using namespace muse;
using namespace musx::dom;

namespace mu::iex::finale {
static engraving::Note* findClosestNote(const MusxInstance<details::ArticulationAssign>& articAssign,
                                        const MusxInstance<others::ArticulationDef>& articDef, Chord* c)
{
    engraving::Note* n = c->upNote();
    // Attach to correct note based on vertical position
    if (c->notes().size() > 1) {
        /// @todo Account for placement (and more articulation position options in general)
        double referencePos = n->y() + n->headHeight() / 2;
        referencePos -= absoluteDoubleFromEvpu(articAssign->vertOffset, n);
        referencePos -= absoluteDoubleFromEvpu(articDef->yOffsetMain + articDef->defVertPos, n);
        double bestMatch = DBL_MAX;
        for (engraving::Note* note : c->notes()) {
            double noteDist = std::abs(note->y() - referencePos);
            if (noteDist < bestMatch) {
                bestMatch = noteDist;
                n = note;
            }
        }
    }
    return n;
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
        }
        [[fallthrough]];

    case others::ArticulationDef::AutoVerticalMode::AlwaysNoteheadSide:
        return !cr->ldata()->up;
    case others::ArticulationDef::AutoVerticalMode::StemSide:
        return cr->ldata()->up;

    case others::ArticulationDef::AutoVerticalMode::AboveEntry:
        return true;
    case others::ArticulationDef::AutoVerticalMode::BelowEntry:
        return false;

    default:
        // Unhandled case: AlwaysOnStem - Placement here shouldn't matter in practice
        break;
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
        return cr->ldata()->up ? ArticulationAnchor::TOP : ArticulationAnchor::BOTTOM;

    case others::ArticulationDef::AutoVerticalMode::AboveEntry:
        return ArticulationAnchor::TOP;
    case others::ArticulationDef::AutoVerticalMode::BelowEntry:
        return ArticulationAnchor::BOTTOM;

    default:
        // Unhandled case: AlwaysOnStem - Placement here shouldn't matter in practice
        break;
    }
    return ArticulationAnchor::AUTO;
}

struct OrnamentDefinition {
    std::string_view name;
    SymId symId;
    AccidentalType accAbove = AccidentalType::NONE;
    AccidentalType accBelow = AccidentalType::NONE;
};

static constexpr OrnamentDefinition ornamentList[] = {
    { "ornamentTrillFlatAbove",          SymId::ornamentTrill, AccidentalType::FLAT,    AccidentalType::NONE },
    { "ornamentTrillFlatAboveLegacy",    SymId::ornamentTrill, AccidentalType::FLAT,    AccidentalType::NONE },
    { "ornamentTrillNaturalAbove",       SymId::ornamentTrill, AccidentalType::NATURAL, AccidentalType::NONE },
    { "ornamentTrillNaturalAboveLegacy", SymId::ornamentTrill, AccidentalType::NATURAL, AccidentalType::NONE },
    { "ornamentTrillSharpAbove",         SymId::ornamentTrill, AccidentalType::SHARP,   AccidentalType::NONE },
    { "ornamentTrillSharpAboveLegacy",   SymId::ornamentTrill, AccidentalType::SHARP,   AccidentalType::NONE },
    { "ornamentTurnFlatAbove",           SymId::ornamentTurn,  AccidentalType::FLAT,    AccidentalType::NONE },
    { "ornamentTurnFlatAboveSharpBelow", SymId::ornamentTurn,  AccidentalType::FLAT,    AccidentalType::SHARP },
    { "ornamentTurnFlatBelow",           SymId::ornamentTurn,  AccidentalType::NONE,    AccidentalType::FLAT },
    { "ornamentTurnNaturalAbove",        SymId::ornamentTurn,  AccidentalType::NATURAL, AccidentalType::NONE },
    { "ornamentTurnNaturalBelow",        SymId::ornamentTurn,  AccidentalType::NONE,    AccidentalType::NATURAL },
    { "ornamentTurnSharpAbove",          SymId::ornamentTurn,  AccidentalType::SHARP,   AccidentalType::NONE },
    { "ornamentTurnSharpAboveFlatBelow", SymId::ornamentTurn,  AccidentalType::SHARP,   AccidentalType::FLAT },
    { "ornamentTurnSharpBelow",          SymId::ornamentTurn,  AccidentalType::NONE,    AccidentalType::SHARP },
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

void FinaleParser::importArticulations()
{
    std::vector<std::map<int, SymId> > pedalList;
    pedalList.assign(m_score->nstaves(), std::map<int, SymId> {});
    /// @todo offset calculations
    for (auto [entryNumber, cr] : m_entryNumber2CR) {
        MusxInstanceList<details::ArticulationAssign> articAssignList = m_doc->getDetails()->getArray<details::ArticulationAssign>(
            m_currentMusxPartId, entryNumber);
        for (const MusxInstance<details::ArticulationAssign>& articAssign : articAssignList) {
            const MusxInstance<others::ArticulationDef>& articDef = m_doc->getOthers()->get<others::ArticulationDef>(m_currentMusxPartId,
                                                                                                                     articAssign->articDef);

            /// @todo perhaps process the articulation defs once and determine their properties. Then they can be assigned per assignment without recalculating
            /// every time.

            std::optional<char32_t> articChar;
            char32_t articMusxChar = 0;
            std::string articFontName;

            auto calcArticSymbol = [&](char32_t theChar, const MusxInstance<FontInfo>& font, bool isShape,
                                       Cmper shapeId) -> std::optional<SymId> {
                if (isShape) {
                    if (MusxInstance<others::ShapeDef> shape = m_doc->getOthers()->get<others::ShapeDef>(m_currentMusxPartId, shapeId)) {
                        if (std::optional<KnownShapeDefType> knownShape = shape->recognize()) {
                            switch (knownShape.value()) {
                            case KnownShapeDefType::TenutoMark:
                                return SymId::articTenutoAbove;     // MuseScore figures out the actual above/below symbol
                            /// @todo: add other cases if ever defined in musxdom
                            default:
                                break;
                            }
                        }
                    }
                } else if (theChar) {
                    SymId result = FinaleTextConv::symIdFromFinaleChar(theChar, font); // articDef fonts are guaranteed non-null by musxdom
                    articChar = FinaleTextConv::mappedChar(theChar, font);
                    articMusxChar = theChar;
                    articFontName = font->getName();
                    return result;
                }
                return std::nullopt;
            };

            auto articSym = [&]() -> std::optional<SymId> {
                if (std::optional<SymId> mainSym
                        = calcArticSymbol(articDef->charMain, articDef->fontMain, articDef->mainIsShape, articDef->mainShape)) {
                    return mainSym;
                }
                return calcArticSymbol(articDef->charAlt, articDef->fontAlt, articDef->altIsShape, articDef->altShape);
            }();

            if (!articSym.has_value()) {
                // unknown value or shape
                continue;
            }

            // Pedal lines
            if (String::fromAscii(SymNames::nameForSymId(articSym.value()).ascii()).contains(std::wregex(LR"(keyboardPedal)"))) {
                pedalList.at(cr->vStaffIdx()).emplace(cr->segment()->tick().ticks(), articSym.value());
                continue;
            }

            // Fermatas
            /// @todo table copied from engraving/dom/fermata.cpp, maybe expose?
            static const std::unordered_map<SymId, FermataType> FERMATA_TYPES = {
                { SymId::fermataAbove, FermataType::Normal },
                { SymId::fermataBelow, FermataType::Normal },
                { SymId::fermataLongAbove, FermataType::Long },
                { SymId::fermataLongBelow, FermataType::Long },
                { SymId::fermataLongHenzeAbove, FermataType::LongHenze },
                { SymId::fermataLongHenzeBelow, FermataType::LongHenze },
                { SymId::fermataVeryLongAbove, FermataType::VeryLong },
                { SymId::fermataVeryLongBelow, FermataType::VeryLong },
                { SymId::fermataShortHenzeAbove, FermataType::ShortHenze },
                { SymId::fermataShortHenzeBelow, FermataType::ShortHenze },
                { SymId::fermataVeryShortAbove, FermataType::VeryShort },
                { SymId::fermataVeryShortBelow, FermataType::VeryShort },
                { SymId::fermataShortAbove, FermataType::Short },
                { SymId::fermataShortBelow, FermataType::Short },
            };
            FermataType ft = muse::value(FERMATA_TYPES, articSym.value(), FermataType::Undefined);
            if (ft != FermataType::Undefined) {
                Fermata* fermata = Factory::createFermata(cr->segment());
                fermata->setTrack(cr->track());
                fermata->setPlacement(calculateUp(articAssign, articDef->autoVertMode, cr) ? PlacementV::ABOVE : PlacementV::BELOW);
                /// @todo Verify that fermatas have no playback effect in Finale.
                fermata->setSymIdAndTimeStretch(articSym.value());
                fermata->setPlay(false);
                // fermata->setSymId(mainSym);
                // fermata->setPlay(articDef->playArtic);
                fermata->setVisible(!articAssign->hide);
                cr->segment()->add(fermata);
                collectElementStyle(fermata);
                continue;
            }

            // Breaths and pauses
            bool breathCreated = false;
            for (BreathType breathType : Breath::BREATH_LIST) {
                if (articSym.value() == breathType.id) {
                    Segment* breathSegment = cr->measure()->getSegment(SegmentType::Breath, cr->endTick());
                    Breath* breath = Factory::createBreath(breathSegment);
                    breath->setTrack(cr->track());
                    breath->setPlacement(breath->track() & 1 ? PlacementV::BELOW : PlacementV::ABOVE);
                    breath->setSymId(breathType.id);
                    // breath->setPause(breathType.pause); until there is a toggleable play property, leave unset
                    breath->setVisible(!articAssign->hide);
                    breathSegment->add(breath);
                    collectElementStyle(breath);
                    breathCreated = true;
                    break;
                }
            }
            if (breathCreated) {
                continue;
            }

            if (!cr->isChord()) {
                // Rests can only have fermatas or breaths, no other articulations
                continue;
            }
            Chord* c = toChord(cr);

            // Notehead parentheses
            if (articSym.value() == SymId::noteheadParenthesisLeft || (articMusxChar == U'(' && !fontIsEngravingFont(articFontName))) {
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
            if (articSym.value() == SymId::noteheadParenthesisRight || (articMusxChar == U')' && !fontIsEngravingFont(articFontName))) {
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
            TremoloType tt = tremoloTypeFromSymId(articSym.value());
            if (tt != TremoloType::INVALID_TREMOLO) {
                TremoloSingleChord* tremolo = Factory::createTremoloSingleChord(c);
                tremolo->setTrack(c->track());
                tremolo->setTremoloType(tt);
                tremolo->setParent(c);
                tremolo->setDurationType(c->durationType());
                tremolo->setPlayTremolo(articDef->playArtic);
                tremolo->setVisible(!articAssign->hide);
                c->setTremoloSingleChord(tremolo);
                collectElementStyle(tremolo);
                continue;
            }

            // Arpeggios
            // The Finale symbol is an optional character and not in SMuFL
            if (articSym.value() == SymId::noSym && !c->arpeggio() && articChar.has_value()
                && FinaleTextConv::charNameFinale(articDef->charMain, articDef->fontMain) == "arpeggioVerticalSegment") {
                if (c->isGrace()) {
                    continue;
                }
                Segment* s = c->segment();
                track_idx_t topChordTrack = c->track();
                track_idx_t bottomChordTrack = c->track();
                if (const System* sys = c->measure()->system()) {
                    int line = c->staffType()->isTabStaff() ? c->upNote()->string() * 2 : c->upNote()->line();
                    // x in chord coords, y in system coords
                    PointF baseStartPos(c->upNote()->ldata()->pos().x(),
                                        sys->staff(c->vStaffIdx())->y() + c->staffOffsetY()
                                        + (line * c->spatium() * c->staffType()->lineDistance().val() * 0.5));
                    if (articDef->autoVert) {
                        // determine up/down and add corresponding offset (flipped/main)
                        // and also other factors (centering, stacking, etc.)
                    } else {
                        // observed: always use above pos
                        baseStartPos += evpuToPointF(articDef->xOffsetMain, -articDef->yOffsetMain) * c->defaultSpatium();
                    }
                    baseStartPos += evpuToPointF(articAssign->horzOffset, -articAssign->vertOffset) * c->defaultSpatium();

                    muse::draw::FontMetrics fm = FontTracker(articDef->fontMain).toFontMetrics(c->spatium() / c->defaultSpatium());
                    baseStartPos.ry() -= fm.boundingRect(articChar.value()).height();

                    line = c->staffType()->isTabStaff() ? c->downNote()->string() * 2 : c->downNote()->line();
                    PointF baseEndPos(c->downNote()->ldata()->pos().x(),
                                      sys->staff(c->vStaffIdx())->y() + c->staffOffsetY()
                                      + (++line * c->spatium() * c->staffType()->lineDistance().val() * 0.5));
                    baseEndPos += evpuToPointF(articAssign->horzAdd, -articAssign->vertAdd) * c->defaultSpatium();
                    /// @todo (How) is this point affected by baseStartPos

                    double upDiff = DBL_MAX;
                    double downDiff = DBL_MAX;
                    for (track_idx_t track = 0; track < m_score->ntracks(); ++track) {
                        if (!sys->staff(track2staff(track))->show()) {
                            continue;
                        }
                        if (s->element(track) && s->element(track)->isChord()) {
                            Chord* potentialMatch = toChord(s->element(track));
                            // Check for up match
                            // Iterate through and find best top/bottom matches
                            // Then add to top chord (not c) and set spanArpeggio (only for lower chord?) as appropriate
                            line = potentialMatch->staffType()->isTabStaff() ? potentialMatch->upNote()->string()
                                   * 2 : potentialMatch->upNote()->line();
                            double upPos = sys->staff(potentialMatch->vStaffIdx())->y() + potentialMatch->staffOffsetY()
                                           + (line * potentialMatch->spatium() * potentialMatch->staffType()->lineDistance().val() * 0.5);
                            double diff = std::abs(upPos - baseStartPos.y());
                            if (diff < upDiff) {
                                upDiff = diff;
                                topChordTrack = potentialMatch->track();
                            }
                            line = potentialMatch->staffType()->isTabStaff() ? potentialMatch->downNote()->string()
                                   * 2 : potentialMatch->downNote()->line();
                            double downPos = sys->staff(potentialMatch->vStaffIdx())->y() + potentialMatch->staffOffsetY()
                                             + (line * potentialMatch->spatium() * potentialMatch->staffType()->lineDistance().val() * 0.5);
                            diff = std::abs(downPos - baseEndPos.y());
                            if (diff < downDiff) {
                                downDiff = diff;
                                bottomChordTrack = potentialMatch->track();
                            }
                        }
                    }
                }
                Chord* arpChord = toChord(s->element(topChordTrack));
                Arpeggio* arpeggio = Factory::createArpeggio(arpChord);
                arpeggio->setTrack(topChordTrack);
                arpeggio->setArpeggioType(ArpeggioType::NORMAL);
                arpeggio->setSpan(int(bottomChordTrack + 1 - topChordTrack));
                for (track_idx_t track = topChordTrack; track <= bottomChordTrack; ++track) {
                    if (s->element(track) && s->element(track)->isChord()) {
                        toChord(s->element(track))->setSpanArpeggio(arpeggio);
                    }
                }
                // Unused, probably don't map nicely
                // arpeggio->setUserLen1(absoluteDouble);
                // arpeggio->setUserLen2(absoluteDouble);
                arpeggio->setPlayArpeggio(articDef->playArtic);
                // Playback values in finale are EDUs by default, or in % by non-default (exact workings needs to be investigated)
                // MuseScore is relative to BPM 120 (8 notes take the spread time beats).
                Fraction totalArpDuration = eduToFraction(articDef->startTopNoteDelta - articDef->startBotNoteDelta);
                double beatsPerSecondRatio = m_score->tempo(s->tick()).val / 2.0; // We are this much faster/slower than 120 bpm
                double timeIn120BPM = totalArpDuration.toDouble() / beatsPerSecondRatio;
                arpeggio->setStretch(timeIn120BPM * 8.0 / c->notes().size());
                arpeggio->setParent(arpChord);
                arpChord->setArpeggio(arpeggio);
            }

            /// @todo Ornament properties, chordlines, fingerings, trills, figured bass?, pedal lines?
            Articulation* a = nullptr;

            if (muse::contains(ornamentSymbols, articSym.value())) {
                a = toArticulation(Factory::createOrnament(c));
            } else if (articChar.has_value()) {
                if (const std::string_view* glyphName
                        = smufl_mapping::getGlyphName(articChar.value(), smufl_mapping::SmuflGlyphSource::Finale)) {
                    for (OrnamentDefinition od : ornamentList) {
                        if (od.name == *glyphName) {
                            articSym = od.symId;
                            a = toArticulation(Factory::createOrnament(c)); /// @todo accidentals
                            break;
                        }
                    }
                }
            }
            if (!a) {
                a = Factory::createArticulation(c);
            }

            // Other articulations
            a->setTrack(c->track());
            a->setSymId(articSym.value());
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

    static const std::unordered_set<SymId> pedalEndTypes = {
        SymId::keyboardPedalUp,
        SymId::keyboardPedalUpNotch,
        SymId::keyboardPedalUpSpecial,
    };

    for (staff_idx_t i = 0; i < m_score->nstaves(); ++i) {
        Pedal* currentPedal = nullptr;
        for (auto [ticks, sym] : pedalList.at(i)) {
            if (currentPedal && muse::contains(pedalEndTypes, sym)) {
                String pedalEndText = u"<sym>" + String::fromAscii(SymNames::nameForSymId(sym).ascii()) + u"</sym>";
                setAndStyleProperty(currentPedal, Pid::END_TEXT, pedalEndText, true);
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
                String pedalBeginText = u"<sym>" + String::fromAscii(SymNames::nameForSymId(sym).ascii()) + u"</sym>";
                setAndStyleProperty(currentPedal, Pid::BEGIN_TEXT, pedalBeginText, true);
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
