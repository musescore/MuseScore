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

#ifndef MU_VST_VSTERRORS_H
#define MU_VST_VSTERRORS_H

#include "ret.h"
#include "translation.h"

namespace mu::vst {
enum class Err {
    Undefined       = static_cast<int>(Ret::Code::Undefined),
    UnknownError    = static_cast<int>(Ret::Code::VstFirst),

    NoPluginModule = 1401,
    NoPluginFactory = 1402,
    NoPluginProvider = 1403,
    NoPluginController = 1404,
    NoPluginWithId = 1405
};

inline Ret make_ret(Err e)
{
    int retCode = static_cast<int>(e);

    switch (e) {
    case Err::Undefined: return Ret(retCode);
    case Err::UnknownError: return Ret(retCode);
    case Err::NoPluginModule: return Ret(retCode, trc("vst", "Could not create VstModule"));
    case Err::NoPluginFactory: return Ret(retCode, trc("vst", "Could not get Vst Plugin Factory from file"));
    case Err::NoPluginProvider: return Ret(retCode, trc("vst", "No VST3 Audio Module Class found"));
    case Err::NoPluginController: return Ret(retCode, trc("vst", "No VST3 Editor Controller Class found"));
    case Err::NoPluginWithId: return Ret(retCode, trc("vst", "VST3 plugin is not found"));
    }

    return Ret(retCode);
}
}

#endif // MU_VST_VSTERRORS_H
