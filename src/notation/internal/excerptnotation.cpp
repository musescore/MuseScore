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

#include "excerptnotation.h"

#include "libmscore/excerpt.h"

using namespace mu::notation;

ExcerptNotation::ExcerptNotation(Ms::Excerpt* excerpt)
    : Notation(excerpt->partScore()), m_excerpt(excerpt)
{
}

ExcerptNotation::~ExcerptNotation()
{
    if (!m_excerpt) {
        return;
    }

    Ms::MasterScore* master = m_excerpt->oscore();
    if (master) {
        master->deleteExcerpt(m_excerpt);
    }

    delete m_excerpt;
    m_excerpt = nullptr;

    setScore(nullptr);
}

INotationPtr ExcerptNotation::notation()
{
    return shared_from_this();
}

Ms::Excerpt* ExcerptNotation::excerpt() const
{
    return m_excerpt;
}

void ExcerptNotation::setExcerpt(Ms::Excerpt* excerpt)
{
    m_excerpt = excerpt;
    setScore(m_excerpt->partScore());
    setMetaInfo(m_metaInfo);
}

Meta ExcerptNotation::metaInfo() const
{
    return isInited() ? Notation::metaInfo() : m_metaInfo;
}

void ExcerptNotation::setMetaInfo(const Meta& meta)
{
    m_metaInfo = meta;

    if (isInited()) {
        m_excerpt->setTitle(meta.title);
        Notation::setMetaInfo(meta);
    }
}

bool ExcerptNotation::isInited() const
{
    return m_excerpt;
}

INotationPtr ExcerptNotation::clone() const
{
    Ms::Excerpt* copy = new Ms::Excerpt(*m_excerpt);
    return std::make_shared<ExcerptNotation>(copy);
}
