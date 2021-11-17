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

namespace Ms {
class Score;
class XmlWriter;
class Volta;
class Measure;

extern void vdebug(int n);
extern LineSegment* voltaDebug;

//---------------------------------------------------------
//   @@ VoltaSegment
//---------------------------------------------------------

class VoltaSegment final : public TextLineBaseSegment
{
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
    QList<int> _endings;
    static constexpr Anchor VOLTA_ANCHOR = Anchor::MEASURE;

public:
    enum class Type : char {
        OPEN, CLOSED
    };

    Volta(EngravingItem* parent);

    Volta* clone() const override { return new Volta(*this); }

    LineSegment* createLineSegment(System* parent) override;

    void write(XmlWriter&) const override;
    void read(XmlReader& e) override;

    bool readProperties(XmlReader&) override;
    SpannerSegment* layoutSystem(System* system) override;

    void setVelocity() const;
    void setChannel() const;
    void setTempo() const;

    QList<int> endings() const { return _endings; }
    QList<int>& endings() { return _endings; }
    void setEndings(const QList<int>& l);
    void setText(const QString& s);
    QString text() const;

    bool hasEnding(int repeat) const;
    int firstEnding() const;
    int lastEnding() const;
    void setVoltaType(Volta::Type);       // deprecated
    Type voltaType() const;               // deprecated

    mu::engraving::PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const mu::engraving::PropertyValue&) override;
    mu::engraving::PropertyValue propertyDefault(Pid) const override;

    QString accessibleInfo() const override;
};
}     // namespace Ms

Q_DECLARE_METATYPE(Ms::Volta::Type);

#endif
