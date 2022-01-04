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
#ifndef MU_FRAMEWORK_ID_H
#define MU_FRAMEWORK_ID_H

#include <cstdint>
#include <vector>

#ifndef NO_QT_SUPPORT
#include <QVariant>
#endif

namespace mu {
class ID
{
public:
    ID();
    ID(const ID& id);
    ID(const uint64_t& id);

    bool isValid() const;

    ID& operator=(const ID& id);
    ID& operator+=(const ID& id);

    ID operator+(const ID& id) const;
    ID operator^(const ID& id) const;

    bool operator==(const ID& id) const;
    bool operator==(uint64_t id) const;
    bool operator!=(const ID& id) const;
    bool operator<(const ID& id) const;
    bool operator>(const ID& id) const;

    uint64_t toUint64() const;

#ifndef NO_QT_SUPPORT
    ID(const QString& id);
    ID(const QVariant& id);

    QString toQString() const;
#endif

private:
    uint64_t m_id = 0;
};

using IDList = std::vector<ID>;

inline bool containsId(const IDList& list, const ID& id)
{
    return std::find(list.cbegin(), list.cend(), id) != list.cend();
}

inline uint qHash(const ID& id)
{
    return ::qHash(id.toUint64());
}
}

#endif // MU_FRAMEWORK_ID_H
