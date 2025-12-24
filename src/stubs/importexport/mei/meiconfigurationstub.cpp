/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
#include "meiconfigurationstub.h"

#include "log.h"
#include "thirdparty/kors_logger/src/log_base.h"

using namespace muse;
using namespace mu::iex::mei;

bool MeiConfigurationStub::meiImportLayout() const
{
    return false;
}

void MeiConfigurationStub::setMeiImportLayout(bool)
{
    NOT_IMPLEMENTED;
}

async::Channel<bool> MeiConfigurationStub::meiImportLayoutChanged() const
{
    return {};
}

bool MeiConfigurationStub::meiExportLayout() const
{
    return false;
}

void MeiConfigurationStub::setMeiExportLayout(bool)
{
    NOT_IMPLEMENTED;
}

bool MeiConfigurationStub::meiUseMuseScoreIds() const
{
    return false;
}

void MeiConfigurationStub::setMeiUseMuseScoreIds(bool)
{
    NOT_IMPLEMENTED;
}
