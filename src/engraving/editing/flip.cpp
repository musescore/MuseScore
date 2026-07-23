/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "flip.h"

#include <functional>
#include <set>

#include "../dom/articulation.h"
#include "../dom/beam.h"
#include "../dom/chord.h"
#include "../dom/chordbracket.h"
#include "../dom/guitarbend.h"
#include "../dom/hairpin.h"
#include "../dom/hook.h"
#include "../dom/note.h"
#include "../dom/ornament.h"
#include "../dom/score.h"
#include "../dom/select.h"
#include "../dom/slurtie.h"
#include "../dom/spanner.h"
#include "../dom/staff.h"
#include "../dom/stem.h"
#include "../dom/tremolotwochord.h"
#include "../dom/trill.h"
#include "../dom/tuplet.h"

#include "log.h"

using namespace mu::engraving;

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void Flip::flip(Transaction&, Score* score)
{
    const std::vector<EngravingItem*>& el = score->selection().elements();
    if (el.empty()) {
        MScore::setError(MsError::NO_FLIPPABLE_SELECTED);
        return;
    }

    std::set<const EngravingItem*> alreadyFlippedElements;
    auto flipOnce = [&alreadyFlippedElements](const EngravingItem* element, std::function<void()> flipFunction) -> void {
        if (alreadyFlippedElements.insert(element).second) {
            flipFunction();
        }
    };

    for (EngravingItem* e : el) {
        if (e->hasVoiceAssignmentProperties()) {
            flipOnce(e, [e]() {
                PlacementV curPlacement = e->getProperty(Pid::PLACEMENT).value<PlacementV>();
                e->undoChangeProperty(Pid::DIRECTION, curPlacement == PlacementV::ABOVE ? DirectionV::DOWN : DirectionV::UP);
            });
        } else if (e->isNote() || e->isStem() || e->isHook()) {
            Chord* chord = nullptr;
            if (e->isNote()) {
                chord = toNote(e)->chord();
            } else if (e->isStem()) {
                chord = toStem(e)->chord();
            } else {
                chord = toHook(e)->chord();
            }

            IF_ASSERT_FAILED(chord) {
                continue;
            }

            if (chord->beam()) {
                if (!score->selection().isRange()) {
                    e = chord->beam();
                } else {
                    continue;
                }
            } else if (chord->tremoloTwoChord()) {
                if (!score->selection().isRange()) {
                    e = chord->tremoloTwoChord();
                } else {
                    continue;
                }
            } else {
                flipOnce(chord, [chord]() {
                    DirectionV dir = chord->up() ? DirectionV::DOWN : DirectionV::UP;
                    chord->undoChangeProperty(Pid::STEM_DIRECTION, dir);
                });
            }
        }

        if (e->isBeam()) {
            Beam* beam = toBeam(e);
            flipOnce(beam, [beam]() {
                if (beam->cross()) {
                    int newCrossStaffMove = beam->crossStaffMove() + 1;
                    if (beam->acceptCrossStaffMove(newCrossStaffMove)) {
                        beam->undoChangeProperty(Pid::BEAM_CROSS_STAFF_MOVE, newCrossStaffMove);
                    } else {
                        beam->undoChangeProperty(Pid::BEAM_CROSS_STAFF_MOVE,
                                                 beam->minCRMove() - beam->defaultCrossStaffIdx());
                    }
                } else {
                    DirectionV dir = beam->up() ? DirectionV::DOWN : DirectionV::UP;
                    beam->undoChangeProperty(Pid::STEM_DIRECTION, dir);
                }
            });
        } else if (e->isType(ElementType::TREMOLO_TWOCHORD)) {
            TremoloTwoChord* tremolo = item_cast<TremoloTwoChord*>(e);
            flipOnce(tremolo, [tremolo]() {
                DirectionV dir = tremolo->up() ? DirectionV::DOWN : DirectionV::UP;
                tremolo->undoChangeProperty(Pid::STEM_DIRECTION, dir);
            });
        } else if (e->isSlurTieSegment()) {
            auto slurTieSegment = toSlurTieSegment(e)->slurTie();
            flipOnce(slurTieSegment, [slurTieSegment]() {
                DirectionV dir = slurTieSegment->up() ? DirectionV::DOWN : DirectionV::UP;
                slurTieSegment->undoChangeProperty(Pid::SLUR_DIRECTION, dir);
            });
        } else if (e->isArticulationFamily()) {
            auto artic = toArticulation(e);
            flipOnce(artic, [artic]() {
                ArticulationAnchor articAnchor = artic->anchor();
                switch (articAnchor) {
                    case ArticulationAnchor::TOP:
                        articAnchor = ArticulationAnchor::BOTTOM;
                        break;
                    case ArticulationAnchor::BOTTOM:
                        articAnchor = ArticulationAnchor::TOP;
                        break;
                    case ArticulationAnchor::AUTO:
                        articAnchor = artic->up() ? ArticulationAnchor::BOTTOM : ArticulationAnchor::TOP;
                        break;
                }
                PropertyFlags pf = artic->propertyFlags(Pid::ARTICULATION_ANCHOR);
                if (pf == PropertyFlags::STYLED) {
                    pf = PropertyFlags::UNSTYLED;
                }
                artic->undoChangeProperty(Pid::ARTICULATION_ANCHOR, int(articAnchor), pf);
            });
        } else if (e->isTuplet()) {
            auto tuplet = toTuplet(e);
            flipOnce(tuplet, [tuplet]() {
                DirectionV dir = tuplet->isUp() ? DirectionV::DOWN : DirectionV::UP;
                tuplet->undoChangeProperty(Pid::DIRECTION, PropertyValue::fromValue<DirectionV>(dir), PropertyFlags::UNSTYLED);
            });
        } else if (e->isNoteDot() && e->explicitParent()->isNote()) {
            Note* note = toNote(e->explicitParent());
            DirectionV d = note->dotIsUp() ? DirectionV::DOWN : DirectionV::UP;
            note->undoChangeProperty(Pid::DOT_POSITION, PropertyValue::fromValue<DirectionV>(d));
        } else if (e->isChordBracket()) {
            ChordBracket* cb = toChordBracket(e);
            flipOnce(cb, [cb]() {
                DirectionV d = cb->hookPos() == DirectionV::UP ? DirectionV::DOWN : DirectionV::UP;
                cb->undoChangeProperty(Pid::BRACKET_HOOK_POS, PropertyValue::fromValue<DirectionV>(d));
            });
        } else if (e->isGuitarBendSegment()) {
            GuitarBend* bend = toGuitarBendSegment(e)->guitarBend();
            flipOnce(bend, [bend] {
                DirectionV direction = bend->ldata()->up() ? DirectionV::DOWN : DirectionV::UP;
                bend->undoChangeProperty(Pid::DIRECTION, PropertyValue::fromValue<DirectionV>(direction));
            });
        } else if (e->isTrillSegment()) {
            TrillSegment* trillSegment = toTrillSegment(e);
            Trill* trill = trillSegment->trill();
            Ornament* ornament = trill->ornament();

            flipOnce(ornament, [ornament]() {
                ArticulationAnchor articAnchor = ArticulationAnchor(ornament->getProperty(Pid::ARTICULATION_ANCHOR).toInt());

                switch (articAnchor) {
                    case ArticulationAnchor::TOP:
                        articAnchor = ArticulationAnchor::BOTTOM;
                        break;
                    case ArticulationAnchor::BOTTOM:
                        articAnchor = ArticulationAnchor::TOP;
                        break;
                    case ArticulationAnchor::AUTO:
                        articAnchor = ornament->up() ? ArticulationAnchor::BOTTOM : ArticulationAnchor::TOP;
                        break;
                }
                PropertyFlags pf = ornament->propertyFlags(Pid::ARTICULATION_ANCHOR);
                if (pf == PropertyFlags::STYLED) {
                    pf = PropertyFlags::UNSTYLED;
                }
                ornament->undoChangeProperty(Pid::ARTICULATION_ANCHOR, int(articAnchor), pf);
            });
        } else if (e->isStaffText()
                   || e->isSystemText()
                   || e->isTempoText()
                   || e->isTripletFeel()
                   || e->isJump()
                   || e->isMarker()
                   || e->isRehearsalMark()
                   || e->isMeasureNumber()
                   || e->isInstrumentChange()
                   || e->isPlayTechAnnotation()
                   || e->isCapo()
                   || e->isStringTunings()
                   || e->isSticking()
                   || e->isFingering()
                   || e->isHarmony()
                   || e->isFretDiagram()
                   || e->isHarpPedalDiagram()
                   || e->isOttava()
                   || e->isOttavaSegment()
                   || e->isTextLine()
                   || e->isTextLineSegment()
                   || e->isLetRing()
                   || e->isLetRingSegment()
                   || e->isVibrato()
                   || e->isVibratoSegment()
                   || e->isPalmMute()
                   || e->isPalmMuteSegment()
                   || e->isWhammyBar()
                   || e->isWhammyBarSegment()
                   || e->isRasgueado()
                   || e->isRasgueadoSegment()
                   || e->isHarmonicMark()
                   || e->isHarmonicMarkSegment()
                   || e->isGradualTempoChange()
                   || e->isGradualTempoChangeSegment()
                   || e->isPedal()
                   || e->isPedalSegment()
                   || e->isLyrics()
                   || e->isBreath()
                   || e->isFermata()
                   || e->isHammerOnPullOffText()) {
            e->undoChangeProperty(Pid::AUTOPLACE, true);
            // TODO: undoChangeProperty() should probably do this directly
            // see https://musescore.org/en/node/281432
            EngravingItem* ee = toEngravingItem(e->propertyDelegate(Pid::PLACEMENT));
            if (!ee) {
                ee = e;
            }

            flipOnce(ee, [ee]() {
                // getProperty() delegates call from spannerSegment to Spanner
                PlacementV p = ee->getProperty(Pid::PLACEMENT).value<PlacementV>();
                p = (p == PlacementV::ABOVE) ? PlacementV::BELOW : PlacementV::ABOVE;
                PropertyFlags pf = ee->propertyFlags(Pid::PLACEMENT);
                if (pf == PropertyFlags::STYLED) {
                    pf = PropertyFlags::UNSTYLED;
                }
                double oldDefaultY = ee->propertyDefault(Pid::OFFSET).value<PointF>().y();
                ee->undoChangeProperty(Pid::PLACEMENT, p, pf);
                // flip and rebase user offset to new default now that placement has changed
                double newDefaultY = ee->propertyDefault(Pid::OFFSET).value<PointF>().y();
                if (ee->isSpanner()) {
                    Spanner* spanner = toSpanner(ee);
                    for (SpannerSegment* ss : spanner->spannerSegments()) {
                        if (!ss->offset().isNull()) {
                            PointF off = ss->getProperty(Pid::OFFSET).value<PointF>();
                            double oldY = off.y() - oldDefaultY;
                            off.ry() = newDefaultY - oldY;
                            ss->undoChangeProperty(Pid::OFFSET, off);
                            ss->setOffsetChanged(false);
                        }
                    }
                } else if (!ee->offset().isNull()) {
                    PointF off = ee->getProperty(Pid::OFFSET).value<PointF>();
                    double oldY = off.y() - oldDefaultY;
                    off.ry() = newDefaultY - oldY;
                    ee->undoChangeProperty(Pid::OFFSET, off);
                    ee->setOffsetChanged(false);
                }
            });
        }
    }
}

//---------------------------------------------------------
//   flipHorizontally
//---------------------------------------------------------

void Flip::flipHorizontally(Transaction&, Score* score)
{
    const std::vector<EngravingItem*>& el = score->selection().elements();
    if (el.empty()) {
        MScore::setError(MsError::NO_FLIPPABLE_SELECTED);
        return;
    }

    std::set<const EngravingItem*> alreadyFlippedElements;
    auto flipOnce = [&alreadyFlippedElements](const EngravingItem* element, std::function<void()> flipFunction) -> void {
        if (alreadyFlippedElements.insert(element).second) {
            flipFunction();
        }
    };

    for (EngravingItem* e : el) {
        if (e->isHairpinSegment() || e->isHairpin()) {
            Hairpin* h = e->isHairpin() ? toHairpin(e) : toHairpinSegment(e)->hairpin();
            flipOnce(h, [h] {
                if (h->hairpinType() == HairpinType::CRESC_HAIRPIN) {
                    h->undoChangeProperty(Pid::HAIRPIN_TYPE, int(HairpinType::DIM_HAIRPIN));
                } else if (h->hairpinType() == HairpinType::DIM_HAIRPIN) {
                    h->undoChangeProperty(Pid::HAIRPIN_TYPE, int(HairpinType::CRESC_HAIRPIN));
                } else if (h->hairpinType() == HairpinType::CRESC_LINE) {
                    h->undoChangeProperty(Pid::HAIRPIN_TYPE, int(HairpinType::DIM_LINE));
                } else if (h->hairpinType() == HairpinType::DIM_LINE) {
                    h->undoChangeProperty(Pid::HAIRPIN_TYPE, int(HairpinType::CRESC_LINE));
                }
            });
        } else if (e->isChordBracket()) {
            ChordBracket* cb = toChordBracket(e);
            flipOnce(cb, [cb] {
                cb->undoChangeProperty(Pid::BRACKET_RIGHT_SIDE, !cb->rightSide());
            });
        }
    }
}

//---------------------------------------------------------
//   mirrorNoteHead
//---------------------------------------------------------

void Flip::mirrorNoteHead(Transaction&, Score* score)
{
    const std::vector<EngravingItem*>& el = score->selection().elements();
    for (EngravingItem* e : el) {
        if (e->isNote()) {
            Note* note = toNote(e);
            if (note->staff() && note->staff()->isTabStaff(note->chord()->tick())) {
                e->undoChangeProperty(Pid::DEAD, !note->deadNote());
            } else {
                DirectionH d = note->userMirror();
                if (d == DirectionH::AUTO) {
                    d = note->chord()->up() ? DirectionH::RIGHT : DirectionH::LEFT;
                } else {
                    d = d == DirectionH::LEFT ? DirectionH::RIGHT : DirectionH::LEFT;
                }
                note->undoChangeProperty(Pid::MIRROR_HEAD, d);
            }
        }
    }
}
