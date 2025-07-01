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

#ifndef MUSE_VST_VSTERRORS_H
#define MUSE_VST_VSTERRORS_H

#include "types/ret.h"
#include "translation.h"

namespace muse::vst {
enum class Err {
    Undefined       = static_cast<int>(Ret::Code::Undefined),
    UnknownError    = static_cast<int>(Ret::Code::VstFirst),

    NoPluginModule = 1401,
    NoPluginFactory = 1402,
    NoPluginProvider = 1403,
    NoPluginController = 1404,
    NoPluginWithId = 1405,
    NoAudioEffect = 1406,
};

inline Ret make_ret(Err e)
{
    int retCode = static_cast<int>(e);

    switch (e) {
    case Err::Undefined: return Ret(retCode);
    case Err::UnknownError: return Ret(retCode);
    case Err::NoPluginModule: return Ret(retCode, "Could not create VstModule");
    case Err::NoPluginFactory: return Ret(retCode, "Could not get VST plugin factory from file");
    case Err::NoPluginProvider: return Ret(retCode, "No VST3 audio module class found");
    case Err::NoPluginController: return Ret(retCode, "No VST3 editor controller class found");
    case Err::NoPluginWithId: return Ret(retCode, "VST3 plugin is not found");
    case Err::NoAudioEffect: return Ret(retCode, "VST3 file contains no audio effect");
    }

    return Ret(retCode);
}
}

#endif // MUSE_VST_VSTERRORS_H
