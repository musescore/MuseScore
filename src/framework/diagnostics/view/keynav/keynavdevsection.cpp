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
#include "keynavdevsection.h"

using namespace muse::diagnostics;
using namespace muse::ui;

KeyNavDevSection::KeyNavDevSection(INavigationSection* section)
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
    emit panelsCountChanged();
    emit controlsCountChanged();
}

int KeyNavDevSection::panelsCount() const
{
    return int(m_section->panels().size());
}

int KeyNavDevSection::controlsCount() const
{
    size_t count = 0;
    for (const INavigationPanel* p : m_section->panels()) {
        count += p->controls().size();
    }
    return int(count);
}
