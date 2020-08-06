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
#ifndef MU_FRAMEWORK_FLAGS_H
#define MU_FRAMEWORK_FLAGS_H

namespace mu {
namespace flags {
template<typename F, typename E>
inline F toflags(E en)
{
    return static_cast<F>(en);
}

template<typename F, typename E>
inline bool isset(F flags, E en)
{
    return static_cast<int>(flags) & static_cast<int>(en);
}
}
}

#endif // MU_FRAMEWORK_FLAGS_H
