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
#include "engraving/dom/fingering.h"
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
#include "engraving/dom/system.h"
// #include "engraving/dom/tie.h"
#include "engraving/dom/textbase.h"
#include "engraving/dom/tremolosinglechord.h"

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

ReadableArticulation::ReadableArticulation(const FinaleParser& context, const MusxInstance<others::ArticulationDef>& articDef)
{
    auto calcArticSymbol = [&](char32_t theChar, const MusxInstance<FontInfo>& font,
                               bool isShape, Cmper shapeId) -> bool {
        if (isShape) {
            if (const auto shape = context.musxDocument()->getOthers()->get<others::ShapeDef>(context.currentMusxPartId(), shapeId)) {
                if (std::optional<KnownShapeDefType> knownShape = shape->recognize()) {
                    switch (knownShape.value()) {
                    case KnownShapeDefType::TenutoMark:
                        articSym = SymId::articTenutoAbove;     // MuseScore figures out the actual above/below symbol
                        return true;
                    /// @todo: add other cases if ever defined in musxdom
                    default:
                        break;
                    }
                }
            }
            symName = String::fromAscii(SymNames::nameForSymId(articSym).ascii());
        } else if (theChar) {
            articSym = FinaleTextConv::symIdFromFinaleChar(theChar, font); // articDef fonts are guaranteed non-null by musxdom
            isMusicalSymbol = context.fontIsEngravingFont(font, true);
            articChar = isMusicalSymbol ? FinaleTextConv::mappedChar(theChar, font) : theChar;
            if (articSym == SymId::noSym) {
                symName = String::fromStdString(FinaleTextConv::charNameFinale(theChar, font));
            } else {
                symName = String::fromAscii(SymNames::nameForSymId(articSym).ascii());
            }
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

    context.logger()->logInfo(String(u"Added articulation %1 with symbol %2 to library.").arg(String::number(articDef->getCmper()),
                                                                                              symName));

    if (articSym == SymId::noteheadParenthesisLeft || (!isMusicalSymbol && articChar.value() == U'(')) {
        isLeftNoteheadParen = true;
    } else if (articSym == SymId::noteheadParenthesisRight || (!isMusicalSymbol && articChar.value() == U')')) {
        isRightNoteheadParen = true;
    } else if (!isMusicalSymbol) {
        String articText = String::fromUcs4(articChar.value());
        if (articText.contains(std::wregex(LR"([A-z]|[0-9])"))) {
            isFingering = true;
            isSticking = articText.contains(std::wregex(LR"(L|R)"));
        } else {
            unrecognised = true;
            return;
        }
    } else if (symName.contains(std::wregex(LR"(keyboardPedal)"))) {
        isPedalSym = true;
        isPedalEnd = muse::contains(pedalEndTypes, articSym);
    } else if (fermataTypeFromSymId(articSym) != FermataType::Undefined) {
        isFermataSym = true;
    } else if (muse::contains(ornamentSymbols, articSym)) {
        isStandardOrnament = true;
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
}

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
            if (cr->isRest()) {
                return muse::RealIsEqualOrLess(cr->y() * 2, cr->staff()->staffHeight(cr->segment()->tick()));
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

static void setOrnamentIntervalFromAccidental(Ornament* o, engraving::Note* n, AccidentalType at, bool above)
{
    // Uses default
    if (at == AccidentalType::NONE) {
        return;
    }

    // Make sure accidentals are visible (prioritise false positives over false negatives)
    int newLine = n->line() + (above ? 1 : -1);
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
    int pitchDifference = std::abs(absStep2pitchByKey(newLine, Key::C) + int(actualAcc) - n->pitch());
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
    /// @todo offset calculations
    for (auto [entryNumber, cr] : m_entryNumber2CR) {
        const MusxInstanceList<details::ArticulationAssign> articAssignList = m_doc->getDetails()->getArray<details::ArticulationAssign>(
            m_currentMusxPartId, entryNumber);
        const bool isDrumStaff = cr->staff()->isDrumStaff(cr->segment()->tick());
        for (const MusxInstance<details::ArticulationAssign>& articAssign : articAssignList) {
            const MusxInstance<others::ArticulationDef>& articDef = m_doc->getOthers()->get<others::ArticulationDef>(m_currentMusxPartId,
                                                                                                                     articAssign->articDef);
            ReadableArticulation* musxArtic = [&]() -> ReadableArticulation* {
                // Search our converted shape library, or if not found add to it
                ReadableArticulation* line = muse::value(m_articulations, articAssign->articDef, nullptr);
                if (!line) {
                    line = new ReadableArticulation(*this, articDef);
                    m_articulations.emplace(articAssign->articDef, line);
                }
                return line;
            }();

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
            if (musxArtic->isFermataSym) {
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

            // Rests can't have any other articulations, so add as symbols instead
            if (!cr->isChord()) {
                if (musxArtic->articSym != SymId::noSym) {
                    Rest* r = toRest(cr);
                    Symbol* sym = new Symbol(r);
                    sym->setTrack(r->track());
                    sym->setSym(musxArtic->articSym);
                    sym->setVisible(!articAssign->hide && !articDef->noPrint);
                    r->add(sym);
                }
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

            // Arpeggios
            // The Finale symbol is an optional character and not in SMuFL
            if (!c->arpeggio() && musxArtic->symName == String(u"arpeggioVerticalSegment")) {
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
                    baseStartPos.ry() -= fm.boundingRect(musxArtic->articChar.value()).height();

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
            }
            // Other articulations
            /// @todo chordlines, figured bass?
            if (!a) {
                if (musxArtic->articSym == SymId::noSym) {
                    continue;
                }
                a = Factory::createArticulation(c);
                a->setSymId(musxArtic->articSym);
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
