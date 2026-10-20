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

using namespace muse;
using namespace mu::project;

static constexpr int NO_RESULTS_FOUND_ROW = 1;
static constexpr int FIRST_SCORE_ROW = 2; // after "New score" and "No results found"

RecentScoresModel::RecentScoresModel(QObject* parent)
    : AbstractScoresModel(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
}

void RecentScoresModel::load()
{
    reloadRecentScores();

    recentFilesController()->recentFilesListChanged().onReceive(this, [this](const RecentFilesChange& change) {
        if (const RecentFilePrepended* prepended = std::get_if<RecentFilePrepended>(&change)) {
            prependItem(prepended->file);
        } else if (const RecentFilePathChanged* pathChanged = std::get_if<RecentFilePathChanged>(&change)) {
            updateItem(pathChanged->oldPath, pathChanged->file);
        } else if (const RecentFileRemoved* removed = std::get_if<RecentFileRemoved>(&change)) {
            removeItem(removed->path);
        } else {
            reloadRecentScores();
        }
    });
}

void RecentScoresModel::removeRecentScore(const QString& scorePath)
{
    recentFilesController()->removeRecentFile(scorePath);
}

void RecentScoresModel::reloadRecentScores()
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

    QVariantMap noResultsFoundItem;
    noResultsFoundItem[NAME_KEY] = "";
    noResultsFoundItem[IS_CREATE_NEW_KEY] = false;
    noResultsFoundItem[IS_NO_RESULTS_FOUND_KEY] = true;
    noResultsFoundItem[IS_CLOUD_KEY] = false;
    items.push_back(noResultsFoundItem);

    for (const RecentFile& file : recentScores) {
        items.push_back(makeItem(file));
    }

    setRecentScores(items);
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

void RecentScoresModel::prependItem(const RecentFile& file)
{
    const int row = findRow(file.path);
    if (row >= 0) {
        beginRemoveRows(QModelIndex(), row, row);
        m_items.erase(m_items.begin() + row);
        endRemoveRows();
    }

    beginInsertRows(QModelIndex(), FIRST_SCORE_ROW, FIRST_SCORE_ROW);
    m_items.insert(m_items.begin() + FIRST_SCORE_ROW, makeItem(file));
    endInsertRows();
}

void RecentScoresModel::updateItem(const muse::io::path_t& oldPath, const RecentFile& file)
{
    const int row = findRow(oldPath);
    if (row < 0) {
        reloadRecentScores();
        return;
    }

    m_items[row] = makeItem(file);

    const QModelIndex idx = index(row);
    emit dataChanged(idx, idx);
}

void RecentScoresModel::removeItem(const muse::io::path_t& path)
{
    const int row = findRow(path);
    if (row < 0) {
        reloadRecentScores();
        return;
    }

    beginRemoveRows(QModelIndex(), row, row);
    m_items.erase(m_items.begin() + row);
    endRemoveRows();
}

QVariantMap RecentScoresModel::makeItem(const RecentFile& file) const
{
    QVariantMap obj;

    std::string suffix = io::suffix(file.path);
    bool isSuffixInteresting = suffix != engraving::MSCZ;

    RetVal<uint64_t> fileSize = fileSystem()->fileSize(file.path);
    QString fileSizeString = (fileSize.ret && fileSize.val > 0) ? DataFormatter::formatFileSize(fileSize.val).toQString() : QString();

    obj[NAME_KEY] = file.displayName(isSuffixInteresting);
    obj[PATH_KEY] = file.path.toQString();
    obj[SUFFIX_KEY] = QString::fromStdString(suffix);
    obj[FILE_SIZE_KEY] = fileSizeString;
    obj[IS_CLOUD_KEY] = configuration()->isCloudProject(file.path);
    obj[CLOUD_SCORE_ID_KEY] = configuration()->cloudScoreIdFromPath(file.path);
    obj[TIME_SINCE_MODIFIED_KEY] = DataFormatter::formatTimeSince(io::FileInfo(file.path).lastModified().date()).toQString();
    obj[IS_CREATE_NEW_KEY] = false;
    obj[IS_NO_RESULTS_FOUND_KEY] = false;

    return obj;
}

int RecentScoresModel::findRow(const muse::io::path_t& path) const
{
    const QString pathStr = path.toQString();

    for (size_t i = FIRST_SCORE_ROW; i < m_items.size(); ++i) {
        if (m_items[i].value(PATH_KEY).toString() == pathStr) {
            return static_cast<int>(i);
        }
    }

    return -1;
}

QList<int> RecentScoresModel::nonScoreItemIndices() const
{
    return { 0, NO_RESULTS_FOUND_ROW };
}
