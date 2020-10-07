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
#ifndef MU_IO_PATH_H
#define MU_IO_PATH_H

#include <QString>
#include <QDebug>

namespace mu {
namespace io {
struct path;
using paths = std::vector<path>;
struct path {
    path() = default;
    path(const path& p);
    path(const QString& s);
    path(const std::string& s);
    path(const char* s);

    bool empty() const;

    inline path& operator=(const QString& other) { m_path = other.toUtf8(); return *this; }

    inline bool operator==(const path& other) const { return m_path == other.m_path; }

    inline path operator+(const path& other) const { path p = *this; p.m_path += other.m_path; return p; }
    inline path operator+(const QString& other) const { path p = *this; p.m_path += other; return p; }
    inline path operator+(const char* other) const { path p = *this; p.m_path += other; return p; }

    QString toQString() const;
    std::string toStdString() const;
    const char* c_str() const;

    static paths pathsFromString(const std::string& str, const std::string& delim = ";");

private:
    QByteArray m_path;
};

inline QDebug operator<<(QDebug debug, const mu::io::path& p)
{
    debug << p.c_str();
    return debug;
}

std::string syffix(const path& path);
path filename(const path& path);
path basename(const path& path);
path dirname(const path& path);

path escapeFileName(const path& fn);
}
}

#endif // MU_IO_PATH_H
