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

using namespace mu::diagnostics;
using namespace mu::framework;

static const Settings::Key IS_DUMP_UPLOAD_ALLOWED("diagnostics", "diagnostics/is_dump_upload_allowed");

void DiagnosticsConfiguration::init()
{
    settings()->setDefaultValue(IS_DUMP_UPLOAD_ALLOWED, Val(true));
}

bool DiagnosticsConfiguration::isDumpUploadAllowed() const
{
    return settings()->value(IS_DUMP_UPLOAD_ALLOWED).toBool();
}

void DiagnosticsConfiguration::setIsDumpUploadAllowed(bool val)
{
    settings()->setSharedValue(IS_DUMP_UPLOAD_ALLOWED, Val(val));
}
