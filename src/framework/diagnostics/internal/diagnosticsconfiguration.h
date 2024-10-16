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
#ifndef MUSE_DIAGNOSTICS_DIAGNOSTICSCONFIGURATION_H
#define MUSE_DIAGNOSTICS_DIAGNOSTICSCONFIGURATION_H

#include "../idiagnosticsconfiguration.h"

#include "modularity/ioc.h"
#include "iglobalconfiguration.h"

namespace muse::diagnostics {
class DiagnosticsConfiguration : public IDiagnosticsConfiguration, public Injectable
{
    Inject<muse::IGlobalConfiguration> globalConfiguration = { this };

public:
    DiagnosticsConfiguration(const modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    void init();

    bool isDumpUploadAllowed() const override;
    void setIsDumpUploadAllowed(bool val) override;

    bool shouldWarnBeforeSavingDiagnosticFiles() const override;
    void setShouldWarnBeforeSavingDiagnosticFiles(bool val) override;

    muse::io::path_t diagnosticFilesDefaultSavingPath() const override;
};
}

#endif // MUSE_DIAGNOSTICS_DIAGNOSTICSCONFIGURATION_H
