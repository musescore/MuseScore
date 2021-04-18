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
#ifndef MU_FRAMEWORK_PTRUTILS_H
#define MU_FRAMEWORK_PTRUTILS_H

#include "runtime.h"
#include "log.h"

namespace mu {
namespace ptr {
template<typename T, typename E> T* checked_cast(E* source)
{
#ifndef NDEBUG
    T* casted = dynamic_cast<T*>(source);
    if (source && !casted) {
        Q_ASSERT_X(false, "checked_cast", "bad cast");
    }
    return casted;
#else
    return static_cast<T*>(source);
#endif
}

template<typename T, typename E> const T* checked_cast(const E* source)
{
#ifndef NDEBUG
    T* casted = dynamic_cast<T*>(source);
    if (source && !casted) {
        Q_ASSERT_X(false, "checked_cast", "bad cast");
    }
    return casted;
#else
    return static_cast<T*>(source);
#endif
}
}
}

#endif // MU_FRAMEWORK_PTRUTILS_H
