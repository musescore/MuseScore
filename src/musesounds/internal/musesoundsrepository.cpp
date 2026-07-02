/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore Studio
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

#include <QBuffer>

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

    // Thumbnails can live under different CDN folders (library-images, bundle-images,
    // platform-subscription-images, etc.), so resize by host rather than a fixed folder name
    static const String host(u"muse-cdn.com/");
    const size_t hostIdx = uriStr.indexOf(host);
    if (hostIdx != muse::nidx) {
        uriStr.insert(hostIdx + host.size(), u"cdn-cgi/image/w=240,q=80,f=webp/");
    }

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
                productCards { ...CardFields }
              }
              ... on ProductPageSectionRegular {
                title(locale: {locale: "%1"})
                productCards { ...CardFields }
              }
            }
          }
        }
        fragment CardFields on ProductCard {
          ... on ProductCardRegular {
            iconImageUrl
            product(locale: {locale: "%1"}) {
              ... on ProductBase {
                code
                title
                subtitle
              }
              ... on ProductLibrary {
                compatibleWith {
                  museSoundManager
                  museHub
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

    if (!m_networkManager) {
        m_networkManager = networkManagerCreator()->makeNetworkManager();
    }

    RetVal<Progress> progress = m_networkManager->post(url, device, receivedData, headers);
    if (!progress.ret) {
        LOGE() << progress.ret.toString();
        return;
    }

    progress.val.finished().onReceive(this, [this, receivedData](const muse::ProgressResult& res) {
        if (!res.ret) {
            LOGE() << "Unable to download MuseSounds list: " << res.ret.toString();
            return;
        }

        RetVal<SoundCatalogInfoList> result;

        std::string err;
        JsonDocument soundsInfoDoc = JsonDocument::fromJson(ByteArray::fromQByteArray(receivedData->data()), &err);
        if (!err.empty()) {
            result.ret = make_ret(Ret::Code::InternalError, err);
        } else {
            result.ret = make_ok();
            result.val = parseSounds(soundsInfoDoc);
        }

        m_soundsCatalogs = result.val;
        m_soundsCatalogsChanged.notify();
    });
}

const SoundCatalogInfoList& MuseSoundsRepository::soundsCatalogs() const
{
    return m_soundsCatalogs;
}

async::Notification MuseSoundsRepository::soundsCatalogsChanged() const
{
    return m_soundsCatalogsChanged;
}

SoundCatalogInfoList MuseSoundsRepository::parseSounds(const JsonDocument& soundsDoc) const
{
    SoundCatalogInfoList result;

    JsonObject obj = soundsDoc.rootObject();
    JsonObject data = !obj.empty() ? obj.value("data").toObject() : JsonObject();
    JsonObject productsSearch = !data.empty() ? data.value("product_pages_configuration").toObject() : JsonObject();
    JsonArray catalogs = !productsSearch.empty() ? productsSearch.value("museScoreStudioPageSections").toArray() : JsonArray();

    std::string museSoundsAppName = platformMuseSoundsAppName();

    for (size_t catalogIdx = 0; catalogIdx < catalogs.size(); ++catalogIdx) {
        JsonObject catalogObj = catalogs.at(catalogIdx).toObject();
        if (catalogObj.empty()) {
            continue;
        }

        SoundCatalogInfo catalog;
        catalog.title = catalogObj.value("title").toString();

        JsonArray soundsItems = catalogObj.value("productCards").toArray();
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
            if (compatibleWithObj.empty()) {
                // Products without compatibleWith info are only manageable through MuseHub
                if (museSoundsAppName != "museHub") {
                    continue;
                }
            } else if (compatibleWithObj.contains(museSoundsAppName) && !compatibleWithObj.value(museSoundsAppName).toBool()) {
                continue;
            }

            SoundLibraryInfo soundLibrary;
            soundLibrary.title = productObj.value("title").toString();
            soundLibrary.subtitle = productObj.value("subtitle").toString();
            soundLibrary.thumbnail = correctThumbnailSize(UriQuery(soundItemObj.value("iconImageUrl").toStdString()));
            soundLibrary.code = productObj.value("code").toString();
            soundLibrary.uri = configuration()->soundPageUri(soundLibrary.code);

            catalog.soundLibraries.emplace_back(soundLibrary);
        }

        if (catalog.soundLibraries.empty()) {
            continue;
        }

        result.emplace_back(catalog);
    }

    return result;
}
