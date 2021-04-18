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
#include "keynavdevsubsection.h"

using namespace mu::ui;

KeyNavDevSubSection::KeyNavDevSubSection(IKeyNavigationSubSection* subsection)
    : AbstractKeyNavDevItem(subsection), m_subsection(subsection)
{
}

QString KeyNavDevSubSection::direction() const
{
    using Direction = IKeyNavigationSubSection::Direction;
    switch (m_subsection->direction()) {
    case Direction::Horizontal: return QString("Horizontal");
    case Direction::Vertical: return QString("Vertical");
    case Direction::Both: return QString("Both");
    }
    return QString();
}

QVariantList KeyNavDevSubSection::controls() const
{
    return m_controls;
}

void KeyNavDevSubSection::setControls(const QVariantList& controls)
{
    m_controls = controls;
    emit controlsChanged();
}
