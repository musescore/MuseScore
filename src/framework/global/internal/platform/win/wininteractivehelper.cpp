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
#include <winrt/Windows.ApplicationModel.h>
#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.Management.Deployment.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>

#include <QDesktopServices>

using namespace muse;
using namespace muse::async;

async::Promise<Ret> WinInteractiveHelper::openApp(const std::string& appIdentifier)
{
    return Promise<Ret>([&appIdentifier](auto resolve, auto reject) {
        using namespace winrt::Windows;
        using namespace winrt::Windows::System;

        std::wstring wsAppIdentifier = std::wstring(appIdentifier.begin(), appIdentifier.end());

        Foundation::Uri uri(wsAppIdentifier.c_str());
        if (!uri.SchemeName().empty()) {
            auto asyncOperation = Launcher::LaunchUriAsync(uri);
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
        }

        using namespace winrt::Windows::Management::Deployment;
        using namespace winrt::Windows::ApplicationModel;
        using namespace winrt::Windows::Foundation::Collections;

        PackageManager packageManager;
        IIterable<Package> foundPackages = packageManager.FindPackagesForUser(L"", wsAppIdentifier.c_str());
        IIterator<Package> packagIt = foundPackages.First();
        if (!packagIt.HasCurrent()) {
            Ret ret = make_ret(Ret::Code::InternalError);
            return reject(ret.code(), ret.text());
        }

        IIterable<Core::AppListEntry> entries = packagIt.Current().GetAppListEntries();
        IIterator<Core::AppListEntry> entryIt = entries.First();
        if (!entryIt.HasCurrent()) {
            Ret ret = make_ret(Ret::Code::InternalError);
            return reject(ret.code(), ret.text());
        }

        entries.First().Current().LaunchAsync();

        return resolve(make_ok());
    });
}
