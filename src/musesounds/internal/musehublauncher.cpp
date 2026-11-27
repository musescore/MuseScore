/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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
#include "musehublauncher.h"

#include "global/log.h"

using namespace mu::musesounds;
using namespace muse;

#if !defined(Q_OS_WIN) && !defined(Q_OS_MACOS)
static std::vector<std::string> museSoundsManagerArgs(MuseHubPage page)
{
    switch (page) {
    case MuseHubPage::MuseSamplerUpdate:
        return { "--show-sampler-update", "--show-hidden-statuses" };
    case MuseHubPage::SoundLibrary:
    case MuseHubPage::MuseSounds:
    case MuseHubPage::FreeMuseSounds:
    case MuseHubPage::MuseSoundsCore:
    case MuseHubPage::MuseSoundsPro:
        break;
    case MuseHubPage::Plugins: // not reached: MuseSounds Manager doesn't provide plugins
        break;
    }

    return {}; // just open the main window
}

#endif

async::Promise<Ret> MuseHubLauncher::open(MuseHubPage page, const std::string& utmContent)
{
    const MuseHubUrls urls = configuration()->museHubUrls(page, utmContent);

#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
    return tryOpenMuseHub(urls);
#else
    if (page == MuseHubPage::Plugins) { // MuseSounds Manager doesn't provide plugins
        return async::make_promise<Ret>([this, webUrl = urls.webUrl](auto resolve, auto) {
            return resolve(interactive()->openUrl(webUrl));
        });
    }

    return tryOpenMuseSoundsManager(museSoundsManagerArgs(page), urls.webUrl);
#endif
}

async::Promise<Ret> MuseHubLauncher::openSoundLibrary(const SoundLibraryInfo& soundLibrary, const std::string& utmContent)
{
    const MuseHubUrls urls = configuration()->museHubUrls(MuseHubPage::SoundLibrary, utmContent, soundLibrary);

#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
    return tryOpenMuseHub(urls);
#else
    return tryOpenMuseSoundsManager(museSoundsManagerArgs(MuseHubPage::SoundLibrary), urls.webUrl);
#endif
}

async::Promise<Ret> MuseHubLauncher::tryOpenMuseHub(const MuseHubUrls& urls)
{
    LOGI() << "Opening MuseHub: " << urls.museHubUri;

    return async::make_promise<Ret>([this, urls](auto resolve, auto) {
        const UriQuery museHubUri(urls.museHubUri);
        async::Promise<Ret> canOpenPromise = interactive()->canOpenApp(museHubUri);

        canOpenPromise.onResolve(this, [this, museHubUri, webUrl = urls.webUrl, resolve](const Ret& ret) {
            if (!ret) {
                LOGW() << "MuseHub can't be opened, opening web URL instead: " << ret.toString();
                openWebUrl(webUrl, resolve);
                return;
            }

            openMuseHubApp(museHubUri, webUrl, resolve);
        });

        canOpenPromise.onReject(this, [this, webUrl = urls.webUrl, resolve](int, const std::string& err) {
            LOGW() << "MuseHub can't be opened, opening web URL instead: " << err;
            openWebUrl(webUrl, resolve);
        });

        return async::Promise<Ret>::dummy_result();
    });
}

async::Promise<Ret> MuseHubLauncher::tryOpenMuseSoundsManager(const std::vector<std::string>& args, const std::string& webUrl)
{
    LOGI() << "Opening MuseSounds Manager";

    return async::make_promise<Ret>([this, args, webUrl](auto resolve, auto) {
        if (process()->startDetached("muse-sounds-manager", args)) {
            return resolve(make_ok());
        }

        LOGW() << "Failed to open MuseSounds Manager, opening web URL instead";
        return resolve(interactive()->openUrl(webUrl));
    });
}

void MuseHubLauncher::openMuseHubApp(const UriQuery& museHubUri, const std::string& webUrl, const Resolve& resolve)
{
    async::Promise<Ret> promise = interactive()->openApp(museHubUri);

    promise.onResolve(this, [this, webUrl, resolve](const Ret& ret) {
        if (ret) {
            (void)resolve(ret);
            return;
        }

        LOGW() << "Failed to open MuseHub, opening web URL instead: " << ret.toString();
        openWebUrl(webUrl, resolve);
    });

    promise.onReject(this, [this, webUrl, resolve](int, const std::string& err) {
        LOGW() << "Failed to open MuseHub, opening web URL instead: " << err;
        openWebUrl(webUrl, resolve);
    });
}

void MuseHubLauncher::openWebUrl(const std::string& webUrl, const Resolve& resolve)
{
    (void)resolve(interactive()->openUrl(webUrl));
}
