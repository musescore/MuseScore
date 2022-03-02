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

#ifndef __FERMATA_H__
#define __FERMATA_H__

#include "engravingitem.h"
#include "mscore.h"

#include "types/types.h"

namespace mu::engraving {
class Factory;
}

namespace Ms {
class ChordRest;
class Segment;
class Measure;
class System;
class Page;

//---------------------------------------------------------
//    Fermata
//---------------------------------------------------------

class Fermata final : public EngravingItem
{
    SymId _symId;
    qreal _timeStretch;
    bool _play;

    friend class mu::engraving::Factory;
    Fermata(EngravingItem* parent);

    void draw(mu::draw::Painter*) const override;
    Sid getPropertyStyle(Pid) const override;

public:

    Fermata& operator=(const Fermata&) = delete;

    Fermata* clone() const override { return new Fermata(*this); }

    qreal mag() const override;

    SymId symId() const { return _symId; }
    void setSymId(SymId id) { _symId  = id; }
    FermataType fermataType() const;
    int subtype() const override;
    QString typeUserName() const override;

    void layout() override;

    void read(XmlReader&) override;
    void write(XmlWriter& xml) const override;
    bool readProperties(XmlReader&) override;

    QVector<mu::LineF> dragAnchorLines() const override;

    mu::engraving::PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const mu::engraving::PropertyValue&) override;
    mu::engraving::PropertyValue propertyDefault(Pid) const override;
    void resetProperty(Pid id) override;

    Pid propertyId(const QStringRef& xmlName) const override;

    ChordRest* chordRest() const;
    Segment* segment() const { return toSegment(explicitParent()); }
    Measure* measure() const;
    System* system() const;
    Page* page() const;

    qreal timeStretch() const { return _timeStretch; }
    void setTimeStretch(qreal val) { _timeStretch = val; }

    bool play() const { return _play; }
    void setPlay(bool val) { _play = val; }

    QString accessibleInfo() const override;

protected:
    void added() override;
    void removed() override;
};
}     // namespace Ms
#endif
