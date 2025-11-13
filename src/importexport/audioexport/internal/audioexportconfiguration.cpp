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

#include "audioexportconfiguration.h"

#include "settings.h"
#include "translation.h"

using namespace muse;
using namespace mu;
using namespace mu::iex::audioexport;
using namespace muse::audio;

static const Settings::Key EXPORT_SAMPLE_RATE_KEY("iex_audioexport", "export/audio/sampleRate");
static const Settings::Key EXPORT_MP3_BITRATE("iex_audioexport", "export/audio/mp3Bitrate");
static const Settings::Key EXPORT_WAV_SAMPLE_FORMAT_KEY("iex_audioexport", "export/audio/wavSampleFormat");
static const Settings::Key EXPORT_FLAC_SAMPLE_FORMAT_KEY("iex_audioexport", "export/audio/flacSampleFormat");

void AudioExportConfiguration::init()
{
    settings()->setDefaultValue(EXPORT_SAMPLE_RATE_KEY, Val(44100));
    settings()->setDefaultValue(EXPORT_MP3_BITRATE, Val(128));
    settings()->setDefaultValue(EXPORT_WAV_SAMPLE_FORMAT_KEY, Val(static_cast<int>(AudioSampleFormat::Float32)));
    settings()->setDefaultValue(EXPORT_FLAC_SAMPLE_FORMAT_KEY, Val(static_cast<int>(AudioSampleFormat::Int16)));
}

int AudioExportConfiguration::exportMp3Bitrate() const
{
    return m_exportMp3BitrateOverride ? m_exportMp3BitrateOverride.value() : settings()->value(EXPORT_MP3_BITRATE).toInt();
}

void AudioExportConfiguration::setExportMp3Bitrate(int bitrate)
{
    settings()->setSharedValue(EXPORT_MP3_BITRATE, Val(bitrate));
}

void AudioExportConfiguration::setExportMp3BitrateOverride(std::optional<int> bitrate)
{
    m_exportMp3BitrateOverride = bitrate;
}

const std::vector<int>& AudioExportConfiguration::availableMp3BitRates() const
{
    static const std::vector<int> rates { 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, };
    return rates;
}

int AudioExportConfiguration::exportSampleRate() const
{
    return settings()->value(EXPORT_SAMPLE_RATE_KEY).toInt();
}

void AudioExportConfiguration::setExportSampleRate(int rate)
{
    settings()->setSharedValue(EXPORT_SAMPLE_RATE_KEY, Val(rate));
}

const std::vector<int>& AudioExportConfiguration::availableSampleRates() const
{
    static const std::vector<int> rates { 32000, 44100, 48000 };
    return rates;
}

samples_t AudioExportConfiguration::exportBufferSize() const
{
    return 4096;
}

AudioSampleFormat AudioExportConfiguration::exportSampleFormat() const
{
    return m_exportSampleFormat;
}

void AudioExportConfiguration::setExportSampleFormat(AudioSampleFormat format)
{
    m_exportSampleFormat = format;
}

void AudioExportConfiguration::setExportSampleFormat(const QString& extension, AudioSampleFormat format)
{
    m_exportSampleFormat = format;
    if (extension == QLatin1String("wav")) {
        settings()->setSharedValue(EXPORT_WAV_SAMPLE_FORMAT_KEY, Val(static_cast<int>(format)));
    } else if (extension == QLatin1String("flac")) {
        settings()->setSharedValue(EXPORT_FLAC_SAMPLE_FORMAT_KEY, Val(static_cast<int>(format)));
    }
}

const std::vector<AudioSampleFormat>& AudioExportConfiguration::availableSampleFormats(const QString& extension) const
{
    if (extension == QLatin1String("wav")) {
        static const std::vector<muse::audio::AudioSampleFormat> wavSampleFormats {
            AudioSampleFormat::Int16,
            AudioSampleFormat::Int24,
            AudioSampleFormat::Float32,
        };
        return wavSampleFormats;
    }
    if (extension == QLatin1String("flac")) {
        static const std::vector<muse::audio::AudioSampleFormat> flacSampleFormats {
            AudioSampleFormat::Int16,
            AudioSampleFormat::Int24,
        };
        return flacSampleFormats;
    }
    static const std::vector<muse::audio::AudioSampleFormat> emptySampleFormats {};
    return emptySampleFormats;
}

void AudioExportConfiguration::loadSampleFormatSetting(const QString& extension)
{
    if (extension == QLatin1String("wav")) {
        setExportSampleFormat(static_cast<AudioSampleFormat>(settings()->value(EXPORT_WAV_SAMPLE_FORMAT_KEY).toInt()));
    } else if (extension == QLatin1String("flac")) {
        setExportSampleFormat(static_cast<AudioSampleFormat>(settings()->value(EXPORT_FLAC_SAMPLE_FORMAT_KEY).toInt()));
    }
}

QString AudioExportConfiguration::sampleFormatToString(AudioSampleFormat format) const
{
    switch (format) {
    case AudioSampleFormat::Int16:
        return muse::qtrc("project/export", "16-bit integer");
    case AudioSampleFormat::Int24:
        return muse::qtrc("project/export", "24-bit integer");
    case AudioSampleFormat::Float32:
        return muse::qtrc("project/export", "32-bit float");
    default:
        return QString();
    }
}
