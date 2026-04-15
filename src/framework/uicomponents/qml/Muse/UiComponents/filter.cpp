/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include "filter.h"

namespace muse::uicomponents {
Filter::Filter(QObject* parent)
    : QObject(parent)
{
}

bool Filter::enabled() const
{
    return m_enabled;
}

void Filter::setEnabled(const bool enabled)
{
    if (m_enabled == enabled) {
        return;
    }

    m_enabled = enabled;
    emit dataChanged();
}

bool Filter::async() const
{
    return m_async;
}

void Filter::setAsync(const bool async)
{
    if (m_async == async) {
        return;
    }

    m_async = async;
    emit dataChanged();
}
}
