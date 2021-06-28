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
#include "oveconfiguration.h"

#include "settings.h"

using namespace mu::framework;
using namespace mu::iex::ove;

static const Settings::Key IMPORT_OVERTUNE_CHARSET_KEY("iex_ove", "import/overture/charset");

void OveConfiguration::init()
{
    settings()->setDefaultValue(IMPORT_OVERTUNE_CHARSET_KEY, Val("GBK"));
}

std::string OveConfiguration::importOvertuneCharset() const
{
    return settings()->value(IMPORT_OVERTUNE_CHARSET_KEY).toString();
}

void OveConfiguration::setImportOvertuneCharset(const std::string& charset)
{
    settings()->setSharedValue(IMPORT_OVERTUNE_CHARSET_KEY, Val(charset));
}
