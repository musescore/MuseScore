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
#include "guitarproconfiguration.h"

#include "settings.h"

#include "libmscore/mscore.h"

using namespace mu::framework;
using namespace mu::iex::guitarpro;

static const Settings::Key IMPORT_GUITARPRO_CHARSET_KEY("iex_guitarpro", "import/guitarpro/charset");

void GuitarProConfiguration::init()
{
    settings()->setDefaultValue(IMPORT_GUITARPRO_CHARSET_KEY, Val("UTF-8"));
}

std::string GuitarProConfiguration::importGuitarProCharset() const
{
    return settings()->value(IMPORT_GUITARPRO_CHARSET_KEY).toString();
}

void GuitarProConfiguration::setImportGuitarProCharset(const std::string& charset)
{
    settings()->setSharedValue(IMPORT_GUITARPRO_CHARSET_KEY, Val(charset));
}
