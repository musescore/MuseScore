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
#ifndef MU_CLOUD_CLOUDTYPES_H
#define MU_CLOUD_CLOUDTYPES_H

#include <QUrl>

namespace mu::cloud {
struct AccountInfo {
    int id = 0;
    QString userName;
    QUrl profileUrl;
    QUrl sheetmusicUrl;
    QUrl avatarUrl;

    bool operator==(const AccountInfo& another) const
    {
        bool equals = true;

        equals &= (id == another.id);
        equals &= (userName == another.userName);
        equals &= (profileUrl == another.profileUrl);
        equals &= (sheetmusicUrl == another.sheetmusicUrl);
        equals &= (avatarUrl == another.avatarUrl);

        return equals;
    }

    bool isValid() const
    {
        return !userName.isEmpty();
    }
};

struct ScoreInfo {
    int id = 0;
    QString title;
    QString description;
    bool isPrivate = false;
    QString license;
    QStringList tags;
    QString url;

    bool operator==(const ScoreInfo& another) const
    {
        bool equals = true;

        equals &= (id == another.id);
        equals &= (description == another.description);
        equals &= (isPrivate == another.isPrivate);
        equals &= (license == another.license);
        equals &= (tags == another.tags);
        equals &= (url == another.url);

        return equals;
    }

    bool isValid() const
    {
        return id > 0 && !title.isEmpty();
    }
};
}

#endif // MU_CLOUD_ACCOUNTTYPES_H
