/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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
#include "mnxconfiguration.h"

#include <algorithm>

#include "settings.h"

using namespace mu;
using namespace mu::iex::mnxio;
using namespace muse;

static const std::string module_name("iex_mnx");

static const Settings::Key MNX_EXPORT_INDENT_KEY(module_name, "export/mnx/indentSpaces");
static const Settings::Key MNX_EXPORT_BEAMS_KEY(module_name, "export/mnx/exportBeams");
static const Settings::Key MNX_EXPORT_REST_POSITIONS_KEY(module_name, "export/mnx/exportRestPositions");

void MnxConfiguration::init()
{
    settings()->setDefaultValue(MNX_EXPORT_INDENT_KEY, Val(4));
    settings()->setDefaultValue(MNX_EXPORT_BEAMS_KEY, Val(true));
    settings()->setDefaultValue(MNX_EXPORT_REST_POSITIONS_KEY, Val(false));
}

int MnxConfiguration::mnxIndentSpaces() const
{
    const int value = settings()->value(MNX_EXPORT_INDENT_KEY).toInt();
    return std::clamp(value, -1, 8);
}

void MnxConfiguration::setMnxIndentSpaces(int value)
{
    const int clamped = std::clamp(value, -1, 8);
    settings()->setSharedValue(MNX_EXPORT_INDENT_KEY, Val(clamped));
}

bool MnxConfiguration::mnxExportBeams() const
{
    return settings()->value(MNX_EXPORT_BEAMS_KEY).toBool();
}

void MnxConfiguration::setMnxExportBeams(bool value)
{
    settings()->setSharedValue(MNX_EXPORT_BEAMS_KEY, Val(value));
}

bool MnxConfiguration::mnxExportRestPositions() const
{
    return settings()->value(MNX_EXPORT_REST_POSITIONS_KEY).toBool();
}

void MnxConfiguration::setMnxExportRestPositions(bool value)
{
    settings()->setSharedValue(MNX_EXPORT_REST_POSITIONS_KEY, Val(value));
}
