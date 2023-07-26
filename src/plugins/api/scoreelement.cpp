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

#include "scoreelement.h"

#include "apitypes.h"
#include "elements.h"
#include "fraction.h"
#include "score.h"

#include "libmscore/engravingobject.h"
#include "libmscore/score.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::plugins::api {
//---------------------------------------------------------
//   ScoreElement
//---------------------------------------------------------

ScoreElement::~ScoreElement()
{
    if (_ownership == Ownership::PLUGIN) {
        delete e;
    }
}

QString ScoreElement::name() const
{
    return QString(e->typeName());
}

int ScoreElement::type() const
{
    return int(e->type());
}

//---------------------------------------------------------
//   ScoreElement::userName
///   \brief Human-readable element type name
///   \returns Name of the element type, translated
///   according to the current MuseScore locale settings.
//---------------------------------------------------------

QString ScoreElement::userName() const
{
    return e->translatedTypeUserName();
}

//---------------------------------------------------------
//   ScoreElement::spatium
//---------------------------------------------------------

qreal ScoreElement::spatium() const
{
    return e->isEngravingItem() ? toEngravingItem(e)->spatium() : e->score()->style().spatium();
}

//---------------------------------------------------------
//   ScoreElement::get
//---------------------------------------------------------

QVariant ScoreElement::get(mu::engraving::Pid pid) const
{
    if (!e) {
        return QVariant();
    }
    const PropertyValue val = e->getProperty(pid);
    switch (val.type()) {
    case P_TYPE::FRACTION: {
        const Fraction f(val.value<Fraction>());
        return QVariant::fromValue(wrap(f));
    }
    case P_TYPE::POINT:
        return val.value<PointF>().toQPointF() / spatium();
    case P_TYPE::MILLIMETRE:
        return val.toReal() / spatium();
    case P_TYPE::SPATIUM:
        return val.value<Spatium>().val();
    default:
        break;
    }
    return val.toQVariant();
}

static AlignH alignHFromApiValue(api::enums::Align apiValue)
{
    switch (apiValue & enums::Align::HMASK) {
    case api::enums::Align::LEFT: return engraving::AlignH::LEFT;
    case api::enums::Align::RIGHT: return engraving::AlignH::RIGHT;
    case api::enums::Align::HCENTER: return engraving::AlignH::HCENTER;
    default:
        break;
    }

    return engraving::AlignH::LEFT;
}

static AlignV alignVFromApiValue(api::enums::Align apiValue)
{
    switch (apiValue & enums::Align::VMASK) {
    case api::enums::Align::TOP: return engraving::AlignV::TOP;
    case api::enums::Align::BOTTOM: return engraving::AlignV::BOTTOM;
    case api::enums::Align::VCENTER: return engraving::AlignV::VCENTER;
    case api::enums::Align::BASELINE: return engraving::AlignV::BASELINE;
    default:
        break;
    }

    return engraving::AlignV::TOP;
}

//---------------------------------------------------------
//   ScoreElement::set
//---------------------------------------------------------

void ScoreElement::set(mu::engraving::Pid pid, const QVariant& val)
{
    if (!e) {
        return;
    }

    PropertyValue newValue;

    switch (propertyType(pid)) {
    case P_TYPE::FRACTION: {
        FractionWrapper* f = val.value<FractionWrapper*>();
        if (!f) {
            LOGW() << "trying to assign value of wrong type to fractional property";
            return;
        }
        newValue = f->fraction();
    }
    break;
    case P_TYPE::POINT:
        newValue = PointF::fromQPointF(val.toPointF() * spatium());
        break;
    case P_TYPE::MILLIMETRE:
        newValue = Millimetre(val.toReal() * spatium());
        break;
    case P_TYPE::ALIGN: {
        api::enums::Align apiValue = api::enums::Align(val.toInt());

        newValue = Align { alignHFromApiValue(apiValue), alignVFromApiValue(apiValue) };
    } break;

    default:
        newValue = PropertyValue::fromQVariant(val, propertyType(pid));
        break;
    }

    const PropertyFlags f = e->propertyFlags(pid);
    const PropertyFlags newFlags = (f == PropertyFlags::NOSTYLE) ? f : PropertyFlags::UNSTYLED;

    if (_ownership == Ownership::SCORE) {
        e->undoChangeProperty(pid, newValue, newFlags);
    } else { // not added to a score so no need (and dangerous) to deal with undo stack
        e->setProperty(pid, newValue);
        e->setPropertyFlags(pid, newFlags);
    }
}

//---------------------------------------------------------
//   wrap
///   \cond PLUGIN_API \private \endcond
///   Wraps mu::engraving::ScoreElement choosing the correct wrapper
///   type at runtime based on the actual element type.
//---------------------------------------------------------

ScoreElement* wrap(mu::engraving::EngravingObject* se, Ownership own)
{
    if (!se) {
        return nullptr;
    }
    if (se->isEngravingItem()) {
        return wrap(toEngravingItem(se), own);
    }

    using mu::engraving::ElementType;
    switch (se->type()) {
    case ElementType::SCORE:
        return wrap<Score>(toScore(se), own);
    case ElementType::PART:
        return wrap<Part>(toPart(se), own);
    case ElementType::STAFF:
        return wrap<Staff>(toStaff(se), own);
    default:
        break;
    }
    return wrap<ScoreElement>(se, own);
}
} // namespace mu::plugins::api
