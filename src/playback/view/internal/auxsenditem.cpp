/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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

#include "auxsenditem.h"

#include "realfn.h"

using namespace mu::playback;

AuxSendItem::AuxSendItem(QObject* parent)
    : QObject(parent)
{
}

QString AuxSendItem::title() const
{
    return m_title;
}

bool AuxSendItem::isActive() const
{
    return m_isActive;
}

int AuxSendItem::audioSignalPercentage() const
{
    return m_audioSignalPercentage;
}

void AuxSendItem::setTitle(const QString& title)
{
    if (m_title == title) {
        return;
    }

    m_title = title;
    emit titleChanged(title);
}

void AuxSendItem::setIsActive(bool active)
{
    if (m_isActive == active) {
        return;
    }

    m_isActive = active;
    emit isActiveChanged(active);
}

void AuxSendItem::setAudioSignalPercentage(int percentage)
{
    if (m_audioSignalPercentage == percentage) {
        return;
    }

    m_audioSignalPercentage = percentage;
    emit audioSignalPercentageChanged(percentage);
}
