/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "audiomidipreferencesmodel.h"

#include "translation.h"
#include "log.h"

using namespace mu::preferences;
using namespace muse::audio;
using namespace muse::midi;

AudioMidiPreferencesModel::AudioMidiPreferencesModel(QObject* parent)
    : QObject(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
}

int AudioMidiPreferencesModel::currentAudioApiIndex() const
{
    QString currentApi = QString::fromStdString(audioDriverController()->currentAudioApi());
    return audioApiList().indexOf(currentApi);
}

void AudioMidiPreferencesModel::setCurrentAudioApiIndex(int index)
{
    if (index == currentAudioApiIndex()) {
        return;
    }

    std::vector<std::string> apiList = audioDriverController()->availableAudioApiList();
    if (index < 0 || index >= static_cast<int>(apiList.size())) {
        return;
    }

    std::string fallbackApi = audioDriverController()->currentAudioApi();

    audioDriverController()->availableOutputDevicesChanged().onNotify(this, [this, fallbackApi]() {
        audioDriverController()->availableOutputDevicesChanged().disconnect(this);

        if (!audioDriverController()->availableOutputDevices().empty()) {
            return;
        }

        auto promise = interactive()->warning(
            muse::trc("preferences", "No audio devices available"),
            muse::qtrc("preferences", "The selected audio driver does not have any available audio devices. "
                                      "MuseScore Studio will use the default audio driver instead. "
                                      "To use %1, please check your hardware settings and try again.")
            .arg(QString::fromStdString(audioDriverController()->currentAudioApi())).toStdString());

        promise.onResolve(this, [this, fallbackApi](const muse::IInteractive::Result&) {
            audioDriverController()->changeCurrentAudioApi(fallbackApi);
            emit currentAudioApiIndexChanged(currentAudioApiIndex());
        });
    }, Asyncable::Mode::SetReplace);

    audioDriverController()->changeCurrentAudioApi(apiList.at(index));
    emit currentAudioApiIndexChanged(index);
}

QString AudioMidiPreferencesModel::midiInputDeviceId() const
{
    return QString::fromStdString(midiInPort()->deviceID());
}

void AudioMidiPreferencesModel::inputDeviceSelected(const QString& deviceId)
{
    midiConfiguration()->setMidiInputDeviceId(deviceId.toStdString());
}

QString AudioMidiPreferencesModel::midiOutputDeviceId() const
{
    return QString::fromStdString(midiOutPort()->deviceID());
}

void AudioMidiPreferencesModel::outputDeviceSelected(const QString& deviceId)
{
    midiConfiguration()->setMidiOutputDeviceId(deviceId.toStdString());
}

void AudioMidiPreferencesModel::init()
{
    midiInPort()->availableDevicesChanged().onNotify(this, [this]() {
        emit midiInputDevicesChanged();
    });

    midiInPort()->deviceChanged().onNotify(this, [this]() {
        emit midiInputDeviceIdChanged();
    });

    midiOutPort()->availableDevicesChanged().onNotify(this, [this]() {
        emit midiOutputDevicesChanged();
    });

    midiOutPort()->deviceChanged().onNotify(this, [this]() {
        emit midiOutputDeviceIdChanged();
    });

    midiConfiguration()->useMIDI20OutputChanged().onReceive(this, [this](bool) {
        emit useMIDI20OutputChanged();
    });

    playbackConfiguration()->muteHiddenInstrumentsChanged().onReceive(this, [this](bool mute) {
        emit muteHiddenInstrumentsChanged(mute);
    });

    playbackConfiguration()->shouldShowOnlineSoundsProcessingErrorChanged().onNotify(this, [this]() {
        emit shouldShowOnlineSoundsProcessingErrorChanged();
    });

    playbackConfiguration()->onlineSoundsShowProgressBarModeChanged().onNotify(this, [this]() {
        emit onlineSoundsShowProgressBarModeChanged();
    });

    audioDriverController()->currentAudioApiChanged().onNotify(this, [this]() {
        emit currentAudioApiIndexChanged(currentAudioApiIndex());
    });

    audioConfiguration()->autoProcessOnlineSoundsInBackgroundChanged().onReceive(this, [this](bool) {
        emit autoProcessOnlineSoundsInBackgroundChanged();
    });
}

QStringList AudioMidiPreferencesModel::audioApiList() const
{
    const std::vector<std::string> apiList = audioDriverController()->availableAudioApiList();

    QStringList result;
    result.reserve(apiList.size());

    for (const std::string& api: apiList) {
        result.emplace_back(QString::fromStdString(api));
    }

    return result;
}

void AudioMidiPreferencesModel::restartAudioAndMidiDevices()
{
    NOT_IMPLEMENTED;
}

bool AudioMidiPreferencesModel::onlineSoundsSectionVisible() const
{
#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
    return true;
#else
    return false;
#endif
}

QVariantList AudioMidiPreferencesModel::midiInputDevices() const
{
    std::vector<MidiDevice> devices = midiInPort()->availableDevices();

    QVariantList result;
    result.reserve(devices.size());

    for (const MidiDevice& device : devices) {
        QVariantMap obj;
        obj["value"] = QString::fromStdString(device.id);
        obj["text"] = QString::fromStdString(device.name);

        result << obj;
    }

    return result;
}

QVariantList AudioMidiPreferencesModel::midiOutputDevices() const
{
    std::vector<MidiDevice> devices = midiOutPort()->availableDevices();

    QVariantList result;
    result.reserve(devices.size());

    for (const MidiDevice& device : devices) {
        QVariantMap obj;
        obj["value"] = QString::fromStdString(device.id);
        obj["text"] = QString::fromStdString(device.name);

        result << obj;
    }

    return result;
}

bool AudioMidiPreferencesModel::isMIDI20OutputSupported() const
{
    return midiOutPort()->supportsMIDI20Output();
}

bool AudioMidiPreferencesModel::useMIDI20Output() const
{
    return midiConfiguration()->useMIDI20Output();
}

void AudioMidiPreferencesModel::setUseMIDI20Output(bool use)
{
    if (use == useMIDI20Output()) {
        return;
    }

    midiConfiguration()->setUseMIDI20Output(use);
}

void AudioMidiPreferencesModel::showMidiError(const MidiDeviceID& deviceId, const std::string& text) const
{
    // todo: display error
    LOGE() << "failed connect to device, deviceID: " << deviceId << ", err: " << text;
}

bool AudioMidiPreferencesModel::muteHiddenInstruments() const
{
    return playbackConfiguration()->muteHiddenInstruments();
}

void AudioMidiPreferencesModel::setMuteHiddenInstruments(bool mute)
{
    if (mute == muteHiddenInstruments()) {
        return;
    }

    playbackConfiguration()->setMuteHiddenInstruments(mute);
}

bool AudioMidiPreferencesModel::shouldShowOnlineSoundsProcessingError() const
{
    return playbackConfiguration()->shouldShowOnlineSoundsProcessingError();
}

void AudioMidiPreferencesModel::setShouldShowOnlineSoundsProcessingError(bool value)
{
    if (value == shouldShowOnlineSoundsProcessingError()) {
        return;
    }

    playbackConfiguration()->setShouldShowOnlineSoundsProcessingError(value);
}

bool AudioMidiPreferencesModel::autoProcessOnlineSoundsInBackground() const
{
    return audioConfiguration()->autoProcessOnlineSoundsInBackground();
}

void AudioMidiPreferencesModel::setAutoProcessOnlineSoundsInBackground(bool value)
{
    if (value == autoProcessOnlineSoundsInBackground()) {
        return;
    }

    audioConfiguration()->setAutoProcessOnlineSoundsInBackground(value);

    if (!value) {
        if (playbackConfiguration()->onlineSoundsShowProgressBarMode() == playback::OnlineSoundsShowProgressBarMode::DuringPlayback) {
            playbackConfiguration()->setOnlineSoundsShowProgressBarMode(playback::OnlineSoundsShowProgressBarMode::Always);
        }
    }
}

int AudioMidiPreferencesModel::onlineSoundsShowProgressBarMode() const
{
    return static_cast<int>(playbackConfiguration()->onlineSoundsShowProgressBarMode());
}

void AudioMidiPreferencesModel::setOnlineSoundsShowProgressBarMode(int mode)
{
    if (mode == onlineSoundsShowProgressBarMode()) {
        return;
    }

    playbackConfiguration()->setOnlineSoundsShowProgressBarMode(static_cast<playback::OnlineSoundsShowProgressBarMode>(mode));
}
