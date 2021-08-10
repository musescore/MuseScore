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
    : Notation(), m_excerpt(excerpt)
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

bool ExcerptNotation::isInited() const
{
    return m_isInited;
}

void ExcerptNotation::init()
{
    if (m_isInited) {
        return;
    }

    if (!m_title.isEmpty()) {
        m_excerpt->setTitle(m_title);
    }

    setScore(m_excerpt->partScore());
    m_isInited = true;
}

Ms::Excerpt* ExcerptNotation::excerpt() const
{
    return m_excerpt;
}

QString ExcerptNotation::title() const
{
    return m_excerpt ? m_excerpt->title() : m_title;
}

void ExcerptNotation::setTitle(const QString& title)
{
    if (m_excerpt) {
        m_excerpt->setTitle(title);
    } else {
        m_title = title;
    }
}

INotationPtr ExcerptNotation::notation()
{
    return shared_from_this();
}

IExcerptNotationPtr ExcerptNotation::clone() const
{
    if (!m_excerpt) {
        return nullptr;
    }

    Ms::Excerpt* copy = new Ms::Excerpt(*m_excerpt);
    return std::make_shared<ExcerptNotation>(copy);
}
