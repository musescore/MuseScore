/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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

#include "musesamplercheckupdateservice.h"

#include <QBuffer>
#include <QJsonDocument>

#include "global/types/version.h"
#include "defer.h"

using namespace mu::musesounds;
using namespace muse::network;

bool MuseSamplerCheckUpdateService::canCheckForUpdate() const
{
#ifdef QT_CONCURRENT_SUPPORTED
    return museSampler() && !museSampler()->version().isNull();
#else
    return false;
#endif
}

bool MuseSamplerCheckUpdateService::incompatibleLocalVersion() const
{
    if (!museSampler()) {
        return true;
    }

    return museSampler()->version() < museSamplerConfiguration()->minSupportedVersion();
}

muse::async::Promise<muse::RetVal<bool> > MuseSamplerCheckUpdateService::checkForUpdate()
{
    const muse::Version localVersion = museSampler() ? museSampler()->version() : muse::Version();

    return muse::async::Promise<muse::RetVal<bool> >([this, localVersion](auto resolve, auto) {
        if (localVersion.isNull()) {
            muse::RetVal<bool> result;
            result.ret = muse::Ret(int(muse::Ret::Code::UnknownError), "Unable to obtain the local MuseSampler version");
            result.val = false;
            return resolve(result);
        }

#ifdef QT_CONCURRENT_SUPPORTED
        Concurrent::run([this, localVersion, resolve]() {
            if (configuration()->museSamplerCheckForUpdateTestMode()) {
                (void)resolve(muse::RetVal<bool>::make_ok(true));
                return;
            }

            QByteArray query = configuration()->getMuseSamplerVersionQuery().toUtf8();
            QBuffer queryBuffer(&query);

            OutgoingDevice outgoingDevice(&queryBuffer);
            QBuffer receivedData;

            INetworkManagerPtr manager = networkManagerCreator()->makeNetworkManager();
            muse::Ret ret = manager->post(configuration()->checkForMuseSamplerUpdateUrl(), &outgoingDevice, &receivedData,
                                          configuration()->headers());

            muse::RetVal<bool> result = muse::RetVal<bool>::make_ok(false);
            DEFER {
                (void)resolve(result);
            };

            if (!ret) {
                result.ret = ret;
                return;
            }

            QJsonParseError err;
            const QJsonDocument doc = QJsonDocument::fromJson(receivedData.data(), &err);
            if (err.error != QJsonParseError::NoError || !doc.isObject()) {
                result.ret = muse::Ret(int(muse::Ret::Code::UnknownError), err.errorString().toStdString());
                return;
            }

            const QJsonObject rootObj = doc.object();
            const QJsonObject dataObj = rootObj.value("data").toObject();
            const QJsonObject productObj = dataObj.value("product").toObject();
            const QJsonArray assetsArray = productObj.value("assets").toArray();

            if (assetsArray.empty()) {
                result.ret = muse::Ret(int(muse::Ret::Code::UnknownError), "Invalid JSON format");
                return;
            }

            const QJsonObject firstAsset = assetsArray.first().toObject();
            const QString versionStr = firstAsset.value("version").toString();
            const muse::Version version(versionStr);

            result.val = localVersion < version;
        });

        return muse::async::Promise<muse::RetVal<bool> >::dummy_result();
#else
        UNUSED(resolve);
        UNUSED(this);

        muse::RetVal<bool> result;
        result.ret = muse::Ret(int(muse::Ret::Code::NotSupported), "NotSupported");
        result.val = false;

        return resolve(result);
#endif
    }, muse::async::PromiseType::AsyncByBody);
}
