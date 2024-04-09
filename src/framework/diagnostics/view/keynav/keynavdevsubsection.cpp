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
#include "keynavdevsubsection.h"

using namespace muse::diagnostics;
using namespace muse::ui;

KeyNavDevSubSection::KeyNavDevSubSection(INavigationPanel* subsection)
    : AbstractKeyNavDevItem(subsection), m_subsection(subsection)
{
}

QString KeyNavDevSubSection::direction() const
{
    using Direction = INavigationPanel::Direction;
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
    emit controlsCountChanged();
}

int KeyNavDevSubSection::controlsCount() const
{
    return m_controls.count();
}
