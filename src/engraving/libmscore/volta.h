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

#ifndef __VOLTA_H__
#define __VOLTA_H__

#include "textlinebase.h"

namespace mu::engraving {
class Score;
class XmlWriter;
class Volta;
class Measure;

//---------------------------------------------------------
//   @@ VoltaSegment
//---------------------------------------------------------

class VoltaSegment final : public TextLineBaseSegment
{
    OBJECT_ALLOCATOR(engraving, VoltaSegment)
public:
    VoltaSegment(Volta*, System* parent);

    VoltaSegment* clone() const override { return new VoltaSegment(*this); }

    Volta* volta() const { return (Volta*)spanner(); }
    void layout() override;

    EngravingItem* propertyDelegate(Pid) override;
};

//---------------------------------------------------------
//   @@ Volta
//   @P voltaType  enum (Volta.CLOSE, Volta.OPEN)
//---------------------------------------------------------

class Volta final : public TextLineBase
{
    OBJECT_ALLOCATOR(engraving, Volta)
    DECLARE_CLASSOF(ElementType::VOLTA)

    std::vector<int> _endings;

public:
    enum class Type : char {
        OPEN, CLOSED
    };

    static constexpr Anchor VOLTA_ANCHOR = Anchor::MEASURE;

    Volta(EngravingItem* parent);

    Volta* clone() const override { return new Volta(*this); }

    LineSegment* createLineSegment(System* parent) override;

    SpannerSegment* layoutSystem(System* system) override;

    void setVelocity() const;
    void setChannel() const;
    void setTempo() const;

    std::vector<int> endings() const { return _endings; }
    std::vector<int>& endings() { return _endings; }
    void setEndings(const std::vector<int>& l);
    void setText(const String& s);
    String text() const;

    bool hasEnding(int repeat) const;
    int firstEnding() const;
    int lastEnding() const;
    void setVoltaType(Volta::Type);       // deprecated
    Type voltaType() const;               // deprecated

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

    String accessibleInfo() const override;
};
} // namespace mu::engraving

#ifndef NO_QT_SUPPORT
Q_DECLARE_METATYPE(mu::engraving::Volta::Type);
#endif

#endif
