/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#ifndef MU_ENGRAVING_FACTORY_H
#define MU_ENGRAVING_FACTORY_H

#include <memory>
#include <QStringRef>
#include <QString>

#include "engravingitem.h"
#include "durationtype.h"
#include "types.h"

namespace mu::engraving {
class Instrument;
}

namespace mu::engraving {
class RootItem;
class Factory
{
public:

    static mu::engraving::ElementType name2type(const AsciiString& name, bool silent = false);
    static const char* name(mu::engraving::ElementType type);
    static const char* userName(mu::engraving::ElementType type);

    static mu::engraving::EngravingItem* createItem(mu::engraving::ElementType type, mu::engraving::EngravingItem* parent,
                                                    bool isAccessibleEnabled = true);
    static mu::engraving::EngravingItem* createItemByName(const AsciiString& name, mu::engraving::EngravingItem* parent,
                                                          bool isAccessibleEnabled = true);

    static mu::engraving::Accidental* createAccidental(mu::engraving::EngravingItem* parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<mu::engraving::Accidental> makeAccidental(mu::engraving::EngravingItem* parent);

    static mu::engraving::Ambitus* createAmbitus(mu::engraving::Segment* parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<mu::engraving::Ambitus> makeAmbitus(mu::engraving::Segment* parent);

    static mu::engraving::Arpeggio* createArpeggio(mu::engraving::Chord* parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<mu::engraving::Arpeggio> makeArpeggio(mu::engraving::Chord* parent);

    static mu::engraving::Articulation* createArticulation(mu::engraving::ChordRest* parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<mu::engraving::Articulation> makeArticulation(mu::engraving::ChordRest* parent);

    static mu::engraving::BarLine* createBarLine(mu::engraving::Segment* parent, bool isAccessibleEnabled = true);
    static mu::engraving::BarLine* copyBarLine(const mu::engraving::BarLine& src);
    static std::shared_ptr<mu::engraving::BarLine> makeBarLine(mu::engraving::Segment* parent);

    static mu::engraving::Beam* createBeam(mu::engraving::System* parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<mu::engraving::Beam> makeBeam(mu::engraving::System* parent);

    static mu::engraving::Bend* createBend(mu::engraving::Note* parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<mu::engraving::Bend> makeBend(mu::engraving::Note* parent);

    static mu::engraving::Bracket* createBracket(mu::engraving::EngravingItem* parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<mu::engraving::Bracket> makeBracket(mu::engraving::EngravingItem* parent);
    static mu::engraving::BracketItem* createBracketItem(mu::engraving::EngravingItem* parent);
    static mu::engraving::BracketItem* createBracketItem(mu::engraving::EngravingItem* parent, mu::engraving::BracketType a, int b);

    static mu::engraving::Breath* createBreath(mu::engraving::Segment* parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<mu::engraving::Breath> makeBreath(mu::engraving::Segment* parent);

    static mu::engraving::Chord* createChord(mu::engraving::Segment* parent, bool isAccessibleEnabled = true);
    static mu::engraving::Chord* copyChord(const mu::engraving::Chord& src, bool link = false);
    static std::shared_ptr<mu::engraving::Chord> makeChord(mu::engraving::Segment* parent);

    static mu::engraving::ChordLine* createChordLine(mu::engraving::Chord* parent, bool isAccessibleEnabled = true);
    static mu::engraving::ChordLine* copyChordLine(const mu::engraving::ChordLine& src);
    static std::shared_ptr<mu::engraving::ChordLine> makeChordLine(mu::engraving::Chord* parent);

    static mu::engraving::Slide* createSlide(mu::engraving::Chord* parent, bool isAccessibleEnabled = true);
    static mu::engraving::Slide* copySlide(const mu::engraving::Slide& src);
    static std::shared_ptr<mu::engraving::Slide> makeSlide(mu::engraving::Chord* parent);

    static mu::engraving::Clef* createClef(mu::engraving::Segment* parent, bool isAccessibleEnabled = true);
    static mu::engraving::Clef* copyClef(const mu::engraving::Clef& src);
    static std::shared_ptr<mu::engraving::Clef> makeClef(mu::engraving::Segment* parent);

    static mu::engraving::Fermata* createFermata(mu::engraving::EngravingItem* parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<mu::engraving::Fermata> makeFermata(mu::engraving::EngravingItem* parent);

    static mu::engraving::FiguredBass* createFiguredBass(mu::engraving::Segment* parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<mu::engraving::FiguredBass> makeFiguredBass(mu::engraving::Segment* parent);

    static mu::engraving::FretDiagram* createFretDiagram(mu::engraving::Segment* parent, bool isAccessibleEnabled = true);
    static mu::engraving::FretDiagram* copyFretDiagram(const mu::engraving::FretDiagram& src);
    static std::shared_ptr<mu::engraving::FretDiagram> makeFretDiagram(mu::engraving::Segment* parent);

    static mu::engraving::KeySig* createKeySig(mu::engraving::Segment* parent, bool isAccessibleEnabled = true);
    static mu::engraving::KeySig* copyKeySig(const mu::engraving::KeySig& src);
    static std::shared_ptr<mu::engraving::KeySig> makeKeySig(mu::engraving::Segment* parent);

    static mu::engraving::LayoutBreak* createLayoutBreak(mu::engraving::MeasureBase* parent, bool isAccessibleEnabled = true);
    static mu::engraving::LayoutBreak* copyLayoutBreak(const mu::engraving::LayoutBreak& src);
    static std::shared_ptr<mu::engraving::LayoutBreak> makeLayoutBreak(mu::engraving::MeasureBase* parent);

    static mu::engraving::Lyrics* createLyrics(mu::engraving::ChordRest* parent, bool isAccessibleEnabled = true);
    static mu::engraving::Lyrics* copyLyrics(const mu::engraving::Lyrics& src);

    static mu::engraving::Measure* createMeasure(mu::engraving::System* parent, bool isAccessibleEnabled = true);
    static mu::engraving::Measure* copyMeasure(const mu::engraving::Measure& src);

    static mu::engraving::MeasureRepeat* createMeasureRepeat(mu::engraving::Segment* parent, bool isAccessibleEnabled = true);
    static mu::engraving::MeasureRepeat* copyMeasureRepeat(const mu::engraving::MeasureRepeat& src);

    static mu::engraving::Note* createNote(mu::engraving::Chord* parent, bool isAccessibleEnabled = true);
    static mu::engraving::Note* copyNote(const mu::engraving::Note& src, bool link = false);
    static std::shared_ptr<mu::engraving::Note> makeNote(mu::engraving::Chord* parent);

    static mu::engraving::NoteDot* createNoteDot(mu::engraving::Note* parent, bool isAccessibleEnabled = true);
    static mu::engraving::NoteDot* createNoteDot(mu::engraving::Rest* parent, bool isAccessibleEnabled = true);
    static mu::engraving::NoteDot* copyNoteDot(const mu::engraving::NoteDot& src);

    static mu::engraving::Page* createPage(RootItem* parent, bool isAccessibleEnabled = true);

    static mu::engraving::Rest* createRest(mu::engraving::Segment* parent, bool isAccessibleEnabled = true);
    static mu::engraving::Rest* createRest(mu::engraving::Segment* parent, const mu::engraving::TDuration& t,
                                           bool isAccessibleEnabled = true);
    static mu::engraving::Rest* copyRest(const mu::engraving::Rest& src, bool link = false);
    static mu::engraving::MMRest* createMMRest(mu::engraving::EngravingItem* parent, bool isAccessibleEnabled = true);

    static mu::engraving::Segment* createSegment(mu::engraving::Measure* parent, bool isAccessibleEnabled = true);
    static mu::engraving::Segment* createSegment(mu::engraving::Measure* parent, mu::engraving::SegmentType type,
                                                 const mu::engraving::Fraction& t, bool isAccessibleEnabled = true);

    static mu::engraving::Slur* createSlur(mu::engraving::EngravingItem* parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<mu::engraving::Slur> makeSlur(mu::engraving::EngravingItem* parent);

    static mu::engraving::Spacer* createSpacer(mu::engraving::Measure* parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<mu::engraving::Spacer> makeSpacer(mu::engraving::Measure* parent);

    static mu::engraving::Staff* createStaff(mu::engraving::Part* parent);

    static mu::engraving::StaffLines* createStaffLines(mu::engraving::Measure* parent, bool isAccessibleEnabled = true);
    static mu::engraving::StaffLines* copyStaffLines(const mu::engraving::StaffLines& src);

    static mu::engraving::StaffState* createStaffState(mu::engraving::EngravingItem* parent, bool isAccessibleEnabled = true);

    static mu::engraving::StaffTypeChange* createStaffTypeChange(mu::engraving::MeasureBase* parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<mu::engraving::StaffTypeChange> makeStaffTypeChange(mu::engraving::MeasureBase* parent);

    static mu::engraving::StaffText* createStaffText(mu::engraving::Segment* parent,
                                                     mu::engraving::TextStyleType textStyleType = TextStyleType::STAFF,
                                                     bool isAccessibleEnabled = true);

    static mu::engraving::RehearsalMark* createRehearsalMark(mu::engraving::Segment* parent, bool isAccessibleEnabled = true);

    static mu::engraving::Stem* createStem(mu::engraving::Chord* parent, bool isAccessibleEnabled = true);
    static mu::engraving::Stem* copyStem(const mu::engraving::Stem& src);

    static mu::engraving::StemSlash* createStemSlash(mu::engraving::Chord* parent, bool isAccessibleEnabled = true);
    static mu::engraving::StemSlash* copyStemSlash(const mu::engraving::StemSlash& src);

    static mu::engraving::System* createSystem(mu::engraving::Page* parent, bool isAccessibleEnabled = true);
    static mu::engraving::SystemText* createSystemText(mu::engraving::Segment* parent,
                                                       mu::engraving::TextStyleType textStyleType = mu::engraving::TextStyleType::SYSTEM,
                                                       bool isAccessibleEnabled = true);

    static mu::engraving::InstrumentChange* createInstrumentChange(mu::engraving::Segment* parent, bool isAccessibleEnabled = true);
    static mu::engraving::InstrumentChange* createInstrumentChange(mu::engraving::Segment* parent,
                                                                   const mu::engraving::Instrument& instrument,
                                                                   bool isAccessibleEnabled = true);

    static mu::engraving::Sticking* createSticking(mu::engraving::Segment* parent, bool isAccessibleEnabled = true);

    static mu::engraving::Fingering* createFingering(mu::engraving::Note* parent, bool isAccessibleEnabled = true);
    static mu::engraving::Fingering* createFingering(mu::engraving::Note* parent, mu::engraving::TextStyleType textStyleType,
                                                     bool isAccessibleEnabled = true);

    static mu::engraving::Harmony* createHarmony(mu::engraving::Segment* parent, bool isAccessibleEnabled = true);

    static mu::engraving::TempoText* createTempoText(mu::engraving::Segment* parent, bool isAccessibleEnabled = true);

    static mu::engraving::Text* createText(mu::engraving::EngravingItem* parent,
                                           mu::engraving::TextStyleType tid = mu::engraving::TextStyleType::DEFAULT,
                                           bool isAccessibleEnabled = true);
    static mu::engraving::Text* copyText(const mu::engraving::Text& src);

    static mu::engraving::Tie* createTie(mu::engraving::EngravingItem* parent, bool isAccessibleEnabled = true);
    static mu::engraving::Tie* copyTie(const mu::engraving::Tie& src);

    static mu::engraving::TimeSig* createTimeSig(mu::engraving::Segment* parent, bool isAccessibleEnabled = true);
    static mu::engraving::TimeSig* copyTimeSig(const mu::engraving::TimeSig& src);
    static std::shared_ptr<mu::engraving::TimeSig> makeTimeSig(mu::engraving::Segment* parent);

    static mu::engraving::Tremolo* createTremolo(mu::engraving::Chord* parent, bool isAccessibleEnabled = true);
    static mu::engraving::Tremolo* copyTremolo(const mu::engraving::Tremolo& src);
    static std::shared_ptr<mu::engraving::Tremolo> makeTremolo(mu::engraving::Chord* parent);

    static mu::engraving::TremoloBar* createTremoloBar(mu::engraving::EngravingItem* parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<mu::engraving::TremoloBar> makeTremoloBar(mu::engraving::EngravingItem* parent);

    static mu::engraving::Tuplet* createTuplet(mu::engraving::Measure* parent, bool isAccessibleEnabled = true);
    static mu::engraving::Tuplet* copyTuplet(const mu::engraving::Tuplet& src);

    static mu::engraving::Hairpin* createHairpin(mu::engraving::Segment* parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<mu::engraving::Hairpin> makeHairpin(mu::engraving::Segment* parent);

    static mu::engraving::Glissando* createGlissando(mu::engraving::EngravingItem* parent, bool isAccessibleEnabled = true);
    static std::shared_ptr<mu::engraving::Glissando> makeGlissando(mu::engraving::EngravingItem* parent);

    static mu::engraving::Jump* createJump(mu::engraving::Measure* parent, bool isAccessibleEnabled = true);

    static mu::engraving::Trill* createTrill(mu::engraving::EngravingItem* parent, bool isAccessibleEnabled = true);

    static mu::engraving::Vibrato* createVibrato(mu::engraving::EngravingItem* parent, bool isAccessibleEnabled = true);

    static mu::engraving::TextLine* createTextLine(mu::engraving::EngravingItem* parent, bool isAccessibleEnabled = true);

    static mu::engraving::Ottava* createOttava(mu::engraving::EngravingItem* parent, bool isAccessibleEnabled = true);

    static mu::engraving::LetRing* createLetRing(mu::engraving::EngravingItem* parent, bool isAccessibleEnabled = true);

    static mu::engraving::Marker* createMarker(mu::engraving::EngravingItem* parent, bool isAccessibleEnabled = true);

    static mu::engraving::Marker* createMarker(mu::engraving::EngravingItem* parent, TextStyleType tid, bool isAccessibleEnabled = true);

    static mu::engraving::TempoChangeRanged* createTempoChangeRanged(mu::engraving::EngravingItem* parent, bool isAccessibleEnabled = true);

    static mu::engraving::PalmMute* createPalmMute(mu::engraving::EngravingItem* parent, bool isAccessibleEnabled = true);

    static mu::engraving::WhammyBar* createWhammyBar(mu::engraving::EngravingItem* parent, bool isAccessibleEnabled = true);

    static mu::engraving::Rasgueado* createRasgueado(mu::engraving::EngravingItem* parent, bool isAccessibleEnabled = true);

    static mu::engraving::HarmonicMark* createHarmonicMark(mu::engraving::EngravingItem* parent, bool isAccessibleEnabled = true);

    static mu::engraving::Volta* createVolta(mu::engraving::EngravingItem* parent, bool isAccessibleEnabled = true);

    static mu::engraving::Pedal* createPedal(mu::engraving::EngravingItem* parent, bool isAccessibleEnabled = true);

    static mu::engraving::Dynamic* createDynamic(mu::engraving::Segment* parent, bool isAccessibleEnabled = true);

    static mu::engraving::Harmony* createHarmony(mu::engraving::EngravingItem* parent, bool isAccessibleEnabled = true);

    static mu::engraving::VBox* createVBox(mu::engraving::System* parent, bool isAccessibleEnabled = true);

    static mu::engraving::VBox* createVBox(const mu::engraving::ElementType& type, mu::engraving::System* parent,
                                           bool isAccessibleEnabled = true);

    static mu::engraving::HBox* createHBox(mu::engraving::System* parent, bool isAccessibleEnabled = true);

    static mu::engraving::TBox* createTBox(mu::engraving::System* parent, bool isAccessibleEnabled = true);

    static mu::engraving::FBox* createFBox(mu::engraving::System* parent, bool isAccessibleEnabled = true);

private:
    static mu::engraving::EngravingItem* doCreateItem(mu::engraving::ElementType type, mu::engraving::EngravingItem* parent);
};
}

#endif // MU_ENGRAVING_FACTORY_H
