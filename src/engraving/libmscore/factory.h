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
class RootItem;
class Factory
{
public:

    static Ms::ElementType name2type(const QStringRef& name, bool silent = false);
    static Ms::ElementType name2type(const QString& name) { return name2type(QStringRef(&name)); }
    static const char* name(Ms::ElementType type);
    static const char* userName(Ms::ElementType type);

    static Ms::EngravingItem* createItem(Ms::ElementType type, Ms::EngravingItem* parent);
    static Ms::EngravingItem* createItemByName(const QStringRef& name, Ms::EngravingItem* parent);

    static Ms::Accidental* createAccidental(Ms::EngravingItem* parent);
    static std::shared_ptr<Ms::Accidental> makeAccidental(Ms::EngravingItem* parent);

    static Ms::Ambitus* createAmbitus(Ms::Segment* parent);
    static std::shared_ptr<Ms::Ambitus> makeAmbitus(Ms::Segment* parent);

    static Ms::Arpeggio* createArpeggio(Ms::Chord* parent);
    static std::shared_ptr<Ms::Arpeggio> makeArpeggio(Ms::Chord* parent);

    static Ms::Articulation* createArticulation(Ms::ChordRest* parent);
    static std::shared_ptr<Ms::Articulation> makeArticulation(Ms::ChordRest* parent);

    static Ms::BarLine* createBarLine(Ms::Segment* parent);
    static Ms::BarLine* copyBarLine(const Ms::BarLine& src);
    static std::shared_ptr<Ms::BarLine> makeBarLine(Ms::Segment* parent);

    //! NOTE Ms::Score* is needed temporarily
    static Ms::Beam* createBeam(Ms::EngravingItem* parent, Ms::Score* score);
    static std::shared_ptr<Ms::Beam> makeBeam(Ms::EngravingItem* parent, Ms::Score* score);

    static Ms::Bend* createBend(Ms::Note* parent);
    static std::shared_ptr<Ms::Bend> makeBend(Ms::Note* parent);

    static Ms::Bracket* createBracket(Ms::EngravingItem* parent);
    static std::shared_ptr<Ms::Bracket> makeBracket(Ms::EngravingItem* parent);
    static Ms::BracketItem* createBracketItem(Ms::EngravingItem* parent);
    static Ms::BracketItem* createBracketItem(Ms::EngravingItem* parent, Ms::BracketType a, int b);

    static Ms::Breath* createBreath(Ms::Segment* parent);
    static std::shared_ptr<Ms::Breath> makeBreath(Ms::Segment* parent);

    static Ms::Chord* createChord(Ms::Segment* parent);
    static Ms::Chord* copyChord(const Ms::Chord& src, bool link = false);
    static std::shared_ptr<Ms::Chord> makeChord(Ms::Segment* parent);

    static Ms::ChordLine* createChordLine(Ms::Chord* parent);
    static Ms::ChordLine* copyChordLine(const Ms::ChordLine& src);
    static std::shared_ptr<Ms::ChordLine> makeChordLine(Ms::Chord* parent);

    static Ms::Clef* createClef(Ms::Segment* parent);
    static Ms::Clef* copyClef(const Ms::Clef& src);
    static std::shared_ptr<Ms::Clef> makeClef(Ms::Segment* parent);

    static Ms::Fermata* createFermata(Ms::EngravingItem* parent);
    static std::shared_ptr<Ms::Fermata> makeFermata(Ms::EngravingItem* parent);

    static Ms::FiguredBass* createFiguredBass(Ms::Segment* parent);
    static std::shared_ptr<Ms::FiguredBass> makeFiguredBass(Ms::Segment* parent);

    static Ms::FretDiagram* createFretDiagram(Ms::Segment* parent);
    static Ms::FretDiagram* copyFretDiagram(const Ms::FretDiagram& src);
    static std::shared_ptr<Ms::FretDiagram> makeFretDiagram(Ms::Segment* parent);

    static Ms::KeySig* createKeySig(Ms::Segment* parent);
    static Ms::KeySig* copyKeySig(const Ms::KeySig& src);
    static std::shared_ptr<Ms::KeySig> makeKeySig(Ms::Segment* parent);

    static Ms::LayoutBreak* createLayoutBreak(Ms::MeasureBase* parent);
    static Ms::LayoutBreak* copyLayoutBreak(const Ms::LayoutBreak& src);
    static std::shared_ptr<Ms::LayoutBreak> makeLayoutBreak(Ms::MeasureBase* parent);

    static Ms::Measure* createMeasure(Ms::System* parent);
    static Ms::Measure* copyMeasure(const Ms::Measure& src);

    static Ms::Note* createNote(Ms::Chord* parent);
    static Ms::Note* copyNote(const Ms::Note& src, bool link = false);
    static std::shared_ptr<Ms::Note> makeNote(Ms::Chord* parent);

    static Ms::NoteDot* createNoteDot(Ms::Note* parent);
    static Ms::NoteDot* createNoteDot(Ms::Rest* parent);
    static Ms::NoteDot* copyNoteDot(const Ms::NoteDot& src);

    static Ms::Page* createPage(RootItem* parent);

    static Ms::Rest* createRest(Ms::Segment* parent);
    static Ms::Rest* createRest(Ms::Segment* parent, const Ms::TDuration& t);
    static Ms::Rest* copyRest(const Ms::Rest& src, bool link = false);

    static Ms::Segment* createSegment(Ms::Measure* parent);
    static Ms::Segment* createSegment(Ms::Measure* parent, Ms::SegmentType type, const Ms::Fraction& t);

    static Ms::Slur* createSlur(Ms::EngravingItem* parent);
    static std::shared_ptr<Ms::Slur> makeSlur(Ms::EngravingItem* parent);

    static Ms::Spacer* createSpacer(Ms::Measure* parent);
    static std::shared_ptr<Ms::Spacer> makeSpacer(Ms::Measure* parent);

    static Ms::Staff* createStaff(Ms::Part* parent);

    static Ms::StaffLines* createStaffLines(Ms::Measure* parent);
    static Ms::StaffLines* copyStaffLines(const Ms::StaffLines& src);

    static Ms::StaffState* createStaffState(Ms::EngravingItem* parent);

    static Ms::StaffTypeChange* createStaffTypeChange(Ms::MeasureBase* parent);
    static std::shared_ptr<Ms::StaffTypeChange> makeStaffTypeChange(Ms::MeasureBase* parent);

    static Ms::Stem* createStem(Ms::Chord* parent);
    static Ms::Stem* copyStem(const Ms::Stem& src);

    static Ms::StemSlash* createStemSlash(Ms::Chord* parent);
    static Ms::StemSlash* copyStemSlash(const Ms::StemSlash& src);

    static Ms::System* createSystem(Ms::Page* parent);

    static Ms::Text* createText(Ms::EngravingItem* parent, Ms::Tid tid = Ms::Tid::DEFAULT);
    static Ms::Text* createTextJustForRead(Ms::EngravingItem* parent);
    static Ms::Text* copyText(const Ms::Text& src);

    static Ms::TimeSig* createTimeSig(Ms::Segment* parent);
    static Ms::TimeSig* copyTimeSig(const Ms::TimeSig& src);
    static std::shared_ptr<Ms::TimeSig> makeTimeSig(Ms::Segment* parent);

    static Ms::Tremolo* createTremolo(Ms::Chord* parent);
    static Ms::Tremolo* copyTremolo(const Ms::Tremolo& src);
    static std::shared_ptr<Ms::Tremolo> makeTremolo(Ms::Chord* parent);

    static Ms::TremoloBar* createTremoloBar(Ms::EngravingItem* parent);
    static std::shared_ptr<Ms::TremoloBar> makeTremoloBar(Ms::EngravingItem* parent);

private:
    static Ms::EngravingItem* doCreateItem(Ms::ElementType type, Ms::EngravingItem* parent);
};
}

#endif // MU_ENGRAVING_FACTORY_H
