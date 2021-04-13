//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "keynavdevsection.h"

using namespace mu::ui;

KeyNavDevSection::KeyNavDevSection(IKeyNavigationSection* section)
    : AbstractKeyNavDevItem(section), m_section(section)
{
}

QVariantList KeyNavDevSection::subsections() const
{
    return m_subsections;
}

void KeyNavDevSection::setSubsections(const QVariantList& subsections)
{
    m_subsections = subsections;
    emit subsectionsChanged();
}
