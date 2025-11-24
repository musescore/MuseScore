/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include "global/serialization/json.h"
#include "global/types/version.h"

using namespace muse;
using namespace mu::musesounds;
using namespace muse::network;
using namespace muse::async;

static RetVal<Version> parseVersion(const QByteArray& json)
{
    std::string err;
    const ByteArray ba = ByteArray::fromQByteArrayNoCopy(json);
    const JsonDocument doc = JsonDocument::fromJson(ba, &err);
    if (!err.empty() || !doc.isObject()) {
        return RetVal<Version>::make_ret(int(Ret::Code::UnknownError), err);
    }

    const JsonObject rootObj = doc.rootObject();
    const JsonObject dataObj = rootObj.value("data").toObject();
    const JsonObject productObj = dataObj.value("product").toObject();
    const JsonArray assetsArray = productObj.value("assets").toArray();

    if (assetsArray.empty()) {
        return RetVal<Version>::make_ret(int(Ret::Code::UnknownError), "Invalid JSON format");
    }

    const JsonObject firstAsset = assetsArray.at(0).toObject();
    const String versionStr = firstAsset.value("version").toString();
    const Version version(versionStr);

    return RetVal<Version>::make_ok(version);
}

bool MuseSamplerCheckUpdateService::canCheckForUpdate() const
{
    return museSampler() && !museSampler()->version().isNull();
}

bool MuseSamplerCheckUpdateService::incompatibleLocalVersion() const
{
    if (!museSampler()) {
        return true;
    }

    return museSampler()->version() < museSamplerConfiguration()->minSupportedVersion();
}

Promise<RetVal<bool> > MuseSamplerCheckUpdateService::checkForUpdate()
{
    const Version localVersion = museSampler() ? museSampler()->version() : Version();

    return Promise<RetVal<bool> >([this, localVersion](auto resolve, auto) {
        if (localVersion.isNull()) {
            RetVal<bool> result;
            result.ret = Ret(int(Ret::Code::UnknownError), "Unable to obtain the local MuseSampler version");
            result.val = false;
            return resolve(result);
        }

        auto queryBuffer = std::make_shared<QBuffer>();
        queryBuffer->setData(configuration()->getMuseSamplerVersionQuery().toUtf8());
        auto receivedData = std::make_shared<QBuffer>();

        m_networkManager = networkManagerCreator()->makeNetworkManager();
        RetVal<Progress> progress = m_networkManager->post(configuration()->checkForMuseSamplerUpdateUrl(),
                                                           queryBuffer, receivedData,
                                                           configuration()->headers());
        if (!progress.ret) {
            m_networkManager = nullptr;
            return resolve(RetVal<bool>::make_ret(progress.ret));
        }

        progress.val.finished().onReceive(this, [this, receivedData, localVersion, resolve](const ProgressResult& res) {
            RetVal<bool> result = RetVal<bool>::make_ok(false);

            if (res.ret) {
                const RetVal<Version> version = parseVersion(receivedData->data());
                result.ret = version.ret;
                result.val = localVersion < version.val;
            } else {
                result.ret = res.ret;
            }

            (void)resolve(result);
            m_networkManager = nullptr;
        });

        return Promise<RetVal<bool> >::dummy_result();
    });
}
