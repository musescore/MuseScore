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
#ifndef MUSE_AUDIO_AUDIODEVICESLISTENER_H
#define MUSE_AUDIO_AUDIODEVICESLISTENER_H

#include "wasapitypes.h"

#include "global/async/notification.h"

namespace muse::audio {
class AudioDevicesListener : public IMMNotificationClient
{
public:
    explicit AudioDevicesListener();
    ~AudioDevicesListener();

    async::Notification devicesChanged() const;
    async::Notification defaultDeviceChanged() const;

private:
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    STDMETHOD(QueryInterface)(REFIID iid, void** object);
    STDMETHOD(OnDeviceStateChanged)(LPCWSTR device_id, DWORD new_state);
    STDMETHOD(OnDeviceAdded)(LPCWSTR device_id);
    STDMETHOD(OnDeviceRemoved)(LPCWSTR device_id);
    STDMETHOD(OnDefaultDeviceChanged)(EDataFlow flow, ERole role, LPCWSTR new_default_device_id);
    STDMETHOD(OnPropertyValueChanged)(LPCWSTR device_id, const PROPERTYKEY key);

    winrt::com_ptr<IMMDeviceEnumerator> m_deviceEnumerator;
    winrt::hstring m_previousDefaultDeviceId;

    async::Notification m_devicesChanged;
    async::Notification m_defaultDeviceChanged;

    bool m_successfullyInitializedCOM = false;
};
}

#endif // MUSE_AUDIO_AudioDevicesListener_H
