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
#include "path.h"
#include "stringutils.h"

#include <QFileInfo>

#ifndef NO_QT_SUPPORT
mu::io::path mu::io::pathFromQString(const QString& s)
{
    return s.toStdString();
}

QString mu::io::pathToQString(const path& p)
{
    return QString::fromStdString(p);
}

#endif

mu::io::path mu::io::syffix(const mu::io::path& path)
{
    auto pos = path.find_last_of(".");
    if (pos == std::string::npos) {
        return std::string();
    }
    std::string sfx = path.substr(pos + 1);
    return strings::toLower(sfx);
}

std::string mu::io::basename(const mu::io::path& path)
{
    QFileInfo fi(pathToQString(path));
    return fi.baseName().toStdString();
}
