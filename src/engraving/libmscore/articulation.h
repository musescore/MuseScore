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

#ifndef __ARTICULATION_H__
#define __ARTICULATION_H__

#include <set>

#include "element.h"
#include "mscore.h"
#include "symid.h"

namespace Ms {
class ChordRest;
class Segment;
class Measure;
class System;
class Page;

//---------------------------------------------------------
//   ArticulationInfo
//    gives infos about note attributes
//---------------------------------------------------------

enum class ArticulationAnchor : char {
    TOP_STAFF,        // anchor is always placed at top of staff
    BOTTOM_STAFF,     // anchor is always placed at bottom of staff
    CHORD,            // anchor depends on chord direction, away from stem
    TOP_CHORD,        // attribute is always placed at top of chord
    BOTTOM_CHORD,     // attribute is placed at bottom of chord
};

// flags:
enum class ArticulationShowIn : char {
    PITCHED_STAFF = 1, TABLATURE = 2
};

constexpr ArticulationShowIn operator|(ArticulationShowIn a1, ArticulationShowIn a2)
{
    return static_cast<ArticulationShowIn>(static_cast<unsigned char>(a1) | static_cast<unsigned char>(a2));
}

constexpr bool operator&(ArticulationShowIn a1, ArticulationShowIn a2)
{
    return static_cast<unsigned char>(a1) & static_cast<unsigned char>(a2);
}

enum class ArticulationsUpdateMode {
    Insert,
    Remove
};

std::set<SymId> updateArticulations(const std::set<SymId>& articulationSymbolIds, SymId articulationSymbolId,
                                    ArticulationsUpdateMode updateMode = ArticulationsUpdateMode::Insert);
std::set<SymId> splitArticulations(const std::set<SymId>& articulationSymbolIds);
std::set<SymId> joinArticulations(const std::set<SymId>& articulationSymbolIds);
std::set<SymId> flipArticulations(const std::set<SymId>& articulationSymbolIds, Placement placement);

//---------------------------------------------------------
//   @@ Articulation
///    articulation marks
//---------------------------------------------------------

class Articulation final : public Element
{
    SymId _symId;
    Direction _direction;
    QString _channelName;

    ArticulationAnchor _anchor;

    bool _up;
    MScore::OrnamentStyle _ornamentStyle;       // for use in ornaments such as trill
    bool _playArticulation;

    void draw(mu::draw::Painter*) const override;

    enum class AnchorGroup {
        ARTICULATION,
        LUTE_FINGERING,
        OTHER
    };
    static AnchorGroup anchorGroup(SymId);

public:
    Articulation(Score*);
    Articulation(SymId, Score*);
    Articulation& operator=(const Articulation&) = delete;

    Articulation* clone() const override { return new Articulation(*this); }
    ElementType type() const override { return ElementType::ARTICULATION; }

    qreal mag() const override;

    SymId symId() const { return _symId; }
    void setSymId(SymId id);
    int subtype() const override;
    QString userName() const override;
    const char* articulationName() const;    // type-name of articulation; used for midi rendering
    static const char* symId2ArticulationName(SymId symId);

    void layout() override;
    bool layoutCloseToNote() const;

    void read(XmlReader&) override;
    void write(XmlWriter& xml) const override;
    bool readProperties(XmlReader&) override;

    QVector<mu::LineF> dragAnchorLines() const override;

    QVariant getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const QVariant&) override;
    QVariant propertyDefault(Pid) const override;
    void resetProperty(Pid id) override;
    Sid getPropertyStyle(Pid id) const override;

    Pid propertyId(const QStringRef& xmlName) const override;

    bool up() const { return _up; }
    void setUp(bool val);
    void setDirection(Direction d) { _direction = d; }
    Direction direction() const { return _direction; }

    ChordRest* chordRest() const;
    Segment* segment() const;
    Measure* measure() const;
    System* system() const;
    Page* page() const;

    ArticulationAnchor anchor() const { return _anchor; }
    void setAnchor(ArticulationAnchor v) { _anchor = v; }

    MScore::OrnamentStyle ornamentStyle() const { return _ornamentStyle; }
    void setOrnamentStyle(MScore::OrnamentStyle val) { _ornamentStyle = val; }

    bool playArticulation() const { return _playArticulation; }
    void setPlayArticulation(bool val) { _playArticulation = val; }

    QString channelName() const { return _channelName; }
    void setChannelName(const QString& s) { _channelName = s; }

    QString accessibleInfo() const override;

    bool isDouble() const;
    bool isTenuto() const;
    bool isStaccato() const;
    bool isAccent() const;
    bool isMarcato() const;
    bool isLuteFingering() const;
    bool isOrnament() const;

    void doAutoplace();
};
}     // namespace Ms
#endif
