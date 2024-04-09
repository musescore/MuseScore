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
#include "diagnosticsconfiguration.h"

#include "global/settings.h"

using namespace muse;
using namespace mu;
using namespace muse::diagnostics;

static const Settings::Key IS_DUMP_UPLOAD_ALLOWED("diagnostics", "diagnostics/is_dump_upload_allowed");
static const Settings::Key SHOULD_WARN_BEFORE_SAVING_DIAGNOSTIC_FILES("diagnostics", "diagnostics/shouldWarnBeforeSavingDiagnosticFiles");

void DiagnosticsConfiguration::init()
{
    settings()->setDefaultValue(IS_DUMP_UPLOAD_ALLOWED, Val(true));
    settings()->setDefaultValue(SHOULD_WARN_BEFORE_SAVING_DIAGNOSTIC_FILES, Val(true));
}

bool DiagnosticsConfiguration::isDumpUploadAllowed() const
{
    return settings()->value(IS_DUMP_UPLOAD_ALLOWED).toBool();
}

void DiagnosticsConfiguration::setIsDumpUploadAllowed(bool val)
{
    settings()->setSharedValue(IS_DUMP_UPLOAD_ALLOWED, Val(val));
}

bool DiagnosticsConfiguration::shouldWarnBeforeSavingDiagnosticFiles() const
{
    return settings()->value(SHOULD_WARN_BEFORE_SAVING_DIAGNOSTIC_FILES).toBool();
}

void DiagnosticsConfiguration::setShouldWarnBeforeSavingDiagnosticFiles(bool val)
{
    settings()->setSharedValue(SHOULD_WARN_BEFORE_SAVING_DIAGNOSTIC_FILES, Val(val));
}

muse::io::path_t DiagnosticsConfiguration::diagnosticFilesDefaultSavingPath() const
{
    return globalConfiguration()->homePath();
}
