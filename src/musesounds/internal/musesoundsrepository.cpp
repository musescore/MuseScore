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

#include "musesoundsrepository.h"

#include "serialization/json.h"

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

static QByteArray soundsRequestJson()
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

void MuseSoundsRepository::init()
{
    TRACEFUNC;

    QUrl url = QUrl(QString::fromStdString(configuration()->soundsUri().toString()));
    QByteArray jsonData = soundsRequestJson();
    RequestHeaders headers = configuration()->headers();

    auto device = std::make_shared<QBuffer>();
    device->setData(jsonData);
    auto receivedData = std::make_shared<QBuffer>();

    m_networkManager = networkManagerCreator()->makeNetworkManager();
    RetVal<Progress> progress = m_networkManager->post(url, device, receivedData, headers);
    if (!progress.ret) {
        LOGE() << progress.ret.toString();
        m_networkManager = nullptr;
        return;
    }

    progress.val.finished().onReceive(this, [this, receivedData](const muse::ProgressResult& res) {
        if (!res.ret) {
            LOGE() << res.ret.toString();
            m_networkManager = nullptr;
            return;
        }

        RetVal<SoundCatalogueInfoList> result;

        std::string err;
        JsonDocument soundsInfoDoc = JsonDocument::fromJson(ByteArray::fromQByteArray(receivedData->data()), &err);
        if (!err.empty()) {
            result.ret = make_ret(Ret::Code::InternalError, err);
        } else {
            result.ret = make_ok();
            result.val = parseSounds(soundsInfoDoc);
        }

        m_soundsСatalogs = result.val;
        m_soundsСatalogsChanged.notify();
        m_networkManager = nullptr;
    });
}

const SoundCatalogueInfoList& MuseSoundsRepository::soundsCatalogueList() const
{
    return m_soundsСatalogs;
}

async::Notification MuseSoundsRepository::soundsCatalogueListChanged() const
{
    return m_soundsСatalogsChanged;
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
