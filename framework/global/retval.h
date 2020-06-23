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
#ifndef MU_FRAMEWORK_RETVAL_H
#define MU_FRAMEWORK_RETVAL_H

#include "ret.h"
#include "async/channel.h"

namespace mu {
template <typename T>
struct RetVal {
    Ret ret;
    T val;
};

template <typename T1, typename T2>
struct RetVal2 {
    Ret ret;
    T1 val1;
    T2 val2;
};

template <typename T>
struct RetValCh {
    Ret ret;
    T val;
    async::Channel<T> ch;
};

template <typename T>
struct RetCh {
    Ret ret;
    async::Channel<T> ch;
};
}


#endif // MU_FRAMEWORK_RETVAL_H
