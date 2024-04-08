/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MUSE_CLOUD_CLOUDTYPES_H
#define MUSE_CLOUD_CLOUDTYPES_H

#include <vector>

#include <QDate>
#include <QString>
#include <QUrl>

#include "types/id.h"

namespace muse::cloud {
static const QString MUSESCORE_COM_CLOUD_CODE = "musescorecom";
static const QString AUDIO_COM_CLOUD_CODE = "audiocom";

struct CloudInfo {
    QString code;
    QString title;
    QUrl url;
    QString logoUrl;
    QString logoColor;
};

struct AccountInfo {
    QString id;
    QString userName;
    QUrl profileUrl;
    QUrl collectionUrl;
    QUrl avatarUrl;

    bool operator==(const AccountInfo& another) const
    {
        bool equals = true;

        equals &= (id == another.id);
        equals &= (userName == another.userName);
        equals &= (profileUrl == another.profileUrl);
        equals &= (collectionUrl == another.collectionUrl);
        equals &= (avatarUrl == another.avatarUrl);

        return equals;
    }

    bool isValid() const
    {
        return !id.isEmpty() && !userName.isEmpty();
    }
};

struct ScoreOwnerInfo {
    int id = 0;
    QString userName;
    QUrl profileUrl;

    bool operator==(const ScoreOwnerInfo& info) const
    {
        bool equals = true;

        equals &= (id == info.id);
        equals &= (userName == info.userName);
        equals &= (profileUrl == info.profileUrl);

        return equals;
    }
};

//! Note: these values are currently supposed to be in sync with the MuseScore.com API!
enum class Visibility {
    Public = 0,
    Unlisted = 1,
    Private = 2
};

struct ScoreInfo {
    int id = 0;
    int revisionId = 0;
    QString title;
    QString description;
    Visibility visibility = Visibility::Private;
    QString license;
    QStringList tags;
    QString url;
    ScoreOwnerInfo owner;

    bool operator==(const ScoreInfo& another) const
    {
        bool equals = true;

        equals &= (id == another.id);
        equals &= (revisionId == another.revisionId);
        equals &= (title == another.title);
        equals &= (description == another.description);
        equals &= (visibility == another.visibility);
        equals &= (license == another.license);
        equals &= (tags == another.tags);
        equals &= (url == another.url);
        equals &= (owner == another.owner);

        return equals;
    }

    bool isValid() const
    {
        return id > 0 && !title.isEmpty();
    }
};

struct ScoresList {
    struct Item {
        int id = 0;
        QString title;
        QDateTime lastModified;
        size_t fileSize = 0;
        QString thumbnailUrl;
        Visibility visibility = Visibility::Private;
        int viewCount = 0;
    };

    std::vector<Item> items;

    /// See explanation at `IMuseScoreComService::downloadScoresList`
    struct Meta {
        int totalScoresCount = 0;
        int batchesCount = 0;
        int thisBatchNumber = 0;
        int scoresPerBatch = 0;
    } meta;
};

constexpr muse::ID INVALID_ID = 0;

inline muse::ID idFromCloudUrl(const QUrl& sourceUrl)
{
    QStringList parts = sourceUrl.toString().split("/");
    if (parts.isEmpty()) {
        return INVALID_ID;
    }

    return muse::ID(parts.last());
}
}

#endif // MUSE_CLOUD_ACCOUNTTYPES_H
