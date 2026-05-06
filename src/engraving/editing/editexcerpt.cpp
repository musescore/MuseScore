/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include "editexcerpt.h"

using namespace mu::engraving;

//---------------------------------------------------------
//   AddExcerpt
//---------------------------------------------------------

AddExcerpt::AddExcerpt(Excerpt* ex, bool initIfNeeded)
    : excerpt(ex), m_initIfNeeded(initIfNeeded)
{}

AddExcerpt::~AddExcerpt()
{
    if (deleteExcerpt) {
        delete excerpt;
        excerpt = nullptr;
    }
}

void AddExcerpt::undo()
{
    deleteExcerpt = true;
    excerpt->masterScore()->removeExcerpt(excerpt);
}

void AddExcerpt::redo()
{
    deleteExcerpt = false;
    excerpt->masterScore()->addExcerpt(excerpt, muse::nidx, m_initIfNeeded);
}

std::vector<EngravingObject*> AddExcerpt::objectItems() const
{
    if (excerpt) {
        if (MasterScore* score = excerpt->masterScore()) {
            return { score };
        }
    }

    return {};
}

//---------------------------------------------------------
//   RemoveExcerpt
//---------------------------------------------------------

RemoveExcerpt::RemoveExcerpt(Excerpt* ex)
    : excerpt(ex), m_initIfNeeded(ex->inited())
{
    index = muse::indexOf(excerpt->masterScore()->excerpts(), excerpt);
}

RemoveExcerpt::~RemoveExcerpt()
{
    if (deleteExcerpt) {
        delete excerpt;
        excerpt = nullptr;
    }
}

void RemoveExcerpt::undo()
{
    deleteExcerpt = false;
    excerpt->masterScore()->addExcerpt(excerpt, index, m_initIfNeeded);
}

void RemoveExcerpt::redo()
{
    deleteExcerpt = true;
    excerpt->masterScore()->removeExcerpt(excerpt);
}

std::vector<EngravingObject*> RemoveExcerpt::objectItems() const
{
    if (excerpt) {
        if (MasterScore* score = excerpt->masterScore()) {
            return { score };
        }
    }

    return {};
}

//---------------------------------------------------------
//   SwapExcerpt
//---------------------------------------------------------

void SwapExcerpt::flip()
{
    Excerpt* tmp = score->excerpts().at(pos1);
    score->excerpts()[pos1] = score->excerpts().at(pos2);
    score->excerpts()[pos2] = tmp;
    score->setExcerptsChanged(true);
}

//---------------------------------------------------------
//   ChangeExcerptTitle
//---------------------------------------------------------

void ChangeExcerptTitle::flip()
{
    String s = title;
    title = excerpt->name();
    excerpt->setName(s);
    excerpt->masterScore()->setExcerptsChanged(true);
}

//---------------------------------------------------------
//   AddPartToExcerpt
//---------------------------------------------------------

AddPartToExcerpt::AddPartToExcerpt(Excerpt* e, Part* p, size_t targetPartIdx)
    : m_excerpt(e), m_part(p), m_targetPartIdx(targetPartIdx)
{
    assert(m_excerpt);
    assert(m_part);
}

void AddPartToExcerpt::undo()
{
    muse::remove(m_excerpt->parts(), m_part);

    if (Score* score = m_excerpt->excerptScore()) {
        score->removePart(m_part);
    }
}

void AddPartToExcerpt::redo()
{
    std::vector<Part*>& excerptParts = m_excerpt->parts();
    if (m_targetPartIdx < excerptParts.size()) {
        excerptParts.insert(excerptParts.begin() + m_targetPartIdx, m_part);
    } else {
        excerptParts.push_back(m_part);
    }

    if (Score* score = m_excerpt->excerptScore()) {
        score->insertPart(m_part, m_targetPartIdx);
    }
}

void AddPartToExcerpt::cleanup(bool wasDone)
{
    if (!wasDone) {
        delete m_part;
        m_part = nullptr;
    }
}
