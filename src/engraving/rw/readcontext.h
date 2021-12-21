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

#ifndef MU_ENGRAVING_READCONTEXT_H
#define MU_ENGRAVING_READCONTEXT_H

#include "libmscore/mscore.h"
#include "libmscore/location.h"
#include "compat/dummyelement.h"
#include "linksindexer.h"

namespace Ms {
class Score;
class TimeSigMap;
class Staff;
class Spanner;
class LinkedObjects;
}

namespace mu::engraving {
class ReadContext
{
public:

    ReadContext(Ms::Score* score);

    void setIgnoreVersionError(bool arg);
    bool ignoreVersionError() const;

    QString mscoreVersion() const;
    int mscVersion() const;

    int fileDivision() const;
    int fileDivision(int t) const;

    qreal spatium() const;

    mu::engraving::compat::DummyElement* dummy() const;

    Ms::TimeSigMap* sigmap();
    Ms::Staff* staff(int n);

    void appendStaff(Ms::Staff* staff);
    void addSpanner(Ms::Spanner* s);

    bool undoStackActive() const;

    bool isSameScore(const Ms::EngravingObject* obj) const;

    void initLinks(const ReadContext& ctx);
    void addLink(Ms::Staff* staff, Ms::LinkedObjects* link, const Ms::Location& location);
    Ms::LinkedObjects* getLink(bool isMasterScore, const Ms::Location& location, int localIndexDiff);
    QMap<int, QList<QPair<Ms::LinkedObjects*, Ms::Location> > >& staffLinkedElements();

private:
    Ms::Score* m_score = nullptr;
    bool m_ignoreVersionError = false;

    QMap<int /*staffIndex*/, QList<QPair<Ms::LinkedObjects*, Ms::Location> > > m_staffLinkedElements; // one list per staff
    Ms::LinksIndexer m_linksIndexer;
};
}

#endif // MU_ENGRAVING_READCONTEXT_H
