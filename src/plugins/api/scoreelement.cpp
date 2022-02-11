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
#include "elements.h"
#include "score.h"
#include "fraction.h"
#include "libmscore/masterscore.h"
#include "libmscore/engravingobject.h"

using namespace mu;
using namespace mu::engraving;

namespace Ms {
namespace PluginAPI {
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
    return e->typeUserName();
}

//---------------------------------------------------------
//   ScoreElement::spatium
//---------------------------------------------------------

qreal ScoreElement::spatium() const
{
    return e->isEngravingItem() ? toEngravingItem(e)->spatium() : e->score()->spatium();
}

//---------------------------------------------------------
//   ScoreElement::get
//---------------------------------------------------------

QVariant ScoreElement::get(Ms::Pid pid) const
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

//---------------------------------------------------------
//   ScoreElement::set
//---------------------------------------------------------

void ScoreElement::set(Ms::Pid pid, QVariant val)
{
    if (!e) {
        return;
    }

    switch (propertyType(pid)) {
    case P_TYPE::FRACTION: {
        FractionWrapper* f = val.value<FractionWrapper*>();
        if (!f) {
            qWarning("ScoreElement::set: trying to assign value of wrong type to fractional property");
            return;
        }
        val = f->fraction().toString();
    }
    break;
    case P_TYPE::POINT:
        val = val.toPointF() * spatium();
        break;
    case P_TYPE::MILLIMETRE:
        val = val.toReal() * spatium();
        break;
    default:
        break;
    }

    const PropertyFlags f = e->propertyFlags(pid);
    const PropertyFlags newFlags = (f == PropertyFlags::NOSTYLE) ? f : PropertyFlags::UNSTYLED;

    if (_ownership == Ownership::SCORE) {
        e->undoChangeProperty(pid, PropertyValue::fromQVariant(val, propertyType(pid)), newFlags);
    } else { // not added to a score so no need (and dangerous) to deal with undo stack
        e->setProperty(pid, PropertyValue::fromQVariant(val, propertyType(pid)));
        e->setPropertyFlags(pid, newFlags);
    }
}

//---------------------------------------------------------
//   wrap
///   \cond PLUGIN_API \private \endcond
///   Wraps Ms::ScoreElement choosing the correct wrapper
///   type at runtime based on the actual element type.
//---------------------------------------------------------

ScoreElement* wrap(Ms::EngravingObject* se, Ownership own)
{
    if (!se) {
        return nullptr;
    }
    if (se->isEngravingItem()) {
        return wrap(toEngravingItem(se), own);
    }

    using Ms::ElementType;
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
}
}
