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
#ifndef MU_IMPORTEXPORT_AUDIOEXPORTCONFIGURATION_H
#define MU_IMPORTEXPORT_AUDIOEXPORTCONFIGURATION_H

#include "iaudioexportconfiguration.h"

namespace mu::iex::audioexport {
class AudioExportConfiguration : public IAudioExportConfiguration
{
public:
    int exportMp3Bitrate() override;
    void setExportMp3Bitrate(std::optional<int> bitrate) override;

private:
    std::optional<int> m_exportMp3Bitrate = std::nullopt;
};
}

#endif // MU_IMPORTEXPORT_AUDIOEXPORTCONFIGURATION_H
