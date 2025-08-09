/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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
#pragma once

#include <windows.h>
#include <hstring.h>
#include <winrt/Windows.Devices.Enumeration.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Media.Devices.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.System.Threading.h>
#include <forward_list>
#include <mmreg.h>
#include <mmdeviceapi.h>
#include <mfapi.h>
#include <AudioClient.h>
#include <functional>

namespace winrt {
enum class DeviceState : int32_t
{
    Uninitialized = 0,
    Error = 1,
    Discontinuity = 2,
    Flushing = 3,
    Activated = 4,
    Initialized = 5,
    Starting = 6,
    Playing = 7,
    Capturing = 8,
    Pausing = 9,
    Paused = 10,
    Stopping = 11,
    Stopped = 12,
};

template<typename T>
struct unique_cotaskmem_ptr
{
    unique_cotaskmem_ptr(T* p = nullptr)
        : m_p(p) {}
    ~unique_cotaskmem_ptr() { CoTaskMemFree(m_p); }

    unique_cotaskmem_ptr(unique_cotaskmem_ptr const&) = delete;
    unique_cotaskmem_ptr(unique_cotaskmem_ptr&& other)
        : m_p(std::exchange(other.m_p, nullptr)) {}

    unique_cotaskmem_ptr& operator=(const unique_cotaskmem_ptr& other) = delete;
    unique_cotaskmem_ptr& operator=(unique_cotaskmem_ptr&& other)
    {
        CoTaskMemFree(std::exchange(m_p, std::exchange(other.m_p, nullptr)));
        return *this;
    }

    operator bool() const {
        return static_cast<bool>(m_p);
    }

    T* operator->() { return m_p; }
    T* get() { return m_p; }
    T** put() { return &m_p; }

    T* m_p;
};

template<auto Callback>
struct EmbeddedMFAsyncCallback : ::IMFAsyncCallback
{
    template<typename Parent> static Parent* parent_finder(HRESULT (Parent::*)(IMFAsyncResult*)) { return nullptr; }
    using ParentPtr = decltype(parent_finder(Callback));

    ParentPtr m_parent;
    DWORD m_queueId = 0;

    EmbeddedMFAsyncCallback(ParentPtr parent)
        : m_parent(parent) {}

    STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject) final
    {
        if (is_guid_of<::IMFAsyncCallback, ::IUnknown>(riid)) {
            (*ppvObject) = this;
            AddRef();
            return S_OK;
        }
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }

    STDMETHOD_(ULONG, AddRef)() final {
        return m_parent->AddRef();
    }
    STDMETHOD_(ULONG, Release)() final {
        return m_parent->Release();
    }

    STDMETHOD(GetParameters)(DWORD * flags, DWORD* queueId) final
    {
        *flags = 0;
        *queueId = m_queueId;
        return S_OK;
    }

    STDMETHOD(Invoke)(IMFAsyncResult * result) final
    {
        return (m_parent->*Callback)(result);
    }

    void SetQueueID(DWORD queueId) { m_queueId = queueId; }
};

typedef std::function<void (void* userdata, uint8_t* stream, int len)> SampleRequestCallback;
}
