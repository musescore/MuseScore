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

#include "revisions.h"
#include "rw/xml.h"

using namespace mu;

namespace Ms {
//---------------------------------------------------------
//   Revision
//---------------------------------------------------------

Revision::Revision()
{
    _parent = 0;
    _dateTime = QDateTime::currentDateTime();
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Revision::write(XmlWriter& xml) const
{
    xml.startObject("Revision");
    xml.tag("id",   _id);
    xml.tag("date", _dateTime.toString());
    xml.tag("diff", _diff);
    xml.endObject();
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Revision::read(XmlReader& e)
{
    _dateTime = QDateTime::currentDateTime();
    while (e.readNextStartElement()) {
        const QStringRef& tag(e.name());
        if (tag == "id") {
            _id = e.readElementText();
        } else if (tag == "diff") {
            _diff = e.readElementText();
        } else if (tag == "date") {
            _dateTime = QDateTime::fromString(e.readElementText());
        } else {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   Revisions
//---------------------------------------------------------

Revisions::Revisions()
{
    _trunk = 0;
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Revisions::write(XmlWriter& xml) const
{
    for (Revision* r = _trunk; r; r = r->parent()) {
        write(xml, r);
    }
}

void Revisions::write(XmlWriter& xml, const Revision* r) const
{
    r->write(xml);
    foreach (const Revision* rr, r->branches()) {
        write(xml, rr);
    }
}

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Revisions::add(Revision* r)
{
    if (_trunk == 0) {
        _trunk = r;
        _trunk->setParent(0);
        return;
    }
}

//---------------------------------------------------------
//   getRevision
//---------------------------------------------------------

QString Revisions::getRevision(QString /*id*/)
{
    return QString();
}
}
