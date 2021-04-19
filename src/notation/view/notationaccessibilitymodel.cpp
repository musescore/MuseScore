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

#include "notationaccessibilitymodel.h"

#include "notation/inotationaccessibility.h"

using namespace mu::notation;

void NotationAccessibilityModel::load()
{
    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        auto currentNotation = globalContext()->currentNotation();

        if (!currentNotation) {
            return;
        }

        mu::ValCh<std::string> accessibiltyInfoCh = currentNotation->accessibility()->accessibilityInfo();
        setAccessibilityInfo(accessibiltyInfoCh.val);

        accessibiltyInfoCh.ch.onReceive(this, [this](std::string info) {
            setAccessibilityInfo(info);
        });
    });
}

QString NotationAccessibilityModel::accessibilityInfo() const
{
    return m_accessibilityInfo;
}

void NotationAccessibilityModel::setAccessibilityInfo(const std::string& info)
{
    m_accessibilityInfo = QString::fromStdString(info);
    emit accessibilityInfoChanged(m_accessibilityInfo);
}
