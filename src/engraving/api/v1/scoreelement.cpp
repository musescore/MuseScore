/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "engraving/dom/measure.h"
#include "engraving/dom/range.h"
#include "engraving/dom/engravingobject.h"
#include "engraving/dom/score.h"

// api
#include "apitypes.h"
#include "apistructs.h"
#include "score.h"
#include "part.h"
#include "elements.h"

using namespace mu::engraving::apiv1;

ScoreElement::~ScoreElement()
{
    if (m_ownership == Ownership::PLUGIN) {
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

QQmlListProperty<ScoreElement> ScoreElement::children()
{
    const EngravingObjectList& children = e->scanChildren();
    return wrapContainerProperty<ScoreElement>(this, children);
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
    case P_TYPE::ORNAMENT_INTERVAL: {
        const OrnamentInterval o(val.value<OrnamentInterval>());
        return QVariant::fromValue(wrap(o));
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

static mu::engraving::AlignH alignHFromApiValue(enums::Align apiValue)
{
    switch (apiValue & enums::Align::HMASK) {
    case enums::Align::LEFT: return mu::engraving::AlignH::LEFT;
    case enums::Align::RIGHT: return mu::engraving::AlignH::RIGHT;
    case enums::Align::HCENTER: return mu::engraving::AlignH::HCENTER;
    default:
        break;
    }

    return mu::engraving::AlignH::LEFT;
}

static mu::engraving::AlignV alignVFromApiValue(enums::Align apiValue)
{
    switch (apiValue & enums::Align::VMASK) {
    case enums::Align::TOP: return mu::engraving::AlignV::TOP;
    case enums::Align::BOTTOM: return mu::engraving::AlignV::BOTTOM;
    case enums::Align::VCENTER: return mu::engraving::AlignV::VCENTER;
    case enums::Align::BASELINE: return mu::engraving::AlignV::BASELINE;
    default:
        break;
    }

    return mu::engraving::AlignV::TOP;
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
        // Pid::TIMESIG_ACTUAL is only set when we change the time signature,
        // aside from that it's read-only. What the user intends to do here is
        // change the actual length of the measure, so we do that instead.
        if (pid == Pid::TIMESIG_ACTUAL && e->isMeasure() && m_ownership == Ownership::SCORE) {
            mu::engraving::Measure* m = toMeasure(e);
            if (m->ticks() != f->fraction()) {
                mu::engraving::ScoreRange range;
                range.read(m->first(), m->last());
                m->adjustToLen(f->fraction());
            }
            return;
        }
        newValue = f->fraction();
    }
    break;
    case P_TYPE::ORNAMENT_INTERVAL: {
        OrnamentIntervalWrapper* o = val.value<OrnamentIntervalWrapper*>();
        if (!o) {
            LOGW() << "trying to assign value of wrong type to fractional property";
            return;
        }
        newValue = o->ornamentInterval();
    }
    break;
    case P_TYPE::POINT:
        newValue = PointF::fromQPointF(val.toPointF() * spatium());
        break;
    case P_TYPE::MILLIMETRE:
        newValue = Millimetre(val.toReal() * spatium());
        break;
    case P_TYPE::ALIGN: {
        apiv1::enums::Align apiValue = apiv1::enums::Align(val.toInt());

        newValue = Align { alignHFromApiValue(apiValue), alignVFromApiValue(apiValue) };
    } break;

    default:
        newValue = PropertyValue::fromQVariant(val, propertyType(pid));
        break;
    }

    const PropertyFlags f = e->propertyFlags(pid);
    const PropertyFlags newFlags = (f == PropertyFlags::NOSTYLE) ? f : PropertyFlags::UNSTYLED;

    if (m_ownership == Ownership::SCORE) {
        e->undoChangeProperty(pid, newValue, newFlags);
    } else { // not added to a score so no need (and dangerous) to deal with undo stack
        e->setProperty(pid, newValue);
        e->setPropertyFlags(pid, newFlags);
    }
}

void ScoreElement::reset(mu::engraving::Pid pid)
{
    if (!e) {
        return;
    }

    if (m_ownership == Ownership::SCORE) {
        e->undoResetProperty(pid);
    } else {
        e->resetProperty(pid);
    }
}

//---------------------------------------------------------
//   wrap
///   \cond PLUGIN_API \private \endcond
///   Wraps mu::engraving::ScoreElement choosing the correct wrapper
///   type at runtime based on the actual element type.
//---------------------------------------------------------

ScoreElement* mu::engraving::apiv1::wrap(mu::engraving::EngravingObject* se, Ownership own)
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
        return wrap<mu::engraving::apiv1::Score>(toScore(se), own);
    case ElementType::PART:
        return wrap<mu::engraving::apiv1::Part>(toPart(se), own);
    case ElementType::STAFF:
        return wrap<mu::engraving::apiv1::Staff>(toStaff(se), own);
    default:
        break;
    }
    return wrap<ScoreElement>(se, own);
}
