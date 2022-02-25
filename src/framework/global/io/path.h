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
#ifndef MU_IO_PATH_H
#define MU_IO_PATH_H

#include <QString>
#include "framework/global/logstream.h"

namespace mu::io {
struct path;
using paths = std::vector<path>;
struct path {
    path() = default;
    path(const path&) = default;
    path(const QByteArray& s);
    path(const QString& s);
    path(const std::string& s);
    path(const char* s);

    bool empty() const;

    path appendingComponent(const path& other) const;
    path appendingSuffix(const path& suffix) const;

    inline path& operator=(const QString& other) { m_path = other.toUtf8(); return *this; }

    inline bool operator==(const path& other) const { return m_path == other.m_path; }
    inline bool operator!=(const path& other) const { return !(m_path == other.m_path); }

    inline path operator+(const path& other) const { path p = *this; p += other; return p; }
    inline path operator+(const QString& other) const { path p = *this; p += other; return p; }
    inline path operator+(const char* other) const { path p = *this; p += other; return p; }

    inline path& operator+=(const path& other) { m_path += other.m_path; return *this; }
    inline path& operator+=(const QString& other) { m_path += other.toUtf8(); return *this; }
    inline path& operator+=(const char* other) { m_path += other; return *this; }

    inline bool operator<(const path& other) const { return m_path < other.m_path; }

    QString toQString() const;
    std::string toStdString() const;
    std::wstring toStdWString() const;
    const char* c_str() const;

    static paths pathsFromString(const std::string& str, const std::string& delim = ";");

private:
    QByteArray m_path;
};

inline path operator+(const char* one, const path& other) { return path(one) + other; }
inline path operator+(const QString& one, const path& other) { return path(one) + other; }

inline mu::logger::Stream& operator<<(mu::logger::Stream& s, const mu::io::path& p)
{
    s << p.c_str();
    return s;
}

std::string suffix(const path& path);
path filename(const path& path, bool includingExtension = true);
path basename(const path& path);
path absolutePath(const path& path);
path dirname(const path& path);
path dirpath(const path& path);
path absoluteDirpath(const path& path);

bool isAbsolute(const path& path);

bool isAllowedFileName(const path& fn);
path escapeFileName(const path& fn);

paths pathsFromStrings(const QStringList& list);
}

#endif // MU_IO_PATH_H
