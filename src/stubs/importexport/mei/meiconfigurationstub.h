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
#pragma once

#include "importexport/mei/imeiconfiguration.h"

namespace mu::iex::mei {
class MeiConfigurationStub : public IMeiConfiguration
{
public:

    bool meiImportLayout() const override;
    void setMeiImportLayout(bool value) override;
    muse::async::Channel<bool> meiImportLayoutChanged() const override;

    bool meiExportLayout() const override;
    void setMeiExportLayout(bool value) override;

    bool meiUseMuseScoreIds() const override;
    void setMeiUseMuseScoreIds(bool value) override;
};
}
