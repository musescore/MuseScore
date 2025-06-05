/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

using namespace mu;
using namespace muse;
using namespace mu::iex::ove;

static const Settings::Key IMPORT_OVERTURE_CHARSET_KEY("iex_ove", "import/overture/charset");

void OveConfiguration::init()
{
    settings()->setDefaultValue(IMPORT_OVERTURE_CHARSET_KEY, Val("GBK"));
    settings()->valueChanged(IMPORT_OVERTURE_CHARSET_KEY).onReceive(this, [this](const Val& val) {
        m_importOvertureCharsetChanged.send(val.toString());
    });
}

std::string OveConfiguration::importOvertureCharset() const
{
    return settings()->value(IMPORT_OVERTURE_CHARSET_KEY).toString();
}

void OveConfiguration::setImportOvertureCharset(const std::string& charset)
{
    settings()->setSharedValue(IMPORT_OVERTURE_CHARSET_KEY, Val(charset));
}

async::Channel<std::string> OveConfiguration::importOvertureCharsetChanged() const
{
    return m_importOvertureCharsetChanged;
}
