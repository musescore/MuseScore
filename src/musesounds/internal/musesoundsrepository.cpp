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

#include "musesoundsrepository.h"

#include "serialization/json.h"

#include "global/concurrency/concurrent.h"

using namespace mu::musesounds;
using namespace muse;
using namespace muse::network;

static std::string platformMuseSoundsAppName()
{
#if defined(Q_OS_WIN) | defined(Q_OS_MAC)
    return "museHub";
#else
    return "museSoundManager";
#endif
}

static muse::UriQuery correctThumbnailSize(const UriQuery& uri)
{
    String uriStr = String::fromStdString(uri.toString());
    uriStr.replace(u"/library-images/", u"/cdn-cgi/image/w=240,q=80,f=webp/library-images/");
    return UriQuery(uriStr.toStdString());
}

void MuseSoundsRepository::init()
{
    auto soundsCallBack = [this](const RetVal<SoundCatalogueInfoList>& result) {
        if (!result.ret) {
            LOGE() << result.ret.toString();
            return;
        }

        {
            std::lock_guard lock(m_mutex);
            m_soundsСatalogs = result.val;
        }

        m_soundsСatalogsChanged.notify();
    };

    Concurrent::run(this, &MuseSoundsRepository::th_requestSounds, configuration()->soundsUri(), soundsCallBack);
}

const SoundCatalogueInfoList& MuseSoundsRepository::soundsCatalogueList() const
{
    std::lock_guard lock(m_mutex);
    return m_soundsСatalogs;
}

async::Notification MuseSoundsRepository::soundsCatalogueListChanged() const
{
    return m_soundsСatalogsChanged;
}

QByteArray MuseSoundsRepository::soundsRequestJson() const
{
    QLocale locale = QLocale();
    String localeStr = locale.bcp47Name() + "-" + QLocale::territoryToCode(locale.territory());

    String query = String(
        R"(query MyQuery {
          product_pages_configuration {
            museScoreStudioPageSections {
              ... on ProductPageSectionDynamic {
                title(locale: {locale: "%1"})
                productCards {
                  ... on ProductCardRegular {
                    iconImageUrl
                    product(locale: {locale: "%1"}) {
                      ... on ProductLibrary {
                        code
                        title
                        subtitle
                        compatibleWith {
                          museSoundManager
                          museHub
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        })").arg(localeStr);

    JsonObject json;
    json["query"] = query;

    return JsonDocument(json).toJson(JsonDocument::Format::Compact).toQByteArray();
}

void MuseSoundsRepository::th_requestSounds(const UriQuery& soundsUri, std::function<void(RetVal<SoundCatalogueInfoList>)> callBack) const
{
    TRACEFUNC;

    network::INetworkManagerPtr networkManager = networkManagerCreator()->makeNetworkManager();
    RequestHeaders headers = configuration()->headers();

    QByteArray jsonData = soundsRequestJson();
    QBuffer receivedData(&jsonData);
    OutgoingDevice device(&receivedData);

    Ret soundsItemsRet = networkManager->post(QUrl(QString::fromStdString(soundsUri.toString())), &device, &receivedData, headers);
    if (!soundsItemsRet) {
        callBack(soundsItemsRet);
        return;
    }

    RetVal<SoundCatalogueInfoList> result;

    std::string err;
    JsonDocument soundsInfoDoc = JsonDocument::fromJson(ByteArray::fromQByteArray(receivedData.data()), &err);
    if (!err.empty()) {
        result.ret = make_ret(Ret::Code::InternalError, err);
    } else {
        result.ret = make_ok();
        result.val = parseSounds(soundsInfoDoc);
    }

    callBack(result);
}

SoundCatalogueInfoList MuseSoundsRepository::parseSounds(const JsonDocument& soundsDoc) const
{
    SoundCatalogueInfoList result;

    JsonObject obj = soundsDoc.rootObject();
    JsonObject data = !obj.empty() ? obj.value("data").toObject() : JsonObject();
    JsonObject productsSearch = !data.empty() ? data.value("product_pages_configuration").toObject() : JsonObject();
    JsonArray catalogs = !productsSearch.empty() ? productsSearch.value("museScoreStudioPageSections").toArray() : JsonArray();

    std::string museSoundsAppName = platformMuseSoundsAppName();

    for (size_t catalogIdx = 0; catalogIdx < catalogs.size(); ++catalogIdx) {
        JsonObject catalogueObj = catalogs.at(catalogIdx).toObject();
        if (catalogueObj.empty()) {
            continue;
        }

        SoundCatalogueInfo catalogue;
        catalogue.title = catalogueObj.value("title").toString();

        JsonArray soundsItems = catalogueObj.value("productCards").toArray();
        if (soundsItems.empty()) {
            continue;
        }

        for (size_t soundIdx = 0; soundIdx < soundsItems.size(); ++soundIdx) {
            JsonObject soundItemObj = soundsItems.at(soundIdx).toObject();
            if (soundItemObj.empty()) {
                continue;
            }

            JsonObject productObj = soundItemObj.value("product").toObject();
            if (productObj.empty()) {
                continue;
            }

            JsonObject compatibleWithObj = productObj.value("compatibleWith").toObject();
            if (!compatibleWithObj.empty() && compatibleWithObj.contains(museSoundsAppName)
                && !compatibleWithObj.value(museSoundsAppName).toBool()) {
                continue;
            }

            SoundLibraryInfo soundLibrary;
            soundLibrary.title = productObj.value("title").toString();
            soundLibrary.subtitle = productObj.value("subtitle").toString();
            soundLibrary.thumbnail = correctThumbnailSize(UriQuery(soundItemObj.value("iconImageUrl").toStdString()));
            soundLibrary.code = productObj.value("code").toString();
            soundLibrary.uri = configuration()->soundPageUri(soundLibrary.code);

            catalogue.soundLibraries.emplace_back(soundLibrary);
        }

        if (catalogue.soundLibraries.empty()) {
            continue;
        }

        result.emplace_back(catalogue);
    }

    return result;
}
