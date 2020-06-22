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
#ifndef MU_IO_FILEPATH_H
#define MU_IO_FILEPATH_H

#include <string>
#include <QString>

namespace mu {
namespace io {
using path = std::string;

#ifndef NO_QT_SUPPORT
path pathFromQString(const QString& s);
QString pathToQString(const path& p);
#endif

path syffix(const path& path);
}
}

#endif // MU_IO_FILEPATH_H
