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
#ifndef MU_AUTOBOT_AUTOBOTTYPES_H
#define MU_AUTOBOT_AUTOBOTTYPES_H

#include <string>
#include <vector>

#include "io/path.h"
#include "iteststep.h"

namespace mu::autobot {
enum class FileStatus {
    Undefined = 0,
    None,
    Success,
    Failed
};

struct File {
    io::path path;
    FileStatus status = FileStatus::Undefined;
};

using Files = std::vector<File>;
}

#endif // MU_AUTOBOT_AUTOBOTTYPES_H
