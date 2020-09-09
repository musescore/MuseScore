//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "scoreelement.h"
#include "elements.h"
#include "score.h"
#include "fraction.h"
#include "libmscore/score.h"
#include "libmscore/scoreElement.h"

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
    return QString(e->name());
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
    return e->userName();
}

//---------------------------------------------------------
//   ScoreElement::get
//---------------------------------------------------------

QVariant ScoreElement::get(Ms::Pid pid) const
{
    if (!e) {
        return QVariant();
    }
    const QVariant val = e->getProperty(pid);
    switch (propertyType(pid)) {
    case P_TYPE::FRACTION: {
        const Fraction f(val.value<Fraction>());
        return QVariant::fromValue(wrap(f));
    }
    case P_TYPE::POINT_SP:
    case P_TYPE::POINT_SP_MM:
        if (e->isElement()) {
            return val.toPointF() / toElement(e)->spatium();
        }
        // TODO: handle Staff and other classes?
        break;
    default:
        break;
    }
    return val;
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
        val = QVariant::fromValue(f->fraction());
    }
    break;
    case P_TYPE::POINT_SP:
    case P_TYPE::POINT_SP_MM:
        if (e->isElement()) {
            val = val.toPointF() * toElement(e)->spatium();
        }
        // TODO: handle Staff and other classes?
        break;
    default:
        break;
    }

    const PropertyFlags f = e->propertyFlags(pid);
    const PropertyFlags newFlags = (f == PropertyFlags::NOSTYLE) ? f : PropertyFlags::UNSTYLED;

    if (_ownership == Ownership::SCORE) {
        e->undoChangeProperty(pid, val, newFlags);
    } else { // not added to a score so no need (and dangerous) to deal with undo stack
        e->setProperty(pid, val);
        e->setPropertyFlags(pid, newFlags);
    }
}

//---------------------------------------------------------
//   wrap
///   \cond PLUGIN_API \private \endcond
///   Wraps Ms::ScoreElement choosing the correct wrapper
///   type at runtime based on the actual element type.
//---------------------------------------------------------

ScoreElement* wrap(Ms::ScoreElement* se, Ownership own)
{
    if (!se) {
        return nullptr;
    }
    if (se->isElement()) {
        return wrap(toElement(se), own);
    }

    using Ms::ElementType;
    switch (se->type()) {
    case ElementType::SCORE:
        return wrap<Score>(toScore(se), own);
    case ElementType::PART:
        return wrap<Part>(toPart(se), own);
    default:
        break;
    }
    return wrap<ScoreElement>(se, own);
}
}
}
