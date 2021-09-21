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

#include "readcontext.h"

#include "libmscore/score.h"
#include "libmscore/undo.h"

using namespace mu::engraving;

ReadContext::ReadContext(Ms::Score* score)
    : m_score(score)
{
}

void ReadContext::setIgnoreVersionError(bool arg)
{
    m_ignoreVersionError = arg;
}

bool ReadContext::ignoreVersionError() const
{
    return m_ignoreVersionError;
}

QString ReadContext::mscoreVersion() const
{
    return m_score->mscoreVersion();
}

int ReadContext::mscVersion() const
{
    return m_score->mscVersion();
}

int ReadContext::fileDivision() const
{
    return m_score->fileDivision();
}

int ReadContext::fileDivision(int t) const
{
    return m_score->fileDivision(t);
}

qreal ReadContext::spatium() const
{
    return m_score->spatium();
}

mu::engraving::compat::DummyElement* ReadContext::dummy() const
{
    return m_score->dummy();
}

Ms::TimeSigMap* ReadContext::sigmap()
{
    return m_score->sigmap();
}

Ms::Staff* ReadContext::staff(int n)
{
    return m_score->staff(n);
}

void ReadContext::appendStaff(Ms::Staff* staff)
{
    m_score->appendStaff(staff);
}

void ReadContext::addSpanner(Ms::Spanner* s)
{
    m_score->addSpanner(s);
}

bool ReadContext::undoStackActive() const
{
    return m_score->undoStack()->active();
}

bool ReadContext::isSameScore(const Ms::EngravingObject* obj) const
{
    return obj->score() == m_score;
}
