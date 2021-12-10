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

#ifndef __DYNAMICS_H__
#define __DYNAMICS_H__

#include "text.h"
#include "mscore.h"

namespace Ms {
class Measure;
class Segment;

//-----------------------------------------------------------------------------
//   @@ Dynamic
///    dynamics marker; determines midi velocity
//
//   @P range  enum (Dynamic.STAFF, .PART, .SYSTEM)
//-----------------------------------------------------------------------------

class Dynamic final : public TextBase
{
public:

    struct ChangeSpeedItem {
        DynamicSpeed speed;
        const char* name;
    };

private:
    DynamicType _dynamicType;

    mutable mu::PointF dragOffset;
    int _velocity;       // associated midi velocity 0-127
    DynamicRange _dynRange;     // STAFF, PART, SYSTEM

    int _changeInVelocity         { 128 };
    DynamicSpeed _velChangeSpeed         { DynamicSpeed::NORMAL };

    mu::RectF drag(EditData&) override;

public:
    Dynamic(Segment* parent);
    Dynamic(const Dynamic&);
    Dynamic* clone() const override { return new Dynamic(*this); }
    Segment* segment() const { return (Segment*)explicitParent(); }
    Measure* measure() const { return (Measure*)explicitParent()->explicitParent(); }

    void setDynamicType(DynamicType val) { _dynamicType = val; }
    void setDynamicType(const QString&);

    DynamicType dynamicType() const { return _dynamicType; }
    int subtype() const override { return static_cast<int>(_dynamicType); }
    QString subtypeName() const override;

    void layout() override;
    void write(XmlWriter& xml) const override;
    void read(XmlReader&) override;

    bool isEditable() const override { return true; }
    void startEdit(EditData&) override;
    void endEdit(EditData&) override;
    void reset() override;

    void setVelocity(int v) { _velocity = v; }
    int velocity() const;
    DynamicRange dynRange() const { return _dynRange; }
    void setDynRange(DynamicRange t) { _dynRange = t; }
    void undoSetDynRange(DynamicRange t);

    int changeInVelocity() const;
    void setChangeInVelocity(int val);
    Fraction velocityChangeLength() const;
    bool isVelocityChangeAvailable() const;

    DynamicSpeed velChangeSpeed() const { return _velChangeSpeed; }
    void setVelChangeSpeed(DynamicSpeed val) { _velChangeSpeed = val; }

    mu::engraving::PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const mu::engraving::PropertyValue&) override;
    mu::engraving::PropertyValue propertyDefault(Pid id) const override;
    Pid propertyId(const QStringRef& xmlName) const override;

    std::unique_ptr<ElementGroup> getDragGroup(std::function<bool(const EngravingItem*)> isDragged) override;

    QString accessibleInfo() const override;
    QString screenReaderInfo() const override;
    void doAutoplace();

    static QString dynamicText(DynamicType t);
};
}     // namespace Ms

#endif
