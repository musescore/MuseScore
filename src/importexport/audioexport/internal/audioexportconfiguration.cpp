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

#include "audioexportconfiguration.h"

using namespace mu::iex::audioexport;

static constexpr int DEFAULT_BITRATE = 128;

int AudioExportConfiguration::exportMp3Bitrate()
{
    return m_exportMp3Bitrate ? m_exportMp3Bitrate.value() : DEFAULT_BITRATE;
}

void AudioExportConfiguration::setExportMp3Bitrate(std::optional<int> bitrate)
{
    m_exportMp3Bitrate = bitrate;
}
