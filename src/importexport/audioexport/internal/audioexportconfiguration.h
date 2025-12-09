/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
    void init();

    int exportMp3Bitrate() const override;
    void setExportMp3Bitrate(int bitrate) override;
    void setExportMp3BitrateOverride(std::optional<int> bitrate) override;
    const std::vector<int>& availableMp3BitRates() const override;

    int exportSampleRate() const override;
    void setExportSampleRate(int rate) override;
    const std::vector<int>& availableSampleRates() const override;

    muse::audio::samples_t exportBufferSize() const override;

    muse::audio::AudioSampleFormat exportSampleFormat() const override;
    void setExportSampleFormat(muse::audio::AudioSampleFormat format) override;
    void setExportSampleFormat(const QString& extension, muse::audio::AudioSampleFormat format) override;
    const std::vector<muse::audio::AudioSampleFormat>& availableSampleFormats(const QString& extension) const override;
    QString sampleFormatToString(muse::audio::AudioSampleFormat format) const override;
    void loadSampleFormatSetting(const QString& extension) override;

private:
    std::optional<int> m_exportMp3BitrateOverride = std::nullopt;
    muse::audio::AudioSampleFormat m_exportSampleFormat;
};
}

#endif // MU_IMPORTEXPORT_AUDIOEXPORTCONFIGURATION_H
