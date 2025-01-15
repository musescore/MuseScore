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
#include "meiconfiguration.h"

#include "settings.h"

using namespace mu;
using namespace mu::iex::mei;
using namespace muse;

static const std::string module_name("iex_mei");

static const Settings::Key MEI_IMPORT_LAYOUT_KEY(module_name, "import/mei/importMeiLayout");
static const Settings::Key MEI_EXPORT_LAYOUT_KEY(module_name, "export/mei/exportMeiLayout");
static const Settings::Key MEI_USE_MUSESCORE_IDS_KEY(module_name, "export/mei/useMuseScoreIds");

void MeiConfiguration::init()
{
    settings()->setDefaultValue(MEI_IMPORT_LAYOUT_KEY, Val(true));
    settings()->valueChanged(MEI_IMPORT_LAYOUT_KEY).onReceive(this, [this](const Val& val) {
        m_meiImportLayoutChanged.send(val.toBool());
    });

    settings()->setDefaultValue(MEI_EXPORT_LAYOUT_KEY, Val(true));
    settings()->setDefaultValue(MEI_USE_MUSESCORE_IDS_KEY, Val(false));
}

bool MeiConfiguration::meiImportLayout() const
{
    return settings()->value(MEI_IMPORT_LAYOUT_KEY).toBool();
}

void MeiConfiguration::setMeiImportLayout(bool value)
{
    settings()->setSharedValue(MEI_IMPORT_LAYOUT_KEY, Val(value));
}

async::Channel<bool> MeiConfiguration::meiImportLayoutChanged() const
{
    return m_meiImportLayoutChanged;
}

bool MeiConfiguration::meiExportLayout() const
{
    return settings()->value(MEI_EXPORT_LAYOUT_KEY).toBool();
}

void MeiConfiguration::setMeiExportLayout(bool value)
{
    settings()->setSharedValue(MEI_EXPORT_LAYOUT_KEY, Val(value));
}

bool MeiConfiguration::meiUseMuseScoreIds() const
{
    return settings()->value(MEI_USE_MUSESCORE_IDS_KEY).toBool();
}

void MeiConfiguration::setMeiUseMuseScoreIds(bool value)
{
    settings()->setSharedValue(MEI_USE_MUSESCORE_IDS_KEY, Val(value));
}
