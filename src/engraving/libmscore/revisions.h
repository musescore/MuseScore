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

#ifndef __REVISIONS_H__
#define __REVISIONS_H__

#include <QString>
#include <QDateTime>
#include <vector>

namespace mu::engraving {
class XmlWriter;
class XmlReader;

//---------------------------------------------------------
//   Revision
//---------------------------------------------------------

class Revision
{
    QString _id;
    QString _diff;            // diff to parent
    QDateTime _dateTime;
    Revision* _parent;
    std::vector<Revision*> _branches;

public:
    Revision();
    void read(XmlReader&);
    void write(XmlWriter&) const;
    void setParent(Revision* r) { _parent = r; }
    Revision* parent() const { return _parent; }
    const std::vector<Revision*>& branches() const { return _branches; }
    void setId(const QString& s) { _id = s; }
    void setDiff(const QString& s) { _diff = s; }
};

//---------------------------------------------------------
//   Revisions
//    id:  2.3.1
//         | | +-- revision of branch
//         | +---- branch number
//         +------ revision
//---------------------------------------------------------

class Revisions
{
    Revision* _trunk;

    void write(XmlWriter&, const Revision*) const;

public:
    Revisions();
    void add(Revision*);
    QString getRevision(QString id);
    Revision* trunk() { return _trunk; }
    void write(XmlWriter&) const;
};
} // namespace mu::engraving
#endif
