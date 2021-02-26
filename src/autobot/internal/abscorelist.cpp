//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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
#include "abscorelist.h"
#include "log.h"

using namespace mu::autobot;
using namespace mu::system;

mu::RetVal<mu::io::paths> AbScoreList::scoreList() const
{
    //! TODO
    io::path rootPath(SCORES_ROOT);
    LOGI() << "scores root path: " << rootPath;

    RetVal<io::paths> paths = fileSystem()->scanFiles(rootPath, QStringList(), IFileSystem::ScanMode::OnlyCurrentDir);
    return paths;
}
