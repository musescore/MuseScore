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

#include "systemdivider.h"
#include "rw/xml.h"
#include "types/symnames.h"

#include "score.h"
#include "measure.h"
#include "system.h"

using namespace mu;
using namespace mu::engraving;

namespace Ms {
//---------------------------------------------------------
//   SystemDivider
//---------------------------------------------------------

SystemDivider::SystemDivider(System* parent)
    : Symbol(ElementType::SYSTEM_DIVIDER, parent, ElementFlag::SYSTEM | ElementFlag::NOT_SELECTABLE)
{
    // default value, but not valid until setDividerType()
    _dividerType = SystemDivider::Type::LEFT;
    _sym = SymId::systemDivider;
}

//---------------------------------------------------------
//   SystemDivider
//---------------------------------------------------------

SystemDivider::SystemDivider(const SystemDivider& sd)
    : Symbol(sd)
{
    _dividerType = sd._dividerType;
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void SystemDivider::layout()
{
    SymId sid;
    ScoreFont* sf = score()->scoreFont();

    if (_dividerType == SystemDivider::Type::LEFT) {
        sid = SymNames::symIdByName(score()->styleSt(Sid::dividerLeftSym));
    } else {
        sid = SymNames::symIdByName(score()->styleSt(Sid::dividerRightSym));
    }
    setSym(sid, sf);
    Symbol::layout();
}

//---------------------------------------------------------
//   setDividerType
//---------------------------------------------------------

void SystemDivider::setDividerType(SystemDivider::Type v)
{
    _dividerType = v;
    if (v == SystemDivider::Type::LEFT) {
        setOffset(PointF(score()->styleD(Sid::dividerLeftX), score()->styleD(Sid::dividerLeftY)));
    } else {
        setOffset(PointF(score()->styleD(Sid::dividerRightX), score()->styleD(Sid::dividerRightY)));
    }
}

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

mu::RectF SystemDivider::drag(EditData& ed)
{
    setGenerated(false);
    return Symbol::drag(ed);
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void SystemDivider::write(XmlWriter& xml) const
{
    if (dividerType() == SystemDivider::Type::LEFT) {
        xml.startObject(this, "type=\"left\"");
    } else {
        xml.startObject(this, "type=\"right\"");
    }
    writeProperties(xml);
    xml.endObject();
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void SystemDivider::read(XmlReader& e)
{
    ScoreFont* sf = score()->scoreFont();
    if (e.attribute("type") == "left") {
        _dividerType = SystemDivider::Type::LEFT;
        SymId sym = SymNames::symIdByName(score()->styleSt(Sid::dividerLeftSym));
        setSym(sym, sf);
        setOffset(PointF(score()->styleD(Sid::dividerLeftX), score()->styleD(Sid::dividerLeftY)));
    } else {
        _dividerType = SystemDivider::Type::RIGHT;
        SymId sym = SymNames::symIdByName(score()->styleSt(Sid::dividerRightSym));
        setSym(sym, sf);
        setOffset(PointF(score()->styleD(Sid::dividerRightX), score()->styleD(Sid::dividerRightY)));
    }
    Symbol::read(e);
}
}  // namespace Ms
