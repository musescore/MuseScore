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
#ifndef MU_EXTENSIONS_EXTENSIONSERRORS_H
#define MU_EXTENSIONS_EXTENSIONSERRORS_H

#include "ret.h"
#include "translation.h"

namespace mu::extensions {
enum class Err {
    Undefined       = int(Ret::Code::Undefined),
    NoError         = int(Ret::Code::Ok),
    UnknownError    = int(Ret::Code::ExtensionsFirst),

    ErrorParseConfig,
    ErrorLoadingExtension,
    ErrorExtensionNotFound,
    ErrorRemoveExtensionDirectory,
    ErrorAnotherOperationStarted,

    UnpackDestinationReadOnly,
    UnpackNoFreeSpace,
    UnpackInvalidStructure,
    UnpackInvalidOldExtension,
    UnpackPreviousVersionExists,
    UnpackErrorRemovePreviousVersion,
    UnpackNoActualVersion,
    UnpackError
};

inline Ret make_ret(Err e)
{
    int retCode = static_cast<int>(e);

    switch (e) {
    case Err::Undefined: return Ret(retCode);
    case Err::NoError: return Ret(retCode);
    case Err::UnknownError: return Ret(retCode);
    case Err::ErrorParseConfig: return Ret(retCode, trc("extensions", "Error parsing response from server"));
    case Err::ErrorLoadingExtension: return Ret(retCode, trc("extensions", "Error loading extension"));
    case Err::ErrorExtensionNotFound: return Ret(retCode, trc("extensions", "Extension not found"));
    case Err::ErrorRemoveExtensionDirectory: return Ret(retCode, trc("extensions", "Error remove extension directory"));
    case Err::ErrorAnotherOperationStarted: return Ret(retCode,
                                                       trc("extension", "Another operation on this extension has already started"));
    case Err::UnpackDestinationReadOnly: return Ret(retCode, trc("extensions", "Cannot import extension on read-only storage"));
    case Err::UnpackNoFreeSpace: return Ret(retCode, trc("extensions", "Cannot import extension on full storage"));
    case Err::UnpackInvalidStructure: return Ret(retCode, trc("extensions", "Invalid archive structure"));
    case Err::UnpackInvalidOldExtension: return Ret(retCode, trc("extensions", "Invalid old extension"));
    case Err::UnpackPreviousVersionExists: return Ret(retCode, trc("extensions", "Previous version of extension exists"));
    case Err::UnpackErrorRemovePreviousVersion: return Ret(retCode, trc("extensions", "Error removing previous version"));
    case Err::UnpackNoActualVersion: return Ret(retCode, trc("extensions", "A newer version is already installed"));
    case Err::UnpackError: return Ret(retCode, trc("extensions", "Error unpacking extension"));
    }

    return Ret(static_cast<int>(e));
}
}

#endif // MU_EXTENSIONS_EXTENSIONSERRORS_H
