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
#include "audiodeviceslistener.h"

#include "log.h"

using namespace muse;
using namespace muse::audio;

AudioDevicesListener::AudioDevicesListener()
{
    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr,
                                  CLSCTX_INPROC_SERVER,
                                  IID_PPV_ARGS(&m_deviceEnumerator));

    if (hr == CO_E_NOTINITIALIZED) {
        CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr,
                              CLSCTX_INPROC_SERVER,
                              IID_PPV_ARGS(&m_deviceEnumerator));

        if (hr == CO_E_NOTINITIALIZED) {
            LOGE() << "CoCreateInstance fails with CO_E_NOTINITIALIZED";
            return;
        }

        m_successfullyInitializedCOM = true;
    }

    if (!m_deviceEnumerator) {
        LOGE() << "Failed to create device enumeration";
        return;
    }

    hr = m_deviceEnumerator->RegisterEndpointNotificationCallback(this);
    if (FAILED(hr)) {
        LOGE() << "RegisterEndpointNotificationCallback failed";
        return;
    }
}

AudioDevicesListener::~AudioDevicesListener()
{
    if (m_deviceEnumerator) {
        m_deviceEnumerator->UnregisterEndpointNotificationCallback(this);
    }

    if (m_successfullyInitializedCOM) {
        CoUninitialize();
    }
}

async::Notification AudioDevicesListener::devicesChanged() const
{
    return m_devicesChanged;
}

async::Notification AudioDevicesListener::defaultDeviceChanged() const
{
    return m_defaultDeviceChanged;
}

ULONG AudioDevicesListener::AddRef()
{
    return 1;
}

ULONG AudioDevicesListener::Release()
{
    return 1;
}

HRESULT AudioDevicesListener::QueryInterface(const IID&, void**)
{
    return S_OK;
}

HRESULT AudioDevicesListener::OnDeviceStateChanged(LPCWSTR, DWORD)
{
    return S_OK;
}

HRESULT AudioDevicesListener::OnDeviceAdded(LPCWSTR)
{
    m_devicesChanged.notify();

    return S_OK;
}

HRESULT AudioDevicesListener::OnDeviceRemoved(LPCWSTR)
{
    m_devicesChanged.notify();

    return S_OK;
}

HRESULT AudioDevicesListener::OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR new_default_device_id)
{
    if ((role != eConsole && role != eCommunications) || (flow != eRender && flow != eCapture)) {
        return S_OK;
    }

    winrt::hstring newDefaultDeviceIdString = new_default_device_id ? new_default_device_id : winrt::hstring();

    if (m_previousDefaultDeviceId == newDefaultDeviceIdString) {
        return S_OK;
    }

    m_previousDefaultDeviceId = newDefaultDeviceIdString;

    m_defaultDeviceChanged.notify();
    m_devicesChanged.notify();

    return S_OK;
}

HRESULT AudioDevicesListener::OnPropertyValueChanged(LPCWSTR, const PROPERTYKEY)
{
    return S_OK;
}
