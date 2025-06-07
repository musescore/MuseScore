/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include "recentscoresmodel.h"

#include <algorithm>

#include "translation.h"
#include "dataformatter.h"
#include "io/fileinfo.h"
#include "engraving/infrastructure/mscreader.h"
#include "serialization/xmlstreamreader.h"

#include "engraving/infrastructure/mscio.h"

#include "log.h"

using namespace muse;
using namespace mu::project;
using namespace muse::io;
using namespace mu;
using namespace mu::engraving;

RecentScoresModel::RecentScoresModel(QObject* parent)
    : AbstractScoresModel(parent)
{
}

void RecentScoresModel::load()
{
    updateRecentScores();

    recentFilesController()->recentFilesListChanged().onNotify(this, [this]() {
        updateRecentScores();
    });
}

void RecentScoresModel::setRecentScores(const std::vector<QVariantMap>& items)
{
    if (m_items == items) {
        return;
    }

    beginResetModel();
    m_items = items;
    endResetModel();
}

void RecentScoresModel::updateRecentScores()
{
    const RecentFilesList& recentScores = recentFilesController()->recentFilesList();

    std::vector<QVariantMap> items;
    items.reserve(recentScores.size() + 2);

    QVariantMap addItem;
    addItem[NAME_KEY] = muse::qtrc("project", "New score");
    addItem[IS_CREATE_NEW_KEY] = true;
    addItem[IS_NO_RESULTS_FOUND_KEY] = false;
    addItem[IS_CLOUD_KEY] = false;
    items.push_back(addItem);

    std::vector<QVariantMap> scoreItems;
    for (const RecentFile& file : recentScores) {
        QVariantMap obj;

        std::string suffix = io::suffix(file.path);
        bool isSuffixInteresting = suffix != engraving::MSCZ;

        RetVal<uint64_t> fileSize = fileSystem()->fileSize(file.path);
        QString fileSizeString = (fileSize.ret && fileSize.val > 0) ? DataFormatter::formatFileSize(fileSize.val).toQString() : QString();

        obj[NAME_KEY] = file.displayName(isSuffixInteresting);
        obj[PATH_KEY] = file.path.toQString();
        readFromScore(file.path, obj); //reads metainfo from file
        obj[SUFFIX_KEY] = QString::fromStdString(suffix);
        obj[FILE_SIZE_KEY] = fileSizeString;
        obj[IS_CLOUD_KEY] = configuration()->isCloudProject(file.path);
        obj[CLOUD_SCORE_ID_KEY] = configuration()->cloudScoreIdFromPath(file.path);
        obj[TIME_SINCE_MODIFIED_KEY] = DataFormatter::formatTimeSince(io::FileInfo(file.path).lastModified().date()).toQString();
        obj[IS_CREATE_NEW_KEY] = false;
        obj[IS_NO_RESULTS_FOUND_KEY] = false;
        
        obj["rawModifiedTime"] = io::FileInfo(file.path).lastModified().date().toString().toQString();
        obj["rawFileSize"] = fileSize.val;

        scoreItems.push_back(obj);
    }

    if (!m_sortKey.isEmpty()) {
        sortScoreItems(scoreItems);
    }

    items.insert(items.end(), scoreItems.begin(), scoreItems.end());

    QVariantMap noResultsFoundItem;
    noResultsFoundItem[NAME_KEY] = "";
    noResultsFoundItem[IS_CREATE_NEW_KEY] = false;
    noResultsFoundItem[IS_NO_RESULTS_FOUND_KEY] = true;
    noResultsFoundItem[IS_CLOUD_KEY] = false;
    items.push_back(noResultsFoundItem);

    setRecentScores(items);
}


void RecentScoresModel::readFromScore(const muse::io::path_t& path, QVariantMap &obj)
{
    MscReader::Params params;
    params.filePath = path;

    MscReader reader(params);
    Ret ret = reader.open();
    if (!ret) {
        return;
    }

    ByteArray scoreData = reader.readScoreFile();
    if (scoreData.empty()) {
        return;
    }

    XmlStreamReader xml(scoreData);
    
    while (!xml.atEnd()) {
        xml.readNext();        
        if (xml.isStartElement() && xml.name() == "Score") {
            while (!xml.atEnd()) {
                xml.readNext();                
                if (xml.isEndElement() && xml.name() == "Score") {
                    break;
                }
                
                if (xml.isStartElement() && xml.name() == "metaTag") {
                    QString metaTagNameAttribute = xml.attribute("name").toQString();
                    QString metaTagValue = xml.readText().toQString();
                    
                    if (metaTagNameAttribute == "composer") {
                        obj[COMPOSER_KEY] = metaTagValue;
                    } else if (metaTagNameAttribute == "arranger") {
                        obj[ARRANGER_KEY] = metaTagValue;
                    } else if (metaTagNameAttribute == "creationDate") {
                        obj[CREATION_DATE_KEY] = metaTagValue;
                    }
                }
            }
            break;
        }
    }
}

QList<int> RecentScoresModel::nonScoreItemIndices() const
{
    return { 0, rowCount() - 1 };
}
