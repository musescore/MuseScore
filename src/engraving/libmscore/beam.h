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

#ifndef __BEAM_H__
#define __BEAM_H__

#include "element.h"
#include "durationtype.h"
#include "property.h"

namespace Ms {
class ChordRest;
class MuseScoreView;
class Chord;
class System;
class Skyline;

enum class IconType : signed char;
enum class SpannerSegmentType;

struct BeamFragment;

//---------------------------------------------------------
//   @@ Beam
//---------------------------------------------------------

class Beam final : public Element
{
    Q_GADGET
    QVector<ChordRest*> _elements;          // must be sorted by tick
    QVector<mu::LineF*> beamSegments;
    Direction _direction;

    bool _up;
    bool _distribute;                     // equal spacing of elements
    bool _noSlope;

    bool _userModified[2];                // 0: auto/down  1: up
    bool _isGrace;
    bool _cross;

    qreal _grow1;                         // define "feather" beams
    qreal _grow2;
    qreal _beamDist;

    QVector<BeamFragment*> fragments;       // beam splits across systems

    mutable int _id;            // used in read()/write()

    int minMove;                // set in layout1()
    int maxMove;
    TDuration maxDuration;
    qreal slope { 0.0 };

    void layout2(std::vector<ChordRest*>, SpannerSegmentType, int frag);
    bool twoBeamedNotes();
    void computeStemLen(const std::vector<ChordRest*>& crl, qreal& py1, int beamLevels);
    bool slopeZero(const std::vector<ChordRest*>& crl);
    bool hasNoSlope();
    void addChordRest(ChordRest* a);
    void removeChordRest(ChordRest* a);

public:
    enum class Mode : signed char {
        ///.\{
        AUTO, BEGIN, MID, END, NONE, BEGIN32, BEGIN64, INVALID = -1
                                                                 ///\}
    };
    Q_ENUM(Mode);

    Beam(Score* = 0);
    Beam(const Beam&);
    ~Beam();

    // Score Tree functions
    ScoreElement* treeParent() const override;
    ScoreElement* treeChild(int idx) const override;
    int treeChildCount() const override;

    void scanElements(void* data, void (* func)(void*, Element*), bool all=true) override;

    Beam* clone() const override { return new Beam(*this); }
    ElementType type() const override { return ElementType::BEAM; }
    mu::PointF pagePos() const override;      ///< position in page coordinates
    mu::PointF canvasPos() const override;    ///< position in page coordinates

    bool isEditable() const override { return true; }
    void startEdit(EditData&) override;
    void endEdit(EditData&) override;
    void editDrag(EditData&) override;

    Fraction tick() const override;
    Fraction rtick() const override;
    Fraction ticks() const;

    void write(XmlWriter& xml) const override;
    void read(XmlReader&) override;
    void spatiumChanged(qreal /*oldValue*/, qreal /*newValue*/) override;

    void reset() override;

    System* system() const { return toSystem(parent()); }

    void layout1();
    void layoutGraceNotes();
    void layout() override;

    const QVector<ChordRest*>& elements() const { return _elements; }
    void clear() { _elements.clear(); }
    bool empty() const { return _elements.empty(); }
    bool contains(const ChordRest* cr) const
    {
        return std::find(_elements.begin(), _elements.end(), cr) != _elements.end();
    }

    void add(Element*) override;
    void remove(Element*) override;

    void move(const mu::PointF&) override;
    void draw(mu::draw::Painter*) const override;

    bool up() const { return _up; }
    void setUp(bool v) { _up = v; }
    void setId(int i) const { _id = i; }
    int id() const { return _id; }
    bool isNoSlope() const;
    void alignBeamPosition();

    void setBeamDirection(Direction d);
    Direction beamDirection() const { return _direction; }

    //!Note Unfortunately we have no FEATHERED_BEAM_MODE for now int Beam::Mode enum, so we'll handle this localy
    void setAsFeathered(const bool slower);
    bool acceptDrop(EditData&) const override;
    Element* drop(EditData&) override;

    qreal growLeft() const { return _grow1; }
    qreal growRight() const { return _grow2; }
    void setGrowLeft(qreal val) { _grow1 = val; }
    void setGrowRight(qreal val) { _grow2 = val; }

    bool distribute() const { return _distribute; }
    void setDistribute(bool val) { _distribute = val; }

    bool userModified() const;
    void setUserModified(bool val);

    mu::PointF beamPos() const;
    void setBeamPos(const mu::PointF& bp);

    qreal beamDist() const { return _beamDist; }

    QVariant getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const QVariant&) override;
    QVariant propertyDefault(Pid id) const override;

    bool isGrace() const { return _isGrace; }    // for debugger
    bool cross() const { return _cross; }

    void addSkyline(Skyline&);

    void triggerLayout() const override;

    EditBehavior normalModeEditBehavior() const override { return EditBehavior::Edit; }
    int gripsCount() const override { return 3; }
    Grip initialEditModeGrip() const override { return Grip::END; }
    Grip defaultGrip() const override { return Grip::MIDDLE; }
    std::vector<mu::PointF> gripsPositions(const EditData&) const override;

    static IconType iconType(Mode);

    mu::RectF drag(EditData&) override;
    bool isMovable() const override;
    void startDrag(EditData&) override;

private:
    void initBeamEditData(EditData& ed);
};
}     // namespace Ms
#endif
