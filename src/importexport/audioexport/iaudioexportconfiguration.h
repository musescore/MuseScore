/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#ifndef MU_IMPORTEXPORT_IAUDIOEXPORTCONFIGURATION_H
#define MU_IMPORTEXPORT_IAUDIOEXPORTCONFIGURATION_H

#include <optional>

#include "modularity/imoduleinterface.h"

#include "audio/common/audiotypes.h"

namespace mu::iex::audioexport {
class IAudioExportConfiguration : MODULE_GLOBAL_INTERFACE
{
    INTERFACE_ID(IAudioExportConfiguration)

public:
    virtual ~IAudioExportConfiguration() = default;

    virtual int exportMp3Bitrate() const = 0;
    virtual void setExportMp3Bitrate(int bitrate) = 0;
    virtual void setExportMp3BitrateOverride(std::optional<int> bitrate) = 0;
    virtual const std::vector<int>& availableMp3BitRates() const = 0;

    virtual int exportSampleRate() const = 0;
    virtual void setExportSampleRate(int rate) = 0;
    virtual const std::vector<int>& availableSampleRates() const = 0;

    virtual muse::audio::samples_t exportBufferSize() const = 0;

    //! NOTE When exporting several parts, render them all at the same time
    //! (one file per part, on several threads) instead of one after another
    virtual bool multiStemRender() const = 0;
    //! NOTE See multiStemRender()
    virtual void setMultiStemRender(bool enabled) = 0;

    //! NOTE Multi-stem render: don't process an instrument until shortly before its first note
    virtual bool idleUntilFirstNote() const = 0;
    //! NOTE See idleUntilFirstNote()
    virtual void setIdleUntilFirstNote(bool enabled) = 0;

    virtual muse::audio::AudioSampleFormat exportWavSampleFormat() const = 0;
    virtual void setExportWavSampleFormat(muse::audio::AudioSampleFormat format) = 0;

    virtual muse::audio::AudioSampleFormat exportFlacSampleFormat() const = 0;
    virtual void setExportFlacSampleFormat(muse::audio::AudioSampleFormat format) = 0;

    virtual const std::vector<muse::audio::AudioSampleFormat>& availableWavSampleFormats() const = 0;
    virtual const std::vector<muse::audio::AudioSampleFormat>& availableFlacSampleFormats() const = 0;
    virtual QString sampleFormatToString(muse::audio::AudioSampleFormat format) const = 0;
};
}

#endif // MU_IMPORTEXPORT_IAUDIOEXPORTCONFIGURATION_H
