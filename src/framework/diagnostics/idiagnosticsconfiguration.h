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
#ifndef MUSE_DIAGNOSTICS_IDIAGNOSTICSCONFIGURATION_H
#define MUSE_DIAGNOSTICS_IDIAGNOSTICSCONFIGURATION_H

#include "modularity/imoduleinterface.h"

#include "io/path.h"

namespace muse::diagnostics {
class IDiagnosticsConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IDiagnosticsConfiguration)

public:
    virtual bool isDumpUploadAllowed() const = 0;
    virtual void setIsDumpUploadAllowed(bool val) = 0;

    virtual bool shouldWarnBeforeSavingDiagnosticFiles() const = 0;
    virtual void setShouldWarnBeforeSavingDiagnosticFiles(bool val) = 0;

    virtual muse::io::path_t diagnosticFilesDefaultSavingPath() const = 0;
};
}

#endif // MUSE_DIAGNOSTICS_IDIAGNOSTICSCONFIGURATION_H
