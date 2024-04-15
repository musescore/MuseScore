/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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
#include "wininteractivehelper.h"

#include <winrt/Windows.System.h>
#include <winrt/Windows.Foundation.h>

using namespace muse;
using namespace muse::async;

async::Promise<Ret> WinInteractiveHelper::openApp(const Uri& uri)
{
    return Promise<Ret>([&uri](auto resolve, auto reject) {
        using namespace winrt::Windows;
        using namespace winrt::Windows::System;

        std::string sUri = uri.toString();
        std::wstring wsUri = std::wstring(sUri.begin(), sUri.end());

        Foundation::Uri wUri(wsUri.c_str());

        auto asyncOperation = Launcher::LaunchUriAsync(wUri);
        asyncOperation.Completed([=](Foundation::IAsyncOperation<bool> const&,
                                     Foundation::AsyncStatus const asyncStatus) {
            if (asyncStatus == Foundation::AsyncStatus::Error) {
                Ret ret = make_ret(Ret::Code::InternalError);
                (void)reject(ret.code(), ret.text());
            } else {
                (void)resolve(make_ok());
            }
        });

        return Promise<Ret>::Result::unchecked();
    });
}
