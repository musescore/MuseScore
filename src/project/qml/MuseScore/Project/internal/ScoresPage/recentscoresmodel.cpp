/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "translation.h"
#include "dataformatter.h"
#include "io/fileinfo.h"

#include "engraving/infrastructure/mscio.h"

#include "log.h"

#include <algorithm>
#include <QCollator>

using namespace muse;
using namespace mu::project;

RecentScoresModel::RecentScoresModel(QObject* parent)
    : AbstractScoresModel(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
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

    struct ScoreEntry {
        QVariantMap obj;
        muse::DateTime lastModified;
    };

    std::vector<ScoreEntry> scoreEntries;
    scoreEntries.reserve(recentScores.size());

    for (const RecentFile& file : recentScores) {
        QVariantMap obj;

        std::string suffix = io::suffix(file.path);
        bool isSuffixInteresting = suffix != engraving::MSCZ;

        RetVal<uint64_t> fileSize = fileSystem()->fileSize(file.path);
        QString fileSizeString = (fileSize.ret && fileSize.val > 0) ? DataFormatter::formatFileSize(fileSize.val).toQString() : QString();

        muse::DateTime lastModified = io::FileInfo(file.path).lastModified();

        obj[NAME_KEY] = file.displayName(isSuffixInteresting);
        obj[PATH_KEY] = file.path.toQString();
        obj[SUFFIX_KEY] = QString::fromStdString(suffix);
        obj[FILE_SIZE_KEY] = fileSizeString;
        obj[IS_CLOUD_KEY] = configuration()->isCloudProject(file.path);
        obj[CLOUD_SCORE_ID_KEY] = configuration()->cloudScoreIdFromPath(file.path);
        obj[TIME_SINCE_MODIFIED_KEY] = DataFormatter::formatTimeSince(lastModified.date()).toQString();
        obj[IS_CREATE_NEW_KEY] = false;
        obj[IS_NO_RESULTS_FOUND_KEY] = false;

        scoreEntries.push_back({ obj, lastModified });
    }

    if (m_sortMode == SortByName) {
        QCollator collator;
        collator.setNumericMode(true);
        collator.setCaseSensitivity(Qt::CaseInsensitive);

        std::stable_sort(scoreEntries.begin(), scoreEntries.end(), [&collator](const ScoreEntry& a, const ScoreEntry& b) {
            return collator.compare(a.obj[NAME_KEY].toString(), b.obj[NAME_KEY].toString()) < 0;
        });
    } else {
        // SortByTimeModified: sort by actual file mtime, not recent-files usage order,
        // so it matches what the "Modified" column displays (open+close alone shouldn't reorder).
        std::stable_sort(scoreEntries.begin(), scoreEntries.end(), [](const ScoreEntry& a, const ScoreEntry& b) {
            return a.lastModified.toQDateTime() > b.lastModified.toQDateTime();
        });
    }

    std::vector<QVariantMap> items;
    items.reserve(scoreEntries.size() + 2);

    QVariantMap addItem;
    addItem[NAME_KEY] = muse::qtrc("project", "New score");
    addItem[IS_CREATE_NEW_KEY] = true;
    addItem[IS_NO_RESULTS_FOUND_KEY] = false;
    addItem[IS_CLOUD_KEY] = false;
    items.push_back(addItem);

    for (const ScoreEntry& entry : scoreEntries) {
        items.push_back(entry.obj);
    }

    QVariantMap noResultsFoundItem;
    noResultsFoundItem[NAME_KEY] = "";
    noResultsFoundItem[IS_CREATE_NEW_KEY] = false;
    noResultsFoundItem[IS_NO_RESULTS_FOUND_KEY] = true;
    noResultsFoundItem[IS_CLOUD_KEY] = false;
    items.push_back(noResultsFoundItem);

    setRecentScores(items);
}

RecentScoresModel::SortMode RecentScoresModel::sortMode() const
{
    return m_sortMode;
}

void RecentScoresModel::setSortMode(SortMode mode)
{
    if (m_sortMode == mode) {
        return;
    }

    m_sortMode = mode;
    emit sortModeChanged();

    updateRecentScores();
}

QList<int> RecentScoresModel::nonScoreItemIndices() const
{
    return { 0, rowCount() - 1 };
}
