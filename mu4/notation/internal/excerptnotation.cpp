//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FIT-0NESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "excerptnotation.h"

#include "libmscore/excerpt.h"

using namespace mu::notation;

ExcerptNotation::ExcerptNotation(Ms::Excerpt* excerpt)
    : Notation(excerpt->partScore()), m_excerpt(excerpt)
{
}

ExcerptNotation::~ExcerptNotation()
{
    Ms::MasterScore* master = m_excerpt->oscore();
    if (master) {
        master->removeExcerpt(m_excerpt);
    }

    delete m_excerpt;
    m_excerpt = nullptr;

    setScore(nullptr);
}

void ExcerptNotation::setMetaInfo(const Meta& meta)
{
    m_excerpt->setTitle(meta.title);
    Notation::setMetaInfo(meta);
}

Ms::Excerpt* ExcerptNotation::excerpt() const
{
    return m_excerpt;
}
