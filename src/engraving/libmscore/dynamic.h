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
    Q_GADGET
public:
    enum class Type : char {
        OTHER,
        PPPPPP,
        PPPPP,
        PPPP,
        PPP,
        PP,
        P,
        MP,
        MF,
        F,
        FF,
        FFF,
        FFFF,
        FFFFF,
        FFFFFF,
        FP,
        SF,
        SFZ,
        SFF,
        SFFZ,
        SFP,
        SFPP,
        RFZ,
        RF,
        FZ,
        M,
        R,
        S,
        Z
    };

    enum class Range : char {
        STAFF, PART, SYSTEM
    };

    enum class Speed : char {
        SLOW, NORMAL, FAST
    };

    struct ChangeSpeedItem {
        Speed speed;
        const char* name;
    };

    Q_ENUM(Type);

private:
    Type _dynamicType;

    mutable mu::PointF dragOffset;
    int _velocity;       // associated midi velocity 0-127
    Range _dynRange;     // STAFF, PART, SYSTEM

    int _changeInVelocity         { 128 };
    Speed _velChangeSpeed         { Speed::NORMAL };

    mu::RectF drag(EditData&) override;

public:
    Dynamic(Score*);
    Dynamic(const Dynamic&);
    Dynamic* clone() const override { return new Dynamic(*this); }
    ElementType type() const override { return ElementType::DYNAMIC; }
    Segment* segment() const { return (Segment*)parent(); }
    Measure* measure() const { return (Measure*)parent()->parent(); }

    void setDynamicType(Type val) { _dynamicType = val; }
    void setDynamicType(const QString&);
    static QString dynamicTypeName(Dynamic::Type type);
    QString dynamicTypeName() const { return dynamicTypeName(_dynamicType); }
    Type dynamicType() const { return _dynamicType; }
    int subtype() const override { return static_cast<int>(_dynamicType); }
    QString subtypeName() const override { return dynamicTypeName(); }

    void layout() override;
    void write(XmlWriter& xml) const override;
    void read(XmlReader&) override;

    bool isEditable() const override { return true; }
    void startEdit(EditData&) override;
    void endEdit(EditData&) override;
    void reset() override;

    void setVelocity(int v) { _velocity = v; }
    int velocity() const;
    Range dynRange() const { return _dynRange; }
    void setDynRange(Range t) { _dynRange = t; }
    void undoSetDynRange(Range t);

    int changeInVelocity() const;
    void setChangeInVelocity(int val);
    Fraction velocityChangeLength() const;
    bool isVelocityChangeAvailable() const;

    Speed velChangeSpeed() const { return _velChangeSpeed; }
    void setVelChangeSpeed(Speed val) { _velChangeSpeed = val; }
    static QString speedToName(Speed speed);
    static Speed nameToSpeed(QString name);

    QVariant getProperty(Pid propertyId) const override;
    bool     setProperty(Pid propertyId, const QVariant&) override;
    QVariant propertyDefault(Pid id) const override;
    Pid propertyId(const QStringRef& xmlName) const override;
    QString propertyUserValue(Pid) const override;

    std::unique_ptr<ElementGroup> getDragGroup(std::function<bool(const Element*)> isDragged) override;

    QString accessibleInfo() const override;
    QString screenReaderInfo() const override;
    void doAutoplace();

    static const std::vector<ChangeSpeedItem> changeSpeedTable;
    static int findInString(const QString& text, int& length, QString& type);
};
}     // namespace Ms

Q_DECLARE_METATYPE(Ms::Dynamic::Range);

#endif
