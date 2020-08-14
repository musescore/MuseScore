//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_CLOUD_CLOUDTYPES_H
#define MU_CLOUD_CLOUDTYPES_H

#include <QUrl>

namespace mu {
namespace cloud {
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
}
}

#endif // MU_CLOUD_ACCOUNTTYPES_H
